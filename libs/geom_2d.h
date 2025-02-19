#ifndef GEOM_2D_H
#define GEOM_2D_H

#include <string>
#include <vector>
#include <utility>
#include <tuple>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "typedefs.h"
#include "bbox_2d.h"
#include "triangulation.h"


// 1.0 / 3.0
const number THIRD= 1.0/ 3.0;
// utilisé pour comparaison number à 0.0
const number EPSILON= 1e-6;


class Polygon2D;


pt_type rot(pt_type v, number alpha);
number norm(pt_type v);
pt_type normalized(pt_type v);
number scal(pt_type u, pt_type v);
number cross(pt_type u, pt_type v);

void rotation_float2mat(float rot, mat & mat);
number cross2d(const pt_type & v1, const pt_type & v2);
bool cmp_points(const pt_type & pt1, const pt_type & pt2);
bool is_left(const pt_type & pt_ref, const pt_type & dir_ref, const pt_type & pt_test);
bool is_pt_inside_poly(const pt_type & pt, const Polygon2D * poly);
bool is_poly_inside_poly(const Polygon2D * small_poly, const Polygon2D * big_poly);
bool poly_intersects_poly(const Polygon2D * poly1, const Polygon2D * poly2, pt_type * axis, number * overlap, unsigned int * idx_pt, bool * is_pt_in_poly1);
bool bbox_intersects_bbox(const BBox_2D * bbox1, const BBox_2D * bbox2, pt_type * axis, number * overlap, unsigned int * idx_pt, bool * is_pt_in_poly1);
bool segment_intersects_segment(const pt_type & pt1_begin, const pt_type & pt1_end, const pt_type & pt2_begin, const pt_type & pt2_end, pt_type * result, bool exclude_seg1_extremities=false, bool exclude_seg2_extremities=false);
bool ray_intersects_segment(const pt_type & origin, const pt_type & direction, const pt_type & pt_begin, const pt_type & pt_end, pt_type * result);
bool ray_intersects_ray(const pt_type & origin1, const pt_type & direction1, const pt_type & origin2, const pt_type & direction2, pt_type * result);
bool segment_intersects_poly(const pt_type & pt_begin, const pt_type & pt_end, const Polygon2D * poly, pt_type * result);
bool segment_intersects_poly_multi(const pt_type & pt_begin, const pt_type & pt_end, const Polygon2D * poly, std::vector<pt_type> * result);
bool distance_segment_pt(const pt_type & seg1, const pt_type & seg2, const pt_type & pt, number * dist, pt_type * proj);
number distance_poly_pt(const Polygon2D * poly, const pt_type & pt, pt_type * proj);
number distance_poly_segment(const Polygon2D * poly, const pt_type & seg1, const pt_type & seg2, pt_type * proj);
void convex_hull_2d(std::vector<pt_type> & pts);
bool is_ccw(const pt_type & pt1, const pt_type & pt2, const pt_type & pt3);
bool is_ccw(const std::vector<pt_type> & pts);
std::pair<pt_type, number> circumcircle(const pt_type & circle_pt1, const pt_type & circle_pt2, const pt_type & circle_pt3);
bool point_in_circumcircle(const pt_type & circle_pt1, const pt_type & circle_pt2, const pt_type & circle_pt3, const pt_type & pt);
bool point_in_circle(const pt_type & center, number radius, const pt_type & pt);
void get_circle_center(const pt_type & circle_pt1, const pt_type & circle_pt2, const pt_type & circle_pt3, pt_type & center, number * radius);
bool is_quad_convex(const pt_type * pts);


class Polygon2D {
public:
	Polygon2D();
	Polygon2D(const Polygon2D & polygon);
	~Polygon2D();
	void set_points(const number * points, unsigned int n_points, bool convexhull=false);
	void set_points(const std::vector<pt_type> pts, bool convexhull=false);
	void randomize(unsigned int n_points, number radius=1.0, pt_type center=pt_type(0.0), bool convexhull=false);
	void set_rectangle(const pt_type origin, const pt_type size);
	void set_bbox(const BBox_2D & bbox);
	void translate(pt_type v);
	void rotate(pt_type center, number alpha);
	void update_attributes();
	void min_max_pt_along_dir(const pt_type direction, unsigned int * idx_pt_min, number * dist_min, unsigned int * idx_pt_max, number * dist_max) const;
	void triangulate();
	friend std::ostream & operator << (std::ostream & os, const Polygon2D & polygon);


	std::vector<pt_type> _pts;
	std::vector<pt_type> _normals;
	number _area;
	pt_type _centroid;
	number _radius; // rayon cercle englobant
	AABB_2D * _aabb;
	std::vector<std::vector<int> > _triangles_idx;
};


#endif
