#include <fstream>

#include "cv_in.h"


using namespace std;
using json = nlohmann::json;


CVIn::CVIn() {

}


CVIn::CVIn(string json_path, unsigned int n_input_channels) : _n_input_channels(n_input_channels) {
	_input_channels= new CVChannel[_n_input_channels];
	for (unsigned int i=0; i<_n_input_channels; ++i) {
		_input_channels[i]._active= false;
		_input_channels[i]._frame_idx= 0;
	}
	load_json(json_path);
}


CVIn::~CVIn() {
	
}


void CVIn::load_json(string json_path) {
	ifstream istr(json_path);
	json js;
	istr >> js;
	load_json(js);
}


void CVIn::load_json(json js) {
	_mapping.clear();
	for (auto & mapping : js.items()) {
		unsigned int idx_channel_in= (unsigned int)(stoi(mapping.key()));
		json val= mapping.value();
		for (auto & event : val) {
			unsigned int idx_track= event["track"].get<unsigned int>();
			key_type key= (key_type)(event["key"].get<string>().c_str()[0]);
			float amplitude= event["amplitude"].get<float>();

			CVEvent cv_event= {idx_track, key, amplitude};
			_mapping[idx_channel_in].push_back(cv_event);
		}
	}
}


bool CVIn::is_active(unsigned int idx_channel) {
	return _input_channels[idx_channel]._active;
}


void CVIn::set_active(unsigned int idx_channel) {
	_input_channels[idx_channel]._active= true;
	_input_channels[idx_channel]._frame_idx= 0;

	for (auto & cv_event : _mapping[idx_channel]) {
		_data[cv_event._idx_track]._key= cv_event._key;
		_data[cv_event._idx_track]._amplitude= cv_event._amplitude;
	}
}


void CVIn::set_inactive(unsigned int idx_channel) {
	_input_channels[idx_channel]._active= false;
	_input_channels[idx_channel]._frame_idx= 0;

	for (auto & cv_event : _mapping[idx_channel]) {
		_data[cv_event._idx_track].set_null();
	}
}


ostream & operator << (ostream & os, const CVIn & c) {
	os << "n_input_channels=" << c._n_input_channels << "\n";
	for (auto & m : c._mapping) {
		os << "channel_idx=" << m.first << "\n";
		for (auto & e : m.second) {
			os << "\t" << "idx_track=" << e._idx_track << " ; key=" << e._key << " ; amplitude=" << e._amplitude << "\n";
		}
	}
	return os;
}
