#include <cmath>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <chrono>
#include <ctime>
#include <vector>
#include <map>
#include <thread>

#include "sndfile.h"
#include "portaudio.h"

#include "utile.h"


using namespace std;

/*
	basé sur : http://www.katjaas.nl/beatdetection/beatdetection.html
	
	BLOCK_SIZE est la taille de calcul du RMS et du suivi de peak
	SILENT_TRESH est l'amplitude min pour qu'un bloc soit considéré
	TRIGGERING_RATIO compare l'amplitude de blocs consécutifs; plus il est grand, moins il y aura de markers
	plus DECAY_FACTOR est grand, plus l'exp est petite, et donc plus la pente descendante sera raide et plus il y aura de markers
	MIN_MARKERS_DISTANCE est le temps min en secondes entre 2 markers
	
	jeux tests :
	bcl1 : galère
	blc2 : pas mal: SILENT_TRESH= 0.02f; TRIGGERING_RATIO= 1.7f; DECAY_FACTOR= 1800.0f; MIN_MARKERS_DISTANCE= 0.03f;
	bcl3 : nickel : SILENT_TRESH= 0.01f; TRIGGERING_RATIO= 2.0f; DECAY_FACTOR= 1800.0f; MIN_MARKERS_DISTANCE= 0.03f;
	bcl4 : pas mal: SILENT_TRESH= 0.01f; TRIGGERING_RATIO= 1.7f; DECAY_FACTOR= 1800.0f; MIN_MARKERS_DISTANCE= 0.03f;
*/

uint BLOCK_SIZE= 64;
float SILENT_TRESH= 0.01f;
float TRIGGERING_RATIO= 1.7f;
float DECAY_FACTOR= 1800.0f;
float MIN_MARKERS_DISTANCE= 0.03f;



