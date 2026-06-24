#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "typedefs.h"
#include "bbox.h"


const uint N_MAX_FRAGMENTS = 5000;
const number FRAGMENT_OPACITY_DECREMENT = 0.1;
const number FRAGMENT_OPACITY_THRESHOLD = 0.05;
const uint N_MIN_FRAGMENTS_PER_EXPLOSION = 10;
const uint N_MAX_FRAGMENTS_PER_EXPLOSION = 100;


struct ExplosionFragment : public InstancePosRot {
	ExplosionFragment();
	~ExplosionFragment();
	void reinit(pt_3d origin);
	void anim(time_point t);


	bool _is_alive;
	number _opacity;
	pt_3d _velocity;
	pt_3d _acceleration;
	pt_3d _impulse;
	pt_3d _rot_axis;
	number _angle;
	number _angle_inc;
	number _scale_inc;
	bool _impulse_time;
	glm::vec3 _color;
};


struct Explosion {
	ExplosionSystem();
	~ExplosionSystem();
	void new_explosion(pt_3d pt, time_point t);
	void anim(time_point t);


	ExplosionFragment ** _fragments;
};


#endif
