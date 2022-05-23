#ifndef CV_OUT_H
#define CV_OUT_H

#include <map>
#include <string>
#include <iostream>

#include "json.hpp"

#include "shared_mem.h"



class CVTrack {
public:
	CVTrack();
	~CVTrack();

	sharedata_type _info;
	unsigned int _frame_idx;
	bool _playing;
};


class CVOut : public Receiver {
public:
	CVOut();
	CVOut(std::string json_path, unsigned int n_output_channels);
	~CVOut();
	void load_json(std::string json_path);
	void load_json(nlohmann::json js);
	void note_on(unsigned int idx_track);
	void note_off(unsigned int idx_track);
	unsigned int get_idx_channel(key_type key);
	friend std::ostream & operator << (std::ostream & os, const CVOut & c);


	CVTrack * _cv_tracks[N_MAX_TRACKS];
	std::map<key_type, unsigned int> _map;
	unsigned int _n_output_channels;
};


#endif
