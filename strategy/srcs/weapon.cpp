#include "weapon.h"


Weapon::Weapon() {

}


Weapon::Weapon(pt_3d position, pt_3d target, number damage) :
	_position(position), _target(target), _damage(damage), _target_hit(false) 
{
	_velocity = 1.0;
}


Weapon::~Weapon() {

}


void Weapon::anim() {
	if (_target_hit) {
		return;
	}

	number d = glm::length(_target - _position);
	if (d < 1.0) {
		_target_hit = true;
		return;
	}

	pt_3d direction = (_target - _position) / d;
	_position += _velocity * direction;
}
