#ifndef VIDEO_SAMPLER_H
#define VIDEO_SAMPLER_H

#include <string>

#include "sampler.h"


class VideoSample : public Sample {
public:
	VideoSample();
	VideoSample(std::string video_sample_path);
	~VideoSample();

	unsigned char * _data;
	unsigned int width;
	unsigned int height;
};

#enfif
