#ifndef PHYSICS_2D_H
#define PHYSICS_2D_H

#include <string>
#include <vector>
#include <map>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "gl_utils.h"
#include "geom_2d.h"
#include "typedefs.h"

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

// vecteur gravité
const pt_2d GRAVITY(0.0f, -50.0f);

// nombre d'itérations max lors de la résolution des collisions
const uint N_ITER_MAX= 100;
const number IMPULSE_TRESH= 0.0001f;

// cf biased_cmp()
// valeurs initiales tuto : 0.95 / 0.01
const number BIAS_CMP_RELATIVE= 0.95f;
const number BIAS_CMP_ABSOLUTE= 0.01f;

// si _penetration en dessous de ce seuil pas de positional_correction pour éviter jitter (valeurs tuto = 0.05, 0.4)
const number PENETRATION_ALLOWANCE= 0.02f; // valeurs OK : 0.01 -> 0.1
const number PENETRATION_PERCENT2CORRECT= 0.5f; // valeurs OK : 0.2 -> 0.8

// en dessous de ce y les objets sont supprimés
const number BODY_MIN_Y= -100.0f;

// facteur multiplicatif lors d'une nouvelle force ajoutée par user
const number NEW_EXTERNAL_FORCE_FACTOR= 700.0f;
const number NEW_EXPLOSION_FACTOR= 700.0f;


// fonctions utiles -------------------------------------------------------------------------
class RigidBody2D;


bool is_pt_inside_body(pt_2d pt, RigidBody2D * body);
bool segment_intersects_body(pt_2d pt_begin, pt_2d pt_end, RigidBody2D * body, pt_2d * result);
number distance_body_pt(RigidBody2D * body, pt_2d pt, pt_2d * proj);
void axis_least_penetration(RigidBody2D * body_a, RigidBody2D * body_b, uint * idx_pt_max, number * penetration_max);
bool biased_cmp(number a, number b);


// classes ----------------------------------------------------------------------------------
class Material {
public:
	Material();
	Material(number density, number static_friction, number dynamic_friction, number restitution);
	~Material();
	void print();


	number _density;
    number _static_friction;
    number _dynamic_friction;
    number _restitution; // elasticité du corps, capacité à rebondir
};


// utile a l'affichage (fixed time step cf https://gafferongames.com/post/fix_your_timestep/)
class RigidBody2DState {
public:
    RigidBody2DState();
    RigidBody2DState(pt_2d position, number orientation);
    ~RigidBody2DState();

    pt_2d _position;
    number _orientation;
};


class RigidBody2D {
public:
    RigidBody2D();
    RigidBody2D(Polygon2D * polygon, Material * material, pt_2d position, number orientation);
    ~RigidBody2D();
    void set_orientation(number orientation);
    void integrate_forces(number dt);
    void integrate_velocity(number dt);
    void apply_impulse(pt_2d impulse, pt_2d contact);
    void clear_forces();
    void update_previous_state();
    void print();
    void save(std::string ch_file);


    unsigned short _id;
    static unsigned short CurrentID;

    Polygon2D * _polygon;
	Material * _material;

    number _mass;
    number _mass_inv;
    number _inertia_moment;
    number _inertia_moment_inv;
    bool _is_static;

    pt_2d _position;
    pt_2d _velocity;
    pt_2d _force;

    number _orientation;
    mat_2d _orientation_mat;
    number _angular_velocity; // en 2D on a juste besoin d'un scalaire
    number _torque; // en 2D on a juste besoin d'un scalaire

    // utile a l'affichage (fixed time step cf https://gafferongames.com/post/fix_your_timestep/)
    RigidBody2DState * _previous_state;
};


class Contact2D {
public:
    Contact2D();
    Contact2D(pt_2d position, RigidBody2D * body_reference, RigidBody2D * body_incident, pt_2d normal);
    ~Contact2D();
    pt_2d contact_relative_velocity();
    void update_normal(number dt);
	void update_tangent(number dt);
    void print();


    pt_2d _position;
    RigidBody2D * _body_reference;
    RigidBody2D * _body_incident;
    pt_2d _normal;
    pt_2d _r_ref;
    pt_2d _r_incid;
    pt_2d _normal_impulse;
    pt_2d _normal_impulse_cumul;
    pt_2d _tangent_impulse;
    bool _is_valid;
    bool _is_tangent_valid;
    number _mass_inv_sum;
	number _j, _jt;
    bool _is_resting;
};


class Collision2D {
public:
    Collision2D();
    Collision2D(RigidBody2D * body_a, RigidBody2D * body_b, bool verbose=false);
    ~Collision2D();
	uint incident_face_idx(uint reference_face_idx);
    void apply_impulse(number dt);
    void positional_correction();
    void print();


    RigidBody2D * _body_reference;
    RigidBody2D * _body_incident;
    std::vector<Contact2D *> _contacts;
    number _penetration;
    pt_2d _normal;
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


    std::vector<pt_2d> _normal_impulses;
    std::vector<pt_2d> _tangent_impulses;
    bool _is_2delete;
};


class Physics2D {
public:
    Physics2D();
    Physics2D(number dt);
    ~Physics2D();
	// on peut générer plusieurs body avec le meme polygon et avec des matériaux différents par ex
	void add_polygon(Polygon2D * polygon);
    void add_body(uint idx_polygon, uint idx_material, pt_2d position, number orientation);
    void step(bool verbose=false);
    void new_external_force(pt_2d pt_begin, pt_2d pt_end);
    void new_explosion(pt_2d center, number radius);
    void load_body(std::string ch_file, uint idx_material);


	std::vector<Polygon2D *> _polygons;
	std::vector<Material *> _materials;
    std::vector<RigidBody2D *> _bodies;
    std::vector<Collision2D *> _collisions;
    number _dt;
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
    void update(number alpha);


    Physics2D * _physics_2d;

    ScreenGL * _screengl;
    GLuint _prog_draw;
	GLint _camera2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffers[6];
	float _camera2clip[16];
    uint _n_pts, _n_bodies, _n_collisions, _n_contacts;
	bool _visible_pt, _visible_normal, _visible_center, _visible_vel_force, _visible_collision, _visible_contact;
};

#endif
