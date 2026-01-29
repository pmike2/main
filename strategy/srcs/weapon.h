#ifndef WEAPON_H
#define WEAPON_H

#include "typedefs.h"


struct Weapon {
	Weapon();
	Weapon(pt_3d position, pt_3d target, number damage);
	~Weapon();
	void anim();


	pt_3d _position;
	pt_3d _target;
	number _velocity;
	//void * _data;
	bool _target_hit;
	number _damage;
};


#endif
