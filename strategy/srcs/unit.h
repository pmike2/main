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
#include "elevation.h"


const uint N_MAX_UNITS_PER_GROUP = 1024;


struct Path {
	Path();
	~Path();
	void clear();
	bool empty();
	friend std::ostream & operator << (std::ostream & os, Path & p);


	pt_2d _start;
	pt_2d _goal;
	std::vector<pt_3d> _pts;
	std::vector<uint> _nodes;
	std::vector<number> _weights;
	std::vector<BBox_2D *> _bboxs;
	uint _idx_path;
};


struct Instruction {
	pt_2d _destination;
	time_point _t;
};


struct Unit : public InstancePosRot {
	Unit();
	Unit(UnitType * type, pt_3d pos);
	~Unit();
	void anim(Elevation * elevation);
	bool checkpoint_checked();
	//void goto_next_checkpoint(time_point t);
	//void stop();
	void set_status(UNIT_STATUS status);
	friend std::ostream & operator << (std::ostream & os, Unit & unit);
	
	
	uint _id;
	UnitType * _type;
	//bool _selected;
	//AABB * _aabb;
	UNIT_STATUS _status;
	Path * _path;
	pt_3d _velocity;
	std::queue<Instruction> _instructions;
	//time_point _last_anim_t;
};


struct UnitGroup {
	UnitGroup();
	~UnitGroup();
	void add_unit(Unit * unit);
	void update_unit(Unit * unit);


	std::vector<Unit *> _units;
	float * _matrices;
};

#endif
