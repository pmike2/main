/*

Référence :

http://archive.gamedev.net/archive/reference/programming/features/beatdetection/index.html

*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <sys/time.h>
#include <unistd.h>

#include <fftw3.h>
#include <curses.h>
#include "sndfile.h"
#include "portaudio.h"

#ifdef WIN32
#include <windows.h>
#if PA_USE_ASIO
#include "pa_asio.h"
#endif
#endif

#include "oscpack/osc/OscOutboundPacketStream.h"
#include "oscpack/ip/UdpSocket.h"

#include "pa_utils.h"


using namespace std;


// ------------------------------------------------------------------------------------
// paramètres OSC
const string OSC_ADDRESS= "127.0.0.1";
const int OSC_PORT= 12001;
const int OSC_BUFFER_SIZE= 1024;

// nombre de samples dans 1s d'audio
const int SAMPLE_RATE= 44100;

// nombre de samples a traiter a chaque appel du callback portaudio
const int FRAMES_PER_BUFFER= 1024;

// cf http://www.fftw.org/fftw3_doc/One_002dDimensional-DFTs-of-Real-Data.html
const int FFT_INPUT_SIZE= FRAMES_PER_BUFFER;
const int FFT_OUTPUT_SIZE= FRAMES_PER_BUFFER/ 2+ 1;

// nombre de samples audio regroupés par bloc pour calcul fft
const int FFT_BLOCK_SIZE= 16;

const int N_BLOCKS_FFT= FFT_OUTPUT_SIZE/ FFT_BLOCK_SIZE;

const int N_COEFFS_IN_FIRST_BLOCK= 1;

// temps en ms de l'historique d'énergie
const int HISTORY_TIME_MS= 100;
// nombre de valeurs énergie a stocker dans l'historique
const int HISTORY_SIZE= (int)(HISTORY_TIME_MS* SAMPLE_RATE/ (FRAMES_PER_BUFFER* 1000));


// facteur multiplicatif pour test déclenchement événement ; a adapter a chaque bloc ?
const float CMP_RATIO= 20.0f;

// seuil variance pour test déclenchement événement ; a adapter a chaque bloc ?
const float VARIANCE_TRESH= 100.0f;

// record stereo
const unsigned int RECORD_TIME= 10; // en secondes
const unsigned int NFRAMES_RECORD= ((SAMPLE_RATE* RECORD_TIME)/ FRAMES_PER_BUFFER)* FRAMES_PER_BUFFER; // on tronque à un multiple de FRAMES_PER_BUFFER
const unsigned int NBLOCKS_RECORD= NFRAMES_RECORD/ FRAMES_PER_BUFFER;

// affichage ncurses
const bool IS_NCURSES= true;


// ------------------------------------------------------------------------------------
// bloc regroupant une partie du spectre audio après fft
typedef struct _BlockFFT {
	float _instant_energy; // somme des carrés des coeffs fft pour la partie du spectre concernée
	float * _history_energy; // tableau d'énergies passées
	float _history_energy_average; // moyenne de l'historique
	float _cmp_ratio; // facteur multiplicatif pour test déclenchement événement
	float _variance_tresh; // seuil variance pour test déclenchement événement
	float _energy_variance; // variance de l'énergie
	int _history_idx; // idx courant pour aide a stockage dans _history_energy
	bool _is_triggered; // déclenchement
	int _width; // nombre de coeffs fft regroupés dans ce bloc
} BlockFFT;

typedef struct _BlockFFTRecord {
	float _instant_energy;
	bool _is_triggered;
} BlockFFTRecord;


// ------------------------------------------------------------------------------------
// variables globales
PaStream *stream;
PaError err;
int idx_device= -1;

float fft_energies[FFT_OUTPUT_SIZE];
BlockFFT blocks_fft[N_BLOCKS_FFT];

UdpTransmitSocket *transmit_socket;
char osc_buffer[OSC_BUFFER_SIZE];
osc::OutboundPacketStream *osc_packet;

float buf_frame_record[NFRAMES_RECORD* 2];
unsigned long idx_frame_record= 0;
string path_frame_record= "frame_record.wav";

BlockFFTRecord buf_block_record[NBLOCKS_RECORD][N_BLOCKS_FFT];
string path_block_record= "block_record.txt";


// ------------------------------------------------------------------------------------------------------------------------------
int pa_callback(const void *input, void *output, unsigned long frame_count, const PaStreamCallbackTimeInfo *time_info, PaStreamCallbackFlags status_flags, void *user_data) {

	float *in = (float *)input;
	float input_left, input_right;
	int i, j;
	unsigned int idx_block_record= idx_frame_record/ FRAMES_PER_BUFFER; // a faire avant de modifier idx_frame_record

	//printf("%lu\n", framesPerBuffer);

	// calcul de la fft
	double in_fft[FFT_INPUT_SIZE];
	fftw_complex *out_fft;
	fftw_plan p;

	out_fft= (fftw_complex*) fftw_malloc(sizeof(fftw_complex)* FFT_OUTPUT_SIZE);
	p= fftw_plan_dft_r2c_1d(FFT_INPUT_SIZE, in_fft, out_fft, FFTW_ESTIMATE);
	
	for (i=0; i<FFT_INPUT_SIZE; i++) {
		input_left= *in++;
		input_right= *in++;
		
		double hanning= 0.5* (1.0- cos(2.0* M_PI* (double)(i)/ (double)(FFT_INPUT_SIZE- 1)));
		double mix= (double)(input_left+ input_right)* 0.5;
		in_fft[i]= mix* hanning;

		buf_frame_record[2* idx_frame_record]= input_left;
		buf_frame_record[2* idx_frame_record+ 1]= input_right;
		idx_frame_record++;
		if (idx_frame_record>= NFRAMES_RECORD) {
			idx_frame_record= 0;
		}
	}

	fftw_execute(p);

	// calcul des énergies a partir des coeffs fft + passage en décibels
	for (i=0; i<FFT_OUTPUT_SIZE; i++) {
		fft_energies[i]= 20.0* log(out_fft[i][0]* out_fft[i][0]+ out_fft[i][1]* out_fft[i][1])/ log(10.0);
		if (fft_energies[i]< 0.0) {
			fft_energies[i]= 0.0;
		}
	}

	// les coeffs ne sont pas distribués uniformément dans les blocs. plus les fréquences sont hautes plus il y a de coeffs dans le bloc
	int idx_fft= 0;
	for (i=0; i<N_BLOCKS_FFT; i++) {
		// calcul de _instant_energy
		blocks_fft[i]._instant_energy= 0.0f;
		for (j=idx_fft; j<idx_fft+ blocks_fft[i]._width; ++j)
			blocks_fft[i]._instant_energy+= fft_energies[j];
		idx_fft+= blocks_fft[i]._width;
		
		// calcul de _history_energy_average
		blocks_fft[i]._history_energy_average= 0.0f;
		for (j=0; j<HISTORY_SIZE; ++j) {
			blocks_fft[i]._history_energy_average+= blocks_fft[i]._history_energy[j];
		}
		blocks_fft[i]._history_energy_average/= HISTORY_SIZE;

		// stockage dans _history_energy
		blocks_fft[i]._history_energy[blocks_fft[i]._history_idx]= blocks_fft[i]._instant_energy;
		blocks_fft[i]._history_idx++;
		if (blocks_fft[i]._history_idx== HISTORY_SIZE)
			blocks_fft[i]._history_idx= 0;
		
		// calcul de _energy_variance
		blocks_fft[i]._energy_variance= 0.0f;
		for (j=0; j<HISTORY_SIZE; ++j) {
			blocks_fft[i]._energy_variance+= (blocks_fft[i]._history_energy[j]- blocks_fft[i]._history_energy_average)* (blocks_fft[i]._history_energy[j]- blocks_fft[i]._history_energy_average);
		}
		blocks_fft[i]._energy_variance/= HISTORY_SIZE;

		// calcul de _is_triggered
		blocks_fft[i]._cmp_ratio= CMP_RATIO;
		blocks_fft[i]._variance_tresh= VARIANCE_TRESH;

		if ((blocks_fft[i]._instant_energy> blocks_fft[i]._cmp_ratio* blocks_fft[i]._history_energy_average) && (blocks_fft[i]._energy_variance> blocks_fft[i]._variance_tresh)) {
		//if (blocks_fft[i]._instant_energy> blocks_fft[i]._cmp_ratio* blocks_fft[i]._history_energy_average* blocks_fft[i]._energy_variance) {
			if (blocks_fft[i]._is_triggered== false) {
				osc_packet->Clear();
				*osc_packet << osc::BeginBundleImmediate << osc::BeginMessage("/test1") << i << osc::EndMessage << osc::EndBundle;
				transmit_socket->Send(osc_packet->Data(), osc_packet->Size());
			}

			blocks_fft[i]._is_triggered= true;
		}
		else {
			blocks_fft[i]._is_triggered= false;
		}

		buf_block_record[idx_block_record][i]._instant_energy= blocks_fft[i]._instant_energy;
		buf_block_record[idx_block_record][i]._is_triggered  = blocks_fft[i]._is_triggered;
		//cout << idx_frame_record << ";" << idx_block_record << endl;

	}

	// nettoyage fft
	fftw_destroy_plan(p);
	fftw_free(out_fft);

	return 0;
}


void init() {
	setbuf(stdout, NULL); // disable buffering on stdout

	// calcul de 2 coeffs utilisés par _width, de sorte que le 1er bloc (le + basse fréquence) regroupe N_COEFFS_IN_FIRST_BLOCK coeffs fft et que la somme des 
	// _width s'approche sans dépasser (!) FFT_OUTPUT_SIZE

	float a, b;
	a= ((float)(FFT_OUTPUT_SIZE)- (float)(N_COEFFS_IN_FIRST_BLOCK)* (float)(N_BLOCKS_FFT))/ ((float)(N_BLOCKS_FFT)* ((float)(N_BLOCKS_FFT)- 1.0f)* 0.5f);
	b= N_COEFFS_IN_FIRST_BLOCK- a;

	// init des blocs
	for (unsigned int i=0; i<N_BLOCKS_FFT; ++i) {
		blocks_fft[i]._instant_energy= 0.0f;
		blocks_fft[i]._history_energy= new float[HISTORY_SIZE];
		for (unsigned int j=0; j<HISTORY_SIZE; ++j)
			blocks_fft[i]._history_energy[j]= 0.0f;
		blocks_fft[i]._history_energy_average= 0.0f;
		blocks_fft[i]._cmp_ratio= 0.0f;
		blocks_fft[i]._variance_tresh= 0.0f;
		blocks_fft[i]._energy_variance= 0.0f;
		blocks_fft[i]._history_idx= 0;
		blocks_fft[i]._is_triggered= false;
		blocks_fft[i]._width= (int)(a* ((float)(i)+ 1.0f)+ b);
		printf("%i ; ", blocks_fft[i]._width);
	}
	printf("\n"); exit(0);

	for (unsigned int i=0; i<NFRAMES_RECORD* 2; ++i)
		buf_frame_record[i]= 0.0f;
	for (unsigned int i=0; i<NBLOCKS_RECORD; ++i) 
		for (unsigned int j=0; j<N_BLOCKS_FFT; j++) {
			buf_block_record[i][j]._instant_energy= 0.0f;
			buf_block_record[i][j]._is_triggered= false;
		}

	err= Pa_Initialize();
	if (err!= paNoError) {
		printf("ERROR: Pa_Initialize returned 0x%x\n", err);
	}
	//printf("PortAudio version number = %d\nPortAudio version text = '%s'\n", Pa_GetVersion(), Pa_GetVersionText());
	
	// si pas d'argument donné à l'executable
	if (idx_device< 0) {
		list_devices();
		printf("Quel device ? ");
		scanf("%d", &idx_device);
	}
	
	const PaDeviceInfo *deviceInfo= Pa_GetDeviceInfo(idx_device);
	//printf("%i channels\n", deviceInfo->maxInputChannels);
	
	PaStreamParameters inputParameters;
	bzero(&inputParameters, sizeof(inputParameters));
	inputParameters.device= idx_device;
	inputParameters.channelCount= 2; //deviceInfo->maxInputChannels; // 2 pour l'instant ...
	inputParameters.hostApiSpecificStreamInfo= NULL;
	inputParameters.sampleFormat= paFloat32;
	inputParameters.suggestedLatency= Pa_GetDeviceInfo(idx_device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo= NULL;
	
	err= Pa_OpenStream(&stream, &inputParameters, 0, (double)(SAMPLE_RATE), (unsigned long)(FRAMES_PER_BUFFER), paNoFlag, pa_callback, NULL);
	Pa_StartStream(stream);

	// init ncurses
	if (IS_NCURSES) {
		initscr();
		nodelay(stdscr, TRUE);
		noecho();
	}

	transmit_socket= new UdpTransmitSocket(IpEndpointName(OSC_ADDRESS.c_str(), OSC_PORT));
	osc_packet= new osc::OutboundPacketStream(osc_buffer, OSC_BUFFER_SIZE);

}


void main_loop() {
	unsigned int i;
	int ch;
	int x, y;
	int h, w;
	
	getmaxyx(stdscr, h, w);

	int block_w= 16;
	int block_h= 4;
	int sep_w= 3;
	int sep_h= 1;
	int n_blocks_w= w/ (block_w+ sep_w);
	
	while (1) {
		usleep(10000); // microseconds
	
		if (IS_NCURSES) {
			clear();
			for (i=0; i<N_BLOCKS_FFT; i++) {

				if (blocks_fft[i]._is_triggered) {
					attron(A_STANDOUT);
				}
				else {
					attroff(A_STANDOUT);
				}
				
				x= (i% n_blocks_w)* (block_w+ sep_w);
				y= (i/ n_blocks_w)* (block_h+ sep_h);

				move(y+ 0, x);
				printw("%i", i);

				move(y+ 1, x);
				printw("%8.1f", blocks_fft[i]._instant_energy);
				
				move(y+ 2, x);
				printw("%8.1f", blocks_fft[i]._history_energy_average);

				move(y+ 3, x);
				printw("%8.1f", blocks_fft[i]._energy_variance);
			}
			refresh();
		}

		if ((ch= getch())!= ERR) {
			break;
		}
	}
}


void clean() {
	err= Pa_StopStream(stream);
	Pa_CloseStream(stream);
	Pa_Terminate();

	for (unsigned int i=0; i<N_BLOCKS_FFT; ++i)
		delete[] blocks_fft[i]._history_energy;

	if (IS_NCURSES) {
		endwin();
	}

	// record stereo
	SF_INFO info_record;
	info_record.channels= 2;
	info_record.samplerate= 44100.0;
	info_record.format= SF_FORMAT_WAV | SF_FORMAT_FLOAT;

	SNDFILE* file_frame_record= sf_open(path_frame_record.c_str(), SFM_WRITE, &info_record);
	sf_write_float(file_frame_record, buf_frame_record, NFRAMES_RECORD* 2);
	sf_close(file_frame_record);

	ofstream file_block_record;
	file_block_record.open(path_block_record);
	for (unsigned int i=0; i<NBLOCKS_RECORD; ++i) {
		for (unsigned int j=0; j<N_BLOCKS_FFT; ++j) {
			file_block_record << i << " " << j << " " << buf_block_record[i][j]._instant_energy << " " << buf_block_record[i][j]._is_triggered << endl;
		}
	}
	file_block_record.close();
}


// ------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
	if (argc> 1) {
		sscanf(argv[1], "%d", &idx_device);
	}
	init();
	main_loop();
	clean();

	return 0;
}
