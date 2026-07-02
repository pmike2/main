#ifndef UNIT_TYPE_H
#define UNIT_TYPE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "typedefs.h"
#include "obj_parser.h"
#include "path_find.h"

#include "const.h"
#include "ammo_type.h"



struct UnitType : public GridMovingObjectType {
	UnitType();
	UnitType(std::string json_path);
	~UnitType();
	pt_2d get_max_square_size();
	//pt_2d get_size();
	//number buffer_size();
	friend std::ostream & operator << (std::ostream & os, UnitType & ut);
	
	
	number _max_velocity;
	number _life_init;
	number _regen;
	number _vision_distance;
	number _vision_angle;
	number _slerp_speed;
	uint _creation_duration;
	bool _floats;
	bool _flies;
	ObjData * _obj_data;
	AmmoType * _ammo_type;
	std::string _ammo_type_str;
};


#endif
