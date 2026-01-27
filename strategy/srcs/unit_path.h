#ifndef UNIT_PATH_H
#define UNIT_PATH_H

#include <iostream>
#include <vector>
#include <map>

#include "typedefs.h"
#include "bbox_2d.h"

#include "unit_type.h"


const uint N_ACTIVE_INTERVALS = 3;


struct PathInterval {
	PathInterval();
	PathInterval(number weight);
	PathInterval(const PathInterval & interval);
	~PathInterval();
	void clear();


	number _weight;
	BBox_2D * _bbox;
	std::map<UnitType *, std::vector<uint_pair> > _edges;
	bool _active;
};


struct UnitPath {
	UnitPath();
	~UnitPath();
	void clear();
	bool empty();
	bool is_last_checkpoint();
	PathInterval * get_current_interval();
	PathInterval * get_last_active_interval();
	pt_3d get_current_pt();
	std::vector<PathInterval *> get_intervals();
	void copy_path(UnitPath * path);
	void next_checkpoint();
	void update_active_intervals();
	friend std::ostream & operator << (std::ostream & os, UnitPath & p);


	pt_3d _start;
	pt_3d _goal;
	std::vector<uint> _nodes;
	uint _idx_path;
	bool _use_line_of_sight;
	
	std::vector<pt_3d> _pts;
	//std::vector<number> _weights;
	//std::vector<BBox_2D *> _bboxs;
	std::vector<PathInterval *> _intervals;
	
	std::vector<pt_3d> _pts_los;
	//std::vector<number> _weights_los;
	//std::vector<BBox_2D *> _bboxs_los;
	std::vector<PathInterval *> _intervals_los;
};



#endif
