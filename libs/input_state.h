
#ifndef INPUT_STATE_H
#define INPUT_STATE_H

#include <map>

#include <SDL2/SDL_keycode.h>

#include "typedefs.h"


class InputState {
public:
	InputState();
	~InputState();
	void update_mouse(int x, int y, int xrel, int yrel, bool left_mouse, bool middle_mouse, bool right_mouse);
	void update_mouse(int x, int y, bool left_mouse, bool middle_mouse, bool right_mouse);
	void update_wheel(int x_wheel, int y_wheel);
	void key_down(SDL_Keycode key);
	void key_up(SDL_Keycode key);
	bool get_key(SDL_Keycode key);
	void joystick_down(unsigned int button_idx);
	void joystick_up(unsigned int button_idx);
	void joystick_axis(unsigned int axis_idx, int value);


	bool _left_mouse;
	bool _middle_mouse;
	bool _right_mouse;
	int _x, _y, _xrel, _yrel;
	int _x_wheel, _y_wheel;
	std::map<SDL_Keycode, bool> _keys;
	bool _is_joystick; // joystick est-t'il activé
	pt_2d _joystick; // valeurs x, y stick joystick
	bool _joystick_a, _joystick_b; // boutons 
};


#endif
