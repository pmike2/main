#ifndef GEOM_2D_H
#define GEOM_2D_H

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>


// 1.0 / 3.0
const float THIRD= 1.0f/ 3.0f;
// utilisé pour comparaison float à 0.0f
const float EPSILON= 1e-5f;


class Polygon2D;

float cross2d(glm::vec2 v1, glm::vec2 v2);
bool cmp_points(glm::vec2 pt1, glm::vec2 pt2);
bool is_left(glm::vec2 pt_ref, glm::vec2 dir_ref, glm::vec2 pt_test);
bool is_pt_inside_poly(glm::vec2 pt, Polygon2D * poly);
bool segment_intersects_segment(glm::vec2 pt1_begin, glm::vec2 pt1_end, glm::vec2 pt2_begin, glm::vec2 pt2_end, glm::vec2 * result);
bool segment_intersects_poly(glm::vec2 pt_begin, glm::vec2 pt_end, Polygon2D * poly, glm::vec2 * result);
bool distance_segment_pt(glm::vec2 seg1, glm::vec2 seg2, glm::vec2 pt, float * dist, glm::vec2 * proj);
float distance_poly_pt(Polygon2D * poly, glm::vec2 pt, glm::vec2 * proj);
float distance_poly_segment(Polygon2D * poly, glm::vec2 seg1, glm::vec2 seg2, glm::vec2 * proj);
void convex_hull_2d(std::vector<glm::vec2> & pts);


class Polygon2D {
public:
    Polygon2D();
    ~Polygon2D();
    void set_points(float * points, unsigned int n_points);
    void randomize(unsigned int n_points, float radius=1.0f, glm::vec2 center=glm::vec2(0.0f));
    void set_rectangle(float width, float height);
    void update_attributes();
    glm::vec2 farthest_pt_along_dir(glm::vec2 direction);
    void print();


    std::vector<glm::vec2> _pts;
    std::vector<glm::vec2> _normals;
    float _area;
    glm::vec2 _centroid;
    float _radius; // rayon cercle englobant
};


#endif