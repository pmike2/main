#include <fstream>

#include "json.hpp"

#include "utile.h"

#include "explosion.h"


using json = nlohmann::json;


// ExplosionConfig --------------------------------------------------------
ExplosionConfig::ExplosionConfig() {

}


ExplosionConfig::ExplosionConfig(std::string json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	_n_min_fragments_per_explosion = js["n_min_fragments_per_explosion"];
	_n_max_fragments_per_explosion = js["n_max_fragments_per_explosion"];
	_opacity_init = pt_2d(js["opacity_init_min"], js["opacity_init_max"]);
	_opacity_decrement = js["opacity_decrement"];
	_opacity_threshold = js["opacity_threshold"];
	_bbox_size_init = js["bbox_size_init"];
	_impulse_rho = pt_2d(js["impulse_rho_min"], js["impulse_rho_max"]);
	_impulse_theta = pt_2d(js["impulse_theta_min"], js["impulse_theta_max"]);
	_angle_inc = pt_2d(js["angle_inc_min"], js["angle_inc_max"]);
	_scale_inc = pt_2d(js["scale_inc_min"], js["scale_inc_max"]);
	_gravity_factor = js["gravity_factor"];
	_drag_factor = js["drag_factor"];
}


ExplosionConfig::~ExplosionConfig() {

}


void ExplosionConfig::DEBUG(std::string json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	_n_min_fragments_per_explosion = js["n_min_fragments_per_explosion"];
	_n_max_fragments_per_explosion = js["n_max_fragments_per_explosion"];
	_opacity_init = pt_2d(js["opacity_init_min"], js["opacity_init_max"]);
	_opacity_decrement = js["opacity_decrement"];
	_opacity_threshold = js["opacity_threshold"];
	_bbox_size_init = js["bbox_size_init"];
	_impulse_rho = pt_2d(js["impulse_rho_min"], js["impulse_rho_max"]);
	_impulse_theta = pt_2d(js["impulse_theta_min"], js["impulse_theta_max"]);
	_angle_inc = pt_2d(js["angle_inc_min"], js["angle_inc_max"]);
	_scale_inc = pt_2d(js["scale_inc_min"], js["scale_inc_max"]);
	_gravity_factor = js["gravity_factor"];
	_drag_factor = js["drag_factor"];
}


// ExplosionFragment ------------------------------------------------------
ExplosionFragment::ExplosionFragment() : 
	InstancePosRot(), _is_alive(false), _opacity(1.0), _velocity(pt_3d(0.0)), _acceleration(pt_3d(0.0)),
	_rot_axis(pt_3d(0.0, 0.0, 1.0)), _angle_inc(0.0), _scale_inc(1.0), _impulse(pt_3d(0.0)), _impulse_time(false),
	_angle(0.0), _color(glm::vec3(1.0, 1.0, 1.0)), _config(NULL)
{

}


ExplosionFragment::~ExplosionFragment() {

}


void ExplosionFragment::reinit(pt_3d origin, ExplosionConfig * config) {
	_config = config;

	_bbox->set_aabb(pt_3d(-0.5 * _config->_bbox_size_init), pt_3d(0.5 * _config->_bbox_size_init));
	set_pos_rot_scale(origin, glm::identity<quat>(), pt_3d(1.0));
	_is_alive = true;
	_opacity = rand_number(_config->_opacity_init[0], _config->_opacity_init[1]);
	_velocity = pt_3d(0.0);
	_acceleration = pt_3d(0.0);
	_impulse = rand_pt_3d_spherical(_config->_impulse_rho[0], _config->_impulse_rho[1], _config->_impulse_theta[0], _config->_impulse_theta[1], 0.0, M_PI * 2.0);
	_rot_axis = rand_pt_3d_spherical(1.0, 1.0, 0.0, M_PI, 0.0, M_PI * 2.0);
	_angle_inc = rand_number(_config->_angle_inc[0], _config->_angle_inc[1]);
	_scale_inc = rand_number(_config->_scale_inc[0], _config->_scale_inc[1]);
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
		_acceleration = _impulse;
	}

	_acceleration += _config->_gravity_factor * pt_3d(0.0, 0.0, -1.0);
	_acceleration -= _config->_drag_factor * _velocity;

	_velocity += _acceleration;

	pt_3d next_position = _position + _velocity;

	_angle += _angle_inc;
	glm::quat next_quat = glm::angleAxis(_angle, _rot_axis);

	pt_3d next_scale = _scale_inc * _scale;
	//pt_3d next_scale = pt_3d(1.0);

	set_pos_rot_scale(next_position, next_quat, next_scale);

	_opacity -= _config->_opacity_decrement;
	if (_opacity < _config->_opacity_threshold) {
		_opacity = 0.0;
		_is_alive = false;
	}
}


// ExplosionSystem ---------------------------------------------------------
ExplosionSystem::ExplosionSystem() {

}


ExplosionSystem::ExplosionSystem(std::string dir_configs) {
	_fragments = new ExplosionFragment*[N_MAX_FRAGMENTS];
	for (uint i=0; i<N_MAX_FRAGMENTS; ++i) {
		_fragments[i] = new ExplosionFragment();
	}

	std::vector<std::string> config_json_paths = list_files(dir_configs, "json");
	for (auto & json_path : config_json_paths) {
		_configs[basename(json_path)] = new ExplosionConfig(json_path);
	}
}


ExplosionSystem::~ExplosionSystem() {
	for (uint i=0; i<N_MAX_FRAGMENTS; ++i) {
		delete _fragments[i];
	}
	delete[] _fragments;
}


void ExplosionSystem::new_explosion(pt_3d pt, time_point t, ExplosionConfig * config) {
	uint n_fragments = rand_int(config->_n_min_fragments_per_explosion, config->_n_max_fragments_per_explosion);
	for (uint i=0; i<n_fragments; ++i) {
		ExplosionFragment * fragment = NULL;
		for (uint i=0; i<N_MAX_FRAGMENTS; ++i) {
			if (!_fragments[i]->_is_alive) {
				fragment = _fragments[i];
				break;
			}
		}
		if (fragment != NULL) {
			fragment->reinit(pt, config);
		}
		else {
			std::cout << "ExplosionSystem saturé\n";
			break;
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

