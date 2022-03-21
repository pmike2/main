#ifndef VIDEO_SAMPLER_H
#define VIDEO_SAMPLER_H

#include <string>
#include <map>

#include "looper.h"


enum SUB_SAMPLE_MODE {HOLD, FROM_START, TO_END, ALL};

SUB_SAMPLE_MODE get_sample_mode(std::string str_mode);


class VideoSample {
public:
	VideoSample();
	VideoSample(std::string sample_path);
	~VideoSample();
	void * get_frame(unsigned int frame_idx);

	unsigned int _n_frames;
	unsigned int _fps;
	unsigned char * _data;
	unsigned int _width;
	unsigned int _height;
	unsigned int _frame_size;
};


class VideoSamplePool {
public:
	VideoSamplePool();
	~VideoSamplePool();
	VideoSample * get_sample(std::string sample_path);

	std::map<std::string, VideoSample *> _samples;
};


class VideoSubSample {
public:
	VideoSubSample();
	VideoSubSample(VideoSample * sample, time_type t_start, time_type t_end, SUB_SAMPLE_MODE mode, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
	~VideoSubSample();

	VideoSample * _sample;
	time_type _t_start;
	time_type _t_end;
	SUB_SAMPLE_MODE _mode;
	unsigned int _x, _y, _w, _h;
};


class VideoTrackSample {
public:
	VideoTrackSample();
	~VideoTrackSample();

	sharedata_type _info;
	unsigned int _frame_idx;
	bool _playing;
	double _frame_idx_inc;
};


class VideoSampler : public Receiver {
public:
	VideoSampler();
	VideoSampler(std::string json_path);
	~VideoSampler();
	void note_on(unsigned int idx_track);
	void note_off(unsigned int idx_track);
	VideoSubSample * get_subsample(key_type key);

	VideoSamplePool * _sample_pool;
	std::map<key_type, VideoSubSample *> _map;
	VideoTrackSample * _track_samples[N_MAX_TRACKS];
};

#endif
