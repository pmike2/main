#ifndef SAMPLER_H
#define SAMPLER_H

#include <map>
#include <string>

#include "json.hpp"

#include "shared_mem.h"


enum SUB_SAMPLE_MODE {HOLD, FROM_START, TO_END, ALL};

SUB_SAMPLE_MODE get_sample_mode(std::string str_mode);


class AudioSample {
public:
	AudioSample();
	AudioSample(std::string sample_path);
	~AudioSample();

	unsigned int _n_frames;
	unsigned int _fps;
	float * _data;
	unsigned int _n_channels;
};


class AudioSamplePool {
public:
	AudioSamplePool();
	~AudioSamplePool();
	AudioSample * get_sample(std::string sample_path);
	void clear();

	std::map<std::string, AudioSample *> _samples;
};


class AudioSubSample {
public:
	AudioSubSample();
	AudioSubSample(AudioSample * sample, time_type t_start, time_type t_end, SUB_SAMPLE_MODE mode);
	~AudioSubSample();

	AudioSample * _sample;
	time_type _t_start;
	time_type _t_end;
	SUB_SAMPLE_MODE _mode;
};


class AudioTrackSample {
public:
	AudioTrackSample();
	~AudioTrackSample();

	sharedata_type _info;
	unsigned int _frame_idx;
	bool _playing;
};


class AudioSampler : public Receiver {
public:
	AudioSampler();
	AudioSampler(std::string json_path);
	~AudioSampler();
	void load_json(std::string json_path);
	void load_json(nlohmann::json js);
	void note_on(unsigned int idx_track);
	void note_off(unsigned int idx_track);
	AudioSubSample * get_subsample(key_type key);

	AudioSamplePool * _sample_pool;
	std::map<key_type, AudioSubSample *> _map;
	AudioTrackSample * _track_samples[N_MAX_TRACKS];
};


#endif
