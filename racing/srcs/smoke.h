#ifndef SMOKE_H
#define SMOKE_H

#include <chrono>

#include "geom_2d.h"
#include "car.h"
#include "typedefs.h"


const unsigned int N_SMOKES_PER_CAR= 200;
const number SMOKE_SIZE_INCREMENT= 0.04;
const number SMOKE_OPACITY_DECREMENT= 0.04;
const number SMOKE_OPACITY_THRESHOLD= 0.02;
const unsigned int NEW_SMOKE_DELTA_MS= 20;
const number SMOKE_SIZE_INIT_FACTOR= 0.1;
const number SMOKE_Z_INCREMENT= 0.01;
const number SMOKE_Z_INIT= -45.0;
const number SMOKE_DIST_FROM_COM= 0.7;
const number SMOKE_OPACITY_INIT= 0.8;


class Smoke {
public:
	Smoke();
	~Smoke();
	void reinit(pt_type position, number alpha, pt_type scale, unsigned int idx_texture);
	void anim();


	BBox_2D * _bbox;
	number _opacity;
	unsigned int _idx_texture;
	bool _is_alive;
	number _z;
};


class SmokeSystem {
public:
	SmokeSystem();
	SmokeSystem(Car * car, unsigned int n_pngs, std::chrono::system_clock::time_point t);
	~SmokeSystem();
	void anim(std::chrono::system_clock::time_point t);
	

	Smoke ** _smokes;
	Car * _car;
	std::chrono::system_clock::time_point _last_creation_t;
	unsigned int _n_pngs;
};

#endif
