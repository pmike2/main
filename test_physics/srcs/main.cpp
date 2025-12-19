/*

*/


#include "CoreFoundation/CoreFoundation.h"

#include <iostream>
#include <sstream>
#include <string>

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "gl_utils.h"
#include "utile.h"
#include "font.h"
#include "constantes.h"
#include "input_state.h"
#include "physics_2d.h"


using namespace std;



// ---------------------------------------------------------------------------------------
SDL_Window * window= NULL;
SDL_GLContext main_context;
InputState * input_state;
ScreenGL * screengl;

bool done;
unsigned int val_fps, compt_fps;
unsigned int tikfps1, tikfps2, tikanim1, tikanim2, tikphysics1, tikphysics2;
unsigned int accumulator;
bool verbose;

GLuint prog_2d, prog_font;
GLuint g_vao;

Font * arial_font;

Physics2D * physics_2d;
DebugPhysics2D * debug_physics_2d;

glm::vec2 user_force_begin;
glm::vec2 user_force_end;


// ---------------------------------------------------------------------------------------
void mouse_motion(int x, int y, int xrel, int yrel) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, xrel, yrel, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

}


void mouse_button_up(unsigned int x, unsigned int y) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	float xf, yf;
	screengl->screen2gl(input_state->_x, input_state->_y, xf, yf);
	//cout << "up : " << xf << " ; " << yf << "\n";
	user_force_end= glm::vec2(xf, yf);
	physics_2d->new_external_force(user_force_begin, user_force_end);
}


void mouse_button_down(unsigned int x, unsigned int y, unsigned short button) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

	float xf, yf;
	screengl->screen2gl(input_state->_x, input_state->_y, xf, yf);
	//cout << "down : " << xf << " ; " << yf << "\n";
	user_force_begin= glm::vec2(xf, yf);
}


void key_down(SDL_Keycode key) {
	input_state->key_down(key);

	if (key== SDLK_ESCAPE) {
		done= true;
	}
	else if (key== SDLK_a) {
		physics_2d->add_body(0, 2, glm::vec2(0.0f, 9.0f), 0.0f);
		//physics_2d->add_body(0, 2, glm::vec2(0.0f, 9.0f), M_PI* 0.5f);
	}
	else if (key== SDLK_b) {
		physics_2d->add_body(1, 2, glm::vec2(0.0f, 9.0f), 0.0f);
	}
	else if (key== SDLK_c) {
		Polygon2D * poly= new Polygon2D();
		poly->randomize(50, rand_float(0.1f, 2.5f), pt_2d(0.0), true);
		//physics_2d->_polygons.erase(physics_2d->_polygons.begin()+ 1);
		physics_2d->add_polygon(poly);
		//physics_2d->add_body(1, rand_int(1, physics_2d->_materials.size()- 1), glm::vec2(0.0f, 8.0f), 0.0f);
		for (int i=0; i<1; ++i) {
			physics_2d->add_body(physics_2d->_polygons.size()- 1, 2, glm::vec2(rand_float(-6.0f, 6.0f), 8.0f), 0.0f);
		}
		physics_2d->_bodies[physics_2d->_bodies.size()- 1]->save("last.txt");
	}
	else if (key== SDLK_l) {
		physics_2d->load_body("last.txt", 2);
	}
	else if (key== SDLK_w) {
		physics_2d->_warm_starting_enabled= !physics_2d->_warm_starting_enabled;
		cout << "warm start : " << physics_2d->_warm_starting_enabled << "\n";
	}
	else if (key== SDLK_p) {
		physics_2d->_paused= !physics_2d->_paused;
	}
	else  if (key== SDLK_d) {
		for (auto it_body : physics_2d->_bodies) {
			it_body->print();
			cout << "---------\n";
		}
	}
	else if (key== SDLK_r) {
		for (int i=physics_2d->_bodies.size()- 1; i>=1; --i) {
			physics_2d->_bodies.erase(physics_2d->_bodies.begin()+ i);
		}
	}
	else if (key== SDLK_n) {
		physics_2d->_paused= false;
		physics_2d->step(true);
		physics_2d->_paused= true;
	}
	else if (key== SDLK_v) {
		debug_physics_2d->_visible_normal= !debug_physics_2d->_visible_normal;
		debug_physics_2d->_visible_center= !debug_physics_2d->_visible_center;
		debug_physics_2d->_visible_vel_force= !debug_physics_2d->_visible_vel_force;
		debug_physics_2d->_visible_collision= !debug_physics_2d->_visible_collision;
		debug_physics_2d->_visible_contact= !debug_physics_2d->_visible_contact;
	}
	else if (key== SDLK_e) {
		float xf, yf;
		screengl->screen2gl(input_state->_x, input_state->_y, xf, yf);
		physics_2d->new_explosion(glm::vec2(xf, yf), 6.0f);
	}
}


