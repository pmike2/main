#ifndef SAMPLER_H
#define SAMPLER_H

#include <map>
#include <string>

#include "sndfile.h"
#include "json.hpp"

#include "looper.h"


enum SUB_SAMPLE_MODE {HOLD, FROM_START, TO_END, ALL};

SUB_SAMPLE_MODE get_sample_mode(std::string str_mode);



class Sample {
public:
	Sample();
	Sample(std::string wav);
	~Sample();

	float * _data;
	unsigned int _n_frames;
	unsigned int _n_channels;
};


class SamplePool {
public:
	SamplePool();
	~SamplePool();
	Sample * get_sample(std::string wav);

	std::map<std::string, Sample *> _samples;
};


class SubSample {
public:
	SubSample();
	SubSample(Sample * sample, time_type t_start, time_type t_end, SUB_SAMPLE_MODE mode);
	~SubSample();

	Sample * _sample;
	time_type _t_start;
	time_type _t_end;
	SUB_SAMPLE_MODE _mode;
};


class TrackSample {
public:
	TrackSample();
	~TrackSample();

	sharedata_type _info;
	unsigned int _frame_idx;
	bool _playing;
};


class Sampler : public Receiver {
public:
	Sampler();
	Sampler(std::string json_path);
	~Sampler();
	void note_on(unsigned int idx_track);
	void note_off(unsigned int idx_track);
	//SamplePlaying * get_first_not_playing();

	SamplePool * _sample_pool;
	std::map<key_type,SubSample *> _map;
	TrackSample * _track_samples[N_MAX_TRACKS];
};


#endif