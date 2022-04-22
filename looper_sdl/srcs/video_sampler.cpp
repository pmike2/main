#include <iostream>

#include "video_sampler.h"

using namespace std;



VideoSampler::VideoSampler() {

}


VideoSampler::VideoSampler(unsigned int base_index, int movie_loc, int alpha_loc,
	int time_loc, int index_time_loc, int index_movie_loc, int global_alpha_loc, string json_path) {
	_mpeg_readers= new MPEGReaders(base_index, movie_loc, alpha_loc, time_loc, index_time_loc, index_movie_loc, global_alpha_loc);
	_mpeg_readers->load_json(json_path);
}


VideoSampler::~VideoSampler() {
	delete _mpeg_readers;
}


void VideoSampler::update() {
	Receiver::update();
	_mpeg_readers->update();
}


void VideoSampler::note_on(unsigned int idx_track) {
	//cout << "note_on " << idx_track << "\n";
	_mpeg_readers->note_on(idx_track, _data_current[idx_track]._key, _data_current[idx_track]._amplitude);
}


void VideoSampler::note_off(unsigned int idx_track) {
	//cout << "note_off " << idx_track << "\n";
	_mpeg_readers->note_off(idx_track);
}
