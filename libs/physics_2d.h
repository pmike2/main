#ifndef PHYSICS_2D_H
#define PHYSICS_2D_H

#include <string>
#include <vector>
#include <map>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "gl_utils.h"

/*

Sources :

https://gamedevelopment.tutsplus.com/series/how-to-create-a-custom-physics-engine--gamedev-12715
https://code.google.com/archive/p/box2d/downloads?page=1
http://www.chrishecker.com/images/e/e7/Gdmphys3.pdf
http://www.cs.cmu.edu/~baraff/sigcourse/index.html
https://gafferongames.com/post/fix_your_timestep/
https://gafferongames.com/post/integration_basics/

*/


// constantes ------------------------------------------------------------------------------

// 1.0 / 3.0
const float THIRD= 1.0f/ 3.0f;

// vecteur gravité
const glm::vec2 GRAVITY(0.0f, -50.0f);

// nombre d'itérations max lors de la résolution des collisions
const unsigned int N_ITER_MAX= 100;
const float IMPULSE_TRESH= 0.0001f;

// cf biased_cmp()
// valeurs initiales tuto : 0.95 / 0.01
const float BIAS_CMP_RELATIVE= 0.95f;
const float BIAS_CMP_ABSOLUTE= 0.01f;

// si _penetration en dessous de ce seuil pas de positional_correction pour éviter jitter (valeurs tuto = 0.05, 0.4)
const float PENETRATION_ALLOWANCE= 0.02f; // valeurs OK : 0.01 -> 0.1
const float PENETRATION_PERCENT2CORRECT= 0.5f; // valeurs OK : 0.2 -> 0.8

// en dessous de ce y les objets sont supprimés
const float BODY_MIN_Y= -100.0f;

// utilisé pour comparaison float à 0.0f
const float EPSILON= 1e-5f;

// facteur multiplicatif lors d'une nouvelle force ajoutée par user
const float NEW_EXTERNAL_FORCE_FACTOR= 700.0f;
const float NEW_EXPLOSION_FACTOR= 700.0f;


// fonctions utiles -------------------------------------------------------------------------
class Polygon2D;
class RigidBody2D;

void rotation_float2mat(float rot, glm::mat2 & mat);
float cross2d(glm::vec2 v1, glm::vec2 v2);
bool cmp_points(glm::vec2 pt1, glm::vec2 pt2);
bool is_left(glm::vec2 pt_ref, glm::vec2 dir_ref, glm::vec2 pt_test);
bool is_pt_inside_body(glm::vec2 pt, RigidBody2D * body);
bool segment_intersects_segment(glm::vec2 pt1_begin, glm::vec2 pt1_end, glm::vec2 pt2_begin, glm::vec2 pt2_end, glm::vec2 * result);
bool segment_intersects_line(glm::vec2 seg1, glm::vec2 seg2, glm::vec2 line_origin, glm::vec2 line_direction, glm::vec2 * result);
bool segment_intersects_body(glm::vec2 pt_begin, glm::vec2 pt_end, RigidBody2D * body, glm::vec2 * result);
bool distance_segment_pt(glm::vec2 seg1, glm::vec2 seg2, glm::vec2 pt, float * dist, glm::vec2 * proj);
float distance_body_pt(RigidBody2D * body, glm::vec2 pt, glm::vec2 * proj);
void convex_hull_2d(std::vector<glm::vec2> & pts);
void axis_least_penetration(RigidBody2D * body_a, RigidBody2D * body_b, unsigned int * idx_pt_max, float * penetration_max);
bool biased_cmp(float a, float b);


// classes ----------------------------------------------------------------------------------
class Polygon2D {
public:
    Polygon2D();
    ~Polygon2D();
    void set_points(float * points, unsigned int n_points);
    void randomize(unsigned int n_points, float radius=1.0f);
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


class Material {
public:
	Material();
	Material(float density, float static_friction, float dynamic_friction, float restitution);
	~Material();
	void print();


	float _density;
    float _static_friction;
    float _dynamic_friction;
    float _restitution; // elasticité du corps, capacité à rebondir
};


// utile a l'affichage (fixed time step cf https://gafferongames.com/post/fix_your_timestep/)
class RigidBody2DState {
public:
    RigidBody2DState();
    RigidBody2DState(glm::vec2 position, float orientation);
    ~RigidBody2DState();

