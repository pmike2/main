#ifndef GEOM_H
#define GEOM_H

#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "typedefs.h"
#include "dcel.h"


bool triangle_intersects_triangle(pt_type_3d v[3], pt_type_3d w[3]);


class ConvexHull {
public:
	ConvexHull();
	~ConvexHull();
	void randomize(uint n_pts, number xmin, number xmax, number ymin, number ymax, number zmin, number zmax);
	void compute();


	std::vector<pt_type_3d> _pts;
	DCEL * _dcel;
	std::vector<std::pair<DCEL_Vertex *, DCEL_Face *> > _conflict;
};

#endif
