#ifndef PHYSICS_2D_H
#define PHYSICS_2D_H

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <cfloat> // FLT_MAX

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp>

#include "utile.h"
#include "gl_utils.h"


const float THIRD= 1.0f/ 3.0f;
const glm::vec2 GRAVITY(0.0f, -50.0f);
// si _penetration en dessous de ce seuil pas de positional_correction pour éviter jitter
const float PENETRATION_ALLOWANCE= 0.05f; // valeurs OK : 0.01 -> 0.1
const float PENETRATION_PERCENT2CORRECT= 0.4f; // valeurs OK : 0.2 -> 0.8


class RigidBody2D;


float cross2d(glm::vec2 v1, glm::vec2 v2);
bool cmp_points(glm::vec2 pt1, glm::vec2 pt2);
bool right_turn(glm::vec2 pt1, glm::vec2 pt2, glm::vec2 pt3);
bool segment_intersects_line(glm::vec2 seg1, glm::vec2 seg2, glm::vec2 line_origin, glm::vec2 line_direction, glm::vec2 * result);
bool is_left(glm::vec2 pt_ref, glm::vec2 dir_ref, glm::vec2 pt_test);
float distance_segment_pt(glm::vec2 seg1, glm::vec2 seg2, glm::vec2 pt);
void axis_least_penetration(RigidBody2D * body_a, RigidBody2D * body_b, unsigned int * idx_pt_max, float * penetration_max);
bool biased_cmp(float a, float b);
unsigned int incident_face_idx(RigidBody2D * body_reference, RigidBody2D * body_incident, unsigned int reference_face_idx);


class Polygon2D {
public:
    Polygon2D();
    ~Polygon2D();
    void set_points(float * points, unsigned int n_points);
    void randomize(unsigned int n_points);
    void update_attributes(float density=1.0f);
    void print();
    glm::vec2 farthest_pt_along_dir(glm::vec2 direction);


    std::vector<glm::vec2> _pts;
    std::vector<glm::vec2> _normals;
    float _area;
    glm::vec2 _centroid;
    float _mass;
    float _mass_inv;
    float _density;
    float _inertia_moment;
    float _inertia_moment_inv;
};


class RigidBody2D {
public:
    RigidBody2D();
    RigidBody2D(Polygon2D * polygon, glm::vec2 position, float orientation);
    ~RigidBody2D();
    void set_orientation(float orientation);
    void integrate_forces(float dt);
    void integrate_velocity(float dt);
    void apply_impulse(glm::vec2 impulse, glm::vec2 contact);
    void clear_forces();


    glm::vec2 _position;
    glm::vec2 _velocity;
    float _orientation;
    float _angular_velocity; // en 2D on a juste besoin d'un scalaire
    glm::vec2 _force;
    float _torque; // en 2D on a juste besoin d'un scalaire
    Polygon2D * _polygon;
    glm::mat2 _orientation_mat;
    float _static_friction;
    float _dynamic_friction;
    float _restitution; // elasticité du corps, capacité à rebondir

/*
cf tuto : quelques valeurs ex :
Rock       Density : 0.6  Restitution : 0.1
Wood       Density : 0.3  Restitution : 0.2
Metal      Density : 1.2  Restitution : 0.05
BouncyBall Density : 0.3  Restitution : 0.8
SuperBall  Density : 0.3  Restitution : 0.95
Pillow     Density : 0.1  Restitution : 0.2
Static     Density : 0.0  Restitution : 0.4
*/
};


class Collision2D {
public:
    Collision2D();
    Collision2D(RigidBody2D * body_a, RigidBody2D * body_b);
    ~Collision2D();
    void solve();
    glm::vec2 contact_relative_velocity(unsigned int idx_contact);
    void initialize(float dt);
    void apply_impulse();
    void positional_correction();


    RigidBody2D * _body_a;
    RigidBody2D * _body_b;
    //unsigned int _contact_count;
    std::vector<glm::vec2> _contacts;
    float _penetration;
    glm::vec2 _normal;
    float _mixed_static_friction;
    float _mixed_dynamic_friction;
    float _mixed_restitution;
};


class Physics2D {
public:
    Physics2D();
    Physics2D(float dt, unsigned int n_iterations_collision);
    ~Physics2D();
    void add_body(Polygon2D * polygon, glm::vec2 position, float orientation);
    void step();


    std::vector<RigidBody2D *> _bodies;
    std::vector<Collision2D *> _collisions;
    float _dt;
    unsigned int _n_iterations_collision;
};


class DebugPhysics2D {
public:
    DebugPhysics2D();
    DebugPhysics2D(Physics2D * physics_2d, GLuint prog_draw_2d, ScreenGL * screengl);
    ~DebugPhysics2D();
    void draw();
    void update();


    Physics2D * _physics_2d;

    ScreenGL * _screengl;
    GLuint _prog_draw;
	GLint _camera2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffer;
	float _camera2clip[16];
    unsigned int _n_pts;
};

#endif
