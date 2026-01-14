#ifndef UNIT_TYPE
#define UNIT_TYPE

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "typedefs.h"
#include "obj_parser.h"

#include "const.h"


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
	
	
	std::string _name;
	pt_3d _size;
	number _max_velocity;
	std::map<TERRAIN_TYPE, number> _terrain_weights;
	std::vector<UnitElevationCoeff> _delta_elevation_coeffs;
	ObjData * _obj_data;
};


#endif
