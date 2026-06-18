#ifndef LAKE_H
#define LAKE_H

#include <vector>
#include <utility>

#include "typedefs.h"
#include "bbox_2d.h"

#include "element.h"


struct Lake : public Element {
	Lake();
	Lake(Elevation * elevation, pt_2d src);
	~Lake();
	void update_data();
	json get_json();


	std::vector<uint> _id_nodes;
	std::vector<std::tuple<uint, uint, uint> > _triangles;
	number _alti_lake;
	bool _valid;
	Polygon2D * _polygon;
};


#endif
