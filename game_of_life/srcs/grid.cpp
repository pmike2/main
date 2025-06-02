#include "grid.h"


// Cell ---------------------------------------------------------------
Cell::Cell() {

}


Cell::Cell(unsigned int col, unsigned int row) : _col(col), _row(row), _state(0) {
	_aabb= new AABB_2D();
}


Cell::~Cell() {
	
}


// Grid ----------------------------------------------------------------
Grid::Grid() {
	
}


Grid::Grid(std::map<unsigned int, glm::vec4> cell_states, std::map<std::string, GLuint> progs, ScreenGL * screengl, InputState * input_state, time_point t) :
	_cell_states(cell_states), _width(DEFAULT_WIDTH), _height(DEFAULT_HEIGHT), _cell_size(DEFAULT_CELL_SIZE), 
	_cell_margin(DEFAULT_CELL_MARGIN), _delta_anim(DEFAULT_DELTA_ANIM), _last_anim(t), _screengl(screengl), 
	_input_state(input_state), _draw_grid(true), _anim(false)
{
	_camera2clip= glm::ortho(float(-_screengl->_gl_width)* 0.5f, float(_screengl->_gl_width)* 0.5f, -float(_screengl->_gl_height)* 0.5f, float(screengl->_gl_height)* 0.5f, Z_NEAR, Z_FAR);
	_world2camera= glm::translate(glm::mat4(1.0f), glm::vec3(-0.5* _screengl->_gl_width+ 1.0, -0.5* _screengl->_gl_height+ 1.0, 0.0));
	//_world2camera= glm::mat4(1.0);

	_font= new Font(progs["font"], "../../fonts/Silom.ttf", 48, _screengl);
	_font->_z= 100.0f; // pour que l'affichage des infos se fassent par dessus le reste

	// buffers
	unsigned int n_buffers= 2;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	// contextes de dessin
	_contexts["grid"]= new DrawContext(progs["simple"], _buffers[0],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["cell"]= new DrawContext(progs["simple"], _buffers[1],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	update_grid();
	update_cell();
}


Grid::~Grid() {
	for (auto cell : _cells) {
		delete cell;
	}
	_cells.clear();
}


Cell * Grid::get_cell(unsigned int col, unsigned int row) {
	unsigned int idx= _width* row+ col;
	return _cells[idx];
}


pt_type Grid::idx2pt(unsigned int col, unsigned int row) {
	return pt_type(col* _cell_size, row* _cell_size);
}


void Grid::randomize() {
	for (auto cell : _cells) {
		cell->_state= rand_int(0, _cell_states.size()- 1);
	}
}


void Grid::clear() {
	for (auto cell : _cells) {
		cell->_state= 0;
	}
}


void Grid::draw_grid() {
	DrawContext * context= _contexts["grid"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	glUniform1f(context->_locs_uniform["z"], Z_GRID);
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Grid::draw_cell() {
	DrawContext * context= _contexts["cell"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	glUniform1f(context->_locs_uniform["z"], Z_CELL);
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Grid::draw() {
	draw_cell();
	if (_draw_grid) {
		draw_grid();
	}
}


void Grid::update_grid() {
	DrawContext * context= _contexts["grid"];
	context->_n_pts= (_width+ 1)* 2+ (_height+ 1)* 2;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;
	for (unsigned int i=0; i<= _width; ++i) {
		data[compt++]= float(i)* _cell_size; data[compt++]= 0.0; data[compt++]= GRID_COLOR.r; data[compt++]= GRID_COLOR.g; data[compt++]= GRID_COLOR.b; data[compt++]= GRID_COLOR.a;
		data[compt++]= float(i)* _cell_size; data[compt++]= float(_height)* _cell_size; data[compt++]= GRID_COLOR.r; data[compt++]= GRID_COLOR.g; data[compt++]= GRID_COLOR.b; data[compt++]= GRID_COLOR.a;
	}
	for (unsigned int i=0; i<= _height; ++i) {
		data[compt++]= 0.0; data[compt++]= float(i)* _cell_size; data[compt++]= GRID_COLOR.r; data[compt++]= GRID_COLOR.g; data[compt++]= GRID_COLOR.b; data[compt++]= GRID_COLOR.a;
		data[compt++]= float(_width)* _cell_size; data[compt++]= float(i)* _cell_size; data[compt++]= GRID_COLOR.r; data[compt++]= GRID_COLOR.g; data[compt++]= GRID_COLOR.b; data[compt++]= GRID_COLOR.a;
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Grid::update_cell() {
	const unsigned int n_pts_per_obj= 6;

	DrawContext * context= _contexts["cell"];
	context->_n_pts= _cells.size()* n_pts_per_obj;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;
	for (auto cell : _cells) {
		glm::vec4 color= _cell_states[cell->_state];
		data[compt++]= cell->_aabb->_pos.x; data[compt++]= cell->_aabb->_pos.y; data[compt++]= color.r; data[compt++]= color.g; data[compt++]= color.b; data[compt++]= color.a;
		data[compt++]= cell->_aabb->_pos.x+ cell->_aabb->_size.x; data[compt++]= cell->_aabb->_pos.y; data[compt++]= color.r; data[compt++]= color.g; data[compt++]= color.b; data[compt++]= color.a;
		data[compt++]= cell->_aabb->_pos.x+ cell->_aabb->_size.x; data[compt++]= cell->_aabb->_pos.y+ cell->_aabb->_size.y; data[compt++]= color.r; data[compt++]= color.g; data[compt++]= color.b; data[compt++]= color.a;

		data[compt++]= cell->_aabb->_pos.x; data[compt++]= cell->_aabb->_pos.y; data[compt++]= color.r; data[compt++]= color.g; data[compt++]= color.b; data[compt++]= color.a;
		data[compt++]= cell->_aabb->_pos.x+ cell->_aabb->_size.x; data[compt++]= cell->_aabb->_pos.y+ cell->_aabb->_size.y; data[compt++]= color.r; data[compt++]= color.g; data[compt++]= color.b; data[compt++]= color.a;
		data[compt++]= cell->_aabb->_pos.x; data[compt++]= cell->_aabb->_pos.y+ cell->_aabb->_size.y; data[compt++]= color.r; data[compt++]= color.g; data[compt++]= color.b; data[compt++]= color.a;
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool Grid::key_down(SDL_Keycode key) {
	if (key== SDLK_g) {
		_draw_grid= !_draw_grid;
		return true;
	}
	else if (key== SDLK_SPACE) {
		_anim= !_anim;
		return true;
	}
	else if (key== SDLK_r) {
		randomize();
		update_cell();
		return true;
	}
	else if (key== SDLK_c) {
		clear();
		update_cell();
		return true;
	}
	return false;
}