    glm::vec2 _position;
    float _orientation;
};


class RigidBody2D {
public:
    RigidBody2D();
    RigidBody2D(Polygon2D * polygon, Material * material, glm::vec2 position, float orientation);
    ~RigidBody2D();
    void set_orientation(float orientation);
    void integrate_forces(float dt);
    void integrate_velocity(float dt);
    void apply_impulse(glm::vec2 impulse, glm::vec2 contact);
    void clear_forces();
    void update_previous_state();
    void print();
    void save(std::string ch_file);


    unsigned short _id;
    static unsigned short CurrentID;

    Polygon2D * _polygon;
	Material * _material;

    float _mass;
    float _mass_inv;
    float _inertia_moment;
    float _inertia_moment_inv;
    bool _is_static;

    glm::vec2 _position;
    glm::vec2 _velocity;
    glm::vec2 _force;

    float _orientation;
    glm::mat2 _orientation_mat;
    float _angular_velocity; // en 2D on a juste besoin d'un scalaire
    float _torque; // en 2D on a juste besoin d'un scalaire

    // utile a l'affichage (fixed time step cf https://gafferongames.com/post/fix_your_timestep/)
    RigidBody2DState * _previous_state;
};


class Contact2D {
public:
    Contact2D();
    Contact2D(glm::vec2 position, RigidBody2D * body_reference, RigidBody2D * body_incident, glm::vec2 normal);
    ~Contact2D();
    glm::vec2 contact_relative_velocity();
    void update_normal(float dt);
	void update_tangent(float dt);
    void print();


    glm::vec2 _position;
    RigidBody2D * _body_reference;
    RigidBody2D * _body_incident;
    glm::vec2 _normal;
    glm::vec2 _r_ref;
    glm::vec2 _r_incid;
    glm::vec2 _normal_impulse;
    glm::vec2 _normal_impulse_cumul;
    glm::vec2 _tangent_impulse;
    bool _is_valid;
    bool _is_tangent_valid;
    float _mass_inv_sum;
	float _j, _jt;
    bool _is_resting;
};


class Collision2D {
public:
    Collision2D();
    Collision2D(RigidBody2D * body_a, RigidBody2D * body_b, bool verbose=false);
    ~Collision2D();
	unsigned int incident_face_idx(unsigned int reference_face_idx);
    void apply_impulse(float dt);
    void positional_correction();
    void print();


    RigidBody2D * _body_reference;
    RigidBody2D * _body_incident;
    std::vector<Contact2D *> _contacts;
    float _penetration;
    glm::vec2 _normal;
	bool _verbose;

    unsigned long long _hash;
    bool _warm_starting;
	bool _is_valid;
    bool _is_resting;
};


class LastImpulse {
public:
    LastImpulse();
    ~LastImpulse();


    std::vector<glm::vec2> _normal_impulses;
    std::vector<glm::vec2> _tangent_impulses;
    bool _is_2delete;
};


class Physics2D {
public:
    Physics2D();
    Physics2D(float dt);
    ~Physics2D();
	// on peut générer plusieurs body avec le meme polygon et avec des matériaux différents par ex
	void add_polygon(Polygon2D * polygon);
    void add_body(unsigned int idx_polygon, unsigned int idx_material, glm::vec2 position, float orientation);
    void step(bool verbose=false);
    void new_external_force(glm::vec2 pt_begin, glm::vec2 pt_end);
    void new_explosion(glm::vec2 center, float radius);
    void load_body(std::string ch_file, unsigned int idx_material);


	std::vector<Polygon2D *> _polygons;
	std::vector<Material *> _materials;
    std::vector<RigidBody2D *> _bodies;
    std::vector<Collision2D *> _collisions;
    float _dt;
	bool _paused;
    std::map<unsigned long long, LastImpulse *> _collisions_hash_table;
    bool _warm_starting_enabled;
};


// partie affichage -----------------------------------------------------------------------------------
class DebugPhysics2D {
public:
    DebugPhysics2D();
    DebugPhysics2D(Physics2D * physics_2d, GLuint prog_draw_2d, ScreenGL * screengl);
    ~DebugPhysics2D();
    void draw();
    void update(float alpha);


    Physics2D * _physics_2d;

    ScreenGL * _screengl;
    GLuint _prog_draw;
	GLint _camera2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffers[6];
	float _camera2clip[16];
    unsigned int _n_pts, _n_bodies, _n_collisions, _n_contacts;
	bool _visible_pt, _visible_normal, _visible_center, _visible_vel_force, _visible_collision, _visible_contact;
};

#endif
