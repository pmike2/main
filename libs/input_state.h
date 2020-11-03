
#ifndef INPUT_STATE_H
#define INPUT_STATE_H

#include <map>

#include <SDL2/SDL_keycode.h>


class InputState {
public:
	bool _left_mouse;
	bool _middle_mouse;
	bool _right_mouse;
	int _x, _y, _xrel, _yrel;
	std::map<SDL_Keycode, bool> _keys;


	InputState();
	~InputState();
	void update_mouse(int x, int y, int xrel, int yrel, bool left_mouse, bool middle_mouse, bool right_mouse);
	void update_mouse(int x, int y, bool left_mouse, bool middle_mouse, bool right_mouse);
	void key_down(SDL_Keycode key);
	void key_up(SDL_Keycode key);
	bool get_key(SDL_Keycode key);
};


#endif
