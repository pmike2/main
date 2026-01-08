#ifndef CV_OUT_H
#define CV_OUT_H

#include <map>
#include <string>
#include <iostream>

#include "json.hpp"

#include "constantes.h"
#include "shared_mem.h"



class CVTrack {
public:
	CVTrack();
	~CVTrack();

	sharedata_type _info;
	uint _frame_idx;
	bool _playing;
};


class CVOut : public Receiver {
public:
	CVOut();
	CVOut(std::string json_path, uint n_output_channels);
	~CVOut();
	void load_json(std::string json_path);
	void load_json(nlohmann::json js);
	void note_on(uint idx_track);
	void note_off(uint idx_track);
	uint get_idx_channel(key_type key);
	friend std::ostream & operator << (std::ostream & os, const CVOut & c);


	CVTrack * _cv_tracks[N_TRACKS];
	std::map<key_type, uint> _map;
	uint _n_output_channels;
};


#endif
