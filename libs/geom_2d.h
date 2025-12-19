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
#include "polygon_2d.h"


// 1.0 / 3.0
const number THIRD= 1.0/ 3.0;
// utilisé pour comparaison number à 0.0
const number EPSILON= 1e-6;


class Polygon2D;


// fonctions basiques
pt_2d rot(pt_2d v, number alpha);
number norm(pt_2d v);
number norm2(pt_2d v);
pt_2d normalized(pt_2d v);
number scal(pt_2d u, pt_2d v);
number determinant(pt_2d u, pt_2d v);
pt_2d proj(pt_2d v2proj, pt_2d v_proj_on);
number angle(pt_2d u, pt_2d v);
void rotation_float2mat(float rot, mat_2d & mat);
number cross2d(const pt_2d & v1, const pt_2d & v2);
bool is_left(const pt_2d & pt_ref, const pt_2d & dir_ref, const pt_2d & pt_test);
bool is_ccw(const pt_2d & pt1, const pt_2d & pt2, const pt_2d & pt3);
bool is_ccw(const std::vector<pt_2d> & pts);

// tests AABB
bool point_in_aabb(const pt_2d & pt, const AABB_2D * aabb);
bool aabb_intersects_aabb(const AABB_2D * aabb_1, const AABB_2D * aabb_2);
bool aabb_contains_aabb(const AABB_2D * big_aabb, const AABB_2D * small_aabb);
bool ray_intersects_aabb(const pt_2d & ray_origin, const pt_2d & ray_dir, const AABB_2D * aabb, pt_2d & contact_pt, pt_2d & contact_normal, number & t_hit_near);

// tests BBOX
bool bbox_intersects_bbox(const BBox_2D * bbox1, const BBox_2D * bbox2, pt_2d * axis, number * overlap, unsigned int * idx_pt, bool * is_pt_in_poly1);
bool pt_in_bbox(const pt_2d & pt, const BBox_2D * bbox);
std::pair<BBOX_SIDE, BBOX_CORNER> bbox_side_corner(const BBox_2D * bbox, const pt_2d & pt);

// tests segments / droites
bool segment_intersects_segment(const pt_2d & pt1_begin, const pt_2d & pt1_end, const pt_2d & pt2_begin, const pt_2d & pt2_end, pt_2d * result, bool exclude_seg1_extremities=false, bool exclude_seg2_extremities=false);
bool ray_intersects_segment(const pt_2d & origin, const pt_2d & direction, const pt_2d & pt_begin, const pt_2d & pt_end, pt_2d * result);
bool ray_intersects_ray(const pt_2d & origin1, const pt_2d & direction1, const pt_2d & origin2, const pt_2d & direction2, pt_2d * result);
bool distance_segment_pt(const pt_2d & seg1, const pt_2d & seg2, const pt_2d & pt, number * dist, pt_2d * proj);

// tests Polygon
bool is_pt_inside_poly(const pt_2d & pt, const Polygon2D * poly);
bool is_poly_inside_poly(const Polygon2D * small_poly, const Polygon2D * big_poly);
bool poly_intersects_poly(const Polygon2D * poly1, const Polygon2D * poly2, pt_2d * axis, number * overlap, unsigned int * idx_pt, bool * is_pt_in_poly1);
bool segment_intersects_poly(const pt_2d & pt_begin, const pt_2d & pt_end, const Polygon2D * poly, pt_2d * result);
bool segment_intersects_poly_multi(const pt_2d & pt_begin, const pt_2d & pt_end, const Polygon2D * poly, std::vector<pt_2d> * result);
number distance_poly_pt(const Polygon2D * poly, const pt_2d & pt, pt_2d * proj);
number distance_poly_segment(const Polygon2D * poly, const pt_2d & seg1, const pt_2d & seg2, pt_2d * proj);

// tests cercle
std::pair<pt_2d, number> circumcircle(const pt_2d & circle_pt1, const pt_2d & circle_pt2, const pt_2d & circle_pt3);
bool point_in_circumcircle(const pt_2d & circle_pt1, const pt_2d & circle_pt2, const pt_2d & circle_pt3, const pt_2d & pt);
bool point_in_circle(const pt_2d & center, number radius, const pt_2d & pt);
void get_circle_center(const pt_2d & circle_pt1, const pt_2d & circle_pt2, const pt_2d & circle_pt3, pt_2d & center, number * radius);
bool is_quad_convex(const pt_2d * pts);

// enveloppe convexe
void convex_hull_2d(std::vector<pt_2d> & pts);


#endif
