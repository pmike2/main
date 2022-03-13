#include <iostream>
#include <fstream>

#include "sampler.h"

using namespace std;
using json = nlohmann::json;


SUB_SAMPLE_MODE get_sample_mode(string str_mode) {
	if (str_mode== "HOLD") {
		return HOLD;
	}
	if (str_mode== "FROM_START") {
		return FROM_START;
	}
	if (str_mode== "TO_END") {
		return TO_END;
	}
	if (str_mode== "ALL") {
		return ALL;
	}
	cout << "mode " << str_mode << " non supporté\n";
	return ALL;
}


// ----------------------------------------------------------------------
Sample::Sample() : _data(0), _n_frames(0), _n_channels(0) {

}


Sample::Sample(string wav) {
	SF_INFO info;
	SNDFILE * sound_file= sf_open(wav.c_str(), SFM_READ, &info);
	
	if (!sound_file) {
		cerr << "ERREUR sf_open : " << wav << "\n";
		return;
	}

	_n_frames= info.frames;
	_n_channels= info.channels;
	_data= new float[_n_frames* _n_channels];

	sf_read_float(sound_file, _data, _n_frames* _n_channels);
	sf_close(sound_file);
}


Sample::~Sample() {
	delete[] _data;
}


// ----------------------------------------------------------------------
SamplePool::SamplePool() {

}


SamplePool::~SamplePool() {
	for (const auto &x : _samples) {
		delete x.second;
	}
}


Sample * SamplePool::get_sample(string wav) {
	if (!_samples.count(wav)) {
		Sample * sample= new Sample(wav);
		_samples[wav]= sample;
	}
	return _samples[wav];
}


// ----------------------------------------------------------------------
SubSample::SubSample() :
	_sample(0), _t_start(time_type::zero()), _t_end(time_type::zero()), _mode(ALL) 
{

}


SubSample::SubSample(Sample * sample, time_type t_start, time_type t_end, SUB_SAMPLE_MODE mode) :
	_sample(sample), _t_start(t_start), _t_end(t_end), _mode(mode)
{

}


SubSample::~SubSample() {
	
}


// ----------------------------------------------------------------------
TrackSample::TrackSample() : _frame_idx(0), _playing(false) {
	_info.set_null();

}


TrackSample::~TrackSample() {

}


// ----------------------------------------------------------------------
Sampler::Sampler() {

}


Sampler::Sampler(string json_path) {
	_sample_pool= new SamplePool();
	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		_track_samples[idx_track]= new TrackSample();
	}

	ifstream istr(json_path);
	json js;
	istr >> js;
	
	for (auto & mapping : js.items()) {
		key_type key= (key_type)(mapping.key().c_str()[0]);
		json val= mapping.value();

		string wav= "";
		SUB_SAMPLE_MODE mode= ALL;
		time_type t_start= time_type::zero();
		time_type t_end= time_type::zero();
		
		if (val.contains("wav")) {
			wav= val["wav"];
		}
		else {
			cout << "Attribut wav manquant !\n";
			continue;
		}
		if (val.contains("mode")) {
			mode= get_sample_mode(val["mode"]);
		}
		if (val.contains("t_start")) {
			t_start= chrono::milliseconds(val["t_start"]);
		}
		if (val.contains("t_end")) {
			t_end= chrono::milliseconds(val["t_end"]);
		}

		Sample * sample= _sample_pool->get_sample(wav);
		_map[key]= new SubSample(sample, t_start, t_end, mode);
	}
}


Sampler::~Sampler() {
	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		delete _track_samples[idx_track];
	}
	for (const auto &x : _map) {
		delete x.second;
	}

	delete _sample_pool;
}


void Sampler::note_on(unsigned int idx_track) {
	_track_samples[idx_track]->_info= _data[idx_track];
	_track_samples[idx_track]->_frame_idx= 0;
	_track_samples[idx_track]->_playing= true;
}


void Sampler::note_off(unsigned int idx_track) {
	_track_samples[idx_track]->_info.set_null();
	_track_samples[idx_track]->_frame_idx= 0;
	_track_samples[idx_track]->_playing= false;
}


/*SamplePlaying * Sampler::get_first_not_playing() {
	for (unsigned int idx_sample=0; idx_sample<N_MAX_SAMPLE_PLAYING; ++idx_sample) {
		if (!_playing[idx_sample]->_playing) {
			return _playing[idx_sample];
		}
	}
	cout << "All samples playing !\n";
	return 0;
}
*/
