
#include <stdlib.h>
#include <iostream>
#include <thread>

#include <OpenGL/gl3.h>

#include <SDL2/SDL.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl_utils.h"

#include "looper_gl.h"

using namespace std;


const int SCREEN_WIDTH= 1024;
const int SCREEN_HEIGHT= 512;
const int WINDOW_X= 10;
const int WINDOW_Y= 10;
const float GL_WIDTH= 20.0f;
const float GL_HEIGHT= GL_WIDTH* (float)(SCREEN_HEIGHT)/ (float)(SCREEN_WIDTH);

SDL_Window * window= 0;
SDL_GLContext main_context;
bool done= false;
LooperGL * looper= 0;
ScreenGL * screengl;
thread thr;
atomic_bool stop_thr= ATOMIC_VAR_INIT(false);
mutex mtx;

GLuint prog_2d, prog_font;
GLuint vao;


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


void init_sdl() {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	window= SDL_CreateWindow("looper", WINDOW_X, WINDOW_Y, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	main_context= SDL_GL_CreateContext(window);

	//cout << "OpenGL version=" << glGetString(GL_VERSION) << endl;

	SDL_GL_SetSwapInterval(1);

	/*glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);*/
	glDisable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClearDepth(1.0f);
	
	// frontfaces en counterclockwise
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDisable(GL_BLEND);
	
	//glPointSize(4.0f);
	
	SDL_GL_SwapWindow(window);
}


void init_program() {
	prog_2d= create_prog("../../shaders/vertexshader_2d_alpha.txt", "../../shaders/fragmentshader_basic.txt");
	prog_font= create_prog("../../shaders/vertexshader_font.txt", "../../shaders/fragmentshader_font.txt");
	check_gl_error(); // verif que les shaders ont bien été compilés - linkés
}


void init() {
	init_sdl();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	init_program();

	screengl= new ScreenGL(SCREEN_WIDTH, SCREEN_HEIGHT, GL_WIDTH, GL_HEIGHT);
	looper= new LooperGL(prog_2d, prog_font, screengl);

	thr= thread(update_thread);
}


void idle() {
	//mtx.lock();
	if (!looper->_screen_save) {
		looper->update_vbo_time();
		if (looper->_current_track_changed) {
			unsigned int idx_track= looper->get_current_track_index();
			looper->update_vbo_track_data(idx_track);
		}
		looper->draw();
		SDL_GL_SwapWindow(window);
	}
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
	delete screengl;
	SDL_GL_DeleteContext(main_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


int main() {
	init();
	main_loop();
	clean();
	
	return 0;
}
