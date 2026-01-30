#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ammo.h"


Ammo::Ammo() {

}


Ammo::Ammo(AmmoType * type, pt_3d pos, pt_3d target) :
	InstancePosRot(pos, quat(1.0, 0.0, 0.0, 0.0), pt_3d(1.0), type->_obj_data->_aabb),
	_type(type), _target(target), _target_hit(false), _angle(0.0)
{
	number d = glm::length(_target - _position);
	if (d < 1.0) {
		_target_hit = true;
		return;
	}

	_velocity = _type->_velocity * (_target - _position) / d;
	_angle = atan2(_velocity.y, _velocity.x);
	quat init_quat = glm::angleAxis(float(_angle), glm::vec3(0.0f, 0.0f, 1.0f));
	set_pos_rot_scale(_position, init_quat, pt_3d(1.0));
}


Ammo::~Ammo() {

}


void Ammo::anim() {
	if (_target_hit) {
		return;
	}

	number d = glm::length(_target - _position);
	if (d < 1.0) {
		_target_hit = true;
		return;
	}

	_velocity = _type->_velocity * (_target - _position) / d;
	pt_3d next_position = _position + _velocity;

	number next_angle = atan2(_velocity.y, _velocity.x);
	// pour ne pas faire des 3/4 de tour quand les 2 angles sont de part et d'autre de l'axe x
	if (next_angle - _angle > M_PI) {
		next_angle -= 2.0 * M_PI;
	}
	_angle = next_angle;
	
	// https://en.wikipedia.org/wiki/Slerp
	const number slerp_speed = 0.05;
	quat next_quat = glm::angleAxis(float(_angle), glm::vec3(0.0f, 0.0f, 1.0f));
	quat interpolated_quat = _rotation * glm::pow(glm::inverse(_rotation) * next_quat, slerp_speed);

	set_pos_rot_scale(next_position, interpolated_quat, pt_3d(1.0));
}
