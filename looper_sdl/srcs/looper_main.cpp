
#include <stdlib.h>
#include <iostream>

#include <SDL2/SDL.h>

#include "looper_sdl.h"

using namespace std;


SDL_Window * window= 0;
SDL_Renderer* renderer= 0;
bool done= false;
LooperSDL * looper= 0;
int SCREEN_WIDTH= 1024;
int SCREEN_HEIGHT= 512;


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

	window= SDL_CreateWindow("looper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	renderer= SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	looper= new LooperSDL(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

	/*Event * e1= looper->_tracks[0]->insert_event(1, chrono::milliseconds(500), false);
	e1->set_end(chrono::milliseconds(900));
	Event * e2= looper->_tracks[0]->insert_event(2, chrono::milliseconds(500), false);
	e2->set_end(chrono::milliseconds(900));
	looper->_tracks[0]->update(chrono::milliseconds(600));
	looper->debug();
	done= true;*/
}


void idle() {
	looper->update();
	looper->draw();
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

	SDL_DestroyWindow(window);
	SDL_Quit();
}


int main() {
	init();
	main_loop();
	clean();
	
	return 0;
}
