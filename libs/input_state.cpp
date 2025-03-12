#include <iostream>

#include "input_state.h"

using namespace std;


InputState::InputState() : 
	_left_mouse(false), _middle_mouse(false), _right_mouse(false),
	_x(0), _y(0), _xrel(0), _yrel(0),
	_is_joystick(false), _joystick(pt_type(0.0, 0.0)), _joystick_a(false), _joystick_b(false)
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


void InputState::update_wheel(int x_wheel, int y_wheel) {
	_x_wheel= x_wheel;
	_y_wheel= y_wheel;
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


void InputState::joystick_down(unsigned int button_idx) {
	if (!_is_joystick) {
		return;
	}
	
	if (button_idx== 0) {
		_joystick_a= true;
	}
	else if (button_idx== 1) {
		_joystick_b= true;
	}
}


void InputState::joystick_up(unsigned int button_idx) {
	if (!_is_joystick) {
		return;
	}
	
	if (button_idx== 0) {
		_joystick_a= false;
	}
	else if (button_idx== 1) {
		_joystick_b= false;
	}
}


void InputState::joystick_axis(unsigned int axis_idx, int value) {
	if (!_is_joystick) {
		return;
	}

	// le joy droit a les axis_idx 2 et 3 qui ne sont pas gérés pour l'instant
	if (axis_idx> 1) {
		return;
	}

	// -1 < fvalue < 1
	number fvalue= number(value)/ 32768.0;
	_joystick[axis_idx]= fvalue;
}
