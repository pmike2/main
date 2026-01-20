#ifndef RIVER_H
#define RIVER_H

#include <vector>
#include <utility>

#include "typedefs.h"
#include "polygon_2d.h"

#include "element.h"


struct River : public Element {
	River();
	River(Elevation * elevation, pt_2d src);
	~River();
	void update_data();
	pt_3d lowest_pt();
	pt_3d highest_pt();


	std::vector<uint> _id_nodes;
	std::vector<std::tuple<uint, uint, uint> > _triangles;
	bool _valid;
	Polygon2D * _polygon;
};


#endif
