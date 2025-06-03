#ifndef GRID_H
#define GRID_H

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


const unsigned int DEFAULT_WIDTH= 100;
const unsigned int DEFAULT_HEIGHT= 100;
const number DEFAULT_CELL_SIZE= 0.10;
const number DEFAULT_CELL_MARGIN= 0.0;
const float Z_NEAR= 0.0f;
const float Z_FAR= 1000.0f;
const float Z_CELL= -50.0f;
const float Z_GRID= -30.0f;
const unsigned int DEFAULT_DELTA_ANIM= 1;
const glm::vec4 GRID_COLOR(0.4, 0.4, 0.4, 1.0);


class Cell {
public:
	Cell();
	Cell(unsigned int col, unsigned int row);
	~Cell();


	unsigned int _col, _row;
	std::vector<Cell *> _neighbours;
	AABB_2D * _aabb;
	unsigned int _state;
};


class Grid {
public:
	Grid();
	Grid(std::map<std::string, GLuint> progs, ScreenGL * screengl, InputState * input_state, time_point t);
	~Grid();

	void set_cell_states(std::map<unsigned int, glm::vec4> cell_states);
	Cell * get_cell(unsigned int col, unsigned int row);
	pt_type idx2pt(unsigned int col, unsigned int row);
	void clear();

	void draw_cell();
	void draw_grid();
	void draw();
	void update_grid();
	void update_cell();
	bool key_down(SDL_Keycode key);


	std::map<unsigned int, glm::vec4> _cell_states;
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
