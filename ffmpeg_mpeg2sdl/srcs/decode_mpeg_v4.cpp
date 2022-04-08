#include <iostream>
#include <chrono>
#include <vector>

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


using namespace std;


const int SCREEN_WIDTH= 1024;
const int SCREEN_HEIGHT= 1024;
const float GL_WIDTH= 20.0f;
const float GL_HEIGHT= GL_WIDTH* (float)(SCREEN_HEIGHT)/ (float)(SCREEN_WIDTH);
const float Z_NEAR= -10.0f;
const float Z_FAR= 10.0f;


SDL_Window * window= 0;
SDL_GLContext main_context;
GLuint prog_texture_3d;
GLuint vao;
GLuint vbo;
GLuint texture_index_3d_id;
GLint camera2clip_loc, model2world_loc, position_loc, texture_3d_loc, texture_index_3d_loc, screen_width_loc, screen_height_loc, index_index_loc;
glm::mat4 camera2clip;
glm::mat4 model2world;

ScreenGL * screengl;
bool done= false;
chrono::system_clock::time_point t1, t2;

unsigned int compt= 0;
unsigned int n_index_index= 128;
float index_index= 0.0f;

MPEGTextures * mpeg_textures;


void init_sdl() {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	window= SDL_CreateWindow("decode_mpeg", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	main_context= SDL_GL_CreateContext(window);

	//cout << "OpenGL version=" << glGetString(GL_VERSION) << endl;

	SDL_GL_SetSwapInterval(1);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	//glDepthRange(0.0f, 1.0f);
	//glEnable(GL_DEPTH_CLAMP);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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

	screengl= new ScreenGL(SCREEN_WIDTH, SCREEN_HEIGHT, GL_WIDTH, GL_HEIGHT);

	/*int n;
	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &n);
	cout << n << "\n";*/
	// 2048
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
	prog_texture_3d= create_prog("../../shaders/vertexshader_3d_texture.txt"  , "../../shaders/fragmentshader_3d_texture.txt", false);
	camera2clip= glm::ortho(-screengl->_gl_width* 0.5f, screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f, screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
	model2world= glm::mat4(1.0f);

	glUseProgram(prog_texture_3d);
	camera2clip_loc= glGetUniformLocation(prog_texture_3d, "camera2clip_matrix");
	model2world_loc= glGetUniformLocation(prog_texture_3d, "model2world_matrix");
	texture_3d_loc= glGetUniformLocation(prog_texture_3d, "texture_3d");
	texture_index_3d_loc= glGetUniformLocation(prog_texture_3d, "texture_index_3d");
	screen_width_loc= glGetUniformLocation(prog_texture_3d, "screen_width");
	screen_height_loc= glGetUniformLocation(prog_texture_3d, "screen_height");
	index_index_loc= glGetUniformLocation(prog_texture_3d, "index_index");
	//n_frames_loc= glGetUniformLocation(prog_texture_3d, "n_frames");
	position_loc= glGetAttribLocation(prog_texture_3d, "position_in");

	glUniform1i(texture_index_3d_loc, 0);

	int base_index= 1;
	vector<string> mpeg_paths{"../data/flower_04.mov", "../data/flower_06.mov"};
	//vector<string> mpeg_paths{"../data/flower_04.mov"};
	mpeg_textures= new MPEGTextures(mpeg_paths, texture_3d_loc, base_index);
	//glUniform1i(texture_3d_loc, base_index);

	glUseProgram(0);

	check_gl_program(prog_texture_3d);

	check_gl_error(); // verif que les shaders ont bien été compilés - linkés
}


// fonctions test -----------------------------------------
void linear_index(float * data) {
	for (unsigned int j=0; j<n_index_index; ++j) {
		for (unsigned int i=0; i<SCREEN_WIDTH* SCREEN_HEIGHT; ++i) {
			float x= (float)(i % SCREEN_WIDTH)/ (float)(SCREEN_WIDTH);
			float y= (float)(i / SCREEN_WIDTH)/ (float)(SCREEN_HEIGHT);
			data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 0]= x;
			data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 1]= y;
			//data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 2]= (float)(j)/ (float)(n_frames);
			data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 2]= (float)(j)/ (float)(100);
		}
	}
}


void total_rand_index(float * data) {
	for (unsigned int j=0; j<n_index_index; ++j) {
		for (unsigned int i=0; i<SCREEN_WIDTH* SCREEN_HEIGHT; ++i) {
			data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 0]= rand_float(0.0f, 1.0f);
			data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 1]= rand_float(0.0f, 1.0f);
			data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 2]= rand_float(0.0f, 1.0f);
		}
	}
}

