#include "utile.h"

#include "explosion.h"


// ExplosionFragment ------------------------------------------------------
ExplosionFragment::ExplosionFragment() : 
	InstancePosRot(), _is_alive(false), _opacity(1.0), _velocity(pt_3d(0.0)), _acceleration(pt_3d(0.0)),
	_rot_axis(pt_3d(0.0, 0.0, 1.0)), _angle_inc(0.0), _scale_inc(1.0), _impulse(pt_3d(0.0)), _impulse_time(false),
	_angle(0.0), _color(glm::vec3(1.0, 0.0, 0.0))
{

}


ExplosionFragment::~ExplosionFragment() {

}


void ExplosionFragment::reinit(pt_3d origin) {
	set_pos(origin);
	_is_alive = true;
	_opacity = 1.0;
	_velocity = pt_3d(0.0);
	_acceleration = pt_3d(0.0);
	_impulse = rand_pt_3d(1.0, 2.0, 0.0, M_PI * 0.5, 0.0, M_PI * 2.0);
	_rot_axis = rand_pt_3d(1.0, 1.0, 0.0, M_PI, 0.0, M_PI * 2.0);
	_angle_inc = rand_number(0.0, 0.1);
	_scale_inc = rand_number(1.0, 1.1);
	_impulse_time = true;
	_angle = 0.0;
}


void ExplosionFragment::anim(time_point t) {
	if (!_is_alive) {
		return;
	}

	_acceleration = pt_3d(0.0);
	if (_impulse_time) {
		_impulse_time = false;
		_acceleration = _impulse_time;
	}

	_acceleration += 0.1 * pt_3d(0.0, 0.0, -1.0);
	_acceleration -= 0.1 * _velocity;

	_velocity += _acceleration;

	pt_3d next_position = _position + _velocity;

	_angle += _angle_inc;
	glm::quat next_quat = glm::angleAxis(_angle, _rot_axis);

	pt_3d next_scale = _scale_inc * _scale;

	set_pos_rot_scale(next_position, next_quat, next_scale);

	_opacity-= FRAGMENT_OPACITY_DECREMENT;
	if (_opacity< FRAGMENT_OPACITY_THRESHOLD) {
		_opacity= 0.0;
		_is_alive= false;
	}

}


// ExplosionSystem ---------------------------------------------------------
ExplosionSystem::ExplosionSystem() {
	_fragments = new ExplosionFragment*[N_MAX_FRAGMENTS];
	for (uint i=0; i<N_MAX_FRAGMENTS; ++i) {
		_fragments[i] = new ExplosionFragment();
	}
}


ExplosionSystem::~ExplosionSystem() {
	for (uint i=0; i<N_MAX_FRAGMENTS; ++i) {
		delete _fragments[i];
	}
	delete[] _fragments;
}


void ExplosionSystem::new_explosion(pt_3d pt, time_point t) {
	uint n_fragments = rand_int(N_MIN_FRAGMENTS_PER_EXPLOSION, N_MAX_FRAGMENTS_PER_EXPLOSION);
	for (uint i=0; i<n_fragments; ++i) {
		ExplosionFragment * fragment = NULL;
		for (uint i=0; i<N_MAX_FRAGMENTS; ++i) {
			if (!_fragments[i]->_is_alive) {
				fragment = _fragments[i];
				break;
			}
		}
		if (fragment != NULL) {
			fragment->reinit(pt);
		}
		else {
			std::cout << "ExplosionSystem saturé\n";
		}
	}
}


void ExplosionSystem::anim(time_point t) {
	for (uint i=0; i<N_MAX_FRAGMENTS; ++i) {
		if (!_fragments[i]->_is_alive) {
			continue;
		}

		_fragments[i]->anim(t);
	}
}

