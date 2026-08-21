
#include <iostream>

#include <OpenGL/gl3.h>

#include <SDL2/SDL.h>

#define GLM_FORCE_RADIANS

#include "repere.h"
#include "utile.h"
#include "gl_utils.h"
#include "input_state.h"
#include "typedefs.h"
#include "fps_count.h"

#include "const.h"
#include "strategy.h"


ScreenGL * screengl;
GLSDL * gl_sdl;
InputState * input_state;
FPSCount * fps_count;

Strategy * strategy;


void mouse_motion(int x, int y, int xrel, int yrel, time_point t) {
	uint mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, xrel, yrel, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	if (strategy->mouse_motion(input_state, t)) {
		return;
	}
}


void mouse_button_up(int x, int y, unsigned short button, time_point t) {
	uint mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	if (strategy->mouse_button_up(input_state, t)) {
		return;
	}
}


void mouse_button_down(int x, int y, unsigned short button, time_point t) {
	uint mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	if (strategy->mouse_button_down(input_state, t)) {
		return;
	}
}


void key_down(SDL_Keycode key, time_point t) {
	input_state->key_down(key);

	if (strategy->key_down(input_state, key, t)) {
		return;
	}
}


void key_up(SDL_Keycode key, time_point t) {
	input_state->key_up(key);

	if (strategy->key_up(input_state, key, t)) {
		return;
	}
}


void init() {
	srand(time(NULL));

	gl_sdl = new GLSDL("strategy", MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, true);
	screengl = new ScreenGL(MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, GL_WIDTH, GL_HEIGHT);
	fps_count = new FPSCount(gl_sdl->_window);
	input_state = new InputState();

	time_point now = std::chrono::system_clock::now();
	strategy = new Strategy(screengl, now);
}


void draw() {
	strategy->draw();
	SDL_GL_SwapWindow(gl_sdl->_window);
	fps_count->add_frame();
}


void anim(time_point t) {
	strategy->anim(t);
}


void idle(time_point t) {
	anim(t);
	draw();
	fps_count->update();
}


void main_loop() {
	SDL_Event event;
	bool done = false;

	while (!done) {
		time_point now = std::chrono::system_clock::now();
		
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_MOUSEMOTION:
					mouse_motion(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel, now);
					break;
					
				case SDL_MOUSEBUTTONUP:
					mouse_button_up(event.button.x, event.button.y, event.button.button, now);
					break;
					
				case SDL_MOUSEBUTTONDOWN:
					mouse_button_down(event.button.x, event.button.y, event.button.button, now);
					break;

				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) {
						done = true;
						break;
					}
					key_down(event.key.keysym.sym, now);
					break;
					
				case SDL_KEYUP:
					key_up(event.key.keysym.sym, now);
					break;
					
				case SDL_QUIT:
					done = true;
					break;
					
				default:
					break;
			}
		}
		
		idle(now);
	}
}


void clean() {
	delete strategy;
	delete input_state;
	delete fps_count;
	delete screengl;
	delete gl_sdl;
}


int main() {
	init();
	main_loop();
	clean();

	return 0;
}
