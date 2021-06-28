
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <istream>
#include <cmath>
#include <string>
#include <vector>

#include <OpenGL/gl3.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "constantes.h"
#include "repere.h"
#include "utile.h"
#include "gl_utils.h"
#include "input_state.h"
#include "dungeon.h"
#include "geom.h"


using namespace std;


SDL_Window * window= NULL;
SDL_GLContext main_context;
InputState * input_state;

bool done= false;
float bck_factor= 1.0f;

unsigned int val_fps, compt_fps;
unsigned int tikfps1, tikfps2, tikanim1, tikanim2;

GLuint prog_basic, prog_repere, prog_select;
GLuint g_vao;

ViewSystem * view_system;

Dungeon * dungeon;



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
	
	window= SDL_CreateWindow("Dungeon", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	main_context= SDL_GL_CreateContext(window);

	cout << "OpenGL version=" << glGetString(GL_VERSION) << endl;
	/*int x= 0;
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &x); // 32
	cout << x << endl;*/

	SDL_GL_SetSwapInterval(1);
	// meme couleur que le brouillard
	glClearColor(MAIN_BCK[0], MAIN_BCK[1], MAIN_BCK[2], MAIN_BCK[3]);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

	glPointSize(4.0f);
	
	SDL_GL_SwapWindow(window);
	
	// --------------------------------------------------------------------------
	/* VAO = vertex array object : tableau d'objets, chaque appel à un objet rappelle un contexte de dessin
	incluant tous les attribute array setup (glVertexAttribArray), buffer objects used for attribute arrays
	et GL_ELEMENT_ARRAY_BUFFER eventuellement
	ici je n'en utilise qu'un pour tout le prog ; à terme peut-être faire plusieurs VAOs
	*/
	
	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);

	prog_basic           = create_prog("../../shaders/vertexshader_basic.txt"       , "../../shaders/fragmentshader_basic.txt");
	prog_repere          = create_prog("../../shaders/vertexshader_repere.txt"      , "../../shaders/fragmentshader_basic.txt");
	prog_select          = create_prog("../../shaders/vertexshader_select.txt"      , "../../shaders/fragmentshader_basic.txt");

	/*float eye_direction[]= {0.0f, 0.0f, 1.0f};
	GLuint progs_eye[]= {prog_3d_anim, prog_3d_terrain, prog_3d_obj};
	for (unsigned int i=0; i<sizeof(progs_eye)/ sizeof(progs_eye[0]); ++i) {
		GLint eye_direction_loc= glGetUniformLocation(progs_eye[i], "eye_direction");
		glUseProgram(progs_eye[i]);
		glUniform3fv(eye_direction_loc, 1, eye_direction);
		glUseProgram(0);
	}*/

	// verif que les shaders ont bien été compilés - linkés
	check_gl_error();
	
	// --------------------------------------------------------------------------
	view_system= new ViewSystem(prog_repere, prog_select, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
	view_system->_repere->_is_ground= false;
	view_system->_repere->_is_repere= true;
	view_system->_repere->_is_box= true;
	view_system->set(glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 1.0f, 200.0f);

	// --------------------------------------------------------------------------
	input_state= new InputState();

	float x= 50.0f;
	dungeon= new Dungeon(glm::vec3(-x, -x, -x* 0.5f), glm::vec3(x, x, x* 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), prog_repere, prog_repere);
	dungeon->randomize();
}


