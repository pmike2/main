
#include <iostream>
#include <vector>
#include <string>
#include <OpenGL/gl3.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "repere.h"
#include "utile.h"
#include "gl_utils.h"
#include "input_state.h"
#include "typedefs.h"
#include "fps_count.h"
#include "gl_draw.h"

#include "voro_z.h"


// dimensions écran
const uint MAIN_WIN_WIDTH = 1280;
const uint MAIN_WIN_HEIGHT = 1024;
const number GL_WIDTH = 15.0;
const number GL_HEIGHT = GL_WIDTH * (number)(MAIN_WIN_HEIGHT) / (number)(MAIN_WIN_WIDTH);

GLSDL * gl_sdl;
InputState * input_state;
GLDrawManager * gl_draw_manager;
ViewSystem * view_system;
FPSCount * fps_count;

VoroZ * voroz;


void mouse_motion(int x, int y, int xrel, int yrel, time_point t) {
	uint mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, xrel, yrel, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	if (view_system->mouse_motion(input_state, t)) {
		//return;
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

	if (voroz->key_down(input_state, key)) {
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
	srand(time(NULL));
	
	gl_sdl = new GLSDL("test_obj", MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, true);
	fps_count = new FPSCount(gl_sdl->_window);
	GLDrawManager * gl_draw_manager = new GLDrawManager("../data/draw_context.json");
	ScreenGL * screen_gl = new ScreenGL(MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, GL_WIDTH, GL_HEIGHT);
	input_state= new InputState();

	view_system = new ViewSystem(gl_draw_manager, screen_gl);
	view_system->set(pt_3d(0.0, 0.0, 0.0), 1.0, 1.0, 200.0);

	voroz = new VoroZ(gl_draw_manager, view_system);
}


void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);

	view_system->draw();
	voroz->draw();

	SDL_GL_SwapWindow(gl_sdl->_window);
	fps_count->add_frame();
}


void anim() {
	voroz->anim();
}


void idle() {
	anim();
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
					done= 1;
					break;
					
				default:
					break;
			}
		}
		idle();
	}
}


void clean() {
	delete voroz;
	delete view_system;
	delete input_state;
	delete gl_sdl;
}


int main() {
	init();
	main_loop();
	clean();

	return 0;
}
