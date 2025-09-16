#ifndef TURMITE_H
#define TURMITE_H

// https://en.wikipedia.org/wiki/Turmite

#include <iostream>
#include <vector>
#include <map>

#include <glm/glm.hpp>

#include "input_state.h"
#include "gl_utils.h"
#include "typedefs.h"

#include "grid.h"


enum Rotation {LEFT, RIGHT, NO_TURN, U_TURN};
enum Orientation {NORTH, SOUTH, EAST, WEST};

const int N_MACHINE_STATES_MIN= 2;
const int N_MACHINE_STATES_MAX= 2;
const int N_CELL_STATES_MIN= 2;
const int N_CELL_STATES_MAX= 2;

Orientation rotate(Orientation orientation, Rotation r);
Orientation rand_orientation();
Rotation rand_rotation();


struct Rule {
	unsigned int _write_color;
	Rotation _rotation;
	unsigned int _next_machine_state;
};


class Turmite : public Grid {
public:
	Turmite();
	Turmite(std::map<std::string, GLuint> progs, ScreenGL * screengl, InputState * input_state, time_point t);
	~Turmite();
	void next();
	void anim(time_point t);
	void randomize();
	bool key_down(SDL_Keycode key);
	bool mouse_button_down();
	bool mouse_motion();


	unsigned int _n_machine_states;
	unsigned int _n_cell_states;
	unsigned int _current_machine_state;
	Cell * _current_cell;
	Orientation _orientation;
	Rule * _rules;
};

#endif
