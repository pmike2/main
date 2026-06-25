#ifndef AMMO_TYPE_H
#define AMMO_TYPE_H

#include <string>

#include "typedefs.h"
#include "obj_parser.h"

#include "explosion.h"


struct AmmoType {
	AmmoType();
	AmmoType(std::string json_path);
	~AmmoType();


	std::string _name;
	number _damage;
	number _velocity;
	number _max_distance;
	number _explosion_radius;
	ObjData * _obj_data;

	std::string _explosion_config_str;
	ExplosionConfig * _explosion_config;
};



#endif