void key_up(SDL_Keycode key) {
	input_state->key_up(key);

}


// ---------------------------------------------------------------------------------------
void init() {
	/*glm::vec2 result;
	glm::vec2 pt1_begin(0.0f, 0.0f);
	glm::vec2 pt1_end(1.0f, 0.0f);
	glm::vec2 pt2_begin(0.5f, -0.5f);
	glm::vec2 pt2_end(0.5f, 0.5f);
	bool x= segment_intersects_segment(pt1_begin, pt1_end, pt2_begin, pt2_end, &result);
	cout << x << " ; " << glm::to_string(result) << "\n";*/

	/*glm::vec2 result;
	glm::vec2 pt_begin(0.0f, 0.0f);
	glm::vec2 pt_end(1.0f, 0.0f);
	Polygon2D * poly= new Polygon2D();
	float pts[]= {0.2f, -0.2f, 0.7f, -0.2f, 0.5f, 0.5f};
	poly->set_points(pts, 3);
	bool x= segment_intersects_poly(pt_begin, pt_end, poly, &result);
	cout << x << " ; " << glm::to_string(result) << "\n";*/

	/*Polygon2D * poly= new Polygon2D();
	float pts[]= {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
	poly->set_points(pts, 4);
	Material * material= new Material(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec2 position(0.0f, 0.0f);
	float orientation= 0.0f;
	RigidBody2D * body= new RigidBody2D(poly, material, position, orientation);
	glm::vec2 pt(10.0f, 0.0f);
	//bool x= is_pt_inside_body(pt, body);
	glm::vec2 proj;
	float x= distance_body_pt(body, pt, &proj);
	cout << x << " ; " << glm::to_string(proj) << "\n";*/


	srand(time(NULL));
	
	SDL_Init(SDL_INIT_EVERYTHING);
	//IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF);
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	window= SDL_CreateWindow("sandbox", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	main_context= SDL_GL_CreateContext(window);

	//cout << "OpenGL version=" << glGetString(GL_VERSION) << endl;

	SDL_GL_SetSwapInterval(1);

	glClearColor(MAIN_BCK[0], MAIN_BCK[1], MAIN_BCK[2], MAIN_BCK[3]);
	//glClearDepth(1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	//glEnable(GL_DEPTH_TEST);
	//glDepthMask(GL_TRUE);
	//glDepthFunc(GL_LESS);
	//glDepthFunc(GL_LEQUAL); // ne fonctionne pas je ne sais pas pourquoi; mais necessaire pour bumpmapping et autres
	//glDepthRange(0.0f, 1.0f);
	
	// frontfaces en counterclockwise
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_CLAMP);
	
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

	prog_2d  = create_prog("../../shaders/vertexshader_2d_alpha.txt"  , "../../shaders/fragmentshader_basic.txt");
	prog_font= create_prog("../../shaders/vertexshader_font.txt", "../../shaders/fragmentshader_font.txt");

	check_gl_error(); // verif que les shaders ont bien été compilés - linkés
	
	// --------------------------------------------------------------------------
	screengl= new ScreenGL(MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, GL_WIDTH, GL_HEIGHT);
	arial_font= new Font(prog_font, "../../fonts/Arial.ttf", 24, screengl);
	input_state= new InputState();

	done= false;
	tikfps1= SDL_GetTicks();
	tikanim1= SDL_GetTicks();
	tikphysics1= SDL_GetTicks();
	val_fps= 0;
	compt_fps= 0;
	accumulator= 0;

	physics_2d= new Physics2D(DT_PHYSICS);
	debug_physics_2d= new DebugPhysics2D(physics_2d, prog_2d, screengl);

	
	Polygon2D * poly_poutre= new Polygon2D();
	poly_poutre->set_rectangle(pt_2d(0.0, 1.0), pt_2d(5.0, 1.0));
	
	Polygon2D * poly_bloc= new Polygon2D();
	poly_bloc->set_rectangle(pt_2d(0.0, 1.0), pt_2d(2.0, 2.0));
	
	physics_2d->add_polygon(poly_poutre);
	physics_2d->add_polygon(poly_bloc);

	physics_2d->add_body(0, 0, glm::vec2(0.0f, -9.0f), 0.0f);
	//physics_2d->add_body(0, 0, glm::vec2(-8.0f, 0.0f), M_PI* 0.5f);
	//physics_2d->add_body(0, 0, glm::vec2(8.0f, 0.0f), M_PI* 0.5f);

	//physics_2d->add_body(0, 2, glm::vec2(0.0f, 9.0f), 0.0f);
	//physics_2d->add_body(0, 2, glm::vec2(0.0f, 9.0f), M_PI* 0.5f);
	

	//physics_2d->_bodies[0]->save(physics_2d->_bodies[0], "test.txt");
	//physics_2d->load_body("test.txt", 0);

	/*
	Polygon2D * poly= new Polygon2D();
	float points2[]= {0.0f, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f};
	poly->set_points(points2, 3);
	physics_2d->add_polygon(poly);
	*/

	/*Polygon2D * poly_ref= new Polygon2D();
	float points[]= {-4.0f, -4.0f, 4.0f, -4.0f, 4.0f, 4.0f, -4.0f, 4.0f};
	poly_ref->set_points(points, 4);
	physics_2d->add_polygon(poly_ref);
	physics_2d->add_body(0, 1, glm::vec2(-1.0f, 1.0f), -1.0f);
	physics_2d->add_body(0, 1, glm::vec2(2.0f, -2.0f), 1.0f);

	Polygon2D * poly_incid= new Polygon2D();
	float points2[]= {-4.0f, 4.0f, 0.0f, -4.0f, 4.0f, 4.0f};
	poly_incid->set_points(points2, 3);
	physics_2d->add_polygon(poly_incid);
	//physics_2d->add_body(1, 1, glm::vec2(0.0f, 6.0f), 0.0f);

	physics_2d->_paused= true;*/

	/*
	physics_2d->load_body("body_a.txt", 2);
	physics_2d->load_body("body_b.txt", 2);
	physics_2d->_paused= true;
	Collision2D * collision= new Collision2D(physics_2d->_bodies[0], physics_2d->_bodies[1], true);
	*/
}


