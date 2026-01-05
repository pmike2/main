#ifndef LAKE_H
#define LAKE_H

#include <vector>
#include <utility>

#include "typedefs.h"
#include "bbox_2d.h"

#include "elevation.h"


struct Lake {
	Lake();
	Lake(Elevation * elevation, pt_2d src);
	~Lake();
	void update_data();

	Elevation * _elevation;
	std::vector<uint> _id_nodes;
	std::vector<std::tuple<pt_3d, pt_3d, pt_3d, pt_3d> > _triangles;
	float * _data;
	uint _n_pts;
	bool _valid;
};


#endif
