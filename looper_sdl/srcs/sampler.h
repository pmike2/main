#ifndef SAMPLER_H
#define SAMPLER_H

#include <map>

#include "sndfile.h"
#include "json.hpp"

#include "looper.h"


enum SUB_SAMPLE_MODE {HOLD, END, ALL};


const unsigned int N_MAX_SAMPLE_PLAYING= 128;


class Sample {
public:
	Sample();
	~Sample();
	void read_from_file(std::string file_path);

	float * _data;
	unsigned int _n_samples;
	bool _stereo;
};


class SubSample {
public:
	SubSample();
	~SubSample();

	Sample * _sample;
	unsigned int _start;
	unsigned int _end;
	SUB_SAMPLE_MODE _mode;
};


class SamplePlaying {
public:
	SamplePlaying();
	~SamplePlaying();

	sharedata_type _info;
	unsigned int _current_idx;
	bool _playing;
};


class Sampler : public Receiver {
public:
	Sampler();
	Sampler(nlohmann::json js);
	~Sampler();
	void on_new_data(sharedata_type data);
	SamplePlaying * get_first_not_playing();

	std::map<key_type,SubSample> _map;
	SamplePlaying * _playing[N_MAX_SAMPLE_PLAYING];
};


#endif
