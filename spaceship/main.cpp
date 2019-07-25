
/*

PMB 09/2018
Jeu d'avion 3D

*/


#include "CoreFoundation/CoreFoundation.h"

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <string>
#include <sys/time.h>
#include <vector>

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "constantes.h"
#include "gl_error.h"
#include "utile.h"
#include "repere.h"
#include "ship.h"
#include "light.h"
#include "level.h"
#include "rand_terrain.h"
#include "font.h"


using namespace std;



// ---------------------------------------------------------------------------------------
SDL_Window* window= NULL;
SDL_GLContext main_context;

float mouse_angle_x, mouse_angle_y;
float zoom;
bool is_paused;
bool is_visu_global;

unsigned int val_fps, compt_fps;
unsigned int tikfps1, tikfps2, tikanim1, tikanim2;

GLuint prog_draw, prog_repere, prog_skybox, prog_draw_instanced, prog_basic, prog_map, prog_font;
GLuint g_vao;

float eye_direction[3];
GLint eye_direction_loc, fog_start_loc, fog_end_loc, fog_color_loc;

ViewSystem * view_system;;
Ship * ship;
RandTerrain * level;
SkyBox * skybox;
vector<Light> lights;
LightsUBO * lights_ubo;
vector<IA *> enemies;
vector<Ship *> all_ships;
vector<Explosion *> little_explosions;
vector<Explosion *> big_explosions;
LevelMap * level_map;
Ranking * ranking;
vector<Cloud *> clouds;
GlobalMsg * global_msg;
vector<string> firstnames;


// ---------------------------------------------------------------------------------------
// vue embarquée
void recompute_world2camera_embarked() {
	
	glm::mat4 glm_world2camera;
	
	// cf https://stackoverflow.com/questions/21866866/glmtranslate-with-local-space
	glm::mat4 rotation= glm::mat4(ship->_follow_camera._rotation_matrix);
	glm::mat4 translation= glm::translate(glm::mat4(1.0f), ship->_follow_camera._position);
	
	glm_world2camera= glm::inverse(glm::lookAt(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	));
	glm_world2camera= rotation* glm_world2camera;
	glm_world2camera= translation* glm_world2camera;
	glm_world2camera= glm::inverse(glm_world2camera);

	memcpy(view_system->_world2camera, glm::value_ptr(glm_world2camera), sizeof(float)* 16);
	
	glm::mat4 glm_camera2clip= glm::make_mat4(view_system->_camera2clip);
	glm::mat4 glm_world2clip = glm_camera2clip* glm_world2camera;
	memcpy(view_system->_world2clip, glm::value_ptr(glm_world2clip), sizeof(float)* 16);
}


