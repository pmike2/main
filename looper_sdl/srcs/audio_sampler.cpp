#include <iostream>
#include <fstream>

#include "sndfile.h"
#include "json.hpp"

#include "audio_sampler.h"

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
AudioSample::AudioSample() : _n_frames(0), _fps(0), _n_channels(0), _data(0) {

}


AudioSample::AudioSample(string sample_path) {
	SF_INFO info;
	SNDFILE * sound_file= sf_open(sample_path.c_str(), SFM_READ, &info);
	
	if (!sound_file) {
		cerr << "ERREUR sf_open : " << sample_path << "\n";
		return;
	}

	_n_frames= info.frames;
	_n_channels= info.channels;
	_fps= info.samplerate;
	_data= new float[_n_frames* _n_channels];

	sf_read_float(sound_file, _data, _n_frames* _n_channels);
	sf_close(sound_file);

}

AudioSample::~AudioSample() {
	delete[] _data;
}


// ----------------------------------------------------------------------
AudioSamplePool::AudioSamplePool() {

}


AudioSamplePool::~AudioSamplePool() {
	for (const auto &x : _samples) {
		delete x.second;
	}
}


AudioSample * AudioSamplePool::get_sample(string sample_path) {
	if (!_samples.count(sample_path)) {
		AudioSample * sample= new AudioSample(sample_path);
		_samples[sample_path]= sample;
	}
	return _samples[sample_path];
}


// ----------------------------------------------------------------------
AudioSubSample::AudioSubSample() :
	_sample(0), _t_start(time_type::zero()), _t_end(time_type::zero()), _mode(ALL) 
{

}


AudioSubSample::AudioSubSample(AudioSample * sample, time_type t_start, time_type t_end, SUB_SAMPLE_MODE mode) :
	_sample(sample), _t_start(t_start), _t_end(t_end), _mode(mode)
{

}


AudioSubSample::~AudioSubSample() {
	
}


// ----------------------------------------------------------------------
AudioTrackSample::AudioTrackSample() : _frame_idx(0), _playing(false) {
	_info.set_null();

}


AudioTrackSample::~AudioTrackSample() {

}


// ----------------------------------------------------------------------
AudioSampler::AudioSampler() {

}


AudioSampler::AudioSampler(string json_path) {
	_sample_pool= new AudioSamplePool();
	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		_track_samples[idx_track]= new AudioTrackSample();
	}

	ifstream istr(json_path);
	json js;
	istr >> js;
	
	for (auto & mapping : js.items()) {
		key_type key= (key_type)(mapping.key().c_str()[0]);
		json val= mapping.value();

		string sample_path= "";
		SUB_SAMPLE_MODE mode= ALL;
		time_type t_start= time_type::zero();
		time_type t_end= time_type::zero();
		
		if (val.contains("sample_path")) {
			sample_path= val["sample_path"];
		}
		else {
			cout << "Attribut sample_path manquant !\n";
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

		AudioSample * sample= _sample_pool->get_sample(sample_path);
		_map[key]= new AudioSubSample(sample, t_start, t_end, mode);
	}

	_debug_path= "../data/debug/audio_sampler.txt";
}


AudioSampler::~AudioSampler() {
	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		delete _track_samples[idx_track];
	}
	for (const auto &x : _map) {
		delete x.second;
	}

	delete _sample_pool;
}


void AudioSampler::note_on(unsigned int idx_track) {
	_track_samples[idx_track]->_info= _data[idx_track];
	_track_samples[idx_track]->_frame_idx= 0;
	_track_samples[idx_track]->_playing= true;
}


void AudioSampler::note_off(unsigned int idx_track) {
	_track_samples[idx_track]->_info.set_null();
	_track_samples[idx_track]->_frame_idx= 0;
	_track_samples[idx_track]->_playing= false;
}


AudioSubSample * AudioSampler::get_subsample(key_type key) {
	if (!_map.count(key)) {
		return 0;
	}
	return _map[key];
}