int main(int argc, char* argv[]) {
	// -------------------------------------------------------------------------
	SNDFILE *file_in;
	SF_INFO info_in;
	string path_in= string(argv[1]);
	file_in= sf_open(path_in.c_str(), SFM_READ, &info_in);
	
	float *buf_in       = new float[info_in.frames* info_in.channels]();
	float *buf_in_mono  = new float[info_in.frames]();
	float *buf_rms      = new float[info_in.frames]();
	float *buf_exp_decay= new float[info_in.frames]();
	float *buf_markers  = new float[info_in.frames]();

	float decay= exp(-1.0f* DECAY_FACTOR/ info_in.samplerate);

	sf_count_t num_read= sf_read_float(file_in, buf_in, info_in.frames* info_in.channels);
	
	cout << "nframes=" << info_in.frames << endl;
	cout << "nchannels=" << info_in.channels << endl;
	cout << "samplerate=" << info_in.samplerate << endl;
	cout << "num_read=" << num_read << endl;
	
	sf_close(file_in);
	
	// -------------------------------------------------------------------------
	if (info_in.channels== 1) {
		for (uint i=0; i<info_in.frames; ++i)
			buf_in_mono[i]= buf_in[i];
	}
	else if (info_in.channels== 2) {
		// on moyenne les 2 canaux
		for (uint i=0; i<info_in.frames; ++i)
			buf_in_mono[i]= (buf_in[2* i]+ buf_in[2* i+ 1])* 0.5f;
	}

	// -------------------------------------------------------------------------
	int nblocks= int(floor(info_in.frames/ BLOCK_SIZE));
	uint idx_sample_out= 0;
	for (uint idx_block=0; idx_block<nblocks; ++idx_block) {
		float rms= 0.0f;
		for (int idx_sample=0; idx_sample<BLOCK_SIZE; ++idx_sample) {
			rms+= buf_in_mono[BLOCK_SIZE* idx_block+ idx_sample]* buf_in_mono[BLOCK_SIZE* idx_block+ idx_sample];
		}
		rms= sqrt(rms/ float(BLOCK_SIZE));
		for (uint idx_sample=0; idx_sample<BLOCK_SIZE; ++idx_sample) {
			buf_rms[idx_sample_out]= rms;
			idx_sample_out++;
		}
	}
	
	float current_value= 0.0f;
	idx_sample_out= 0;
	for (uint idx_block=0; idx_block<nblocks; ++idx_block) {
		if (current_value* decay< buf_rms[idx_block* BLOCK_SIZE])
			current_value= buf_rms[idx_block* BLOCK_SIZE];
		else
			current_value*= decay;
		
		for (uint idx_sample=0; idx_sample<BLOCK_SIZE; ++idx_sample) {
			buf_exp_decay[idx_sample_out]= current_value;
			idx_sample_out++;
		}
	}
	
	// -------------------------------------------------------------------------
	vector<uint> markers;
	for (uint idx_block=0; idx_block<nblocks- 1; ++idx_block) {
		float val_before= buf_exp_decay[idx_block* BLOCK_SIZE];
		float val_after= buf_exp_decay[(idx_block+ 1)* BLOCK_SIZE];
		
		// test val_before pour ne pas diviser par 0
		if ( (val_after> SILENT_TRESH) && ((val_before< 1e-6) || (val_after> val_before* TRIGGERING_RATIO)) ) {
			uint idx_marker= 0;
			// on cherche du zero-crossing dans le mono; si le sample est stereo c'est un peu bidon; que faire ?
			for (uint idx_sample=(idx_block+ 1)* BLOCK_SIZE; idx_sample>idx_block* BLOCK_SIZE; --idx_sample) {
				if (buf_in_mono[idx_sample]* buf_in_mono[idx_sample- 1]< 0.0f) {
					idx_marker= idx_sample;
					break;
				}
				// pas de zero-crossing dans le bloc idx_block
				if (idx_sample== idx_block* BLOCK_SIZE+ 1)
					idx_marker= (idx_block+ 1)* BLOCK_SIZE;
			}
			
			if ((markers.size()== 0) || (idx_marker- markers[markers.size()- 1]>= MIN_MARKERS_DISTANCE* info_in.samplerate))
				markers.push_back(idx_marker);
		}
	}
	
	cout << markers.size() << " markers trouvés" << endl;
	for (auto& it_marker : markers) {
		//cout << it_marker << endl;
		for (uint i=it_marker; i<it_marker+ 20; ++i)
			buf_markers[i]= 0.8f;
	}
	
	// écriture des buffers temporaires pour debug
	// -------------------------------------------------------------------------
	string path_rms= "../data/"+ basename(path_in)+ "_rms.wav";
	
	SF_INFO info_rms;
	info_rms.channels= 1;
	info_rms.samplerate= 44100.0;
	info_rms.format= SF_FORMAT_WAV | SF_FORMAT_FLOAT;

	SNDFILE* file_rms= sf_open(path_rms.c_str(), SFM_WRITE, &info_rms);
	sf_write_float(file_rms, buf_rms, info_in.frames);
	sf_close(file_rms);

	// -------------------------------------------------------------------------
	string path_exp_decay= "../data/"+ basename(path_in)+ "_exp_decay.wav";
	
	SF_INFO info_exp_decay;
	info_exp_decay.channels= 1;
	info_exp_decay.samplerate= 44100.0;
	info_exp_decay.format= SF_FORMAT_WAV | SF_FORMAT_FLOAT;

	SNDFILE* file_exp_decay= sf_open(path_exp_decay.c_str(), SFM_WRITE, &info_exp_decay);
	sf_write_float(file_exp_decay, buf_exp_decay, info_in.frames);
	sf_close(file_exp_decay);

	// -------------------------------------------------------------------------
	string path_markers= "../data/"+ basename(path_in)+ "_markers.wav";
	
	SF_INFO info_markers;
	info_markers.channels= 1;
	info_markers.samplerate= 44100.0;
	info_markers.format= SF_FORMAT_WAV | SF_FORMAT_FLOAT;

	SNDFILE* file_markers= sf_open(path_markers.c_str(), SFM_WRITE, &info_markers);
	sf_write_float(file_markers, buf_markers, info_in.frames);
	sf_close(file_markers);

	// -------------------------------------------------------------------------
	delete[] buf_in;
	delete[] buf_in_mono;
	delete[] buf_rms;
	delete[] buf_exp_decay;
	delete[] buf_markers;
	
	return 0;
}

