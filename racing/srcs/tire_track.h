#ifndef TIRE_TRACK_H
#define TIRE_TRACK_H

#include <vector>
#include <chrono>

#include "car.h"
#include "geom_2d.h"
#include "typedefs.h"


// nombre max de traces de pneus
const unsigned int N_MAX_TIRE_TRACKS= 1000;

// écart en ms entre 2 créations de trace de pneu
const unsigned int TIRE_TRACKS_DELTA_T= 20;


const number TIRE_TRACK_OPACITY_INIT= 1.0;
const number TIRE_TRACK_OPACITY_DECREMENT= 0.05;
const number TIRE_TRACK_OPACITY_THRESHOLD= 0.02;


class TireTrack {
public:
	TireTrack();
	~TireTrack();
	void reinit(pt_type position, number alpha, pt_type scale, unsigned int idx_texture);
	void anim();


	BBox_2D * _bbox;
	number _opacity;
	unsigned int _idx_texture;
	bool _is_alive;
};


class TireTrackSystem {
public:
	TireTrackSystem();
	~TireTrackSystem();
	void anim(std::chrono::system_clock::time_point t, std::vector<Car *> cars);


	TireTrack ** _tracks;
};

#endif

