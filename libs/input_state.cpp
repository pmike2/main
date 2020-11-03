#include <iostream>

#include "input_state.h"

using namespace std;


InputState::InputState() : 
	_left_mouse(false), _middle_mouse(false), _right_mouse(false),
	_x(0), _y(0), _xrel(0), _yrel(0)
{

}


InputState::~InputState() {

}


void InputState::update_mouse(int x, int y, int xrel, int yrel, bool left_mouse, bool middle_mouse, bool right_mouse) {
	_x= x;
	_y= y;
	_xrel= xrel;
	_yrel= yrel;
	_left_mouse= left_mouse;
	_middle_mouse= middle_mouse;
	_right_mouse= right_mouse;
}


void InputState::update_mouse(int x, int y, bool left_mouse, bool middle_mouse, bool right_mouse) {
	_x= x;
	_y= y;
	_left_mouse= left_mouse;
	_middle_mouse= middle_mouse;
	_right_mouse= right_mouse;
}


void InputState::key_down(SDL_Keycode key) {
	_keys[key]= true;
}


void InputState::key_up(SDL_Keycode key) {
	_keys[key]= false;
}


bool InputState::get_key(SDL_Keycode key) {
	if (!_keys.count(key)) {
		return false;
	}
	return _keys[key];
}
