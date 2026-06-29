#ifndef AMMO_H
#define AMMO_H

#include "typedefs.h"
#include "bbox.h"

#include "ammo_type.h"


struct Ammo : public InstancePosRot {
	Ammo();
	Ammo(AmmoType * type, pt_3d pos, pt_3d target);
	~Ammo();
	void anim();


	AmmoType * _type;
	pt_3d _target;
	bool _target_hit;
	number _damage;

	pt_3d _pos_ini;
	pt_3d _velocity;
	number _theta;
	number _target_dist;
	pt_3d _parabola_coeffs;
};


#endif
