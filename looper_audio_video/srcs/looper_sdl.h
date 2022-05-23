#ifndef LOOPER_SDL_H
#define LOOPER_SDL_H

#include <iostream>
#include <map>

#include <SDL2/SDL.h>

#include "looper.h"
#include "input_state.h"


class LooperSDL : public Sequence {
public:
	LooperSDL();
	LooperSDL(SDL_Renderer * renderer, int screen_width, int screen_height);
	~LooperSDL();
	bool event_key(SDL_Keycode key);
	void key_down(SDL_Keycode key);
	void key_up(SDL_Keycode key);
	void draw();
	SDL_Color get_color(SDL_Keycode key);
	void draw_rect(int x, int y, int w, int h, SDL_Color c);
	void tap_tempo();

	InputState * _input_state;
	SDL_Renderer * _renderer;
	int _screen_width;
	int _screen_height;
	std::map<key_type, SDL_Color> _colors;
	std::chrono::system_clock::time_point _tap;
	unsigned int _ratio_numerator;
};


#endif
