
#include <iostream>

#include <OpenGL/gl3.h>

#include <SDL2/SDL.h>

#include "repere.h"
#include "utile.h"
#include "gl_utils.h"
#include "input_state.h"
#include "typedefs.h"

#include "elements_gl.h"


// dimensions écran
const int MAIN_WIN_WIDTH= 1280;
const int MAIN_WIN_HEIGHT= 1024;
const float GL_WIDTH= 15.0f;
const float GL_HEIGHT= GL_WIDTH* (float)(MAIN_WIN_HEIGHT)/ (float)(MAIN_WIN_WIDTH);

SDL_Window * window= NULL;
SDL_GLContext main_context;
InputState * input_state;
ViewSystem * view_system;
ScreenGL * screengl;

bool done= false;

unsigned int val_fps, compt_fps;
unsigned int tikfps1, tikfps2;

GLuint g_vao;

ElementsGL * elements_gl;
bool draw_bbox = true;


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

	if (key == SDLK_b) {
		draw_bbox = !draw_bbox;
	}
}


void key_up(SDL_Keycode key) {
	input_state->key_up(key);

	if (view_system->key_up(input_state, key)) {
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
	
	SDL_GL_SetSwapInterval(1);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	//glDepthFunc(GL_LEQUAL); // ne fonctionne pas je ne sais pas pourquoi; mais necessaire pour bumpmapping et autres
	glDepthRange(0.0f, 1.0f);
	
	// frontfaces en counterclockwise
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_CLAMP);
	
	// pour gérer l'alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	SDL_GL_SwapWindow(window);
	
	// --------------------------------------------------------------------------
	/* VAO = vertex array object : tableau d'objets, chaque appel à un objet rappelle un contexte de dessin
	incluant tous les attribute array setup (glVertexAttribArray), buffer objects used for attribute arrays
	et GL_ELEMENT_ARRAY_BUFFER eventuellement
	ici je n'en utilise qu'un pour tout le prog ; à terme peut-être faire plusieurs VAOs
	*/
	
	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);

	std::map<std::string, GLuint> progs;
	progs["repere"]= create_prog("../../shaders/vertexshader_repere.txt", "../../shaders/fragmentshader_basic.txt");
	progs["select"]= create_prog("../../shaders/vertexshader_select.txt", "../../shaders/fragmentshader_basic.txt");
	progs["font"]= create_prog("../../shaders/vertexshader_font.txt", "../../shaders/fragmentshader_font.txt");
	progs["light"]= create_prog("../../shaders/vertexshader_3d_light.txt", "../../shaders/fragmentshader_3d_light.txt");

	check_gl_error();

	screengl= new ScreenGL(MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, GL_WIDTH, GL_HEIGHT);
	
	// --------------------------------------------------------------------------
	view_system= new ViewSystem(progs, screengl);
	view_system->_repere->_is_ground= false;
	view_system->_repere->_is_repere= true;
	view_system->_repere->_is_box= true;
	view_system->set(glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.0f, 100.0f);

	// --------------------------------------------------------------------------
	input_state= new InputState();

	// --------------------------------------------------------------------------
	elements_gl = new ElementsGL(progs);
}


void draw() {
	compt_fps++;

	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
	view_system->draw();

	if (draw_bbox) {
		elements_gl->draw_bbox(view_system->_world2clip);
	}
	elements_gl->draw_light(view_system->_world2clip, view_system->_eye);

	SDL_GL_SwapWindow(window);
}


void anim() {
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
	delete elements_gl;

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
