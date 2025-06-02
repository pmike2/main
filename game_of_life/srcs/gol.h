#ifndef GOL_H
#define GOL_H

#include <iostream>
#include <vector>
#include <map>

#include <glm/glm.hpp>

#include "input_state.h"
#include "gl_utils.h"
#include "typedefs.h"

#include "grid.h"


const std::map<unsigned int, glm::vec4> GOL_CELL_STATES {{0, glm::vec4(0.2, 0.0, 0.0, 1.0)}, {1, glm::vec4(0.0, 1.0, 0.0, 1.0)}};


class GolCell : public Cell {
public:
	GolCell();
	GolCell(unsigned int col, unsigned int row);
	~GolCell();
	void next();


	unsigned int _next_state;
};


class GOL : public Grid {
public:
	GOL();
	GOL(std::map<std::string, GLuint> progs, ScreenGL * screengl, InputState * input_state, time_point t);
	~GOL();
	void next();
	void anim(time_point t);
	bool key_down(SDL_Keycode key);
	void click_cell();
	bool mouse_button_down();
	bool mouse_motion();
};


#endif
