
#include <iostream>
#include <math.h>

#include <OpenGL/gl3.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "repere.h"
#include "utile.h"
#include "gl_utils.h"
#include "input_state.h"
#include "asteroid.h"


// en ms; temps entre 2 anims
const unsigned int DELTA_ANIM= 1;

// dimensions écran
const int MAIN_WIN_WIDTH= 1280;
const int MAIN_WIN_HEIGHT= 1024;
const float MAIN_BCK[]= {0.2f, 0.2f, 0.2f, 1.0f};

SDL_Window * window= NULL;
SDL_GLContext main_context;
InputState * input_state;

bool done= false;

unsigned int val_fps, compt_fps;
unsigned int tikfps1, tikfps2, tikanim1, tikanim2;

GLuint prog_repere, prog_select;
GLuint g_vao;

ViewSystem * view_system;

Level * level;



void mouse_motion(int x, int y, int xrel, int yrel) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, xrel, yrel, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	if (view_system->mouse_motion(input_state)) {
		//return;
	}
}


void mouse_button_up(int x, int y, unsigned short button) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	if (view_system->mouse_button_up(input_state)) {
		return;
	}
}


void mouse_button_down(int x, int y, unsigned short button) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	if (view_system->mouse_button_down(input_state)) {
		return;
	}

}


void key_down(SDL_Keycode key) {
	input_state->key_down(key);

	if (key== SDLK_ESCAPE) {
		done= true;
	}

	if (view_system->key_down(input_state, key)) {
		return;
	}

	if (level->key_down(input_state, key)) {
		return;
	}
}


void key_up(SDL_Keycode key) {
	input_state->key_up(key);

	if (view_system->key_up(input_state, key)) {
		return;
	}

	if (level->key_up(input_state, key)) {
		return;
	}
}


void init() {
	srand(time(NULL));
	
	SDL_Init(SDL_INIT_EVERYTHING);
	//IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF);
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1); // 2, 3 font une seg fault
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	window= SDL_CreateWindow("sandbox", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	main_context= SDL_GL_CreateContext(window);

	std::cout << "OpenGL version=" << glGetString(GL_VERSION) << std::endl;
	/*int x= 0;
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &x); // 32
	cout << x << endl;*/

	SDL_GL_SetSwapInterval(1);
	glClearColor(MAIN_BCK[0], MAIN_BCK[1], MAIN_BCK[2], MAIN_BCK[3]);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glDepthRange(0.0f, 1.0f);
	
	// frontfaces en counterclockwise
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_CLAMP);
	
	// pour gérer l'alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPointSize(2.0f);
	
	SDL_GL_SwapWindow(window);
	
	// --------------------------------------------------------------------------
	/* VAO = vertex array object : tableau d'objets, chaque appel à un objet rappelle un contexte de dessin
	incluant tous les attribute array setup (glVertexAttribArray), buffer objects used for attribute arrays
	et GL_ELEMENT_ARRAY_BUFFER eventuellement
	ici je n'en utilise qu'un pour tout le prog ; à terme peut-être faire plusieurs VAOs
	*/
	
	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);

	prog_repere= create_prog("../../shaders/vertexshader_repere.txt", "../../shaders/fragmentshader_basic.txt");
	prog_select= create_prog("../../shaders/vertexshader_select.txt", "../../shaders/fragmentshader_basic.txt");

	check_gl_error();
	
	// --------------------------------------------------------------------------
	view_system= new ViewSystem(prog_repere, prog_select, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
	view_system->_repere->_is_ground= false;
	view_system->_repere->_is_repere= false;
	view_system->_repere->_is_box= false;
	view_system->set(glm::vec3(0.0f, 0.0f, 0.0f), -0.5* M_PI, 0.0f, 40.0f);

	// --------------------------------------------------------------------------
	input_state= new InputState();

	// --------------------------------------------------------------------------
	level= new Level(prog_repere, prog_repere);
}


void draw() {
	compt_fps++;

	glClearColor(MAIN_BCK[0], MAIN_BCK[1], MAIN_BCK[2], MAIN_BCK[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
	
	view_system->draw();
	level->draw(view_system->_world2clip);

	SDL_GL_SwapWindow(window);
}


void anim() {
	tikanim2= SDL_GetTicks();
	int tikanim_delta= tikanim2- tikanim1;
	if (tikanim_delta< DELTA_ANIM)
		return;

	level->anim();
	
	tikanim1= SDL_GetTicks();
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


void idle() {
	anim();
	draw();
	compute_fps();
}


void main_loop() {
	SDL_Event event;
	
	while (!done) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_MOUSEMOTION:
					mouse_motion(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
					break;
					
				case SDL_MOUSEBUTTONUP:
					mouse_button_up(event.button.x, event.button.y, event.button.button);
					break;
					
				case SDL_MOUSEBUTTONDOWN:
					mouse_button_down(event.button.x, event.button.y, event.button.button);
					break;

				case SDL_KEYDOWN:
					key_down(event.key.keysym.sym);
					break;
					
				case SDL_KEYUP:
					key_up(event.key.keysym.sym);
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
	delete level;
	delete view_system;
	delete input_state;

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
