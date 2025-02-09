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


class Car {
public:
	Car();
	Car(glm::vec2 position);
	~Car();
	void anim();
	friend std::ostream & operator << (std::ostream & os, const Car & car);


	BBox_2D _bbox;
	glm::vec2 _com2force; // vecteur com -> point ou on applique les forces
	glm::vec2 _com2bbox_center; // vecteur com -> centre bbox

	glm::vec2 _com; // center of mass
	glm::vec2 _velocity;
	glm::vec2 _acceleration;
	glm::vec2 _force;

	float _alpha; // angle de rotation
	float _angular_velocity;
	float _angular_acceleration;
	//glm::vec2 _torque;

	float _mass;

	glm::vec2 _forward;
	glm::vec2 _right;
};


class Racing {
public:
	Racing();
	Racing(GLuint prog_bbox, GLuint prog_font, ScreenGL * screengl, bool is_joystick);
	~Racing();

	// dessins
	void draw_bbox();
	void draw();

	// maj des buffers
	void update_bbox();

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

	bool _draw_bbox; // faut-il afficher les BBox
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
