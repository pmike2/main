
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
	const unsigned int WAV_TXT_NSAMPLES = 10000;

	//std::filesystem::path wav_path = "../data/wav/sine_100.wav";
	//std::filesystem::path wav_path = "../data/wav/sine_1000.wav";
	//std::filesystem::path wav_path = "../data/wav/sine_noise.wav";
	//std::filesystem::path wav_path = "../data/wav/bcl7.wav";
	//std::filesystem::path wav_path = "../data/wav/record_beat_simple.wav";
	std::filesystem::path wav_path = "../data/wav/metronome.wav";

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

	SF_INFO wav_info;
	SNDFILE* wav_file = sf_open(wav_path.string().c_str(), SFM_READ, &wav_info);
	if (!wav_file) {
		std::cerr << "sf_open erreur " << wav_path << "\n";
		return 1;
	}
	if (wav_info.channels> 2) {
		std::cerr << "n channels = " << wav_info.channels << " non supporté\n";
		return 2;
	}

	std::ofstream ofs(json_path.string());
	json js;
	js["frames"] = wav_info.frames;
	js["samplerate"] = wav_info.samplerate;
	js["channels"] = wav_info.channels;
	js["block_size"] = BLOCK_SIZE;
	js["delta_offset"] = DELTA_OFFSET;

	std::ofstream wav_txt(wav_txt_path.string());
	double wav_txt_step = wav_info.frames / WAV_TXT_NSAMPLES;

	fftw_complex *in, *out;
	fftw_plan p;
	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * BLOCK_SIZE);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * BLOCK_SIZE);

	unsigned int idx_offset = 0;
	double buff[wav_info.channels* BLOCK_SIZE];
	double wav_txt_pos = 0.0;
	while (true) {
		unsigned int n_frames_read = sf_read_double(wav_file, buff, wav_info.channels* BLOCK_SIZE);
		if (n_frames_read < wav_info.channels * BLOCK_SIZE) {
			break;
		}

		while (true) {
			if (wav_txt_pos < idx_offset * DELTA_OFFSET + BLOCK_SIZE) {
				unsigned int buff_idx = (unsigned int)(wav_txt_pos) - idx_offset * DELTA_OFFSET;
				if (wav_info.channels == 2) {
					wav_txt << buff[2 * buff_idx] << "\n";
				}
				else if (wav_info.channels == 1) {
					wav_txt << buff[buff_idx] << "\n";
				}
				wav_txt_pos+= wav_txt_step;
			}
			else {
				break;
			}
		}

		for (unsigned int i=0; i<BLOCK_SIZE; ++i) {
			double hanning= 0.5* (1.0- cos(2.0* M_PI* (double)(i)/ (double)(BLOCK_SIZE)));

			if (wav_info.channels == 2) {
				in[i][0] = buff[2 * i] * hanning; // left
				in[i][1] = 0.0;
			}
			else if (wav_info.channels == 1) {
				in[i][0] = buff[i] * hanning;
				in[i][1] = 0.0;
			}
		}

		p = fftw_plan_dft_1d(BLOCK_SIZE, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		
		std::filesystem::path fft_file_path = ffts_path / (std::to_string(idx_offset) + ".txt");
		std::ofstream fft_file(fft_file_path.string());
		for (unsigned int i=0; i<BLOCK_SIZE / 2; ++i) {
			double amplitude = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
			fft_file << amplitude << "\n";
		}
		fft_file.close();

		idx_offset++;
		sf_seek(wav_file, idx_offset * DELTA_OFFSET, SEEK_SET);
	}
	js["n_blocks"] = idx_offset;

	ofs << std::setw(4) << js << "\n";
	ofs.close();
	sf_close(wav_file);
	wav_txt.close();

	fftw_destroy_plan(p);
	fftw_free(in);
	fftw_free(out);
	
	return 0;
}
