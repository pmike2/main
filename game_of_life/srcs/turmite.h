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

const std::map<unsigned int, glm::vec4> TURMITE_CELL_STATES {{0, glm::vec4(0.0, 0.0, 0.0, 1.0)}, {1, glm::vec4(0.8, 0.5, 0.0, 1.0)}};

struct Rule {
	unsigned int _write_color;
	Rotation _rotation;
	unsigned int _next_state;
};


// 1ere valeur = état machine ; 2emme valeur = couleur case courante
const std::map<std::pair<unsigned int, unsigned int>, Rule> RULES_1 {
	{std::make_pair(0, 0), {1, RIGHT, 0}},
	{std::make_pair(0, 1), {1, RIGHT, 1}},
	{std::make_pair(1, 0), {0, NO_TURN, 0}},
	{std::make_pair(1, 1), {0, NO_TURN, 1}}
};


Orientation rotate(Orientation orientation, Rotation r);


class Turmite : public Grid {
public:
	Turmite();
	Turmite(std::map<std::string, GLuint> progs, ScreenGL * screengl, InputState * input_state, time_point t);
	~Turmite();
	void next();
	void anim(time_point t);
	bool key_down(SDL_Keycode key);
	bool mouse_button_down();
	bool mouse_motion();


	unsigned int _state;
	Cell * _current_cell;
	Orientation _orientation;
};

#endif
