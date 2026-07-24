
/*
Affichage repère (axes, bbox, sol)
*/

#ifndef REPERE_H
#define REPERE_H

#include <iostream>

#include <glm/glm.hpp>

#include <SDL2/SDL_keycode.h>

#include "input_state.h"
#include "gl_utils.h"
#include "gl_draw.h"
#include "typedefs.h"
#include "view_system.h"


// dimensions repere
const number REPERE_DEFAULT_AXIS_SIZE = 10.0;
const number REPERE_DEFAULT_GROUND_SIZE = 100.0;
const number REPERE_DEFAULT_GROUND_STEP = 1.0;
const number REPERE_DEFAULT_BOX_SIZE = 100.0;

// couleurs
const glm::vec4 GROUND_COLOR(0.3f, 0.3f, 0.3f, 1.0f);
const glm::vec4 BOX_COLOR(0.5f, 0.5f, 0.5f, 1.0f);

// offset z ground pour ne pas gener affichage repere
const number Z_GROUND_EPS = -0.01;


// affichage repère
class Repere {
public:
	Repere();
	Repere(GLDrawManager * gl_draw_manager, ViewSystem * view_system, number axis_size = REPERE_DEFAULT_AXIS_SIZE, number ground_size = REPERE_DEFAULT_GROUND_SIZE, number ground_step = REPERE_DEFAULT_GROUND_STEP, number box_size = REPERE_DEFAULT_BOX_SIZE);
	~Repere();
	void update_axis();
	void update_ground();
	void update_box();
	void draw();
	bool key_down(InputState * input_state, SDL_Keycode key, time_point t);
	
	
	GLDrawManager * _gl_draw_manager;
	ViewSystem * _view_system;
	number _axis_size, _ground_size, _ground_step, _box_size;
};

#endif
