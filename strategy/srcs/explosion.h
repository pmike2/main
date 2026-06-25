#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <map>

#include "typedefs.h"
#include "bbox.h"


const uint N_MAX_FRAGMENTS = 2000;

/*const number FRAGMENT_OPACITY_DECREMENT = 0.01;
const number FRAGMENT_OPACITY_THRESHOLD = 0.01;
const uint N_MIN_FRAGMENTS_PER_EXPLOSION = 3;
const uint N_MAX_FRAGMENTS_PER_EXPLOSION = 10;*/


struct ExplosionConfig {
	ExplosionConfig();
	ExplosionConfig(std::string json_path);
	~ExplosionConfig();
	// a supprimer à terme
	void DEBUG(std::string json_path);


	uint _n_min_fragments_per_explosion;
	uint _n_max_fragments_per_explosion;

	pt_2d _opacity_init;
	number _opacity_decrement;
	number _opacity_threshold;
	number _bbox_size_init;
	pt_2d _impulse_rho;
	pt_2d _impulse_theta;
	pt_2d _angle_inc;
	pt_2d _scale_inc;
	number _gravity_factor;
	number _drag_factor;
};


struct ExplosionFragment : public InstancePosRot {
	ExplosionFragment();
	~ExplosionFragment();
	void reinit(pt_3d origin, ExplosionConfig * config);
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

	ExplosionConfig * _config;
};


struct ExplosionSystem {
	ExplosionSystem();
	ExplosionSystem(std::string dir_configs);
	~ExplosionSystem();
	void new_explosion(pt_3d pt, time_point t, ExplosionConfig * config);
	void anim(time_point t);


	ExplosionFragment ** _fragments;
	std::map<std::string, ExplosionConfig *> _configs;
};


#endif
