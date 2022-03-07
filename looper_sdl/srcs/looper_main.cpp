
#include <stdlib.h>
#include <iostream>

#include <SDL2/SDL.h>

#include "looper_sdl.h"

using namespace std;


SDL_Window * window= NULL;
SDL_GLContext main_context;
bool done;
LooperSDL * looper;


void key_down(SDL_Keycode key) {
	if (key== SDLK_ESCAPE) {
		done= true;
		return;
	}

	looper->key_down(key);
}


void key_up(SDL_Keycode key) {
	looper->key_up(key);
}


void init() {
	srand(time(NULL));

	//SDL_Init(SDL_INIT_EVENTS);
	SDL_Init(SDL_INIT_EVERYTHING);

	window= SDL_CreateWindow("looper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, SDL_WINDOW_SHOWN);
	main_context= SDL_GL_CreateContext(window);

	done= false;
	looper= new LooperSDL();
	/*looper->_tracks[0]->insert_event(1, std::chrono::milliseconds(100));
	looper->_tracks[0]->insert_event(2, std::chrono::milliseconds(80));
	looper->debug();*/
}


void idle() {
	looper->update();
}


void main_loop() {
	SDL_Event event;
	
	while (!done) {
		while (SDL_PollEvent(& event)) {
			switch (event.type) {
				case SDL_KEYDOWN:
					key_down(event.key.keysym.sym);
					break;
					
				case SDL_KEYUP:
					key_up(event.key.keysym.sym);
					break;
					
				case SDL_QUIT:
					done= true;
					break;
					
				default:
					break;
			}
		}
		idle();
	}
}


void clean() {
	delete looper;

	SDL_GL_DeleteContext(main_context);
	SDL_DestroyWindow(window);
}


int main() {
	init();
	main_loop();
	clean();
	
	return 0;
}
