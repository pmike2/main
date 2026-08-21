#ifndef BARRIER_TYPE_H
#define BARRIER_TYPE_H

#include <string>

#include "typedefs.h"
#include "obj_parser.h"


struct BarrierType {
	BarrierType();
	BarrierType(fs json_path);
	~BarrierType();


	std::string _name;
	number _life_init;
	ObjData * _obj_data;
};


#endif
