#ifndef FPS_COUNT_H
#define FPS_COUNT_H

#include <SDL2/SDL.h>

#include "typedefs.h"


struct FPSCount {
	FPSCount();
	FPSCount(SDL_Window * window);
	~FPSCount();
	void add_frame();
	void update();


	SDL_Window * _window;
	uint _val_fps;
	uint _compt_fps;
	uint _tikfps1, _tikfps2;
};


#endif
