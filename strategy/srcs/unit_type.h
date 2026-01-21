#ifndef UNIT_TYPE_H
#define UNIT_TYPE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "typedefs.h"
#include "obj_parser.h"

#include "const.h"


const number EPS_UNIT_TYPE_BUFFER_SIZE = 0.1;


struct UnitElevationCoeff {
	number _elevation_min;
	number _elevation_max;
	number _coeff;
};


struct UnitType {
	UnitType();
	UnitType(std::string json_path);
	~UnitType();
	number elevation_coeff(number delta_elevation);
	number buffer_size();
	friend std::ostream & operator << (std::ostream & os, UnitType & ut);
	
	
	UNIT_TYPE _type;
	number _max_velocity;
	bool _floats;
	bool _flies;
	std::map<TERRAIN_TYPE, number> _terrain_weights;
	std::vector<UnitElevationCoeff> _delta_elevation_coeffs;
	ObjData * _obj_data;
};


#endif
