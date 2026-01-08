#include "gol.h"


// Cell ---------------------------------------------------------------
GolCell::GolCell() {

}


GolCell::GolCell(uint col, uint row) : Cell(col, row), _next_state(0) {
}


GolCell::~GolCell() {
	
}


void GolCell::next() {
	uint n_alive= 0;
	for (auto neighbour : _neighbours) {
		if (neighbour->_state== 1) {
			n_alive++;
		}
	}

	if (n_alive< 2 || n_alive> 3) {
		_next_state= 0;
	}
	else if (n_alive== 2) {
		_next_state= _state;
	}
	else if (n_alive== 3) {
		_next_state= 1;
	}
}


// GOl ----------------------------------------------------------------
GOL::GOL() {
	
}


GOL::GOL(std::map<std::string, GLuint> progs, ScreenGL * screengl, InputState * input_state, time_point t) :
	Grid(progs, screengl, input_state, t)
{
	
	set_cell_states(GOL_CELL_STATES);

	for (uint row=0; row<_height; ++row) {
		for (uint col=0; col<_width; ++col) {
			GolCell * cell= new GolCell(col, row);
			cell->_aabb->_pos= idx2pt(col, row)+ pt_2d(_cell_margin);
			cell->_aabb->_size= pt_2d(_cell_size- 2.0* _cell_margin);
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
}


GOL::~GOL() {
}


void GOL::next() {
	for (auto cell : _cells) {
		GolCell * gol_cell= (GolCell *)(cell);
		gol_cell->next();
	}
	for (auto cell : _cells) {
		GolCell * gol_cell= (GolCell *)(cell);
		gol_cell->_state= gol_cell->_next_state;
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


void GOL::randomize() {
	for (auto cell : _cells) {
		cell->_state= rand_int(0, _cell_states.size()- 1);
	}
}


bool GOL::key_down(SDL_Keycode key) {
	if (key== SDLK_n) {
		next();
		update_cell();
		return true;
	}
	else if (key== SDLK_r) {
		randomize();
		update_cell();
		return true;
	}
	if (Grid::key_down(key)) {
		return true;
	}
	return false;
}


void GOL::click_cell() {
	_anim= false;

	pt_2d pos= _screengl->screen2gl(_input_state->_x, _input_state->_y);
	glm::vec4 coords= glm::inverse(_world2camera)* glm::vec4(float(pos.x), float(pos.y), 0.0f, 1.0f);
	if (coords.x< 0.0 || coords.x>= _cell_size* number(_width) || coords.y< 0.0 || coords.y>= _cell_size* number(_height)) {
		return;
	}
	int col= int(coords.x/ _cell_size);
	int row= int(coords.y/ _cell_size);
	Cell * cell= get_cell(col, row);
	if (_input_state->get_key(SDLK_LSHIFT)) {
		cell->_state= 0;
	}
	else {
		cell->_state= 1;
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
