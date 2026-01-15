
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

// sensibilité souris
//const number LEFT_MOUSE_SENSIVITY= 3.0;
const number MIDDLE_MOUSE_SENSIVITY= 3.0;
const number RIGHT_MOUSE_SENSIVITY= 0.01;

// modes de vision : vue libre , 3e personne libre, 3e personne fixe
enum ViewSystemType {FREE_VIEW, THIRD_PERSON_FREE, THIRD_PERSON_BEHIND};

// couleurs
const glm::vec4 GROUND_COLOR(0.3f, 0.3f, 0.3f, 1.0f);
const glm::vec4 BOX_COLOR(0.5f, 0.5f, 0.5f, 1.0f);
const number EPS= 0.01;



// affichage repère
class Repere {
public:
	Repere();
	Repere(std::map<std::string, GLuint> progs);
	~Repere();
	void draw(const mat_4d & world2clip);
	
	
	std::map<std::string, DrawContext *> _contexts;
};


class RectSelect {
public:
	RectSelect();
	RectSelect(std::map<std::string, GLuint> progs);
	~RectSelect();
	void draw();
	void set_origin(pt_2d gl_v);
	void set_moving(pt_2d gl_v);
	void set_active(bool is_active);
	void update_draw();


	DrawContext * _context;
	number _z;
	bool _is_active;
	pt_2d _gl_origin;
	pt_2d _gl_moving;
	pt_3d _color;
	pt_3d _norms[4];
};


// systeme de vue 3D
class ViewSystem {
public:
	ViewSystem();
	ViewSystem(std::map<std::string, GLuint> progs, ScreenGL * screengl);
	~ViewSystem();
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool mouse_motion(InputState * input_state);
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	void update();
	void draw();
	void anim(const pt_3d & target, const quat & rotation);
	void set(const pt_3d & target, number phi, number theta, number rho);
	void set_2d(number rho);
	//void move_target(const pt_3d & v);
	void move_target(int screen_delta_x, int screen_delta_y, number z);
	void move_phi(number x);
	void move_theta(number x);
	void move_rho(number x);
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
	bool single_selection_intersects_aabb(AABB * aabb, bool check_depth=true);
	bool rect_selection_intersects_bbox(BBox * bbox, bool check_depth=true);

	
	// paramètres en entrée
	ScreenGL * _screengl;
	number _frustum_near, _frustum_far, _frustum_halfsize;
	pt_3d _target;
	number _free_view_x, _free_view_y;

	number _phi, _theta, _rho;
	
	// paramètres calculés
	pt_3d _eye;
	pt_3d _up, _right, _dir;
	mat_4d _camera2clip;
	mat_4d _world2camera;
	mat_4d _world2clip;
	pt_3d _center_near, _center_far;
	pt_3d _norm_near, _norm_far, _norm_left, _norm_right, _norm_top, _norm_bottom;

	ViewSystemType _type;
	
	Repere * _repere;

	//Font * _font;
	
	RectSelect * _rect_select;
	bool _new_single_selection;
	bool _new_rect_selection;

	bool _new_destination;
	pt_3d _destination;
};


#endif
