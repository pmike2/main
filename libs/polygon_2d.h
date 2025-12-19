#ifndef POLYGON_2D
#define POLYGON_2D

#include <iostream>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "typedefs.h"
#include "bbox_2d.h"
#include "triangulation.h"


class Polygon2D {
public:
	Polygon2D();
	Polygon2D(const Polygon2D & polygon);
	Polygon2D(const std::vector<pt_2d> pts, bool convexhull=false);
	~Polygon2D();
	void clear();
	void set_points(const number * points, unsigned int n_points, bool convexhull=false);
	void set_points(const std::vector<pt_2d> pts, bool convexhull=false);
	void centroid2zero();
	void randomize(unsigned int n_points, number radius=1.0, pt_2d center=pt_2d(0.0), bool convexhull=false);
	void set_rectangle(const pt_2d origin, const pt_2d size);
	void set_bbox(const BBox_2D & bbox);
	void translate(pt_2d v);
	void rotate(pt_2d center, number alpha);
	void scale(pt_2d scale);
	void min_max_pt_along_dir(const pt_2d direction, unsigned int * idx_pt_min, number * dist_min, unsigned int * idx_pt_max, number * dist_max) const;
	void triangulate();
	Polygon2D * buffered(number buffer_size); // attention ne fonctionne que pour des polys convexes avec buffer_size > 0

	void update_area();
	void update_centroid();
	void update_normals();
	void update_radius();
	void update_aabb();
	void update_inertia();
	void update_all();

	friend std::ostream & operator << (std::ostream & os, const Polygon2D & polygon);


	std::vector<pt_2d> _pts;
	std::vector<pt_2d> _normals;
	number _area;
	pt_2d _centroid;
	number _radius; // rayon cercle englobant
	AABB_2D * _aabb;
	std::vector<std::vector<int> > _triangles_idx;
	number _inertia; // https://physics.stackexchange.com/questions/493736/moment-of-inertia-for-an-arbitrary-polygon
};

#endif
