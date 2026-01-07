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
	pt_2d lowest_pt();


	Elevation * _elevation;
	std::vector<uint> _id_nodes;
	std::vector<std::tuple<uint, uint, uint> > _triangles;
	float * _data;
	uint _n_pts;
	bool _valid;
};


#endif
