#ifndef AUDIO_SAMPLER_H
#define AUDIO_SAMPLER_H

#include <string>

#include "sampler.h"


class AudioSample : public Sample {
public:
	AudioSample();
	AudioSample(std::string audio_sample_path);
	~AudioSample();

	float * _data;
	unsigned int _n_channels;
};

#endif
