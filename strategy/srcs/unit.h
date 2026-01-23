#ifndef UNIT_H
#define UNIT_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <queue>

#include "typedefs.h"
#include "bbox_2d.h"
#include "bbox.h"

#include "unit_type.h"
#include "unit_path.h"
#include "elevation.h"


const uint N_MAX_UNITS_PER_GROUP = 1024;
const number UNIT_DIST_PATH_EPS = 0.5;


struct Instruction {
	pt_3d _destination;
	time_point _t;
};


struct Unit : public InstancePosRot {
	Unit();
	Unit(UnitType * type, pt_3d pos, Elevation * elevation);
	~Unit();
	//pt_3d pt2dto3d(pt_2d pt);
	void anim();
	bool checkpoint_checked();
	void set_status(UNIT_STATUS status);
	void update_alti_path();
	friend std::ostream & operator << (std::ostream & os, Unit & unit);
	
	
	uint _id;
	UnitType * _type;
	UNIT_STATUS _status;
	UnitPath * _path;
	pt_3d _velocity;
	std::queue<Instruction> _instructions;
	bool _paused;
	Elevation * _elevation;
};


struct Team {
	Team();
	Team(std::string name, Elevation * elevation);
	~Team();
	Unit * add_unit(UnitType * type, uint id, pt_2d pos);
	void remove_unit(Unit * unit);
	void remove_units_in_aabb(AABB_2D * aabb);
	void clear();
	friend std::ostream & operator << (std::ostream & os, Team & team);


	Elevation * _elevation;
	std::string _name;
	std::vector<Unit *> _units;
};

#endif
