#ifndef RIVER_H
#define RIVER_H

#include <vector>
#include <utility>

#include "typedefs.h"
#include "polygon_2d.h"

#include "elevation.h"


struct River {
	River();
	River(Elevation * elevation, pt_2d src);
	~River();
	void update_data();
	

	Elevation * _elevation;
	std::vector<uint> _id_nodes;
	std::vector<std::tuple<pt_3d, pt_3d, pt_3d, pt_3d> > _triangles;
	float * _data;
	uint _n_pts;
	Polygon2D * _polygon;
};


#endif
