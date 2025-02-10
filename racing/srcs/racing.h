#ifndef RACING_H
#define RACING_H

#include <string>
#include <vector>

#include <OpenGL/gl3.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <glm/glm.hpp>

#include <glm/glm.hpp>

#include "bbox_2d.h"
#include "gl_utils.h"
#include "input_state.h"
#include "font.h"


// plans z de contrainte d'affichage de glm::ortho
const float Z_NEAR= 0.0f;
const float Z_FAR= 1000.0f;

const float MAX_WHEEL= M_PI* 0.5;
const float WHEEL_INCREMENT= 0.05;
const float WHEEL_DECREMENT= 0.05;
const float MAX_BRAKE= 0.2;
const float MAX_THRUST= 1.0;
const float THRUST_INCREMENT= 0.02;
const float BRAKE_INCREMENT= 0.005;

const float CROSS_SIZE= 0.1;
const float ARROW_ANGLE= M_PI* 0.1;
const float ARROW_TIP_SIZE= 0.2;
const float INFO_ALPHA= 0.8;
const glm::vec4 COM_CROSS_COLOR(1.0, 0.0, 0.0, INFO_ALPHA);
const glm::vec4 FORCE_CROSS_COLOR(0.0, 1.0, 0.0, INFO_ALPHA);
const glm::vec4 BBOX_CROSS_COLOR(0.0, 0.5, 1.0, INFO_ALPHA);
const glm::vec4 FORCE_ARROW_COLOR(1.0, 1.0, 0.0, INFO_ALPHA);
const glm::vec4 ACCELERATION_ARROW_COLOR(1.0, 0.0, 1.0, INFO_ALPHA);
const glm::vec4 VELOCITY_ARROW_COLOR(0.0, 1.0, 1.0, INFO_ALPHA);
const glm::vec4 FORWARD_ARROW_COLOR(1.0, 0.5, 1.0, INFO_ALPHA);
const glm::vec4 RIGHT_ARROW_COLOR(1.0, 1.0, 0.5, INFO_ALPHA);


glm::vec2 rot(glm::vec2 v, float alpha);
float norm(glm::vec2 v);
float scal(glm::vec2 u, glm::vec2 v);


class Car {
public:
	Car();
	Car(glm::vec2 position, float alpha);
	~Car();
	void reinit(glm::vec2 position, float alpha);
	void preanim_keys(bool key_left, bool key_right, bool key_down, bool key_up);
	void anim();
	friend std::ostream & operator << (std::ostream & os, const Car & car);


	BBox_2D _bbox;
	glm::vec2 _com2force; // vecteur com -> point ou on applique les forces
	glm::vec2 _com2force_ini;
	glm::vec2 _com2bbox_center; // vecteur com -> centre bbox
	glm::vec2 _com2bbox_center_ini;

	glm::vec2 _com; // center of mass
	glm::vec2 _velocity;
	glm::vec2 _acceleration;
	glm::vec2 _force;

	float _alpha; // angle de rotation
	float _angular_velocity;
	float _angular_acceleration;
	float _torque;

	float _wheel;
	float _thrust;

	float _mass;
	glm::vec2 _size;

	glm::vec2 _forward;
	glm::vec2 _forward_ini;
	glm::vec2 _right;
	glm::vec2 _right_ini;
};


class Racing {
public:
	Racing();
	Racing(GLuint prog_bbox, GLuint prog_font, ScreenGL * screengl, bool is_joystick);
	~Racing();

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

	// input
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	bool joystick_down(unsigned int button_idx);
	bool joystick_up(unsigned int button_idx);
	bool joystick_axis(unsigned int axis_idx, int value);


	std::vector<Car *> _cars;

	glm::vec2 _pt_min, _pt_max;

	bool _draw_bbox, _draw_force, _show_info; // faut-il afficher les BBox
	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers; // buffers OpenGL
	GLuint * _textures; // texture arrays pour tous les PNGs
	glm::mat4 _camera2clip; // glm::ortho
	Font * _font; // font pour écriture textes

	bool _key_left, _key_right, _key_up, _key_down, _key_a, _key_z; // les touches sont-elle enfoncées
	bool _is_joystick;
	glm::vec2 _joystick; // valeurs x, y stick joystick
	bool _joystick_a, _joystick_b; // boutons 

};


#endif
