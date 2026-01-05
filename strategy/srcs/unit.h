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


struct Unit {
	Unit();
	Unit(UnitType * type, pt_3d pos, time_point t);
	~Unit();
	void anim(time_point t);
	bool checkpoint_checked();
	void goto_next_checkpoint(time_point t);
	void stop();
	friend std::ostream & operator << (std::ostream & os, Unit & unit);
	
	
	uint _id;
	UnitType * _type;
	bool _selected;
	AABB * _aabb;
	UNIT_STATUS _status;
	Path * _path;
	pt_3d _velocity;
	std::queue<Instruction> _instructions;
	time_point _last_anim_t;
};


#endif
