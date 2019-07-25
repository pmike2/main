
#include "CoreFoundation/CoreFoundation.h"

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <string>
#include <sys/time.h>

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "constantes.h"
#include "utile.h"
#include "gl_error.h"
#include "repere.h"
#include "audio2video.h"


using namespace std;


// --------------------------------------------

SDL_Window* window= NULL;
SDL_GLContext mainContext;
bool fullscreen= false;

// --------------------------------------------

float mouse_angle_x, mouse_angle_y;
float zoom;

unsigned int val_fps, compt_fps;
Uint32 tikfps1, tikfps2, tikanim1, tikanim2;

// --------------------------------------------

GLuint prog_draw, prog_repere;
GLuint g_vao;

float camera2clip[16];
float world2camera[16];
float world2clip[16];
float eye_direction[3];

GLuint eye_direction_loc, world2clip_loc;

// --------------------------------------------

Repere * repere;
Audio2Video * a2v;

// --------------------------------------------




void recompute_world2camera(){
	memset(world2camera, 0, sizeof(float) * 16);
	world2camera[0]= cos(mouse_angle_y* PI_DIV_180);
	world2camera[1]= cos(mouse_angle_x* PI_DIV_180)* sin(mouse_angle_y* PI_DIV_180);
	world2camera[2]= sin(mouse_angle_x* PI_DIV_180)* sin(mouse_angle_y* PI_DIV_180);
	world2camera[3]= 0.;
	world2camera[4]= -sin(mouse_angle_y* PI_DIV_180);
	world2camera[5]=  cos(mouse_angle_x* PI_DIV_180)* cos(mouse_angle_y* PI_DIV_180);
	world2camera[6]=  sin(mouse_angle_x* PI_DIV_180)* cos(mouse_angle_y* PI_DIV_180);
	world2camera[7]= 0.;
	world2camera[8]= 0.;
	world2camera[9]= -sin(mouse_angle_x* PI_DIV_180);
	world2camera[10]= cos(mouse_angle_x* PI_DIV_180);
	world2camera[11]= 0.;
	world2camera[12]= 0.;
	world2camera[13]= 0.;
	world2camera[14]= zoom;
	world2camera[15]= 1.;

	glm::mat4 glm_world2camera= glm::make_mat4(world2camera);
	glm::mat4 glm_camera2clip = glm::make_mat4(camera2clip);
	glm::mat4 glm_world2clip  = glm_camera2clip* glm_world2camera;
	memcpy(world2clip, glm::value_ptr(glm_world2clip), sizeof(float) * 16);
}


void mouse_motion(int xrel, int yrel){

	// si mouvement souris ET click droit
	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_RMASK)
	{
		mouse_angle_x+= (float)(yrel)/ 1.0;
		while (mouse_angle_x> 360.0) mouse_angle_x-= 360.0;
		while (mouse_angle_x< 0.0) mouse_angle_x+= 360.0;
		
		mouse_angle_y+= (float)(xrel)/ 1.0;
		while (mouse_angle_y> 360.0) mouse_angle_y-= 360.0;
		while (mouse_angle_y< 0.0) mouse_angle_y+= 360.0;

		recompute_world2camera();
	}
	// si mouvement souris ET click milieu
	else if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MMASK)
	{

	}
	// si mouvement souris ET click gauche
	else if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK)
	{
		zoom+= (float)(yrel)/ 10.0;
		recompute_world2camera();		
	}
}

void mouse_button_up(unsigned int x, unsigned int y){
}

void mouse_button_down(unsigned int x, unsigned int y){
}


