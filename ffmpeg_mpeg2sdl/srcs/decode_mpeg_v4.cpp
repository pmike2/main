#include <iostream>
#include <chrono>
#include <vector>
#include <utility>
//#include <thread>

#include <OpenGL/gl3.h>

#include <SDL2/SDL.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl_utils.h"
#include "utile.h"
#include "mpeg.h"
#include "input_state.h"


using namespace std;


const int SCREEN_WIDTH= 1024;
const int SCREEN_HEIGHT= 1024;
const float GL_WIDTH= 20.0f;
const float GL_HEIGHT= GL_WIDTH* (float)(SCREEN_HEIGHT)/ (float)(SCREEN_WIDTH);
const float Z_NEAR= -10.0f;
const float Z_FAR= 10.0f;


SDL_Window * window= 0;
SDL_GLContext main_context;
GLuint prog_movie;
GLuint vao;
GLuint vbo;
GLint camera2clip_loc, model2world_loc, position_loc, screen_width_loc, screen_height_loc,
	movie_loc, alpha_loc, movie_time_loc, index_time_loc, index_movie_loc, global_alpha_loc, modifier_loc;
glm::mat4 camera2clip;
glm::mat4 model2world;

ScreenGL * screengl;
InputState * input_state;
bool done= false;
chrono::system_clock::time_point t1, t2;

MPEGReaders * mpeg_readers;



void init_sdl() {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	window= SDL_CreateWindow("decode_mpeg", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
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
	
	// je multiplie rgb par alpha dans le fragment shader, donc pas besoin de GL_BLEND qui me faisait galérer
	// pour gérer l'alpha
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
	
	//glPointSize(4.0f);
	
	SDL_GL_SwapWindow(window);

	screengl= new ScreenGL(SCREEN_WIDTH, SCREEN_HEIGHT, GL_WIDTH, GL_HEIGHT);
	input_state= new InputState();
}


void init_vao_vbo() {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);

	float vertices[18];
	float x= -0.5f* screengl->_gl_width;
	float y= -0.5f* screengl->_gl_height;
	float z= 0.0f;
	float w= screengl->_gl_width;
	float h= screengl->_gl_height;

	vertices[0]= x;
	vertices[1]= y+ h;
	vertices[2]= z;

	vertices[3]= x;
	vertices[4]= y;
	vertices[5]= z;

	vertices[6]= x+ w;
	vertices[7]= y;
	vertices[8]= z;

	vertices[9]= x;
	vertices[10]= y+ h;
	vertices[11]= z;

	vertices[12]= x+ w;
	vertices[13]= y;
	vertices[14]= z;

	vertices[15]= x+ w;
	vertices[16]= y+ h;
	vertices[17]= z;

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 18* sizeof(float), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void init_program() {
	// cf https://community.khronos.org/t/2-samplers-fs-fails-different-sampler-types-for-same-sample-texture-unit-in-fragment/72598
	// il faut faire les glUniform1i avant check_gl_program, d'où l'option check de create_prog à false, et check_gl_program + tard
	prog_movie= create_prog("../../shaders/vertexshader_movie.txt"  , "../../shaders/fragmentshader_movie.txt", false);
	camera2clip= glm::ortho(-screengl->_gl_width* 0.5f, screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f, screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
	model2world= glm::mat4(1.0f);

	glUseProgram(prog_movie);
	camera2clip_loc= glGetUniformLocation(prog_movie, "camera2clip_matrix");
	model2world_loc= glGetUniformLocation(prog_movie, "model2world_matrix");
	screen_width_loc= glGetUniformLocation(prog_movie, "screen_width");
	screen_height_loc= glGetUniformLocation(prog_movie, "screen_height");
	movie_loc= glGetUniformLocation(prog_movie, "movie");
	alpha_loc= glGetUniformLocation(prog_movie, "alpha");
	movie_time_loc= glGetUniformLocation(prog_movie, "movie_time");
	index_time_loc= glGetUniformLocation(prog_movie, "index_time");
	index_movie_loc= glGetUniformLocation(prog_movie, "index_movie");
	global_alpha_loc= glGetUniformLocation(prog_movie, "global_alpha");
	modifier_loc= glGetUniformLocation(prog_movie, "modifier");
	position_loc= glGetAttribLocation(prog_movie, "position_in");

	unsigned int base_index= 0;
	mpeg_readers= new MPEGReaders(base_index, movie_loc, alpha_loc, movie_time_loc, index_time_loc, index_movie_loc,
		global_alpha_loc, modifier_loc);
	//mpeg_readers->randomize();
	mpeg_readers->load_json("../data/config_01.json");
	
	//exit(0);	
	
	glUseProgram(0);
	check_gl_program(prog_movie);
	check_gl_error(); // verif que les shaders ont bien été compilés - linkés
}


void init() {
	srand(time(NULL));

	init_sdl();
	init_vao_vbo();
	init_program();
}


void draw() {
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	glUseProgram(prog_movie);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	mpeg_readers->prepare2draw();

	glUniform1i(screen_width_loc, SCREEN_WIDTH);
	glUniform1i(screen_height_loc, SCREEN_HEIGHT);
	glUniformMatrix4fv(camera2clip_loc, 1, GL_FALSE, glm::value_ptr(camera2clip));
	glUniformMatrix4fv(model2world_loc, 1, GL_FALSE, glm::value_ptr(model2world));
	
	glEnableVertexAttribArray(position_loc);

	glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 3* sizeof(float), (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(position_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glBindTexture(GL_TEXTURE_3D, 0);
	
	SDL_GL_SwapWindow(window);
}


void update() {
	mpeg_readers->update();
}


void compute_fps() {
	t2= chrono::system_clock::now();
	auto d= chrono::duration_cast<chrono::milliseconds>(t2- t1).count();
	t1= t2;

	char s_fps[256];
	sprintf(s_fps, "%lld", d);
	SDL_SetWindowTitle(window, s_fps);
}


void idle() {
	update();
	draw();
	compute_fps();
}

void key_down(SDL_Keycode key) {
	if (key== SDLK_ESCAPE) {
		done= true;
		return;
	}
	if (key== SDLK_r) {
		mpeg_readers->randomize();
	}
	for (unsigned int i=0; i<8; ++i) {
		if (key== SDLK_a+ i) {
			// ici pour tester la juxtaposition on déclenche sur des tracks distinctes
			if (!input_state->get_key(key)) {
				mpeg_readers->note_on(i, 'a'+ i);
			}
			input_state->key_down(key);
			break;
		}
	}
}


void key_up(SDL_Keycode key) {
	input_state->key_up(key);
	for (unsigned int i=0; i<8; ++i) {
		if (key== SDLK_a+ i) {
			mpeg_readers->note_off(i);
			break;
		}
	}
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
	delete mpeg_readers;
	delete screengl;
	delete input_state;
	SDL_GL_DeleteContext(main_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


int main(int argc, char **argv) {
	init();
	main_loop();
	clean();
	
	return 0;
}