void mouse_motion(int x, int y, int xrel, int yrel) {
	if (is_visu_global) {
		return;
	}
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
	GLuint vertex_shader_skybox, fragment_shader_skybox, vertex_shader_draw_instanced, fragment_shader_draw_instanced;
	GLuint vertex_shader_basic, fragment_shader_basic, vertex_shader_map, fragment_shader_map;
	GLuint vertex_shader_font, fragment_shader_font;
	
	Font* arial_font;
	Font* silom_font;

	srand(time(NULL));
	
	SDL_Init(SDL_INIT_EVERYTHING);
	//IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF);
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	window= SDL_CreateWindow("spaceship", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	main_context= SDL_GL_CreateContext(window);

	cout << "OpenGL version=" << glGetString(GL_VERSION) << endl;

	SDL_GL_SetSwapInterval(1);
	// meme couleur que le brouillard
	glClearColor(FOG_COLOR[0], FOG_COLOR[1], FOG_COLOR[2], 0.0f);
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
	
	SDL_GL_SwapWindow(window);
	
	// --------------------------------------------------------------------------
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	eye_direction[0]= 0.0f;
	eye_direction[1]= 0.0f;
	eye_direction[2]= 1.0f;

	mouse_angle_x= -45.0f;
	mouse_angle_y= -45.0f;
	zoom= -100.0f;

	is_paused= false;
	is_visu_global= false;
	
	// --------------------------------------------------------------------------
	/* VAO = vertex array object : tableau d'objets, chaque appel à un objet rappelle un contexte de dessin
	 incluant tous les attribute array setup (glVertexAttribArray), buffer objects used for attribute arrays
	 et GL_ELEMENT_ARRAY_BUFFER eventuellement
	 
	 ici je n'en utilise qu'un pour tout le prog ; à terme peut-être faire plusieurs VAOs
	 */
	
	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);

	// --------------------------------------------------------------------------
	vertex_shader_draw_instanced= load_shader(GL_VERTEX_SHADER, "shaders/vertexshader_draw_instanced.txt");
	fragment_shader_draw_instanced= load_shader(GL_FRAGMENT_SHADER, "shaders/fragmentshader_draw_instanced.txt");
	prog_draw_instanced= glCreateProgram();
	glAttachShader(prog_draw_instanced, vertex_shader_draw_instanced);
	glAttachShader(prog_draw_instanced, fragment_shader_draw_instanced);
	glBindAttribLocation(prog_draw_instanced, 0, "position_in"); // a faire avant link !
	glBindAttribLocation(prog_draw_instanced, 1, "color_in");
	glBindAttribLocation(prog_draw_instanced, 2, "normal_in");
	glBindAttribLocation(prog_draw_instanced, 3, "instanced_matrix"); // prend 4 emplacements : de 3 à 7
	glLinkProgram(prog_draw_instanced);
	check_gl_program(prog_draw_instanced);

	eye_direction_loc= glGetUniformLocation(prog_draw_instanced, "eye_direction");
	fog_start_loc= glGetUniformLocation(prog_draw_instanced, "fog_start");
	fog_end_loc= glGetUniformLocation(prog_draw_instanced, "fog_end");
	
	glUseProgram(prog_draw_instanced);
	glUniform3fv(eye_direction_loc, 1, eye_direction);
	glUniform1f(fog_start_loc, FOG_START);
	glUniform1f(fog_end_loc, FOG_END);
	glUniform3fv(fog_color_loc, 1, FOG_COLOR);
	glUseProgram(0);
	
	// --------------------------------------------------------------------------
	vertex_shader_draw= load_shader(GL_VERTEX_SHADER, "shaders/vertexshader_draw.txt");
	fragment_shader_draw= load_shader(GL_FRAGMENT_SHADER, "shaders/fragmentshader_draw.txt");
	prog_draw= glCreateProgram();
	glAttachShader(prog_draw, vertex_shader_draw);
	glAttachShader(prog_draw, fragment_shader_draw);
	glBindAttribLocation(prog_draw, 0, "position_in"); // a faire avant link !
	glBindAttribLocation(prog_draw, 1, "color_in");
	glBindAttribLocation(prog_draw, 2, "normal_in");
	glLinkProgram(prog_draw);
	check_gl_program(prog_draw);

	eye_direction_loc= glGetUniformLocation(prog_draw, "eye_direction");
	fog_start_loc= glGetUniformLocation(prog_draw, "fog_start");
	fog_end_loc= glGetUniformLocation(prog_draw, "fog_end");
	
	glUseProgram(prog_draw);
	glUniform3fv(eye_direction_loc, 1, eye_direction);
	glUniform1f(fog_start_loc, FOG_START);
	glUniform1f(fog_end_loc, FOG_END);
	glUniform3fv(fog_color_loc, 1, FOG_COLOR);
	glUseProgram(0);
	
	// --------------------------------------------------------------------------
	vertex_shader_repere= load_shader(GL_VERTEX_SHADER, "shaders/vertexshader_repere.txt");
	fragment_shader_repere= load_shader(GL_FRAGMENT_SHADER, "shaders/fragmentshader_repere.txt");
	prog_repere= glCreateProgram();
	glAttachShader(prog_repere, vertex_shader_repere);
	glAttachShader(prog_repere, fragment_shader_repere);
	glBindAttribLocation(prog_repere, 0, "position_in");
	glBindAttribLocation(prog_repere, 1, "color_in");
	glLinkProgram(prog_repere);
	check_gl_program(prog_repere);
	
	// --------------------------------------------------------------------------
	vertex_shader_basic= load_shader(GL_VERTEX_SHADER, "shaders/vertexshader_basic.txt");
	fragment_shader_basic= load_shader(GL_FRAGMENT_SHADER, "shaders/fragmentshader_basic.txt");
	prog_basic= glCreateProgram();
	glAttachShader(prog_basic, vertex_shader_basic);
	glAttachShader(prog_basic, fragment_shader_basic);
	glBindAttribLocation(prog_basic, 0, "position_in");
	glBindAttribLocation(prog_basic, 1, "color_in");
	glLinkProgram(prog_basic);
	check_gl_program(prog_basic);
	
	// --------------------------------------------------------------------------
	vertex_shader_map= load_shader(GL_VERTEX_SHADER, "shaders/vertexshader_map.txt");
	fragment_shader_map= load_shader(GL_FRAGMENT_SHADER, "shaders/fragmentshader_map.txt");
	prog_map= glCreateProgram();
	glAttachShader(prog_map, vertex_shader_map);
	glAttachShader(prog_map, fragment_shader_map);
	glBindAttribLocation(prog_map, 0, "position_in");
	glBindAttribLocation(prog_map, 1, "color_in");
	glLinkProgram(prog_map);
	check_gl_program(prog_map);
	
	// --------------------------------------------------------------------------
	vertex_shader_skybox= load_shader(GL_VERTEX_SHADER, "shaders/vertexshader_skybox.txt");
	fragment_shader_skybox= load_shader(GL_FRAGMENT_SHADER, "shaders/fragmentshader_skybox.txt");
	prog_skybox= glCreateProgram();
	glAttachShader(prog_skybox, vertex_shader_skybox);
	glAttachShader(prog_skybox, fragment_shader_skybox);
	glBindAttribLocation(prog_skybox, 0, "position_in");
	glLinkProgram(prog_skybox);
	check_gl_program(prog_skybox);
	
	// --------------------------------------------------------------------------
	vertex_shader_font= load_shader(GL_VERTEX_SHADER, "shaders/vertexshader_font.txt");
	fragment_shader_font= load_shader(GL_FRAGMENT_SHADER, "shaders/fragmentshader_font.txt");
	prog_font= glCreateProgram();
	glAttachShader(prog_font, vertex_shader_font);
	glAttachShader(prog_font, fragment_shader_font);
	glBindAttribLocation(prog_font, 0, "vertex");
	glLinkProgram(prog_font);
	check_gl_program(prog_font);

	// --------------------------------------------------------------------------
	check_gl_error(); // verif que les shaders ont bien été compilés - linkés
	
	// --------------------------------------------------------------------------
	arial_font= new Font(prog_font, "fonts/Arial.ttf", 48, SCREEN_WIDTH, SCREEN_HEIGHT);
	silom_font= new Font(prog_font, "fonts/Silom.ttf", 48, SCREEN_WIDTH, SCREEN_HEIGHT);

	view_system= new ViewSystem(prog_repere);
	view_system->_repere->_is_repere= false;
	view_system->_repere->_is_ground= false;
	view_system->_repere->_is_box= false;
	view_system->_dist= 100.0f;
	view_system->_alpha= M_PI* 0.2;
	
	ship= new Ship("YOU", prog_draw, prog_basic, "modeles/plane2.obj", "modeles/plane2.mtl", false, SHIP_SIZE_FACTOR, HEROS_COLOR);
	ship->_rigid_body._position.z= 400.0f;
	all_ships.push_back(ship);
	
	ifstream infile("modeles/firstnames.txt");
	string firstname;
	while (infile >> firstname) {
		firstnames.push_back(firstname);
	}
	
	for (unsigned int i=0; i<N_ENEMIES; ++i) {
		//string enemy_name= "enemy_"+ to_string(i);
		string enemy_name= firstnames[rand_int(0, firstnames.size()- 1)];
		
		IA* enemy= new IA(enemy_name, prog_draw, prog_basic, "modeles/plane2.obj", "modeles/plane2.mtl", false, SHIP_SIZE_FACTOR, glm::vec3(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f)));
		
		unsigned int step= (unsigned int)(sqrt(float(N_ENEMIES)));
		unsigned int ii= i% step;
		unsigned int jj= i/ step;
		
		enemy->_ship->_rigid_body._position.x= -REPERE_BOX+ REPERE_BOX/ float(step)+ 2.0f* REPERE_BOX* float(ii)/ float(step);
		enemy->_ship->_rigid_body._position.y= -REPERE_BOX+ REPERE_BOX/ float(step)+ 2.0f* REPERE_BOX* float(jj)/ float(step);
		enemy->_ship->_rigid_body._position.z= rand_float(0.0f, 200.0f)+ 200.0f;

		enemies.push_back(enemy);
		all_ships.push_back(enemy->_ship);
	}
	
	level= new RandTerrain(prog_draw);
	level->gen_altis(ALTI_OFFSET, NLEVELS, GRADIENT_BASE_SIZE, MAX_FACTOR, REDISTRIBUTION_POWER);
	skybox= new SkyBox(prog_skybox);
	
	for (unsigned int i=0; i<NCLOUDS; ++i)
		clouds.push_back(new Cloud(prog_draw, prog_basic, "modeles/cloud.obj", "modeles/cloud.mtl", glm::vec3(rand_float(-REPERE_BOX, REPERE_BOX), rand_float(-REPERE_BOX, REPERE_BOX), rand_float(CLOUD_MIN_ALTI, CLOUD_MAX_ALTI)), glm::mat3(1.0f), rand_float(CLOUD_MIN_SIZE, CLOUD_MAX_SIZE), rand_float(CLOUD_MIN_SPEED, CLOUD_MAX_SPEED)));
	
	lights_ubo= new LightsUBO(0);
	lights_ubo->set_prog(prog_draw);
	lights_ubo->init();
	
	// lumière fixe
	float position_world[]= {0.0f, 0.0f, 5000.0f, 1.0f};
	float spot_cone_direction_world[]= {0.0f, 0.0f, -1.0f};
	Light light(LIGHT_PARAMS_1, prog_repere, position_world, spot_cone_direction_world);
	lights.push_back(light);
	
	// celle-ci suit le joueur
	float position_world2[]= {0.0f, 0.0f, 0.0f, 1.0f};
	float spot_cone_direction_world2[]= {0.0f, 0.0f, -1.0f};
	Light light2(LIGHT_PARAMS_2, prog_repere, position_world2, spot_cone_direction_world2);
	lights.push_back(light2);

	lights_ubo->update(lights);
	
	for (unsigned int i=0; i<N_MAX_LITTLE_EXPLOSIONS; ++i) {
		little_explosions.push_back(new Explosion(prog_draw_instanced, prog_basic, "modeles/explosion.obj", "modeles/explosion.mtl", glm::vec3(0.0f), LITTLE_EXPLOSION_PARAMS));
	}
	for (unsigned int i=0; i<N_MAX_BIG_EXPLOSIONS; ++i) {
		big_explosions.push_back(new Explosion(prog_draw_instanced, prog_basic, "modeles/explosion.obj", "modeles/explosion.mtl", glm::vec3(0.0f), BIG_EXPLOSION_PARAMS));
	}
	
	level_map= new LevelMap(prog_map, all_ships.size());
	ranking= new Ranking(silom_font, all_ships);
	global_msg= new GlobalMsg(arial_font);
	
	// init des matrices
	ship->anim(view_system->_world2camera, view_system->_camera2clip);
	for (auto &it_enemy : enemies)
		it_enemy->_ship->anim(view_system->_world2camera, view_system->_camera2clip);
	level->anim(view_system->_world2camera, view_system->_camera2clip);
	for (unsigned int i=0; i<lights.size(); i++)
		lights[i].anim(view_system->_world2camera);
	lights_ubo->update(lights);
}


