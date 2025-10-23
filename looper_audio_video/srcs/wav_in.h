#ifndef WAV_IN_H
#define WAV_IN_H

#include <string>
#include <map>
#include <vector>
#include <iostream>

#include "json.hpp"

#include "shared_mem.h"


class WavIn : public Emitter {
public:
	WavIn();
	WavIn(std::string json_path);
	~WavIn();
	void new_envelope(std::string s);
};

#endif
