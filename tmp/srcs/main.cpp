#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <math.h>
#include <stdlib.h>
#include <thread>

#include <OpenGL/gl3.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl_utils.h"
#include "thread.h"


using namespace std;


const int SCREEN_WIDTH= 1024;
const int SCREEN_HEIGHT= 1024;
const int WINDOW_X= 10;
const int WINDOW_Y= 10;
const float GL_WIDTH= 20.0f;
const float GL_HEIGHT= GL_WIDTH* (float)(SCREEN_HEIGHT)/ (float)(SCREEN_WIDTH);

SDL_Window * window= 0;
SDL_GLContext main_context;
bool done= false;
ScreenGL * screengl;

GLuint prog_2d;
GLuint vao;
GLint camera2clip_loc, model2world_loc, z_loc, position_loc, color_loc;
glm::mat4 camera2clip, model2world;
GLuint vbo;

float angle_rot;
unsigned int rmask, gmask, bmask, amask;
chrono::system_clock::time_point start_point;
chrono::system_clock::time_point t1, t2;
unsigned int png_compt;

SafeQueue<unsigned char *> safe_queue;
thread thr;
mutex mtx;
atomic_bool stop_thr= ATOMIC_VAR_INIT(false);


void key_down(SDL_Keycode key) {
	if (key== SDLK_ESCAPE) {
		done= true;
		return;
	}

	/*if (key== SDLK_SPACE) {
		for (int j=0; j<SCREEN_HEIGHT; ++j) {
			for (int i=0; i<SCREEN_WIDTH; ++i) {
				cout << +pixel_data[3*(j* SCREEN_WIDTH+ i)+ 0] << ";" << +pixel_data[3*(j* SCREEN_WIDTH+ i)+ 1] << ";" << +pixel_data[3*(j* SCREEN_WIDTH+ i)+ 2];
				cout << " ";
			}
			cout << "\n";
		}
	}*/
}


void key_up(SDL_Keycode key) {
}


void init_sdl() {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	window= SDL_CreateWindow("tmp", WINDOW_X, WINDOW_Y, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
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
	prog_2d= create_prog("../../shaders/vertexshader_2d_aabb.txt", "../../shaders/fragmentshader_basic.txt");
	check_gl_error(); // verif que les shaders ont bien été compilés - linkés
}


void init_buffer() {
	glUseProgram(prog_2d);
	position_loc= glGetAttribLocation(prog_2d, "position_in");
	color_loc= glGetAttribLocation(prog_2d, "color_in");
	camera2clip_loc= glGetUniformLocation(prog_2d, "camera2clip_matrix");
	model2world_loc= glGetUniformLocation(prog_2d, "model2world_matrix");
	z_loc= glGetUniformLocation(prog_2d, "z");
	glUseProgram(0);

	camera2clip= glm::ortho(-screengl->_gl_width* 0.5f, screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f, screengl->_gl_height* 0.5f, -1.0f, 1.0f);
	angle_rot= 0.0f;
	model2world= glm::rotate(glm::mat4(1.0f), angle_rot, glm::vec3(0.f, 0.f, 1.f));

	glGenBuffers(1, &vbo);

	float data[5* 3];
	float ray= 3.0f;
	float k[3]= {0.0f, 1.0f, 2.0f};
	
	data[0]= ray* cos(2.0f* M_PI* k[0]/ 3.0f);
	data[1]= ray* sin(2.0f* M_PI* k[0]/ 3.0f);
	data[2]= 1.0f;
	data[3]= 0.0f;
	data[4]= 0.0f;

	data[5]= ray* cos(2.0f* M_PI* k[1]/ 3.0f);
	data[6]= ray* sin(2.0f* M_PI* k[1]/ 3.0f);
	data[7]= 0.0f;
	data[8]= 1.0f;
	data[9]= 0.0f;

	data[10]= ray* cos(2.0f* M_PI* k[2]/ 3.0f);
	data[11]= ray* sin(2.0f* M_PI* k[2]/ 3.0f);
	data[12]= 0.0f;
	data[13]= 0.0f;
	data[14]= 1.0f;

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 5* 3* sizeof(float), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}


void write_png_thread() {
	unsigned char * pixel_data;
	SDL_Surface * surf;
	string png_path;
	
	while (true) {
		if (stop_thr) {
			break;
		}

		if (safe_queue.next(pixel_data)) {
			int pitch= SCREEN_WIDTH* 3;
			surf= SDL_CreateRGBSurfaceFrom((void*)(pixel_data), SCREEN_WIDTH, SCREEN_HEIGHT, 24, pitch, rmask, gmask, bmask, amask);
			
			/*chrono::system_clock::duration t= chrono::system_clock::now()- start_point;
			unsigned int ms= chrono::duration_cast<chrono::milliseconds>(t).count();
			png_path= "../data/out_"+ to_string(ms)+ ".png";*/

			ostringstream ss;
			ss << setw(5) << setfill('0') << png_compt;
			png_path= "../data/out_"+ ss.str()+ ".png";
			png_compt++;

			IMG_SavePNG(surf, png_path.c_str());
			SDL_FreeSurface(surf);
			delete[] pixel_data;
		}
	}
}


void init() {
	init_sdl();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	init_program();

	screengl= new ScreenGL(SCREEN_WIDTH, SCREEN_HEIGHT, GL_WIDTH, GL_HEIGHT);

	init_buffer();

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
	#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
	#endif

	start_point= chrono::system_clock::now();
	system("rm ../data/*.png");
	png_compt= 0;

	thr= thread(write_png_thread);
}


void draw() {
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, screengl->_screen_width, screengl->_screen_height);

	glUseProgram(prog_2d);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glUniformMatrix4fv(camera2clip_loc, 1, GL_FALSE, glm::value_ptr(camera2clip));
	glUniformMatrix4fv(model2world_loc, 1, GL_FALSE, glm::value_ptr(model2world));
	glUniform1f(z_loc, 0.0f);
	
	glEnableVertexAttribArray(position_loc);
	glEnableVertexAttribArray(color_loc);

	glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
	glVertexAttribPointer(color_loc, 4, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(position_loc);
	glDisableVertexAttribArray(color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	SDL_GL_SwapWindow(window);
}


void update() {
	angle_rot+= 0.01f;
	while (angle_rot> M_PI* 2.0f) {
		angle_rot-= M_PI* 2.0f;
	}
	model2world= glm::rotate(glm::mat4(1.0f), angle_rot, glm::vec3(0.0f, 0.0f, 1.0f));
}


void save2file() {
	glReadBuffer(GL_BACK);
	unsigned char * pixel_data= new unsigned char[SCREEN_WIDTH* SCREEN_HEIGHT* 3];
	glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);
	safe_queue.push(pixel_data);
}


void compute_fps() {
	t2= chrono::system_clock::now();
	auto d= chrono::duration_cast<chrono::milliseconds>(t2- t1).count();
	t1= t2;
	SDL_SetWindowTitle(window, to_string(d).c_str());
}


void idle() {
	update();
	draw();
	save2file();
	compute_fps();
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
