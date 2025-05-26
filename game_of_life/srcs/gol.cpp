#include "gol.h"


// Cell ---------------------------------------------------------------
Cell::Cell() {

}


Cell::Cell(unsigned int col, unsigned int row) : _state(DEAD), _next_state(DEAD), _col(col), _row(row) {
	_aabb= new AABB_2D();
}


Cell::~Cell() {
	
}


void Cell::next() {
	unsigned int n_alive= 0;
	for (auto neighbour : _neighbours) {
		if (neighbour->_state== ALIVE) {
			n_alive++;
		}
	}

	if (n_alive< 2 || n_alive> 3) {
		_next_state= DEAD;
	}
	else if (n_alive== 2) {
		_next_state= _state;
	}
	else if (n_alive== 3) {
		_next_state= ALIVE;
	}

	/*_next_state= _state;
	if (_state== DEAD && n_alive== 3) {
		_next_state= ALIVE;
	}
	if (_state== ALIVE && n_alive!= 2 && n_alive!= 3) {
		_next_state= DEAD;
	}*/
}


// GOl ----------------------------------------------------------------
GOL::GOL() {
	
}


GOL::GOL(std::map<std::string, GLuint> progs, ScreenGL * screengl, InputState * input_state, time_point t) :
	_width(DEFAULT_WIDTH), _height(DEFAULT_HEIGHT), _cell_size(DEFAULT_CELL_SIZE), _cell_margin(DEFAULT_CELL_MARGIN),
	_delta_anim(DEFAULT_DELTA_ANIM), _last_anim(t), _screengl(screengl), _input_state(input_state), _draw_grid(true),
	_anim(false)
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

	for (unsigned int row=0; row<_height; ++row) {
		for (unsigned int col=0; col<_width; ++col) {
			Cell * cell= new Cell(col, row);
			cell->_aabb->_pos= idx2pt(col, row)+ pt_type(_cell_margin);
			cell->_aabb->_size= pt_type(_cell_size- 2.0* _cell_margin);
			_cells.push_back(cell);
		}
	}

	for (auto cell : _cells) {
		for (int row_offset=-1; row_offset<2; ++row_offset) {
			for (int col_offset=-1; col_offset<2; ++col_offset) {
				if (row_offset== 0 && col_offset== 0) {
					continue;
				}

				if ((int(cell->_col)+ col_offset>= 0) && (int(cell->_col)+ col_offset< _width)
				 && (int(cell->_row)+ row_offset>= 0) && (int(cell->_row)+ row_offset< _height)) {
					cell->_neighbours.push_back(get_cell(cell->_col+ col_offset, cell->_row+ row_offset));
				}
			}
		}
	}

	update_grid();
	update_cell();
}


GOL::~GOL() {
	for (auto cell : _cells) {
		delete cell;
	}
	_cells.clear();
}


Cell * GOL::get_cell(unsigned int col, unsigned int row) {
	unsigned int idx= _width* row+ col;
	return _cells[idx];
}


void GOL::next() {
	for (auto cell : _cells) {
		cell->next();
	}
	for (auto cell : _cells) {
		cell->_state= cell->_next_state;
	}
}


pt_type GOL::idx2pt(unsigned int col, unsigned int row) {
	return pt_type(col* _cell_size, row* _cell_size);
}


void GOL::randomize() {
	for (auto cell : _cells) {
		if (rand_bool()) {
			cell->_state= ALIVE;
		}
		else {
			cell->_state= DEAD;
		}
	}
}


void GOL::clear() {
	for (auto cell : _cells) {
		cell->_state= DEAD;
	}
}


void GOL::anim(time_point t) {
	if (!_anim) {
		return;
	}

	auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_anim).count();
	if (dt> _delta_anim) {
		_last_anim= t;
		next();
	}
	update_cell();
}


void GOL::draw_grid() {
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


void GOL::draw_cell() {
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


void GOL::draw() {
	draw_cell();
	if (_draw_grid) {
		draw_grid();
	}
}


void GOL::update_grid() {
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
	/*for (int i=0; i<context->_n_pts* context->_n_attrs_per_pts; ++i) {
		std::cout << data[i] << " ; ";
	}
	std::cout << "\n";*/

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void GOL::update_cell() {
	const unsigned int n_pts_per_obj= 6;

	DrawContext * context= _contexts["cell"];
	context->_n_pts= _cells.size()* n_pts_per_obj;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;
	for (auto cell : _cells) {
		glm::vec4 color= STATE_COLOR.at(cell->_state);
		data[compt++]= cell->_aabb->_pos.x; data[compt++]= cell->_aabb->_pos.y; data[compt++]= color.r; data[compt++]= color.g; data[compt++]= color.b; data[compt++]= color.a;
		data[compt++]= cell->_aabb->_pos.x+ cell->_aabb->_size.x; data[compt++]= cell->_aabb->_pos.y; data[compt++]= color.r; data[compt++]= color.g; data[compt++]= color.b; data[compt++]= color.a;
		data[compt++]= cell->_aabb->_pos.x+ cell->_aabb->_size.x; data[compt++]= cell->_aabb->_pos.y+ cell->_aabb->_size.y; data[compt++]= color.r; data[compt++]= color.g; data[compt++]= color.b; data[compt++]= color.a;

		data[compt++]= cell->_aabb->_pos.x; data[compt++]= cell->_aabb->_pos.y; data[compt++]= color.r; data[compt++]= color.g; data[compt++]= color.b; data[compt++]= color.a;
		data[compt++]= cell->_aabb->_pos.x+ cell->_aabb->_size.x; data[compt++]= cell->_aabb->_pos.y+ cell->_aabb->_size.y; data[compt++]= color.r; data[compt++]= color.g; data[compt++]= color.b; data[compt++]= color.a;
		data[compt++]= cell->_aabb->_pos.x; data[compt++]= cell->_aabb->_pos.y+ cell->_aabb->_size.y; data[compt++]= color.r; data[compt++]= color.g; data[compt++]= color.b; data[compt++]= color.a;
	}
	/*for (int i=0; i<context->_n_pts* context->_n_attrs_per_pts; ++i) {
		std::cout << data[i] << " ; ";
	}
	std::cout << "\n";*/

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool GOL::key_down(SDL_Keycode key) {
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
	else if (key== SDLK_n) {
		next();
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


void GOL::click_cell() {
	_anim= false;

	pt_type pos= _screengl->screen2gl(_input_state->_x, _input_state->_y);
	glm::vec4 coords= glm::inverse(_world2camera)* glm::vec4(float(pos.x), float(pos.y), 0.0f, 1.0f);
	if (coords.x< 0.0 || coords.x>= _cell_size* number(_width) || coords.y< 0.0 || coords.y>= _cell_size* number(_height)) {
		return;
	}
	int col= int(coords.x/ _cell_size);
	int row= int(coords.y/ _cell_size);
	Cell * cell= get_cell(col, row);
	if (_input_state->get_key(SDLK_LSHIFT)) {
		cell->_state= DEAD;
	}
	else {
		cell->_state= ALIVE;
	}
	update_cell();
}


bool GOL::mouse_button_down() {
	if (_input_state->_left_mouse) {
		click_cell();
		return true;
	}
	return false;
}


bool GOL::mouse_motion() {
	if (_input_state->_left_mouse) {
		click_cell();
		return true;
	}
	return false;
}
