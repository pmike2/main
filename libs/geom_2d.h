#ifndef GEOM_2D_H
#define GEOM_2D_H

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "bbox_2d.h"


// 1.0 / 3.0
const float THIRD= 1.0f/ 3.0f;
// utilisé pour comparaison float à 0.0f
const float EPSILON= 1e-8f;


class Polygon2D;

float cross2d(glm::vec2 v1, glm::vec2 v2);
bool cmp_points(glm::vec2 & pt1, glm::vec2 & pt2);
bool is_left(glm::vec2 pt_ref, glm::vec2 dir_ref, glm::vec2 pt_test);
bool is_pt_inside_poly(glm::vec2 & pt, Polygon2D * poly);
bool segment_intersects_segment(glm::vec2 & pt1_begin, glm::vec2 & pt1_end, glm::vec2 & pt2_begin, glm::vec2 & pt2_end, glm::vec2 * result, bool exclude_seg1_extremities=false, bool exclude_seg2_extremities=false);
bool ray_intersects_segment(glm::vec2 & origin, glm::vec2 & direction, glm::vec2 & pt_begin, glm::vec2 & pt_end, glm::vec2 * result);
bool segment_intersects_poly(glm::vec2 & pt_begin, glm::vec2 & pt_end, Polygon2D * poly, glm::vec2 * result);
bool distance_segment_pt(glm::vec2 & seg1, glm::vec2 & seg2, glm::vec2 & pt, float * dist, glm::vec2 * proj);
float distance_poly_pt(Polygon2D * poly, glm::vec2 & pt, glm::vec2 * proj);
float distance_poly_segment(Polygon2D * poly, glm::vec2 & seg1, glm::vec2 & seg2, glm::vec2 * proj);
void convex_hull_2d(std::vector<glm::vec2> & pts);
bool is_ccw(glm::vec2 & pt1, glm::vec2 & pt2, glm::vec2 & pt3);
bool point_in_circumcircle(glm::vec2 & circle_pt1, glm::vec2 & circle_pt2, glm::vec2 & circle_pt3, glm::vec2 & pt);
void get_circle_center(glm::vec2 & circle_pt1, glm::vec2 & circle_pt2, glm::vec2 & circle_pt3, glm::vec2 & center, float * radius);
bool is_quad_convex(glm::vec2 * pts);

class Polygon2D {
public:
    Polygon2D();
    Polygon2D(const Polygon2D & polygon);
    ~Polygon2D();
    void set_points(float * points, unsigned int n_points, bool convexhull=false);
    void set_points(std::vector<glm::vec2> pts, bool convexhull=false);
    void randomize(unsigned int n_points, float radius=1.0f, glm::vec2 center=glm::vec2(0.0f), bool convexhull=false);
    void set_rectangle(glm::vec2 origin, glm::vec2 size);
    void update_attributes();
    glm::vec2 farthest_pt_along_dir(glm::vec2 direction);
    void print();


    std::vector<glm::vec2> _pts;
    std::vector<glm::vec2> _normals;
    float _area;
    glm::vec2 _centroid;
    float _radius; // rayon cercle englobant
	AABB_2D * _aabb;
};


#endif