void init() {
	GLuint vertex_shader_draw, fragment_shader_draw, vertex_shader_repere, fragment_shader_repere;

	srand(time(NULL));
	
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF);
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	mainContext = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(1);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
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
	glPointSize(20.0);
	SDL_GL_SwapWindow(window);
	
	glEnable(GL_BLEND);
	// la couleur de l'objet va être (1-alpha_de_l_objet) * couleur du fond et (le_reste * couleur originale)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	memset(camera2clip, 0, sizeof(float) * 16);
	glm::mat4 glm_frustum= glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, FRUSTUM_NEAR, FRUSTUM_FAR);
	memcpy(camera2clip, glm::value_ptr(glm_frustum), sizeof(float) * 16);
	
	eye_direction[0]= 0.;
	eye_direction[1]= 0.;
	eye_direction[2]= 1.;

	mouse_angle_x= 0.;
	mouse_angle_y= 0.;
	zoom= -10.0;
	
	// --------------------------------------------------------------------------
	/* VAO = vertex array object : tableau d'objets, chaque appel à un objet rappelle un contexte de dessin
	 incluant tous les attribute array setup (glVertexAttribArray), buffer objects used for attribute arrays
	 et GL_ELEMENT_ARRAY_BUFFER eventuellement
	 */
	 
	 // ESSAYER DE LE COMMENTER !
	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);
	
	// --------------------------------------------------------------------------
	vertex_shader_draw= load_shader(GL_VERTEX_SHADER, "vertexshader_draw.txt");
	fragment_shader_draw= load_shader(GL_FRAGMENT_SHADER, "fragmentshader_draw.txt");
	prog_draw= glCreateProgram();
	glAttachShader(prog_draw, vertex_shader_draw);
	glAttachShader(prog_draw, fragment_shader_draw);
	glBindAttribLocation(prog_draw, 0, "position_in"); // a faire avant link !
	glBindAttribLocation(prog_draw, 1, "color_in");
	glBindAttribLocation(prog_draw, 2, "normal_in");
	glLinkProgram(prog_draw);
	check_gl_program(prog_draw);

	world2clip_loc= glGetUniformLocation(prog_draw, "world2clip_matrix");
	eye_direction_loc= glGetUniformLocation(prog_draw, "eye_direction");

	glUseProgram(prog_draw);
	glUniform3fv(eye_direction_loc, 1, eye_direction);
	glUseProgram(0);
	
	// --------------------------------------------------------------------------
	vertex_shader_repere= load_shader(GL_VERTEX_SHADER, "vertexshader_repere.txt");
	fragment_shader_repere= load_shader(GL_FRAGMENT_SHADER, "fragmentshader_repere.txt");
	prog_repere= glCreateProgram();
	glAttachShader(prog_repere, vertex_shader_repere);
	glAttachShader(prog_repere, fragment_shader_repere);
	glBindAttribLocation(prog_repere, 0, "position_in");
	glBindAttribLocation(prog_repere, 1, "color_in");
	glLinkProgram(prog_repere);
	check_gl_program(prog_repere);

	// --------------------------------------------------------------------------
	recompute_world2camera();
	
	check_gl_error(); // verif que init() s'est bien passé

	// --------------------------------------------------------------------------
	repere= new Repere(prog_repere);
	
	a2v= new Audio2Video(prog_draw, prog_repere);
}


void draw() {
	compt_fps++;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	repere->draw(world2clip);
	a2v->draw(world2clip);

	SDL_GL_SwapWindow(window);
}


void anim() {
	tikanim2= SDL_GetTicks();
	if (tikanim2- tikanim1 > DELTA_ANIM)
	{
		tikanim1= SDL_GetTicks();
		a2v->anim(world2camera, camera2clip);
	}
}


void compute_fps() {
	tikfps2= SDL_GetTicks();
	if (tikfps2- tikfps1 > 1000) {
		char s_fps[256];

		tikfps1= SDL_GetTicks();
		val_fps= compt_fps;
		compt_fps= 0;
		sprintf(s_fps, "%d", val_fps);
		SDL_SetWindowTitle(window, s_fps);
	}
}


void idle()
{
	a2v->idle();
	anim();
	draw();
	compute_fps();
}



void main_loop()
{
	SDL_Event event;
	int done= 0;
	
	while (!done) {
		while (SDL_PollEvent(&event)) {
			switch (event.type)
			{
				case SDL_MOUSEMOTION:
					mouse_motion(event.motion.xrel, event.motion.yrel);
					break;
					
				case SDL_MOUSEBUTTONUP:
					mouse_button_up(event.button.x, event.button.y);
					break;
					
				case SDL_MOUSEBUTTONDOWN:
					mouse_button_down(event.button.x, event.button.y);
					break;

				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE: // esc : sortie programme
							done= 1;
							break;			
						case SDLK_d:
							if (a2v->is_debug) {
								a2v->is_debug= false;
								repere->is_active= false;
							}
							else {
								a2v->is_debug= true;
								repere->is_active= true;							
							}
							break;
						case SDLK_p:
							a2v->print();
							break;
						case SDLK_f:
							if (fullscreen) {
								fullscreen= false;
								SDL_SetWindowFullscreen(window, 0);
							}
							else {
								fullscreen= true;
								SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
								//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
							}
							break;
            		case SDLK_SPACE: // tests
							//a2v->config.randomize();
							break;
						default:
							break;
					}
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
	SDL_GL_DeleteContext(mainContext);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	
	a2v->release();
}


int main(void)
{
	init();
	main_loop();
	clean();

	return 0;
}

