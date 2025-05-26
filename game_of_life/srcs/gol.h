#ifndef GOL_H
#define GOL_H

#include <iostream>
#include <vector>
#include <map>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#include "bbox_2d.h"
#include "typedefs.h"
#include "font.h"
#include "gl_utils.h"
#include "utile.h"
#include "input_state.h"


enum cell_state { ALIVE, DEAD };

const unsigned int DEFAULT_WIDTH= 90;
const unsigned int DEFAULT_HEIGHT= 64;
const float Z_NEAR= 0.0f;
const float Z_FAR= 1000.0f;
const float Z_CELL= -50.0f;
const float Z_GRID= -30.0f;
const unsigned int DEFAULT_DELTA_ANIM= 100;
const number DEFAULT_CELL_SIZE= 0.15;
const number DEFAULT_CELL_MARGIN= 0.02;
const std::map<cell_state, glm::vec4> STATE_COLOR {{ALIVE, glm::vec4(0.0, 1.0, 0.0, 1.0)}, {DEAD, glm::vec4(0.2, 0.0, 0.0, 1.0)}};
const glm::vec4 GRID_COLOR(0.4, 0.4, 0.4, 1.0);


class Cell {
public:
	Cell();
	Cell(unsigned int col, unsigned int row);
	~Cell();
	void next();


	unsigned int _col, _row;
	cell_state _state;
	cell_state _next_state;
	std::vector<Cell *> _neighbours;
	AABB_2D * _aabb;
};


class GOL {
public:
	GOL();
	GOL(std::map<std::string, GLuint> progs, ScreenGL * screengl, InputState * input_state, time_point t);
	~GOL();
	Cell * get_cell(unsigned int col, unsigned int row);
	void next();
	pt_type idx2pt(unsigned int col, unsigned int row);
	void randomize();
	void clear();

	void anim(time_point t);
	void draw_cell();
	void draw_grid();
	void draw();
	void update_grid();
	void update_cell();
	bool key_down(SDL_Keycode key);
	void click_cell();
	bool mouse_button_down();
	bool mouse_motion();


	std::vector<Cell *> _cells;
	unsigned int _width, _height;
	number _cell_size;
	number _cell_margin;

	time_point _last_anim;
	unsigned int _delta_anim;
	bool _anim;

	glm::mat4 _camera2clip, _world2camera;
	Font * _font;
	ScreenGL * _screengl;
	InputState * _input_state;
	std::map<std::string, DrawContext *> _contexts;
	GLuint * _buffers;
	bool _draw_grid;
};


#endif
