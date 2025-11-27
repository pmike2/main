#ifndef GEOM_H
#define GEOM_H

#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "typedefs.h"


const bool VERBOSE = false;


bool triangle_intersects_triangle(pt_type_3d v[3], pt_type_3d w[3]);


struct Face;


struct Pt {
	Pt();
	Pt(pt_type_3d coords);
	~Pt();
	friend std::ostream & operator << (std::ostream & os, const Pt & pt);


	pt_type_3d _coords;
	std::vector<Face *> _conflict;
};


struct Face {
	Face();
	Face(glm::uvec3 idx);
	~Face();
	void change_orientation();
	bool operator==(const Face& rhs) { return (rhs._idx[0] == _idx[0] && rhs._idx[1] == _idx[1] && rhs._idx[2] == _idx[2]); }
	friend std::ostream & operator << (std::ostream & os, const Face & face);


	glm::uvec3 _idx;
	pt_type_3d _normal;
	std::vector<Pt *> _conflict;
	bool _delete;
};


struct Horizon {
	Face * _face;
	Face * _opposite_face;
	uint _idx_edge;
};


class ConvexHull {
public:
	ConvexHull();
	~ConvexHull();
	void clear();
	bool is_conflict(Pt * pt, Face * face);
	void add_conflict(Pt * pt, Face * face);
	Pt * add_pt(pt_type_3d coords);
	Pt * add_pt(number x, number y, number z);
	Face * add_face(glm::uvec3 idx);
	//Pt * get_pt_not_in_face(Face * face);
	//void ccw(Face * face);
	Face * opposite_face(Face * face, uint idx_edge);
	void randomize(uint n_pts, number xmin, number xmax, number ymin, number ymax, number zmin, number zmax);
	void compute();
	friend std::ostream & operator << (std::ostream & os, const ConvexHull & hull);


	std::vector<Pt *> _pts;
	std::vector<Face * > _faces;
	//std::vector<std::pair<uint, glm::uvec3> > _conflict;
};

#endif
