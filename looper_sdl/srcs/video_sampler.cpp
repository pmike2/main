#include <iostream>
#include <fstream>

#define __STDC_CONSTANT_MACROS
extern "C" {
#include<libavutil/avutil.h>
#include<libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include "json.hpp"

#include "video_sampler.h"

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
VideoSamplePool::VideoSamplePool() {

}


VideoSamplePool::~VideoSamplePool() {
	for (const auto &x : _samples) {
		delete x.second;
	}
}


VideoSample * VideoSamplePool::get_sample(string sample_path) {
	if (!_samples.count(sample_path)) {
		VideoSample * sample= new VideoSample(sample_path);
		_samples[sample_path]= sample;
	}
	return _samples[sample_path];
}


// ----------------------------------------------------------------------
VideoSubSample::VideoSubSample() :
	_sample(0), _t_start(time_type::zero()), _t_end(time_type::zero()), _mode(ALL) 
{

}


VideoSubSample::VideoSubSample(VideoSample * sample, time_type t_start, time_type t_end, SUB_SAMPLE_MODE mode, unsigned int x, unsigned int y, unsigned int w, unsigned int h) :
	_sample(sample), _t_start(t_start), _t_end(t_end), _mode(mode), _x(x), _y(y), _w(w), _h(h)
{

}


VideoSubSample::~VideoSubSample() {
	
}


// ----------------------------------------------------------------------
VideoTrackSample::VideoTrackSample() : _frame_idx(0), _playing(false), _frame_idx_inc(0.0) {
	_info.set_null();
}


VideoTrackSample::~VideoTrackSample() {

}


// ----------------------------------------------------------------------
VideoSampler::VideoSampler() {

}


VideoSampler::VideoSampler(string json_path) {
	_sample_pool= new VideoSamplePool();
	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		_track_samples[idx_track]= new VideoTrackSample();
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
		unsigned int x= 0;
		unsigned int y= 0;
		unsigned int w= 0;
		unsigned int h= 0;
		
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
		if (val.contains("x")) {
			x= val["x"];
		}
		if (val.contains("y")) {
			y= val["y"];
		}
		if (val.contains("w")) {
			w= val["w"];
		}
		if (val.contains("h")) {
			h= val["h"];
		}

		VideoSample * sample= _sample_pool->get_sample(sample_path);
		_map[key]= new VideoSubSample(sample, t_start, t_end, mode, x, y, w, h);
	}

	_debug_path= "../data/debug/video_sampler.txt";
}


VideoSampler::~VideoSampler() {
	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		delete _track_samples[idx_track];
	}
	for (const auto &x : _map) {
		delete x.second;
	}

	delete _sample_pool;
}


void VideoSampler::note_on(unsigned int idx_track) {
	//cout << "note_on\n";
	_track_samples[idx_track]->_info= _data_current[idx_track];
	_track_samples[idx_track]->_frame_idx= 0;
	_track_samples[idx_track]->_frame_idx_inc= 0.0;
	_track_samples[idx_track]->_playing= true;
}


void VideoSampler::note_off(unsigned int idx_track) {
	//cout << "note_off\n";
	_track_samples[idx_track]->_info.set_null();
	_track_samples[idx_track]->_frame_idx= 0;
	_track_samples[idx_track]->_frame_idx_inc= 0.0;
	_track_samples[idx_track]->_playing= false;
}


VideoSubSample * VideoSampler::get_subsample(key_type key) {
	if (!_map.count(key)) {
		return 0;
	}
	return _map[key];
}
