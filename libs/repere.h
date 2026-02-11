
#ifndef REPERE_H
#define REPERE_H

#include <iostream>
#include <map>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <SDL2/SDL_keycode.h>

#include "input_state.h"
#include "bbox.h"
#include "font.h"
#include "gl_utils.h"
#include "geom.h"
#include "gl_draw.h"
#include "typedefs.h"


// dimensions repere
const number REPERE_AXIS= 10.0;
const number REPERE_GROUND= 10.0;
const number REPERE_BOX= 100.0;

// sert à initialiser camera2clip pour fenetres 3D
const number FRUSTUM_NEAR= 10.0;
const number FRUSTUM_FAR= 500.0;
const number FRUSTUM_HALFSIZE= 5.0;

// ajoute une marge a l'ensemble des points contenus dans le champ de vision
//const number CONTAINS_POINT_TOLERANCE= 20.0;

// sensibilités par défaut
const number DEFAULT_TARGET_SENSIVITY= 0.7;
const number DEFAULT_PHI_SENSIVITY= 0.3;
const number DEFAULT_THETA_SENSIVITY= 0.3;
const number DEFAULT_RHO_SENSIVITY= 2.0;


// animation par touche --------------------
// quantité de mouvement appliqué lorsque l'on déplace _target avec les touches
const int KEY_TARGET_MOVE = 500;
// temps en ms d'anim du mouvement de target
const uint KEY_TARGET_N_MS = 1000;
// exposant appliqué au calcul de déplacement de target
const number KEY_TARGET_EXP = 5.0;

const int KEY_PHI_MOVE = 1000;
const uint KEY_PHI_N_MS = 1000;
const number KEY_PHI_EXP = 3.0;
const number KEY_PHI_FACTOR = 0.02;

const int KEY_THETA_MOVE = 1000;
const uint KEY_THETA_N_MS = 500;
const number KEY_THETA_EXP = 3.0;
const number KEY_THETA_FACTOR = 0.01;

const int KEY_RHO_MOVE = 1000;
const uint KEY_RHO_N_MS = 500;
const number KEY_RHO_EXP = 3.0;
const number KEY_RHO_FACTOR = 1.0;

// couleurs
const glm::vec4 GROUND_COLOR(0.3f, 0.3f, 0.3f, 1.0f);
const glm::vec4 BOX_COLOR(0.5f, 0.5f, 0.5f, 1.0f);
const number GROUND_EPS= 0.01;


// modes de vision : vue libre , 3e personne libre, 3e personne fixe
enum ViewSystemType {FREE_VIEW, THIRD_PERSON_FREE, THIRD_PERSON_BEHIND};


// affichage repère
class Repere {
public:
	Repere();
	Repere(GLDrawManager * gl_draw_manager);
	~Repere();
	void draw(const mat_4d & world2clip);
	
	
	GLDrawManager * _gl_draw_manager;
};


class RectSelect {
public:
	RectSelect();
	~RectSelect();
	void set_origin(pt_2d gl_v);
	void set_moving(pt_2d gl_v);
	void set_active(bool is_active);


	bool _is_active;
	pt_2d _gl_origin;
	pt_2d _gl_moving;
	pt_3d _norms[4];
};


// systeme de vue 3D
class ViewSystem {
public:
	ViewSystem();
	ViewSystem(GLDrawManager * gl_draw_manager, ScreenGL * screengl);
	~ViewSystem();
	
	bool mouse_button_down(InputState * input_state, time_point t);
	bool mouse_button_up(InputState * input_state, time_point t);
	bool mouse_motion(InputState * input_state, time_point t);
	bool key_down(InputState * input_state, SDL_Keycode key, time_point t);
	bool key_up(InputState * input_state, SDL_Keycode key, time_point t);
	
	void update();
	void draw();
	//void anim(const pt_3d & target, const quat & rotation);
	void anim(time_point t);
	
	void set(const pt_3d & target, number phi, number theta, number rho);
	void set_2d(number rho);

