#ifndef CV_IN_H
#define CV_IN_H

#include <string>
#include <map>
#include <vector>
#include <iostream>

#include "json.hpp"

#include "shared_mem.h"


struct CVEvent {
	uint _idx_track;
	key_type _key;
	float _amplitude;
};


struct CVChannel {
	bool _active;
	uint _frame_idx;
};


class CVIn : public Emitter {
public:
	CVIn();
	CVIn(std::string json_path, uint n_input_channels);
	~CVIn();
	void load_json(std::string json_path);
	void load_json(nlohmann::json js);
	bool is_active(uint idx_channel);
	void set_active(uint idx_channel);
	void set_inactive(uint idx_channel);
	friend std::ostream & operator << (std::ostream & os, const CVIn & c);


	std::map<uint, std::vector<CVEvent> > _mapping;
	uint _n_input_channels;
	CVChannel * _input_channels;
};


#endif
