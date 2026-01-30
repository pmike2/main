#ifndef AMMO_TYPE_H
#define AMMO_TYPE_H

#include <string>

#include "typedefs.h"
#include "obj_parser.h"


struct AmmoType {
	AmmoType();
	AmmoType(std::string json_path);
	~AmmoType();


	std::string _name;
	number _damage;
	number _velocity;
	number _max_distance;
	ObjData * _obj_data;
};



#endif
