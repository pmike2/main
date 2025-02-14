#ifndef RACING_H
#define RACING_H

#include <string>
#include <map>
#include <vector>

#include <OpenGL/gl3.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <glm/glm.hpp>

#include "bbox_2d.h"
#include "geom_2d.h"
#include "gl_utils.h"
#include "input_state.h"
#include "font.h"
#include "typedefs.h"


// plans z de contrainte d'affichage de glm::ortho
const float Z_NEAR= 0.0f;
const float Z_FAR= 1000.0f;

const number ANIM_DT= 0.05;

/*const float MAX_WHEEL= M_PI* 0.3;
const float WHEEL_INCREMENT= 0.05;
const float WHEEL_DECREMENT= 0.05;
const float MAX_BRAKE= 1.0;
const float MAX_THRUST= 3.0;
const float THRUST_INCREMENT= 0.01;
const float BRAKE_INCREMENT= 0.06;
const float THRUST_DECREMENT= 0.02;*/

const float CROSS_SIZE= 0.1;
const float ARROW_ANGLE= M_PI* 0.1;
const float ARROW_TIP_SIZE= 0.2;
const float INFO_ALPHA= 0.8;
const glm::vec4 COM_CROSS_COLOR(1.0, 0.0, 0.0, INFO_ALPHA);
const glm::vec4 FORCE_FWD_CROSS_COLOR(1.0, 1.0, 0.0, INFO_ALPHA);
const glm::vec4 FORCE_BWD_CROSS_COLOR(1.0, 1.0, 0.5, INFO_ALPHA);
const glm::vec4 BBOX_CROSS_COLOR(0.0, 0.5, 1.0, INFO_ALPHA);
const glm::vec4 FORCE_FWD_ARROW_COLOR(1.0, 1.0, 0.0, INFO_ALPHA);
const glm::vec4 FORCE_BWD_ARROW_COLOR(1.0, 1.0, 0.5, INFO_ALPHA);
const glm::vec4 ACCELERATION_ARROW_COLOR(1.0, 0.0, 1.0, INFO_ALPHA);
const glm::vec4 VELOCITY_ARROW_COLOR(0.0, 1.0, 1.0, INFO_ALPHA);
const glm::vec4 FORWARD_ARROW_COLOR(1.0, 0.5, 1.0, INFO_ALPHA);
const glm::vec4 RIGHT_ARROW_COLOR(1.0, 1.0, 0.5, INFO_ALPHA);


pt_type rot(pt_type v, number alpha);
number norm(pt_type v);
pt_type normalized(pt_type v);
number scal(pt_type u, pt_type v);
number cross(pt_type u, pt_type v);


class CarModel {
public:
	CarModel();
	CarModel(std::string json_path);
	~CarModel();


	std::string _json_path;

	pt_type _forward;
	pt_type _right;
	pt_type _com2force_fwd;
	pt_type _com2force_bwd;
	pt_type _com2bbox_center;
	pt_type _halfsize;
	number _mass;
	number _inertia;
	number _max_wheel;
	number _wheel_increment;
	number _wheel_decrement;
	number _max_thrust;
	number _thrust_increment;
	number _thrust_decrement;
	number _max_brake;
	number _brake_increment;
	number _forward_static_friction;
	number _backward_static_friction;
	number _backward_dynamic_friction;
	number _friction_threshold;
	number _angular_friction;

	bool _fixed;
};


class Car {
public:
	Car();
	Car(CarModel * model, pt_type position, number alpha);
	~Car();
	void reinit(pt_type position, number alpha);
	void update_direction();
	void update_bbox();
	void preanim_keys(bool key_left, bool key_right, bool key_down, bool key_up);
	void random_ia();
	void anim();
	friend std::ostream & operator << (std::ostream & os, const Car & car);


	CarModel * _model;
	BBox_2D * _bbox;

	pt_type _com2force_fwd; // vecteur com -> point ou on applique les forces
	pt_type _com2force_bwd; // vecteur com -> point ou on applique les forces
	pt_type _com2bbox_center; // vecteur com -> centre bbox
	pt_type _forward;
	pt_type _right;

	pt_type _com; // center of mass
	pt_type _velocity;
	pt_type _acceleration;
	pt_type _force_fwd;
	pt_type _force_bwd;

	number _alpha; // angle de rotation
	number _angular_velocity;
	number _angular_acceleration;
	number _torque;

	number _wheel;
	number _thrust;
	bool _drift;
};


class Racing {
public:
	Racing();
	Racing(GLuint prog_bbox, GLuint prog_font, ScreenGL * screengl, bool is_joystick);
	~Racing();

	void load_models();
	void load_json(std::string json_path);
	void save_json(std::string json_path);
	void randomize();
	void add_boundary();

	// dessins
	void draw_bbox();
	void draw_force();
	void draw();

	void show_info();

	// maj des buffers
	void update_bbox();
	void update_force();

	// animation
	void anim();
	void collision();

	// input
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	bool joystick_down(unsigned int button_idx);
	bool joystick_up(unsigned int button_idx);
	bool joystick_axis(unsigned int axis_idx, int value);


	std::map<std::string, CarModel *> _models;
	std::vector<Car *> _cars;

	pt_type _pt_min, _pt_max;
	pt_type _com_camera;
	number _alpha_camera;

	bool _draw_bbox, _draw_force, _show_info; // faut-il afficher les BBox
	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers; // buffers OpenGL
	GLuint * _textures; // texture arrays pour tous les PNGs
	glm::mat4 _camera2clip; // glm::ortho
	glm::mat4 _world2camera;
	Font * _font; // font pour écriture textes

	bool _key_left, _key_right, _key_up, _key_down, _key_a, _key_z; // les touches sont-elle enfoncées
	bool _is_joystick;
	glm::vec2 _joystick; // valeurs x, y stick joystick
	bool _joystick_a, _joystick_b; // boutons 

	bool _ia;
};


#endif
