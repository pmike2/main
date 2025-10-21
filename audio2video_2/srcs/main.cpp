
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cmath>
#include <filesystem>

#include <fftw3.h>
#include "sndfile.h"
#include "json.hpp"

using json = nlohmann::json;


int main() {
	srand(time(NULL));

	const unsigned int BLOCK_SIZE = 1024;
	const unsigned int DELTA_OFFSET = 512;

	//std::filesystem::path wav_path = "../data/wav/sine_100.wav";
	std::filesystem::path wav_path = "../data/wav/sine_1000.wav";
	//std::filesystem::path wav_path = "../data/wav/sine_noise.wav";
	//std::filesystem::path wav_path = "../data/wav/bcl1.wav";

	std::filesystem::path parent_path = wav_path.parent_path() / wav_path.stem();
	std::filesystem::path ffts_path = parent_path / "fft";
	std::filesystem::path json_path = parent_path / (wav_path.stem().string() + ".json");
	std::filesystem::path wav_txt_path = parent_path / (wav_path.stem().string() + ".txt");

	std::vector<std::string> cmds = {
		"rm -rf " + parent_path.string(),
		"mkdir " + parent_path.string(),
		"mkdir " + ffts_path.string(),
	};
	for (auto cmd : cmds) {
		std::system(cmd.c_str());
	}

	SF_INFO info;
	SNDFILE* file = sf_open(wav_path.string().c_str(), SFM_READ, &info);
	if (!file) {
		std::cerr << "sf_open erreur " << wav_path << "\n";
		return 1;
	}

	std::ofstream ofs(json_path.string());
	json js;
	js["frames"] = info.frames;
	js["samplerate"] = info.samplerate;
	js["channels"] = info.channels;
	js["block_size"] = BLOCK_SIZE;
	js["delta_offset"] = DELTA_OFFSET;
	ofs << std::setw(4) << js << "\n";
	ofs.close();

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

	std::ofstream wav_file(wav_txt_path.string());
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

			//in[i][0] = buff_mono[offset + i];
			in[i][0] = buff_mono[offset + i] * hanning;
			in[i][1] = 0.0;
		}
		if (done) {
			break;
		}

		p = fftw_plan_dft_1d(BLOCK_SIZE, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		
		std::filesystem::path fft_file_path = ffts_path / (std::to_string(offset) + ".txt");
		std::ofstream fft_file(fft_file_path.string());
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
