
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
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "constantes.h"
#include "gl_error.h"
#include "utile.h"
#include "repere.h"
#include "springs.h"


using namespace std;



// --------------------------------------------

SDL_Window* window= NULL;
SDL_GLContext mainContext;

unsigned int val_fps, compt_fps;
unsigned int tikfps1, tikfps2, tikanim1, tikanim2;

GLuint prog_draw, prog_repere;
GLuint g_vao;

float eye_direction[3];
GLuint eye_direction_loc, world2clip_loc;

Repere * repere;
SpringSystemGL * ssgl;
SpringSystemGenetic * ssg;
unsigned int is_debug= 0;
bool is_paused= false;

ViewSystem * view_system;

/*


TODO
refaire ce code avec view_system !!!

*/



// --------------------------------------------
void mouse_motion(int x, int y, int xrel, int yrel) {
	// si mouvement souris ET click gauche
	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) {
		view_system->move_target(-(float)(xrel)* 0.1f, (float)(yrel)* 0.1f);
		view_system->recompute_world2camera();
	}
	// si mouvement souris ET click milieu
	else if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MMASK) {
		view_system->move_dist(-(float)(yrel)* 1.0f);
		view_system->recompute_world2camera();
	}
	// si mouvement souris ET click droit
	else if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_RMASK) {
		view_system->move_alpha((float)(yrel)* 0.01f);
		view_system->move_theta(-(float)(xrel)* 0.01f);
		view_system->recompute_world2camera();
	}
}


void mouse_button_up(unsigned int x, unsigned int y) {

}


void mouse_button_down(unsigned int x, unsigned int y) {

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

	window = SDL_CreateWindow("springs", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	mainContext = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(1);
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
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

	SDL_GL_SwapWindow(window);

	memset(camera2clip, 0, sizeof(float) * 16);
	glm::mat4 glm_frustum= glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, FRUSTUM_NEAR, FRUSTUM_FAR);
	memcpy(camera2clip, glm::value_ptr(glm_frustum), sizeof(float) * 16);
	//glm::mat4 glm_ortho= glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, FRUSTUM_NEAR, FRUSTUM_FAR);
	//memcpy(camera2clip, glm::value_ptr(glm_ortho), sizeof(float) * 16);

	eye_direction[0]= 0.0f;
	eye_direction[1]= 0.0f;
	eye_direction[2]= 1.0f;

	mouse_angle_x= 0.0f;
	mouse_angle_y= 0.0f;
	zoom= -5.0f;

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

	ssgl= new SpringSystemGL(prog_draw);
	ssgl->_ss= new CubeSystem();
	//ssgl->_ss->rand_disposition(N_CUBES);
	ssgl->_ss->load("/Volumes/Cezanne/Cezanne/perso_dev/ovh/www/codes/springs/best.txt");
}


void draw() {
	compt_fps++;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	repere->draw(world2clip);
	ssgl->draw(world2clip);

	SDL_GL_SwapWindow(window);
}


void anim() {
	tikanim2= SDL_GetTicks();
	if (tikanim2- tikanim1 > DELTA_ANIM)
	{
		tikanim1= SDL_GetTicks();
		ssgl->anim();
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
	if (!is_paused)
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
					mouse_motion(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
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
						case SDLK_SPACE: // tests
							ssgl->_ss->rand_disposition(N_CUBES);
							break;
						case SDLK_a:
							ssgl->_is_draw_accel_speed= !ssgl->_is_draw_accel_speed;
							break;
						case SDLK_f:
							ssgl->_is_draw_forces= !ssgl->_is_draw_forces;
							break;
						case SDLK_g:
							repere->_is_ground= !repere->_is_ground;
							break;
						case SDLK_p:
							is_paused= !is_paused;
							break;
						case SDLK_r:
							repere->_is_repere= !repere->_is_repere;
							break;
						case SDLK_s:
							ssgl->_is_draw_springs= !ssgl->_is_draw_springs;
							break;
						case SDLK_z:
							ssgl->_ss->rand_contracts();
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
}


void test1() {
	init();
	main_loop();
	clean();
}


void test2() {
	ssg= new SpringSystemGenetic();
	for (unsigned int i=0; i<N_GENERATIONS; i++) {
		cout << i << endl;
		ssg->next_gen();
	}
	ssg->save_best("/Volumes/Cezanne/Cezanne/perso_dev/ovh/www/codes/springs/best.txt");

}


void test3() {
	CubeSystem* cs= new CubeSystem();
	cs->rand_disposition(N_CUBES);
	cs->rand_contracts();
	cs->save("/Volumes/Cezanne/Cezanne/perso_dev/ovh/www/codes/springs/best.txt");
}


// -----------------------------------------------------------------------------
int main(void) {
	test1();
	//test2();
	//test3();

	return 0;
}
