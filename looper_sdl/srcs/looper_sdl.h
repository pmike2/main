#ifndef LOOPER_SDL_H
#define LOOPER_SDL_H

#include <iostream>

#include <SDL2/SDL_keycode.h>

#include "looper.h"


class LooperSDL : public Sequence {
public:
	LooperSDL();
	~LooperSDL();
	void key_down(SDL_Keycode key);
	void key_up(SDL_Keycode key);

};


#endif
