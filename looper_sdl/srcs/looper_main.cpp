
#include <stdlib.h>
#include <iostream>
#include <thread>

#include <SDL2/SDL.h>

#include "looper_sdl.h"

using namespace std;


SDL_Window * window= 0;
SDL_Renderer* renderer= 0;
bool done= false;
LooperSDL * looper= 0;
int SCREEN_WIDTH= 1024;
int SCREEN_HEIGHT= 512;
thread thr;
atomic_bool stop_thr= ATOMIC_VAR_INIT(false);


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


void update_thread() {
	while (true) {
		if (stop_thr) {
			break;
		}
		looper->update();
	}
}


void init() {
	srand(time(NULL));

	//SDL_Init(SDL_INIT_EVENTS);
	SDL_Init(SDL_INIT_EVERYTHING);

	window= SDL_CreateWindow("looper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	renderer= SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	// activation alpha blending
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	looper= new LooperSDL(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

	unsigned int n= 16;
	for (unsigned int i=0; i<n; ++i) {
		// 97 = 'a'
		looper->_tracks[1]->insert_event(97, (DEFAULT_TRACK_DURATION* i)/ n, 0);
	}
	//Event * e1= looper->_tracks[1]->get_event_at(chrono::milliseconds(250));
	//e1->set_end(chrono::milliseconds(400));
	//looper->_tracks[1]->update(chrono::milliseconds(310));
	//looper->debug();
	//looper->_tracks[1]->insert_event(1, chrono::milliseconds(300), 0, false, 4);
	//looper->_tracks[1]->update(chrono::milliseconds(310));
	//looper->debug();
	//done= true;

	thr= thread(update_thread);
}


void idle() {
	//looper->update();
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
	stop_thr= true;
	thr.join();
	
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
