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


Orientation rand_orientation() {
	unsigned int orientation_idx= rand_int(0, 3);
	Orientation orientation= NORTH;
	if (orientation_idx== 1) {
		orientation= SOUTH;
	}
	else if (orientation_idx== 2) {
		orientation= EAST;
	}
	else if (orientation_idx== 3) {
		orientation= WEST;
	}
	return orientation;
}


Rotation rand_rotation() {
	unsigned int rotation_idx= rand_int(0, 3);
	Rotation rotation= LEFT;
	if (rotation_idx== 1) {
		rotation= RIGHT;
	}
	else if (rotation_idx== 2) {
		rotation= U_TURN;
	}
	else if (rotation_idx== 3) {
		rotation= NO_TURN;
	}
	return rotation;
}


// Turmite -----------------------------------------------------------------------------------------------
Turmite::Turmite() {

}


Turmite::Turmite(std::map<std::string, GLuint> progs, ScreenGL * screengl, InputState * input_state, time_point t) :
	Grid(progs, screengl, input_state, t)
{
	for (unsigned int row=0; row<_height; ++row) {
		for (unsigned int col=0; col<_width; ++col) {
			Cell * cell= new Cell(col, row);
			cell->_aabb->_pos= idx2pt(col, row)+ pt_2d(_cell_margin);
			cell->_aabb->_size= pt_2d(_cell_size- 2.0* _cell_margin);
			_cells.push_back(cell);
		}
	}

	_rules= NULL;
	randomize();
}


Turmite::~Turmite() {
	delete[] _rules;
}


void Turmite::next() {
	Rule r= _rules[_current_machine_state* _n_cell_states+ _current_cell->_state];
	_current_cell->_state= r._write_color;
	_current_machine_state= r._next_machine_state;
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


void Turmite::randomize() {
	_current_cell= get_cell(_width/ 2, _height/ 2);
	_current_machine_state= 0;
	_orientation= rand_orientation();

	_n_machine_states= rand_int(N_MACHINE_STATES_MIN, N_MACHINE_STATES_MAX);
	_n_cell_states= rand_int(N_CELL_STATES_MIN, N_CELL_STATES_MAX);

	if (_rules!= NULL) {
		delete[] _rules;
	}
	_rules= new Rule[_n_machine_states* _n_cell_states];

	for (int idx_machine_state=0; idx_machine_state<_n_machine_states; ++idx_machine_state) {
		for (int idx_cell_state=0; idx_cell_state<_n_cell_states; ++idx_cell_state) {
			unsigned int write_color= rand_int(0, _n_cell_states- 1);
			Rotation rotation= rand_rotation();
			unsigned int next_state= rand_int(0, _n_machine_states- 1);
			Rule r{write_color, rotation, next_state};
			_rules[idx_machine_state* _n_cell_states+ idx_cell_state]= r;
		}
	}

	std::map<unsigned int, glm::vec4> cell_states;
	for (int idx_cell_state=0; idx_cell_state<_n_cell_states; ++idx_cell_state) {
		if (idx_cell_state== 0) {
			cell_states[idx_cell_state]= glm::vec4(0.0, 0.0, 0.0, 1.0);
		}
		else {
			cell_states[idx_cell_state]= glm::vec4(rand_number(0.3, 1.0), rand_number(0.3, 1.0), rand_number(0.3, 1.0), 1.0);
		}
	}
	set_cell_states(cell_states);
	
	clear();
}


bool Turmite::key_down(SDL_Keycode key) {
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
