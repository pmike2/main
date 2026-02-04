
#include <iostream>

#include <OpenGL/gl3.h>

#include <SDL2/SDL.h>

#define GLM_FORCE_RADIANS

#include "repere.h"
#include "utile.h"
#include "gl_utils.h"
#include "input_state.h"
#include "typedefs.h"

#include "strategy.h"


// dimensions écran
const int MAIN_WIN_WIDTH= 1280;
const int MAIN_WIN_HEIGHT= 1024;
const float GL_WIDTH= 10.0f;
const float GL_HEIGHT= GL_WIDTH* (float)(MAIN_WIN_HEIGHT)/ (float)(MAIN_WIN_WIDTH);


SDL_Window * window= NULL;
SDL_GLContext main_context;
InputState * input_state;
ViewSystem * view_system;
GLDrawManager * gl_draw_manager;

bool done= false;

uint val_fps, compt_fps;
uint tikfps1, tikfps2;

Strategy * strategy;


void mouse_motion(int x, int y, int xrel, int yrel, time_point t) {
	uint mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, xrel, yrel, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	if (strategy->mouse_motion(input_state, t)) {
		return;
	}

	if (view_system->mouse_motion(input_state)) {
		return;
	}
}


void mouse_button_up(int x, int y, unsigned short button, time_point t) {
	uint mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	if (view_system->mouse_button_up(input_state)) {
		//return; // on veut récupérer le _rect de view_system dans strategy->mouse_button_up
	}
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

	if (view_system->mouse_button_down(input_state)) {
		return;
	}
}


void key_down(SDL_Keycode key, time_point t) {
	input_state->key_down(key);

	if (key== SDLK_ESCAPE) {
		done= true;
	}

	if (strategy->key_down(input_state, key, t)) {
		return;
	}

	if (view_system->key_down(input_state, key)) {
		return;
	}
}


void key_up(SDL_Keycode key, time_point t) {
	input_state->key_up(key);

	if (strategy->key_up(input_state, key, t)) {
		return;
	}

	if (view_system->key_up(input_state, key)) {
		return;
	}
}


void init() {
	srand(time(NULL));
	
	SDL_Init(SDL_INIT_EVERYTHING);
	//IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF);

	//SDL_ShowCursor(SDL_DISABLE);
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1); // 2, 3 font une seg fault
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	// pour faire du multisampling (suppression aliasing)
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
	
	window= SDL_CreateWindow("strategy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	main_context= SDL_GL_CreateContext(window);

	//std::cout << "OpenGL version=" << glGetString(GL_VERSION) << std::endl;

	/*GLint max_layers;
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_layers);
	std::cout << max_layers << "\n";*/
	
	SDL_GL_SetSwapInterval(1);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glDepthRange(0.0f, 1.0f);

	//glPointSize(5.0f);
	
	// frontfaces en counterclockwise
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_CLAMP);
	
	// pour gérer l'alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	SDL_GL_SwapWindow(window);
	
	GLDrawManager * gl_draw_manager = new GLDrawManager("../data/strategy_draw_context.json");
	//std::cout << *gl_draw_manager << "\n";

	ScreenGL * screengl= new ScreenGL(MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, GL_WIDTH, GL_HEIGHT);
	
	// --------------------------------------------------------------------------
	view_system= new ViewSystem(gl_draw_manager, screengl);
	view_system->set(pt_3d(10.0, 10.0, 0.0), M_PI * 0.25, M_PI * 0.25, 70.0);
	//view_system->set_2d(30.0);

	// --------------------------------------------------------------------------
	input_state= new InputState();

	// --------------------------------------------------------------------------
	time_point now= std::chrono::system_clock::now();
	strategy = new Strategy(gl_draw_manager, view_system, now);
}


void draw() {
	compt_fps++;

	glClearColor(0.1, 1.0, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);

	view_system->draw();
	strategy->draw();

	SDL_GL_SwapWindow(window);
}


void anim(time_point t) {
	strategy->anim(t);
}


void compute_fps() {
	tikfps2= SDL_GetTicks();
	if (tikfps2- tikfps1> 1000) {
		char s_fps[256];

		tikfps1= SDL_GetTicks();
		val_fps= compt_fps;
		compt_fps= 0;
		sprintf(s_fps, "%d", val_fps);
		SDL_SetWindowTitle(window, s_fps);
	}
}


void idle(time_point t) {
	anim(t);
	draw();
	compute_fps();
}


void main_loop() {
	SDL_Event event;
	
	while (!done) {
		time_point now= std::chrono::system_clock::now();
		
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
		
		idle(now);
	}
}


void clean() {
	delete strategy;
	delete view_system;
	delete input_state;
	delete gl_draw_manager;

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
