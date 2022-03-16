#include <iostream>

#include "sndfile.h"

#include "audio_sampler.h"

using namespace std;


AudioSample::AudioSample() : _n_channels(0), _data(0) {

}


AudioSample::AudioSample(string audio_sample_path) : Sample(audio_sample_path) {
	SF_INFO info;
	SNDFILE * sound_file= sf_open(audio_sample_path.c_str(), SFM_READ, &info);
	
	if (!sound_file) {
		cerr << "ERREUR sf_open : " << audio_sample_path << "\n";
		return;
	}

	_n_frames= info.frames;
	_n_channels= info.channels;
	_fps= info.samplerate;
	_data= new float[_n_frames* _n_channels];

	sf_read_float(sound_file, _data, _n_frames* _n_channels);
	sf_close(sound_file);
}


AudioSample::~AudioSample() {
	delete[] _data;
}

