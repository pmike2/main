/*

*/


#include "CoreFoundation/CoreFoundation.h"

#include <cstdlib>
#include <sstream>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gl_utils.h"
#include "utile.h"
#include "font.h"
#include "constantes.h"
#include "input_state.h"
#include "anim_2d.h"
#include "bbox_2d.h"


using namespace std;


// ---------------------------------------------------------------------------------------
SDL_Window * window= NULL;
SDL_GLContext main_context;
InputState * input_state;
ScreenGL * screengl;

bool done;
unsigned int val_fps, compt_fps;
unsigned int tikfps1, tikfps2, tikanim1, tikanim2;

GLuint prog_anim_2d, prog_anim_2d_footprint, prog_static_2d, prog_font;
GLuint g_vao;

Font * arial_font;

Level * level;


// ---------------------------------------------------------------------------------------
void mouse_motion(int x, int y, int xrel, int yrel) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, xrel, yrel, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

}


void mouse_button_up(unsigned int x, unsigned int y) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	/*float xf, yf;
	screengl->screen2gl(input_state->_x, input_state->_y, xf, yf);
	cout << "up : " << xf << " ; " << yf << "\n";*/
}


void mouse_button_down(unsigned int x, unsigned int y, unsigned short button) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	/*float xf, yf;
	screengl->screen2gl(input_state->_x, input_state->_y, xf, yf);
	cout << "down : " << xf << " ; " << yf << "\n";*/
}


void key_down(SDL_Keycode key) {
	input_state->key_down(key);

	if (key== SDLK_ESCAPE) {
		done= true;
		return;
	}

	if (level->key_down(input_state, key)) {
		return;
	}

}


void key_up(SDL_Keycode key) {
	input_state->key_up(key);

	if (level->key_up(input_state, key)) {
		return;
	}
}


// ---------------------------------------------------------------------------------------
void init() {
	srand(time(NULL));
	
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF);
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	window= SDL_CreateWindow("anim_2d", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	main_context= SDL_GL_CreateContext(window);

	//cout << "OpenGL version=" << glGetString(GL_VERSION) << endl;

	SDL_GL_SetSwapInterval(1);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	//glDepthRange(0.0f, 1.0f);
	//glEnable(GL_DEPTH_CLAMP);

	glClearColor(MAIN_BCK[0], MAIN_BCK[1], MAIN_BCK[2], MAIN_BCK[3]);
	//glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClear(GL_COLOR_BUFFER_BIT);
	
	// frontfaces en counterclockwise
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	
	// pour gérer l'alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//glPointSize(4.0f);
	
	SDL_GL_SwapWindow(window);

	// --------------------------------------------------------------------------
	/* VAO = vertex array object : tableau d'objets, chaque appel à un objet rappelle un contexte de dessin
	incluant tous les attribute array setup (glVertexAttribArray), buffer objects used for attribute arrays
	et GL_ELEMENT_ARRAY_BUFFER eventuellement
	ici je n'en utilise qu'un pour tout le prog ; à terme peut-être faire plusieurs VAOs
	*/
	
	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);

	prog_font= create_prog("../shaders/vertexshader_font.txt", "../shaders/fragmentshader_font.txt");
	prog_static_2d= create_prog("../shaders/vertexshader_2d_static.txt"  , "../shaders/fragmentshader_2d_static.txt");
	prog_anim_2d  = create_prog("../shaders/vertexshader_2d_anim.txt"  , "../shaders/fragmentshader_2d_anim.txt");
	prog_anim_2d_footprint= create_prog("../shaders/vertexshader_2d_footprint.txt"  , "../shaders/fragmentshader_basic.txt");

	check_gl_error(); // verif que les shaders ont bien été compilés - linkés
	
	// --------------------------------------------------------------------------
	arial_font= new Font(prog_font, "../fonts/Arial.ttf", 24, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
	input_state= new InputState();
	screengl= new ScreenGL(MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, GL_WIDTH, GL_HEIGHT);
	level= new Level(prog_anim_2d, prog_anim_2d_footprint, prog_static_2d, screengl);

	done= false;
	tikfps1= SDL_GetTicks();
	tikanim1= SDL_GetTicks();
	val_fps= 0;
	compt_fps= 0;

	/*AABB_2D * x= new AABB_2D(glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
	x->_velocity= glm::vec2(100.0f, 0.0f);
	AABB_2D * y= new AABB_2D(glm::vec2(2.0f, 0.5f), glm::vec2(4.0f, 1.5f));
	glm::vec2 pt(0.5f, -0.5f);
	glm::vec2 ray_origin(0.5f, 2.5f);
	glm::vec2 ray_dir(0.1f, -1.0f);
	glm::vec2 contact_pt(0.0f);
	glm::vec2 contact_normal(0.0f);
	float t_hit_near= 0.0f;
	float time_step= 0.1f;
	float contact_time= 0.0f;
	
	cout << point_in_aabb(pt, x) << "\n";
	cout << aabb_intersects_aabb(x, y) << "\n";
	cout << ray_intersects_aabb(ray_origin, ray_dir, x, contact_pt, contact_normal, t_hit_near) << "\n";
	cout << glm::to_string(contact_pt) <<  " ; " << glm::to_string(contact_normal) << " ; " << t_hit_near << "\n";
	cout << dynamic_aabb_intersects_aabb(x, y, time_step, contact_pt, contact_normal, contact_time) << "\n";
	cout << glm::to_string(contact_pt) <<  " ; " << glm::to_string(contact_normal) << " ; " << contact_time << "\n";

	delete x;
	delete y;*/
}


// ---------------------------------------------------------------------------------------
// affichage strings opengl
void show_infos() {
	ostringstream font_str;
	font_str.precision(1);
	font_str << fixed;

	float font_scale= 0.6f;
	glm::vec3 font_color= glm::vec3(1.0f, 1.0f, 0.0f);

	font_str.str("");
	font_str << "hello";
	arial_font->draw(font_str.str(), 10.0f, 15.0f, font_scale, font_color);
}


void draw() {
	compt_fps++;
	
	//glClearColor(MAIN_BCK[0], MAIN_BCK[1], MAIN_BCK[2], MAIN_BCK[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);

	show_infos();

	level->draw();

	SDL_GL_SwapWindow(window);
}


void anim() {
	tikanim2= SDL_GetTicks();
	if (tikanim2- tikanim1< DELTA_ANIM)
		return;

	level->anim(float(tikanim2- tikanim1)* 0.001f);

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
		while (SDL_PollEvent(& event)) {
			switch (event.type) {
				case SDL_MOUSEMOTION:
					mouse_motion(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
					break;
					
				case SDL_MOUSEBUTTONUP:
					mouse_button_up(event.button.x, event.button.y);
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
	delete level;
	delete arial_font;
	delete input_state;
	delete screengl;

	SDL_GL_DeleteContext(main_context);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
}



// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
	init();
	main_loop();
	clean();
	
	return 0;
}

