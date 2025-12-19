#include <iostream>
#include <fstream>
#include <sstream>

#include "ship.h"


// ship -------------------------------------------------------------
Ship::Ship() {

}


Ship::Ship(ShipModel * model, pt_2d pos, bool friendly, std::chrono::system_clock::time_point t) :
	_model(model), _friendly(friendly), _dead(false), _shooting(false), _current_action_name(MAIN_ACTION_NAME),
	_idx_action(0), _velocity(glm::vec2(0.0)), _idx_anim(0), 
	_hit(false), _hit_value(0.0), _delete(false), _alpha(1.0), _lives(_model->_lives),
	_rotation(0.0), _scale(1.0)
{
	_aabb= AABB_2D(pos, model->_size);
	ActionTexture * current_texture= get_current_texture();
	_footprint= AABB_2D(
		pt_2d(pos.x+ current_texture->_footprint._pos.x* _model->_size.x, pos.y+ current_texture->_footprint._pos.y* _model->_size.y),
		pt_2d(_model->_size.x* current_texture->_footprint._size.x, _model->_size.y* current_texture->_footprint._size.y)
	);
	_t_action_start= _t_last_hit= _t_die= _t_last_bullet= _t_anim_start= t;
}


Ship::~Ship() {
	
}


void Ship::anim(std::chrono::system_clock::time_point t, bool play_sounds) {
	_shooting= false;
	
	if (_dead) {
		//_hit= false;
		//_hit_value= 0.0;
		auto d_death= std::chrono::duration_cast<std::chrono::milliseconds>(t- _t_die).count();
		if (d_death> DEATH_MS) {
			_delete= true;
			_alpha= 0.0;
			_scale= 1.0;
			return;
		}
		else {
			_alpha= float(DEATH_MS- d_death)/ float(DEATH_MS);
			if (_model->_type== ENEMY) {
				_scale+= DEATH_SCALE_INC;
			}
		}
	}

	if (_hit) {
		auto d_hit= std::chrono::duration_cast<std::chrono::milliseconds>(t- _t_last_hit).count();
		if (d_hit> HIT_UNTOUCHABLE_MS) {
			_hit= false;
			_hit_value= 0.0;
			_rotation= 0.0;
		}
		else {
			_hit_value= float(HIT_UNTOUCHABLE_MS- d_hit)/ float(HIT_UNTOUCHABLE_MS);
			int i= d_hit/ HIT_ROTATION_MS;
			if (i% 2== 0) {
				_rotation+= HIT_ROTATION_INC;
			}
			else {
				_rotation-= HIT_ROTATION_INC;
			}
		}
	}
	else {
		_hit_value= 0.0;
	}

	Action * current_action= get_current_action();
	
	// chgmt action ; _t < 0 correspond à une action infinie
	auto d_action= std::chrono::duration_cast<std::chrono::milliseconds>(t- _t_action_start).count();
	if (current_action->_t> 0 && d_action> current_action->_t) {
		_t_action_start= t;
		_idx_action++;
		if (_idx_action>= _model->_actions[_current_action_name].size()) {
			_idx_action= 0;
		}
		_idx_anim= 0;
		//_t_last_bullet= t;
	}

	// après le chgmt d'action éventuel on met à jour current_action et on récupère le bullet et la texture courants
	current_action= get_current_action();
	ShipModel * bullet_model= get_current_bullet_model();
	ActionTexture * texture= get_current_texture();

	// faut-il tirer
	if (bullet_model!= NULL) {
		auto d_shoot= std::chrono::duration_cast<std::chrono::milliseconds>(t- _t_last_bullet).count();
		if (d_shoot> current_action->_t_shooting) {
			_t_last_bullet= t;
			_shooting= true;
			if (play_sounds && current_action->_shoot_sound!= NULL) {
				Mix_PlayChannel(-1, current_action->_shoot_sound, 0);
			}
		}
	}

	// maj vitesse et position
	if (_model->_type== ENEMY || _model->_type== BULLET) {
		_velocity= current_action->_direction;
	}
	
	_aabb._pos+= _velocity;

	_footprint._pos.x= _aabb._pos.x+ texture->_footprint._pos.x* _aabb._size.x;
	_footprint._pos.y= _aabb._pos.y+ texture->_footprint._pos.y* _aabb._size.y;
	_footprint._size.x= texture->_footprint._size.x*_aabb._size.x;
	_footprint._size.y= texture->_footprint._size.y*_aabb._size.y;

	// anim texture
	auto d_anim= std::chrono::duration_cast<std::chrono::milliseconds>(t- _t_anim_start).count();
	if (d_anim> texture->_t_anims[_idx_anim]) {
		_t_anim_start= t;
		_idx_anim++;
		if (_idx_anim>= texture->_t_anims.size()) {
			_idx_anim= 0;
		}
	}
}


Action * Ship::get_current_action() {
	return _model->_actions[_current_action_name][_idx_action];
}


ShipModel * Ship::get_current_bullet_model() {
	return get_current_action()->_bullet_model;
}


ActionTexture * Ship::get_current_texture() {
	std::string current_tex_name= get_current_action()->_texture_name;
	return _model->_textures[current_tex_name];
}


void Ship::set_current_action(std::string action_name, std::chrono::system_clock::time_point t) {
	_current_action_name= action_name;
	_idx_action= 0;
	_t_action_start= _t_anim_start= t;
	_idx_anim= 0;
}


bool Ship::hit(std::chrono::system_clock::time_point t, bool play_sounds) {
	if (_hit) {
		return false;
	}

	_lives--;
	if (_lives<= 0) {
		_dead= true;
		_t_die= t;
		if (play_sounds && _model->_death_sound!= NULL) {
			Mix_PlayChannel(-1, _model->_death_sound, 0);
		}
	}
	else {
		_hit= true;
		_t_last_hit= t;
		if (play_sounds && _model->_hit_sound!= NULL) {
			Mix_PlayChannel(-1, _model->_hit_sound, 0);
		}
	}
	return true;
}


std::ostream & operator << (std::ostream & os, const Ship & ship) {
	os << "model = " << *ship._model << " ; aabb = [" << ship._aabb << "]";
	return os;
}

