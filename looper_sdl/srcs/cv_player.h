#ifndef CV_PLAYER_H
#define CV_PLAYER_H

#include <map>
#include <string>

#include "looper.h"



class CVTrack {
public:
	CVTrack();
	~CVTrack();

	sharedata_type _info;
	unsigned int _frame_idx;
	bool _playing;
};


class CVPlayer : public Receiver {
public:
	CVPlayer();
	CVPlayer(std::string json_path, unsigned int n_output_channels);
	~CVPlayer();
	void note_on(unsigned int idx_track);
	void note_off(unsigned int idx_track);
	unsigned int get_idx_channel(key_type key);

	CVTrack * _cv_tracks[N_MAX_TRACKS];
	std::map<key_type, unsigned int> _map;
	unsigned int _n_output_channels;
};


#endif
