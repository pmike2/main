#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ammo.h"


Ammo::Ammo() {

}


Ammo::Ammo(AmmoType * type, pt_3d pos, pt_3d target) :
	InstancePosRot(pos, quat(1.0, 0.0, 0.0, 0.0), pt_3d(1.0), type->_obj_data->_aabb),
	_type(type), _target(target), _target_hit(false), _theta(0.0), _pos_ini(pos)
{
	_target_dist = glm::length(pt_2d(_target) - pt_2d(_position));
	if (_target_dist < 1.0) {
		_target_hit = true;
		return;
	}

	pt_2d p1(0.0, pos.z);
	pt_2d p2(_target_dist * 0.5, pos.z + _target_dist * _type->_apogee);
	pt_2d p3(_target_dist, target.z);
	mat_3d m_parabola = mat_3d(
		p1.x * p1.x , p2.x * p2.x , p3.x * p3.x ,
		p1.x, p2.x, p3.x,
		1.0, 1.0, 1.0
	);
	_parabola_coeffs = glm::inverse(m_parabola) * pt_3d(p1.y, p2.y, p3.y);

	_velocity = _type->_velocity * (_target - _position) / _target_dist;
	
	number phi = atan2(_velocity.y, _velocity.x);
	pt_3d phi_axis(0.0, 0.0, 1.0);
	quat init_quat_1 = glm::angleAxis(phi, phi_axis);

	number theta = atan(2.0 * _parabola_coeffs[0] * 0.0 + _parabola_coeffs[1]);
	pt_3d theta_axis(_target.y - _pos_ini.y, _pos_ini.x - _target.x, 0.0);
	quat init_quat_2 = glm::angleAxis(theta, theta_axis / glm::length(theta_axis));

	set_pos_rot_scale(_position, init_quat_2 * init_quat_1, pt_3d(1.0));

	// A revoir
	
	/*const number delta_z_min = -2.0;
	const number delta_z_max = 2.0;
	const number damage_factor_min = 0.1;
	const number damage_factor_max = 2.0;
	const number a = (damage_factor_max - damage_factor_min) / (delta_z_max - delta_z_min);
	const number b = damage_factor_min - a * delta_z_min;
	const number distance_factor = 0.5;
	
	number delta_z = _position.z - _target.z;
	if (delta_z < delta_z_min) {
		_damage = _type->_damage * damage_factor_min;
	}
	else if (delta_z > delta_z_max) {
		_damage = _type->_damage * damage_factor_max;
	}
	else {
		_damage = _type->_damage * (a * delta_z + b);
	}

	_damage *= (1.0 - distance_factor * (d / _type->_max_distance));*/

	_damage = _type->_damage;
}


Ammo::~Ammo() {

}


void Ammo::anim() {
	if (_target_hit) {
		return;
	}

	number d = glm::length(pt_2d(_target) - pt_2d(_position));
	if (d < 1.0) {
		_target_hit = true;
		return;
	}

	_velocity = _type->_velocity * (_target - _position) / d;
	pt_3d next_position = _position + _velocity;

	next_position.z = _parabola_coeffs[0] * (_target_dist - d) * (_target_dist - d) + _parabola_coeffs[1] * (_target_dist - d) + _parabola_coeffs[2];

	/*number next_angle = atan2(_velocity.y, _velocity.x);
	if (next_angle - _angle > M_PI) {
		next_angle -= 2.0 * M_PI;
	}
	_angle = next_angle;*/
	
	// https://en.wikipedia.org/wiki/Slerp
	/*const number slerp_speed = 0.05;
	quat next_quat = glm::angleAxis(float(_angle), glm::vec3(0.0f, 0.0f, 1.0f));
	quat interpolated_quat = _rotation * glm::pow(glm::inverse(_rotation) * next_quat, slerp_speed);

	set_pos_rot_scale(next_position, interpolated_quat, pt_3d(1.0));*/

	//set_pos(next_position);


	number phi = atan2(_velocity.y, _velocity.x);
	pt_3d phi_axis(0.0, 0.0, 1.0);
	quat init_quat_1 = glm::angleAxis(phi, phi_axis);

	number theta = atan(2.0 * _parabola_coeffs[0] * (_target_dist - d) + _parabola_coeffs[1]);
	pt_3d theta_axis(_target.y - _pos_ini.y, _pos_ini.x - _target.x, 0.0);
	quat init_quat_2 = glm::angleAxis(theta, theta_axis / glm::length(theta_axis));

	set_pos_rot_scale(next_position, init_quat_2 * init_quat_1, pt_3d(1.0));
}