	void constraint_target(const pt_2d & target_min, const pt_2d & target_max);
	void constraint_target(const pt_2d & target);
	void unconstraint_target();
	void constraint_phi(number phi_min, number phi_max);
	void constraint_phi(number phi);
	void unconstraint_phi();
	void constraint_theta(number theta_min, number theta_max);
	void constraint_theta(number theta);
	void unconstraint_theta();
	void constraint_rho(number rho_min, number rho_max);
	void constraint_rho(number rho);
	void unconstraint_rho();
	
	pt_2d get_target_direction(int screen_delta_x, int screen_delta_y);
	void move_target(int screen_delta_x, int screen_delta_y);
	number get_delta_phi(int screen_delta_x);
	void move_phi(int screen_delta_x);
	number get_delta_theta(int screen_delta_y);
	void move_theta(int screen_delta_y);
	number get_delta_rho(int screen_delta_y);
	void move_rho(int screen_delta_y);
	
	pt_2d screen2world(pt_2d gl_coords, number z);
	pt_2d screen2world(uint x, uint y, number z);
	pt_3d screen2world_depthbuffer(pt_2d gl_coords);
	pt_3d screen2world_depthbuffer(uint x, uint y);
	pt_2d screen2gl(uint x, uint y);
	glm::uvec2 gl2screen(pt_2d gl_coords);
	number depthbuffer2world(number depth);
	
	//bool contains_point(const pt_3d & pos);
	bool intersects_bbox(BBox * bbox, bool selection=false);
	bool intersects_aabb(AABB * aabb, bool selection=false);
	//bool intersects_aabb_2d(AABB_2D * aabb, bool selection=false);
	bool intersects_aabb(AABB * aabb, const mat_4d & model2world_matrix, bool selection=false);
	bool intersects_pts(std::vector<pt_3d> pts, bool selection=false);
	bool intersects_pts(pt_3d * pts, uint n_pts, bool selection=false);
	//bool selection_contains_point(const pt_3d & pt);
	void update_selection_norms();
	bool pt_2d_intersects_aabb(pt_2d pt, AABB * aabb, bool check_depth=true);
	bool single_selection_intersects_aabb(AABB * aabb, bool check_depth=true);
	bool rect_selection_intersects_bbox(BBox * bbox, bool check_depth=true);

	
	// paramètres constants
	ScreenGL * _screengl;
	number _frustum_near, _frustum_far, _frustum_halfsize;

	// paramètres en entrée
	pt_3d _target;
	number _phi, _theta, _rho;
	// https://en.wikipedia.org/wiki/Spherical_coordinate_system
	// phi est l'angle horizontal compris entre 0 et 2PI
	// theta est l'angle vertical, compris entre 0 et PI
	
	// paramètres calculés
	pt_3d _eye;
	pt_3d _up, _right, _dir;
	mat_4d _camera2clip;
	mat_4d _world2camera;
	mat_4d _world2clip;
	pt_3d _center_near, _center_far;
	pt_3d _norm_near, _norm_far, _norm_left, _norm_right, _norm_top, _norm_bottom;

	// contraintes
	bool _target_constrained;
	pt_2d _target_min, _target_max;
	bool _phi_constrained;
	number _phi_min, _phi_max;
	bool _theta_constrained;
	number _theta_min, _theta_max;
	bool _rho_constrained;
	number _rho_min, _rho_max;

	// sensitivités
	number _target_sensitivity;
	number _theta_sensitivity;
	number _phi_sensitivity;
	number _rho_sensitivity;

	// type de vue
	ViewSystemType _type;
	
	// dessin
	GLDrawManager * _gl_draw_manager;
	Repere * _repere;

	// sélection
	RectSelect * _rect_select;
	bool _new_single_selection;
	bool _new_rect_selection;

	// anim
	time_point _target_t;
	bool _target_anim;
	pt_2d _target_start, _target_goal, _target_direction;
	number _target_length;

	time_point _phi_t;
	bool _phi_anim;
	number _phi_start, _phi_goal;

	time_point _theta_t;
	bool _theta_anim;
	number _theta_start, _theta_goal;

	time_point _rho_t;
	bool _rho_anim;
	number _rho_start, _rho_goal;
};


#endif
