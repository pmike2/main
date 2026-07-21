
#include <iostream>

#include <OpenGL/gl3.h>

#include <SDL2/SDL.h>

#include "repere.h"
#include "utile.h"
#include "gl_utils.h"
#include "input_state.h"
#include "typedefs.h"
#include "fps_count.h"

#include "test_obj.h"


// dimensions écran
const uint MAIN_WIN_WIDTH = 1280;
const uint MAIN_WIN_HEIGHT = 1024;
const number GL_WIDTH = 15.0;
const number GL_HEIGHT = GL_WIDTH * (number)(MAIN_WIN_HEIGHT) / (number)(MAIN_WIN_WIDTH);


GLSDL * gl_sdl;
InputState * input_state;
ViewSystem * view_system;
GLDrawManager * gl_draw_manager;
FPSCount * fps_count;

TestObj * test_obj;


void mouse_motion(int x, int y, int xrel, int yrel, time_point t) {
	uint mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, xrel, yrel, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	if (view_system->mouse_motion(input_state, t)) {
		return;
	}
}


void mouse_button_up(int x, int y, unsigned short button, time_point t) {
	uint mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	if (view_system->mouse_button_up(input_state, t)) {
		return;
	}
}


void mouse_button_down(int x, int y, unsigned short button, time_point t) {
	uint mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	if (view_system->mouse_button_down(input_state, t)) {
		return;
	}
}


void key_down(SDL_Keycode key, time_point t) {
	input_state->key_down(key);

	if (view_system->key_down(input_state, key, t)) {
		return;
	}

	if (test_obj->key_down(input_state, key, t)) {
		return;
	}
}


void key_up(SDL_Keycode key, time_point t) {
	input_state->key_up(key);

	if (view_system->key_up(input_state, key, t)) {
		return;
	}
}



void init() {
	gl_sdl = new GLSDL("test_obj", MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, true);
	fps_count = new FPSCount(gl_sdl->_window);
	GLDrawManager * gl_draw_manager = new GLDrawManager("../data/draw_context.json");
	ScreenGL * screengl = new ScreenGL(MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, GL_WIDTH, GL_HEIGHT);
	input_state = new InputState();
	
	view_system = new ViewSystem(gl_draw_manager, screengl);
	view_system->set(pt_3d(0.0, 0.0, 0.0), M_PI * 0.25, M_PI * 0.25, 30.0);
	//view_system->set_2d(30.0);

	test_obj = new TestObj(gl_draw_manager, view_system);
}


void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
	view_system->draw();
	test_obj->draw();

	SDL_GL_SwapWindow(gl_sdl->_window);
	fps_count->add_frame();
}


void anim(time_point t) {
	test_obj->anim();
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
	delete test_obj;
	delete view_system;
	delete input_state;
	delete gl_draw_manager;
	delete fps_count;
	delete gl_sdl;
}


int main() {
	init();
	main_loop();
	clean();

	return 0;
}
