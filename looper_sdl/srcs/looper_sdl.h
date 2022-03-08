#ifndef LOOPER_SDL_H
#define LOOPER_SDL_H

#include <iostream>
#include <map>

#include <SDL2/SDL.h>

#include "looper.h"
#include "input_state.h"


struct Color {
	int _r;
	int _g;
	int _b;
};

/*
class EventSDL : public Event {
public:
	EventSDL();
	~EventSDL();

	Color _color;
};
*/

class LooperSDL : public Sequence {
public:
	LooperSDL();
	LooperSDL(SDL_Renderer * renderer, int screen_width, int screen_height);
	~LooperSDL();
	void key_down(SDL_Keycode key);
	void key_up(SDL_Keycode key);
	void draw();
	Color get_color(SDL_Keycode key);
	void draw_rect(int x, int y, int w, int h, Color c);

	Event * _current_event;
	SDL_Keycode _current_event_key;
	InputState * _input_state;
	SDL_Renderer * _renderer;
	int _screen_width;
	int _screen_height;
	std::map<key_type, Color> _colors;
};


#endif
