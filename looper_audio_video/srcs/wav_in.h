#ifndef WAV_IN_H
#define WAV_IN_H

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <deque>
#include <set>
#include <chrono>

#include "json.hpp"

#include "shared_mem.h"
#include "typedefs.h"


struct WavEvent {
	uint _idx_track;
	key_type _key;
	float _amplitude;
	time_point _t;
};

struct WavEventConfig {
	uint _idx_track;
	key_type _key;
	float _amplitude_factor;
};


struct EventCmp {
	bool operator()(const WavEvent lhs, const WavEvent rhs) const;
};


class WavIn : public Emitter {
public:
	WavIn();
	WavIn(std::string json_path);
	~WavIn();
	void load_json(std::string json_path);
	void load_json(nlohmann::json js);
	void new_envelope(std::string s);
	void main_loop();

	std::map<uint, std::vector<WavEventConfig> > _mapping;
	std::set<WavEvent, EventCmp> _events;
	number _delta_event;
};

#endif
