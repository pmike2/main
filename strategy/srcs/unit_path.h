#ifndef UNIT_PATH_H
#define UNIT_PATH_H

#include <iostream>
#include <vector>

#include "typedefs.h"
#include "bbox_2d.h"


struct UnitPath {
	UnitPath();
	~UnitPath();
	void clear();
	bool empty();
	bool is_last_checkpoint();
	number get_weight();
	pt_3d get_pt();
	void copy_path(UnitPath * path);
	friend std::ostream & operator << (std::ostream & os, UnitPath & p);


	pt_3d _start;
	pt_3d _goal;
	std::vector<uint> _nodes;
	std::vector<pt_3d> _pts;
	std::vector<number> _weights;
	std::vector<pt_3d> _pts_los;
	std::vector<number> _weights_los;
	std::vector<BBox_2D *> _bboxs;
	uint _idx_path;
	bool _use_line_of_sight;
};



#endif