void draw() {
	compt_fps++;

	glClearColor(MAIN_BCK[0]* bck_factor, MAIN_BCK[1]* bck_factor, MAIN_BCK[2]* bck_factor, MAIN_BCK[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
	
	view_system->draw();
	dungeon->draw(view_system->_world2clip);

	SDL_GL_SwapWindow(window);
}


void anim() {
	tikanim2= SDL_GetTicks();
	int tikanim_delta= tikanim2- tikanim1;
	if (tikanim_delta< DELTA_ANIM)
		return;
	
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
	delete view_system;
	delete input_state;
	delete dungeon;

	SDL_GL_DeleteContext(main_context);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
}

// ------------------------------------------------------------------------
int main(int argc, char * argv[]) {

	init();
	main_loop();
	clean();

	
	/*
	glm::vec3 v0= glm::vec3(416, 899, 865); glm::vec3 v1= glm::vec3(412, 791, 403); glm::vec3 v2= glm::vec3(486, 414, 943);
	glm::vec3 w0= glm::vec3(665, 244, 757); glm::vec3 w1= glm::vec3(345, 6, 942); glm::vec3 w2= glm::vec3(48, 951, 379);
	glm::vec3 v[]= {v0, v1, v2};
	glm::vec3 w[]= {w0, w1, w2};
	bool x= triangle_intersects_triangle(v, w);
	cout << x << "\n";
*/

/*
	ifstream infile("/Volumes/Vol-Users/Users/pmbeau2/Desktop/tmp/tmp.txt");
	float x1, y1, z1, x2, y2, z2, x3, y3, z3;
	float x4, y4, z4, x5, y5, z5, x6, y6, z6;
	bool b;
	unsigned int compt= 0;
	while (infile >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> x3 >> y3 >> z3 >> x4 >> y4 >> z4 >> x5 >> y5 >> z5 >> x6 >> y6 >> z6 >> b) {
		//if (compt!= 5) {compt++;continue;}
		glm::vec3 v0(x1, y1, z1);
		glm::vec3 v1(x2, y2, z2);
		glm::vec3 v2(x3, y3, z3);
		glm::vec3 w0(x4, y4, z4);
		glm::vec3 w1(x5, y5, z5);
		glm::vec3 w2(x6, y6, z6);
		glm::vec3 v[]= {v0, v1, v2};
		glm::vec3 w[]= {w0, w1, w2};
		cout << "----------\n";
		cout << "debut " << compt << "\n";
		cout << "glm::vec3 v0= glm::" << glm::to_string(v0) << "; glm::vec3 v1= glm::" << glm::to_string(v1) << "; glm::vec3 v2= glm::" << glm::to_string(v2) << ";\n";
		cout << "glm::vec3 w0= glm::" << glm::to_string(w0) << "; glm::vec3 w1= glm::" << glm::to_string(w1) << "; glm::vec3 w2= glm::" << glm::to_string(w2) << ";\n";
		cout << "lx1= [" << v0.x << ", " << v1.x << ", " << v2.x << ", " << v0.x << "]\n";
		cout << "ly1= [" << v0.y << ", " << v1.y << ", " << v2.y << ", " << v0.y << "]\n";
		cout << "lz1= [" << v0.z << ", " << v1.z << ", " << v2.z << ", " << v0.z << "]\n";
		cout << "lx2= [" << w0.x << ", " << w1.x << ", " << w2.x << ", " << w0.x << "]\n";
		cout << "ly2= [" << w0.y << ", " << w1.y << ", " << w2.y << ", " << w0.y << "]\n";
		cout << "lz2= [" << w0.z << ", " << w1.z << ", " << w2.z << ", " << w0.z << "]\n";
		bool x= triangle_intersects_triangle(v, w);
		cout << "pg_inter=" << b << " ; my_inter=" << x << "\n";
		cout << "fin " << compt << "\n";
		if (x!= b) {
			cout << "aie\n";
			break;
		}
		compt++;
	}
*/
/*
	BBox * bbox_1= new BBox(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4(1.0f));
	BBox * bbox_2= new BBox(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.0f)));
	bool x= bbox_intersects_bbox(bbox_1, bbox_2);
	cout << x << "\n";
*/

/*
	float hallway_depth= 2.0f;
	glm::vec3 posf0(10.0f, 10.0f, 30.0f);
	glm::vec3 posf1(20.0f, 10.0f, 30.0f);
	glm::vec3 posf2(20.0f, 50.0f, 20.0f);
	glm::vec3 posf3(10.0f, 50.0f, 20.0f);
	BBox * bbox= new BBox(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(posf1.x- posf0.x, glm::length(glm::vec3(0.0f, posf3.y- posf0.y, posf3.z- posf0.z)), hallway_depth),
		//glm::translate(glm::rotate(glm::mat4(1.0f), atan((posf3.z- posf0.z)/ (posf3.y- posf0.y)), glm::vec3(1.0f, 0.0f, 0.0f)), posf0)
		glm::rotate(glm::translate(glm::mat4(1.0f), posf0), atan((posf3.z- posf0.z)/ (posf3.y- posf0.y)), glm::vec3(1.0f, 0.0f, 0.0f))
	);
	cout << glm::to_string(glm::vec3(posf1.x- posf0.x, glm::length(glm::vec3(0.0f, posf3.y- posf0.y, posf3.z- posf0.z)), hallway_depth)) << "\n";
	cout << atan((posf3.z- posf0.z)/ (posf3.y- posf0.y)) << "\n";
	for (unsigned int i=0; i<8; ++i) {
		cout << glm::to_string(bbox->_pts[i]) << "\n";
	}
*/
	return 0;
}
