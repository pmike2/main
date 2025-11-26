#ifndef GEOM_H
#define GEOM_H

#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "typedefs.h"


bool triangle_intersects_triangle(pt_type_3d v[3], pt_type_3d w[3]);

struct Face;

struct Pt {
	pt_type_3d _coords;
	std::vector<Face *> _conflict;
};

struct Face {
	glm::uvec3 _idx;
	pt_type_3d _normal;
	std::vector<Pt *> _conflict;
	bool operator==(const Face& rhs) { return (rhs._idx[0] == _idx[0] && rhs._idx[1] == _idx[1] && rhs._idx[2] == _idx[2]); }
	friend std::ostream & operator << (std::ostream & os, const Face & face);
};

class ConvexHull {
public:
	ConvexHull();
	~ConvexHull();
	bool is_conflict(Pt * pt, Face * face);
	void add_face(glm::uvec3 idx, bool is_face_init=false);
	Pt * get_pt_not_in_face(Face * face);
	void ccw(Face * face);
	Face * opposite_face(Face * face, uint idx_edge);
	void randomize(uint n_pts, number xmin, number xmax, number ymin, number ymax, number zmin, number zmax);
	void compute();


	std::vector<Pt *> _pts;
	std::vector<Face * > _faces;
	//std::vector<std::pair<uint, glm::uvec3> > _conflict;
};

#endif
