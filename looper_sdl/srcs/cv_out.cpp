#include <iostream>
#include <fstream>

#include "json.hpp"

#include "cv_out.h"

using namespace std;
using json = nlohmann::json;



// ----------------------------------------------------------------------
CVTrack::CVTrack() : _frame_idx(0), _playing(false) {
	_info.set_null();
}


CVTrack::~CVTrack() {

}


// ----------------------------------------------------------------------
CVOut::CVOut() {

}


CVOut::CVOut(string json_path, unsigned int n_output_channels) : _n_output_channels(n_output_channels) {
	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		_cv_tracks[idx_track]= new CVTrack();
	}

	ifstream istr(json_path);
	json js;
	istr >> js;
	
	for (auto & mapping : js.items()) {
		key_type key= (key_type)(mapping.key().c_str()[0]);
		json val= mapping.value();

		_map[key]= val["channel"].get<unsigned int>();
	}

	_debug_path= "../data/debug/cv_player.txt";
}


CVOut::~CVOut() {
	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		delete _cv_tracks[idx_track];
	}
}


void CVOut::note_on(unsigned int idx_track) {
	_cv_tracks[idx_track]->_info= _data[idx_track];
	_cv_tracks[idx_track]->_frame_idx= 0;
	_cv_tracks[idx_track]->_playing= true;
}


void CVOut::note_off(unsigned int idx_track) {
	_cv_tracks[idx_track]->_info.set_null();
	_cv_tracks[idx_track]->_frame_idx= 0;
	_cv_tracks[idx_track]->_playing= false;
}


unsigned int CVOut::get_idx_channel(key_type key) {
	if (!_map.count(key)) {
		return 0;
	}
	return _map[key];
}
