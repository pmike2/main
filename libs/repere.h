
#ifndef REPERE_H
#define REPERE_H

#include <iostream>

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



// dimensions repere
const float REPERE_AXIS= 10.0f;
const float REPERE_GROUND= 10.0f;
const float REPERE_BOX= 100.0f;

// sert à initialiser camera2clip pour fenetres 3D
const float FRUSTUM_NEAR= 10.0f;
const float FRUSTUM_FAR= 5000.0f;
const float FRUSTUM_HALFSIZE= 5.0f;

// ajoute une marge a l'ensemble des points contenus dans le champ de vision
//const float CONTAINS_POINT_TOLERANCE= 20.0f;

// sensibilité souris
const float LEFT_MOUSE_SENSIVITY= 5.0f;
const float MIDDLE_MOUSE_SENSIVITY= 5.0f;
const float RIGHT_MOUSE_SENSIVITY= 0.01f;

// modes de vision : vue libre , 3e personne libre, 3e personne fixe
enum ViewSystemType {FREE_VIEW, THIRD_PERSON_FREE, THIRD_PERSON_BEHIND};


// affichage repère
class Repere {
public:
	Repere();
	Repere(GLuint prog_draw);
	~Repere();
	void draw(const glm::mat4 & world2clip);
	
	
	GLuint _prog_draw;
	GLint _world2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffers[3];
	bool _is_repere;
	bool _is_ground;
	bool _is_box;
};


class RectSelect {
public:
	RectSelect();
	RectSelect(GLuint prog_draw);
	~RectSelect();
	void draw();
	void set_origin(glm::vec2 gl_v);
	void set_moving(glm::vec2 gl_v);
	void set_active(bool is_active);
	void update_draw();


	GLuint _prog_draw;
	GLint _position_loc, _color_loc;
	GLuint _buffer;
	bool _is_active;
	glm::vec2 _gl_origin;
	glm::vec2 _gl_moving;
	glm::vec3 _color;
	glm::vec3 _norms[4];
};


// systeme de vue 3D
class ViewSystem {
public:
	ViewSystem();
	ViewSystem(GLuint prog_repere, GLuint prog_select, unsigned int screen_width, unsigned int screen_height);
	~ViewSystem();
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool mouse_motion(InputState * input_state);
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	void update();
	void draw();
	void anim(const glm::vec3 & target, const glm::quat & rotation);
	void set(const glm::vec3 & target, float phi, float theta, float rho);
	void move_target(const glm::vec3 & v);
	void move_phi(float x);
	void move_theta(float x);
	void move_rho(float x);
	glm::vec2 screen2world(unsigned int x, unsigned int y, float z);
	glm::vec2 screen2world(glm::vec2 gl_coords, float z);
	glm::vec2 screen2gl(unsigned int x, unsigned int y);
	glm::uvec2 gl2screen(glm::vec2 gl_coords);
	float depthbuffer2world(float depth);
	//bool contains_point(const glm::vec3 & pos);
	bool intersects_bbox(BBox * bbox, bool selection=false);
	bool intersects_aabb(AABB * aabb, bool selection=false);
	bool intersects_aabb(AABB * aabb, const glm::mat4 & model2world_matrix, bool selection=false);
	bool intersects_pts(glm::vec3 * pts, unsigned int n_pts, bool selection=false);
	//bool selection_contains_point(const glm::vec3 & pt);
	void update_selection_norms();
	bool single_selection_intersects_aabb(AABB * aabb);
	bool rect_selection_intersects_bbox(BBox * bbox);

	
	// paramètres en entrée
	float _frustum_near, _frustum_far, _frustum_halfsize;
	unsigned int _screen_width, _screen_height;
	glm::vec3 _target;
	float _phi, _theta, _rho;
	
	// paramètres calculés
	glm::vec3 _eye;
	glm::vec3 _up, _right, _dir;
	glm::mat4 _camera2clip;
	glm::mat4 _world2camera;
	glm::mat4 _world2clip;
	glm::vec3 _center_near, _center_far;
	glm::vec3 _norm_near, _norm_far, _norm_left, _norm_right, _norm_top, _norm_bottom;

	ViewSystemType _type;
	
	Repere * _repere;
	
	RectSelect * _rect_select;
	bool _new_single_selection;
	bool _new_rect_selection;

	bool _new_destination;
	glm::vec3 _destination;
};


#endif
