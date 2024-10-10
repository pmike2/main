#ifndef GEOM_2D_H
#define GEOM_2D_H

#include <string>
#include <vector>
#include <utility>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "typedefs.h"
#include "bbox_2d.h"


// 1.0 / 3.0
const number THIRD= 1.0/ 3.0;
// utilisé pour comparaison number à 0.0
const number EPSILON= 1e-6;


class Polygon2D;

number cross2d(pt_type v1, pt_type v2);
bool cmp_points(pt_type & pt1, pt_type & pt2);
bool is_left(pt_type pt_ref, pt_type dir_ref, pt_type pt_test);
bool is_pt_inside_poly(pt_type & pt, Polygon2D * poly);
bool is_poly_inside_poly(Polygon2D * small_poly, Polygon2D * big_poly);
bool segment_intersects_segment(const pt_type & pt1_begin, const pt_type & pt1_end, const pt_type & pt2_begin, const pt_type & pt2_end, pt_type * result, bool exclude_seg1_extremities=false, bool exclude_seg2_extremities=false);
bool ray_intersects_segment(const pt_type & origin, const pt_type & direction, const pt_type & pt_begin, const pt_type & pt_end, pt_type * result);
bool ray_intersects_ray(const pt_type & origin1, const pt_type & direction1, const pt_type & origin2, const pt_type & direction2, pt_type * result);
bool segment_intersects_poly(const pt_type & pt_begin, const pt_type & pt_end, const Polygon2D * poly, pt_type * result);
bool segment_intersects_poly_multi(const pt_type & pt_begin, const pt_type & pt_end, const Polygon2D * poly, std::vector<pt_type> * result);
bool distance_segment_pt(pt_type & seg1, pt_type & seg2, pt_type & pt, number * dist, pt_type * proj);
number distance_poly_pt(Polygon2D * poly, pt_type & pt, pt_type * proj);
number distance_poly_segment(Polygon2D * poly, pt_type & seg1, pt_type & seg2, pt_type * proj);
void convex_hull_2d(std::vector<pt_type> & pts);
bool is_ccw(const pt_type & pt1, const pt_type & pt2, const pt_type & pt3);
bool is_ccw(const std::vector<pt_type> & pts);
std::pair<pt_type, number> circumcircle(pt_type & circle_pt1, pt_type & circle_pt2, pt_type & circle_pt3);
bool point_in_circumcircle(pt_type & circle_pt1, pt_type & circle_pt2, pt_type & circle_pt3, pt_type & pt);
void get_circle_center(pt_type & circle_pt1, pt_type & circle_pt2, pt_type & circle_pt3, pt_type & center, number * radius);
bool is_quad_convex(pt_type * pts);

class Polygon2D {
public:
    Polygon2D();
    Polygon2D(const Polygon2D & polygon);
    ~Polygon2D();
    void set_points(number * points, unsigned int n_points, bool convexhull=false);
    void set_points(std::vector<pt_type> pts, bool convexhull=false);
    void randomize(unsigned int n_points, number radius=1.0, pt_type center=pt_type(0.0), bool convexhull=false);
    void set_rectangle(pt_type origin, pt_type size);
    void update_attributes();
    pt_type farthest_pt_along_dir(pt_type direction);
    void print();


    std::vector<pt_type> _pts;
    std::vector<pt_type> _normals;
    number _area;
    pt_type _centroid;
    number _radius; // rayon cercle englobant
	AABB_2D * _aabb;
};


#endif