// ---------------------------------------------------------------------------------------
// affichage strings opengl
void show_infos() {
	/*ostringstream font_str;
	font_str.precision(1);
	font_str << fixed;

	glm::vec2 position(10.0f, 15.0f);
	float font_scale= 0.6f;
	glm::vec4 font_color(1.0f, 1.0f, 0.0f, 0.5f);

	font_str.str("");
	font_str << "hello";
	Text t(font_str.str(), position, font_scale, font_color);
	arial_font->set_text_group(0, t);
	arial_font->draw();*/
}


void draw() {
	compt_fps++;
	
	glClearColor(MAIN_BCK[0], MAIN_BCK[1], MAIN_BCK[2], MAIN_BCK[3]);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);

	debug_physics_2d->draw();

	show_infos();

	SDL_GL_SwapWindow(window);
}


void anim() {
	tikanim2= SDL_GetTicks();
	if (tikanim2- tikanim1< DELTA_ANIM)
		return;

	tikanim1= SDL_GetTicks();

}


void physics() {
	// fixed time step cf https://gafferongames.com/post/fix_your_timestep/
	tikphysics2= SDL_GetTicks();
	unsigned int delta_tik= tikphysics2- tikphysics1;
	// pour éviter spiral of death
	if (delta_tik> 250) {
		delta_tik= 250;
		cout << "spiral_death\n";
	}
	accumulator+= delta_tik;
	tikphysics1= SDL_GetTicks();
	while (accumulator> DT_PHYSICS_MS) {
		physics_2d->step(verbose);
		accumulator-= DT_PHYSICS_MS;
	}
	// on interpole le visuel pour adoucir l'anim
	debug_physics_2d->update(float(accumulator)/ float(DT_PHYSICS_MS));
	
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
	physics();
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
	delete debug_physics_2d;
	delete physics_2d;

	delete arial_font;
	delete input_state;

	SDL_GL_DeleteContext(main_context);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
}



// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
	/*if ((string(argv[1])== "t") || (string(argv[1])== "true")) {
		verbose= true;
	}
	else {
		verbose= false;
	}*/
	verbose= false;
	init();
	main_loop();
	clean();
	
	return 0;
}

