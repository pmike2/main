#include "turmite.h"


Orientation rotate(Orientation orientation, Rotation r) {
	if (orientation== NORTH) {
		if (r== LEFT) { return WEST; }
		else if (r== RIGHT) { return EAST; }
		else if (r== U_TURN) { return SOUTH; }
	}
	else if (orientation== SOUTH) {
		if (r== LEFT) { return EAST; }
		else if (r== RIGHT) { return WEST; }
		else if (r== U_TURN) { return NORTH; }
	}
	else if (orientation== EAST) {
		if (r== LEFT) { return NORTH; }
		else if (r== RIGHT) { return SOUTH; }
		else if (r== U_TURN) { return WEST; }
	}
	else if (orientation== WEST) {
		if (r== LEFT) { return SOUTH; }
		else if (r== RIGHT) { return NORTH; }
		else if (r== U_TURN) { return EAST; }
	}
	return orientation;
}


Turmite::Turmite() {

}


Turmite::Turmite(std::map<std::string, GLuint> progs, ScreenGL * screengl, InputState * input_state, time_point t) :
	Grid(TURMITE_CELL_STATES, progs, screengl, input_state, t)
{
	for (unsigned int row=0; row<_height; ++row) {
		for (unsigned int col=0; col<_width; ++col) {
			Cell * cell= new Cell(col, row);
			cell->_aabb->_pos= idx2pt(col, row)+ pt_type(_cell_margin);
			cell->_aabb->_size= pt_type(_cell_size- 2.0* _cell_margin);
			_cells.push_back(cell);
		}
	}

	_current_cell= get_cell(_width/ 2, _height/ 2);
	_state= 0;
	_orientation= NORTH;
}


Turmite::~Turmite() {
	
}


void Turmite::next() {
	Rule r= RULES_2.at(std::make_pair(_state, _current_cell->_state));
	_current_cell->_state= r._write_color;
	_state= r._next_state;
	_orientation= rotate(_orientation, r._rotation);
	if (_orientation== NORTH && _current_cell->_row< _height- 1) {
		_current_cell= get_cell(_current_cell->_col, _current_cell->_row+ 1);
	}
	else if (_orientation== SOUTH && _current_cell->_row> 0) {
		_current_cell= get_cell(_current_cell->_col, _current_cell->_row- 1);
	}
	else if (_orientation== EAST && _current_cell->_col< _width- 1) {
		_current_cell= get_cell(_current_cell->_col+ 1, _current_cell->_row);
	}
	else if (_orientation== WEST && _current_cell->_col> 0) {
		_current_cell= get_cell(_current_cell->_col- 1, _current_cell->_row);
	}
}


void Turmite::anim(time_point t) {
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


bool Turmite::key_down(SDL_Keycode key) {
	if (key== SDLK_n) {
		next();
		update_cell();
		return true;
	}
	if (Grid::key_down(key)) {
		return true;
	}
	return false;
}


bool Turmite::mouse_button_down() {
	if (_input_state->_left_mouse) {
		return true;
	}
	return false;
}


bool Turmite::mouse_motion() {
	if (_input_state->_left_mouse) {
		return true;
	}
	return false;
}
