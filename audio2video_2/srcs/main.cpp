
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cmath>

#include <fftw3.h>

#include "sndfile.h"


int main() {
	srand(time(NULL));

	const unsigned int BLOCK_SIZE = 2048;
	const unsigned int DELTA_OFFSET = 1024;

	std::string wav_path = "../data/wav/bcl1.wav";
	//std::string wav_path = "../data/wav/sine_100.wav";
	//std::string wav_path = "../data/wav/sine_1000.wav";


	SF_INFO info;
	SNDFILE* file = sf_open(wav_path.c_str(), SFM_READ, &info);
	if (!file) {
		std::cerr << "sf_open erreur " << wav_path << "\n";
		return 1;
	}

	double buff[info.channels* info.frames];
	sf_read_double(file, buff, info.channels* info.frames);
	sf_close(file);

	double buff_mono[info.frames];
	if (info.channels == 2) {
		for (int i=0; i<info.frames; ++i) {
			buff_mono[i] = buff[2 * i]; // on prend juste left
		}
	}
	else if (info.channels == 1) {
		for (int i=0; i<info.frames; ++i) {
			buff_mono[i] = buff[i];
		}
	}
	else {
		std::cerr << "n channels = " << info.channels << " non supporté\n";
		return 2;
	}

	std::ofstream wav_file("../data/wav/test.txt");
	for (int i=0; i<info.frames; ++i) {
		wav_file << buff_mono[i] << "\n";
	}
	wav_file.close();

	fftw_complex *in, *out;
	fftw_plan p;
	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * BLOCK_SIZE);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * BLOCK_SIZE);

	bool done = false;
	unsigned int offset = 0;
	while (true) {
		for (unsigned int i=0; i<BLOCK_SIZE; ++i) {
			if (offset + i>= info.frames) {
				done = true;
				break;
			}
			double hanning= 0.5* (1.0- cos(2.0* M_PI* (double)(i)/ (double)(BLOCK_SIZE)));

			//in[i][0] = buff_mono[i];
			in[i][0] = buff_mono[offset + i] * hanning;
			in[i][1] = 0.0;
		}
		if (done) {
			break;
		}

		p = fftw_plan_dft_1d(BLOCK_SIZE, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		
		std::string fft_file_path = "../data/fft/test_" + std::to_string(offset) + ".txt";
		std::ofstream fft_file(fft_file_path);
		for (unsigned int i=0; i<BLOCK_SIZE / 2; ++i) {
			double amplitude = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
			fft_file << amplitude << "\n";
		}
		fft_file.close();

		offset+= DELTA_OFFSET;
	}

	fftw_destroy_plan(p);
	fftw_free(in);
	fftw_free(out);
	
	return 0;
}
