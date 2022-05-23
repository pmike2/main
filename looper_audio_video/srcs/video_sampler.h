#ifndef VIDEO_SAMPLER_H
#define VIDEO_SAMPLER_H

#include <string>
#include <map>

#include "shared_mem.h"
#include "mpeg_reader.h"



class VideoSampler : public Receiver {
public:
	VideoSampler();
	VideoSampler(unsigned int base_index, int movie_loc, int alpha_loc, int time_loc, int index_time_loc,
		int index_movie_loc, int global_alpha_loc, int modifier_loc, std::string json_path);
	~VideoSampler();
	void update();
	void note_on(unsigned int idx_track);
	void note_off(unsigned int idx_track);

	MPEGReaders * _mpeg_readers;
};

#endif