/*
void time_rand_index(float * data) {
	for (unsigned int j=0; j<n_index_index; ++j) {
		for (unsigned int i=0; i<SCREEN_WIDTH* SCREEN_HEIGHT; ++i) {
			float x= (float)(i % SCREEN_WIDTH)/ (float)(SCREEN_WIDTH);
			float y= (float)(i / SCREEN_WIDTH)/ (float)(SCREEN_HEIGHT);
			data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 0]= x;
			data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 1]= y;
			data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 2]= rand_float(0.0f, 1.0f);
		}
	}
}


void position_rand_index(float * data) {
	for (unsigned int j=0; j<n_index_index; ++j) {
		for (unsigned int i=0; i<SCREEN_WIDTH* SCREEN_HEIGHT; ++i) {
			data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 0]= rand_float(0.0f, 1.0f);
			data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 1]= rand_float(0.0f, 1.0f);
			data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 2]= (float)(j)/ (float)(n_frames);
		}
	}
}


void smooth_rand_time_index(float * data, int m) {
	for (unsigned int j=0; j<n_index_index; ++j) {
		float r= rand_float(0.0f, 1.0f);
		int k= rand_int(-m, m);
		for (unsigned int i=0; i<SCREEN_WIDTH* SCREEN_HEIGHT; ++i) {
			float x= (float)(i % SCREEN_WIDTH)/ (float)(SCREEN_WIDTH);
			float y= (float)(i / SCREEN_WIDTH)/ (float)(SCREEN_HEIGHT);
			
			data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 0]= x;
			data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 1]= y;
			
			if (j== 0) {
				data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 2]= r;
			}
			else {
				data[(SCREEN_WIDTH* SCREEN_HEIGHT* j+ i)* 3+ 2]= data[(SCREEN_WIDTH* SCREEN_HEIGHT* (j- 1)+ i)* 3+ 2]+ (float)(k)/ (float)(n_index_index);
			}
		}
	}
	for (unsigned int i=0; i<SCREEN_WIDTH* SCREEN_HEIGHT* n_index_index* 3; ++i) {
		if (data[i]< 0.0f) {
			data[i]= 0.0f;
		}
		if (data[i]> 1.0f) {
			data[i]= 1.0f;
		}
	}
}


void mosaic_index(float * data, int size) {
	unsigned int * t= new unsigned int[size* size];
	for (unsigned int i=0; i<size; ++i) {
		for (unsigned int j=0; j<size; ++j) {
			t[i+ j* size]= rand_int(0, n_frames- 1);
		}
	}
	for (unsigned int k=0; k<n_index_index; ++k) {
		for (unsigned int i=0; i<SCREEN_WIDTH; ++i) {
			for (unsigned int j=0; j<SCREEN_HEIGHT; ++j) {
				float x= (float)(i % size)/ (float)(size);
				float y= (float)(j % size)/ (float)(size);
				float z= (float)((t[(i / size)+ (j / size)* size]+ k) % n_frames)/ (float)(n_frames);
				
				data[(SCREEN_WIDTH* SCREEN_HEIGHT* k+ SCREEN_WIDTH* j+ i)* 3+ 0]= x;
				data[(SCREEN_WIDTH* SCREEN_HEIGHT* k+ SCREEN_WIDTH* j+ i)* 3+ 1]= y;
				data[(SCREEN_WIDTH* SCREEN_HEIGHT* k+ SCREEN_WIDTH* j+ i)* 3+ 2]= z;
			}
		}
	}
	delete[] t;
}
*/

// ----------------------------------------------------------------------
void init_texture() {

	// ----------------------------------------
	glGenTextures(1, &texture_index_3d_id);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, texture_index_3d_id);

	float * data_index= new float[SCREEN_WIDTH* SCREEN_HEIGHT* n_index_index* 3];
	linear_index(data_index);
	//total_rand_index(data_index);
	//time_rand_index(data_index);
	//position_rand_index(data_index);
	//smooth_rand_time_index(data_index, 2);
	//mosaic_index(data_index, 100);

	//glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, SCREEN_WIDTH, SCREEN_HEIGHT, n_index_index, 0, GL_RED, GL_FLOAT, data_index);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, SCREEN_WIDTH, SCREEN_HEIGHT, n_index_index, 0, GL_RGB, GL_FLOAT, data_index);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16UI, SCREEN_WIDTH, SCREEN_HEIGHT, n_index_index, 0, GL_RGB, GL_UNSIGNED_SHORT, data_index);
	delete[] data_index;

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_3D, 0);
	glActiveTexture(0);
}


void init() {
	srand(time(NULL));

	init_sdl();
	init_vao_vbo();
	init_program();
	init_texture();
}


void update() {
	compt++;
	if (compt>= n_index_index) {
		compt= 0;
	}
	index_index= (float)(compt)/ (float)(n_index_index);

	/*glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_index_3d_id);
	float data_index[SCREEN_WIDTH* SCREEN_HEIGHT];
	for (unsigned int i=0; i<SCREEN_WIDTH* SCREEN_HEIGHT; ++i) {
		data_index[i]= rand_float(0.0f, 1.0f);
		//data_index[i]= t;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RED, GL_FLOAT, data_index);*/
}


void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	glUseProgram(prog_texture_3d);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture_index_3d_id);

	mpeg_textures->prepare2draw();

	glUniform1i(texture_index_3d_loc, 0);
	glUniform1i(screen_width_loc, SCREEN_WIDTH);
	glUniform1i(screen_height_loc, SCREEN_HEIGHT);
	glUniform1f(index_index_loc, index_index);
	//glUniform1i(n_frames_loc, n_frames);
	glUniformMatrix4fv(camera2clip_loc, 1, GL_FALSE, glm::value_ptr(camera2clip));
	glUniformMatrix4fv(model2world_loc, 1, GL_FALSE, glm::value_ptr(model2world));
	
	glEnableVertexAttribArray(position_loc);

	glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 3* sizeof(float), (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(position_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glBindTexture(GL_TEXTURE_3D, 0);
	//glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(0);
	
	SDL_GL_SwapWindow(window);
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


void main_loop() {
	SDL_Event event;
	
	while (!done) {
		while (SDL_PollEvent(& event)) {
			switch (event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym== SDLK_ESCAPE) {
						done= true;
						return;
					}
					break;
					
				case SDL_KEYUP:
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