void draw() {
	compt_fps++;
	
	glClearColor(FOG_COLOR[0], FOG_COLOR[1], FOG_COLOR[2], 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	for (auto &it_explosion : little_explosions)
		it_explosion->draw();
	for (auto &it_explosion : big_explosions)
		it_explosion->draw();
	
	//skybox->draw();
	
	level->draw();
	
	for (auto &it_cloud : clouds)
		it_cloud->draw();

	if (is_visu_global) {
		for (auto &it_ship : all_ships)
			it_ship->draw(true, true);
		for (unsigned int i=0; i<lights.size(); i++)
			lights[i]._light_draw.draw(view_system->_world2clip);
		view_system->draw();
	}
	else {
		for (auto &it_ship : all_ships)
			it_ship->draw(false, false);
	}
	
	ranking->draw();
	global_msg->draw();
	
	// désactiver ce bloc si on veut mixer les 2 affichages
	/*glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, MAP_WIDTH, MAP_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	*/
	
	glViewport(0, 0, MAP_WIDTH, MAP_HEIGHT);
	level_map->draw();
	
	SDL_GL_SwapWindow(window);
}


void anim() {
	tikanim2= SDL_GetTicks();
	if (tikanim2- tikanim1< DELTA_ANIM)
		return;

	tikanim1= SDL_GetTicks();
	
	// cette lumière suit le joueur
	glm::vec3 above= ship->_rigid_body._position+ DIST_SHIP_LIGHT* glm::vec3(0.0f, 0.0f, 1.0f);
	lights[1].move(above);
	
	for (unsigned int i=0; i<lights.size(); i++)
		lights[i].anim(view_system->_world2camera);
	lights_ubo->update(lights);
	
	//skybox->anim(world2camera, camera2clip);


	for (auto &it_ship : all_ships)
		it_ship->anim(view_system->_world2camera, view_system->_camera2clip);

	level->anim(view_system->_world2camera, view_system->_camera2clip);
		
	for (auto &it_cloud : clouds)
		it_cloud->anim(view_system->_world2camera, view_system->_camera2clip);

	if (!is_visu_global)
		recompute_world2camera_embarked();
	
	for (auto &it_ship : all_ships)
		it_ship->collision(level, all_ships, little_explosions, big_explosions, ranking, global_msg);

	for (auto &it_explosion : little_explosions)
		it_explosion->anim(view_system->_world2camera, view_system->_camera2clip);
	for (auto &it_explosion : big_explosions)
		it_explosion->anim(view_system->_world2camera, view_system->_camera2clip);
	
	for (auto &it_enemy : enemies)
		it_enemy->think(level, all_ships);
	
	level_map->anim(all_ships);
	
	global_msg->anim();
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
	if (!is_paused) {
		anim();
	}
	draw();
	compute_fps();
}


void main_loop() {
	SDL_Event event;
	int done= 0;
	
	while (!done) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
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
						// esc : sortie programme
						case SDLK_ESCAPE:
							done= 1;
							break;
						
						// tests
						case SDLK_SPACE:
							ranking->print();
							break;
						
						// pause
						case SDLK_p:
							is_paused= !is_paused;
							break;
						
						// changement de point de vue
						case SDLK_v:
							is_visu_global= !is_visu_global;
							if (is_visu_global) {
								recompute_world2camera_embarked();
							}
							else {
								view_system->recompute_world2camera();
							}
							break;
						
						// affichage repère
						case SDLK_r:
							view_system->_repere->_is_repere= !view_system->_repere->_is_repere;
							view_system->_repere->_is_ground= !view_system->_repere->_is_ground;
							view_system->_repere->_is_box= !view_system->_repere->_is_box;
							break;
						
						// affichage bbox
						case SDLK_b:
							for (auto it_ship : all_ships) {
								it_ship->_model._bbox_draw= !it_ship->_model._bbox_draw;
								for (auto it_bullet : it_ship->_bullets) {
									it_bullet->_model._bbox_draw= !it_bullet->_model._bbox_draw;
								}
							}
							break;
						
						// navigation
						case SDLK_UP:
						case SDLK_DOWN:
						case SDLK_LEFT:
						case SDLK_RIGHT:
						case SDLK_a:
						case SDLK_z:
							ship->update_keys_pressed();
							break;
						
						// tir
						case SDLK_e:
							ship->_is_shooting= true;
							break;
						
						default:
							break;
					}
					break;
					
				case SDL_KEYUP:
					switch (event.key.keysym.sym) {
						case SDLK_UP:
						case SDLK_DOWN:
						case SDLK_LEFT:
						case SDLK_RIGHT:
						case SDLK_a:
						case SDLK_z:
							ship->update_keys_pressed();
							break;
						
						case SDLK_e:
							ship->_is_shooting= false;
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
	delete view_system;
	delete ship;
	for (unsigned int i=0; i<N_ENEMIES; ++i) {
		delete enemies[i];
	}
	delete level;
	lights_ubo->release();
	delete lights_ubo;
	for (auto it_exp : little_explosions)
		delete it_exp;
	for (auto it_exp : big_explosions)
		delete it_exp;
	delete level_map;
	delete ranking;
	
	SDL_GL_DeleteContext(main_context);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
}



int main(void) {
	init();
	main_loop();
	clean();
	
	return 0;
}

