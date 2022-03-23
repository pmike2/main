
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
mutex mtx;


void key_down(SDL_Keycode key) {
	if (key== SDLK_ESCAPE) {
		done= true;
		return;
	}

	mtx.lock();
	looper->key_down(key);
	mtx.unlock();
}


void key_up(SDL_Keycode key) {
	mtx.lock();
	looper->key_up(key);
	mtx.unlock();
}


void update_thread() {
	while (true) {
		if (stop_thr) {
			break;
		}
		mtx.lock();
		looper->update();
		mtx.unlock();
	}
}


void init() {
	srand(time(NULL));

	SDL_Init(SDL_INIT_EVERYTHING);

	window= SDL_CreateWindow("looper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	renderer= SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	// activation alpha blending
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	looper= new LooperSDL(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

	/*unsigned int n= 16;
	for (unsigned int i=0; i<n; ++i) {
		// 97 = 'a'
		looper->_tracks[1]->insert_event(97, (DEFAULT_TRACK_DURATION* i)/ n, 0);
	}*/

	/*looper->_tracks[1]->_hold= true;
	looper->_tracks[1]->insert_event(97, time_type::zero(), 1.0f);
	looper->_tracks[1]->set_inserted_event_end(looper->_tracks[1]->_duration- std::chrono::milliseconds(100));
	*/

	//looper->debug();
	//done= true;

	thr= thread(update_thread);
}


void idle() {
	//mtx.lock();
	looper->draw();
	//mtx.unlock();
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
