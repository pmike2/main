#ifndef UNIT_PATH_H
#define UNIT_PATH_H

#include <iostream>
#include <vector>
#include <map>

#include "json.hpp"

#include "typedefs.h"
#include "bbox_2d.h"

#include "unit_type.h"


using json = nlohmann::json;


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
	json get_json();
	friend std::ostream & operator << (std::ostream & os, UnitPath & p);


	uint _unit_id;
	UnitType * _unit_type;
	uint _start_id;
	uint _goal_id;
	pt_3d _start;
	pt_3d _goal;
	std::vector<uint> _nodes;
	uint _idx_path;
	bool _use_line_of_sight;
	UNIT_PATH_STATUS _status;
	
	std::vector<pt_3d> _pts;
	std::vector<PathInterval *> _intervals;
	
	std::vector<pt_3d> _pts_los;
	std::vector<PathInterval *> _intervals_los;
};



#endif
