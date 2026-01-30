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
	pt_3d _velocity;
	number _angle;
	bool _target_hit;
};


#endif
