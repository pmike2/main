/*
Programme principal de lancement de racing
*/

#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <chrono>

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "sio_client.h"
#include "json.hpp"

#include "utile.h"
#include "gl_utils.h"
#include "input_state.h"
#include "typedefs.h"

#include "racing.h"


using json = nlohmann::json;


// en ms; temps entre 2 anims
//const unsigned int DELTA_ANIM= 1;

// dimensions écran
const int MAIN_WIN_WIDTH= 1400;
const int MAIN_WIN_HEIGHT= 1024;
const float MAIN_BCK[]= {0.1f, 0.1f, 0.1f, 1.0f};
const float GL_WIDTH= 15.0f;
const float GL_HEIGHT= GL_WIDTH* (float)(MAIN_WIN_HEIGHT)/ (float)(MAIN_WIN_WIDTH);


SDL_Window * window;
SDL_GLContext main_context;
InputState * input_state;
ScreenGL * screengl;
Racing * racing;

bool done= false;
unsigned int val_fps, compt_fps;
unsigned int tikfps1, tikfps2;


// à activer / désactiver le temps d'affinage des params voiture héros
sio::client io;
bool data_send= false;
std::string data_string= "";
std::mutex mut;



void key_down(SDL_Keycode key, time_point t) {
	input_state->key_down(key);

	if (racing->key_down(key, t)) {
		return;
	}

	if (key== SDLK_ESCAPE) {
		done= true;
		return;
	}
}


void key_up(SDL_Keycode key) {
	input_state->key_up(key);
}


void joystick_down(unsigned int button_idx) {
	input_state->joystick_down(button_idx);
}


void joystick_up(unsigned int button_idx) {
	input_state->joystick_up(button_idx);
}


void joystick_axis(unsigned int axis_idx, int value) {
	input_state->joystick_axis(axis_idx, value);
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
	// pour faire du multisampling (suppression aliasing) ; inutile ici et fait saturer le GPU de l'imac
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
	
	window= SDL_CreateWindow("racing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	main_context= SDL_GL_CreateContext(window);

	std::cout << "OpenGL version=" << glGetString(GL_VERSION) << std::endl;
	/*int x= 0;
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &x); // 32
	cout << x << endl;*/

	SDL_GL_SetSwapInterval(1);
	glClearColor(MAIN_BCK[0], MAIN_BCK[1], MAIN_BCK[2], MAIN_BCK[3]);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glDepthRange(0.0f, 1.0f);
	glViewport(0, 0, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
	
	// frontfaces en counterclockwise
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_CLAMP);
	
	// pour gérer l'alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glPointSize(2.0f);
	
	SDL_GL_SwapWindow(window);
	
	// --------------------------------------------------------------------------
	/* VAO = vertex array object : tableau d'objets, chaque appel à un objet rappelle un contexte de dessin
	incluant tous les attribute array setup (glVertexAttribArray), buffer objects used for attribute arrays
	et GL_ELEMENT_ARRAY_BUFFER eventuellement
	ici je n'en utilise qu'un pour tout le prog ; à terme peut-être faire plusieurs VAOs
	*/
	
	GLuint g_vao;
	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);

	std::map<std::string, GLuint> progs;
	progs["font"]= create_prog("../../shaders/vertexshader_font.txt", "../../shaders/fragmentshader_font.txt");
	progs["simple"]= create_prog("../shaders/vertexshader_simple.txt", "../shaders/fragmentshader_simple.txt");
	progs["texture"]= create_prog("../shaders/vertexshader_texture.txt", "../shaders/fragmentshader_texture.txt");
	progs["smoke"]= create_prog("../shaders/vertexshader_smoke.txt", "../shaders/fragmentshader_smoke.txt");
	progs["tire_track"]= create_prog("../shaders/vertexshader_tire_track.txt", "../shaders/fragmentshader_tire_track.txt");
	progs["spark"]= create_prog("../shaders/vertexshader_spark.txt", "../shaders/fragmentshader_spark.txt");
	progs["choose_track"]= create_prog("../shaders/vertexshader_choose_track.txt", "../shaders/fragmentshader_choose_track.txt");
	progs["driver_face"]= create_prog("../shaders/vertexshader_driver_face.txt", "../shaders/fragmentshader_driver_face.txt");
	progs["barrier"]= create_prog("../shaders/vertexshader_barrier.txt", "../shaders/fragmentshader_barrier.txt");
	progs["map"]= create_prog("../shaders/vertexshader_map.txt", "../shaders/fragmentshader_map.txt");
	progs["water"]= create_prog("../shaders/vertexshader_water.txt", "../shaders/fragmentshader_water.txt");

	check_gl_error();

	// --------------------------------------------------------------------------
	screengl= new ScreenGL(MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, GL_WIDTH, GL_HEIGHT);
	input_state= new InputState();
	time_point now= std::chrono::system_clock::now();
	racing= new Racing(progs, screengl, input_state, now);

	if (SDL_NumJoysticks()> 0){
		SDL_Joystick * joy= SDL_JoystickOpen(0);
		if (joy) {
			input_state->_is_joystick= true;
			std::cout << "joystick OK; n axes=" << SDL_JoystickNumAxes(joy) << " ; n buttons=" << SDL_JoystickNumButtons(joy) << "\n";
		}
	}

	// la taille du buffer influe sur la latence
	/*if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 512)== -1) {
		std::cerr << "Echec audio\n";
	}
	// permet d'allouer des channels pour faire du polyphonique; il faut alors bien gérer le 1er arg de Mix_PlayChannel
	Mix_AllocateChannels(16);*/

	
	// à activer / désactiver le temps d'affinage des params voiture héros
	/*io.set_open_listener([&]() {
		io.socket()->on("data", [&](sio::event& ev) {
			mut.lock();
			data_string= ev.get_message()->get_string();
			data_send= true;
			mut.unlock();
		});
	});
	io.connect("http://127.0.0.1:3001");*/
}


void draw() {
	compt_fps++;

	racing->draw();

	SDL_GL_SwapWindow(window);
}


void anim(time_point t) {
	racing->anim(t);
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


// à activer / désactiver le temps d'affinage des params voiture héros
void check_data_send() {
	mut.lock();
	if (data_send) {
		data_send= false;
		std::cout << data_string << "\n";
		json js= json::parse(data_string);
		std::string json_path= "../data/cars/hero_car.json";
		std::ofstream ofs(json_path);
		ofs << std::setw(4) << js << "\n";
		ofs.close();
		CarModel * model= (CarModel *)(racing->_track->_hero->_model);
		model->load(json_path);
	}
	mut.unlock();
}


void idle(time_point t) {
	anim(t);
	draw();
	compute_fps();
	//check_data_send();
}


void main_loop() {
	SDL_Event event;
	
	while (!done) {
		time_point now= std::chrono::system_clock::now();

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				/*case SDL_MOUSEMOTION:
					mouse_motion(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
					break;
					
				case SDL_MOUSEBUTTONUP:
					mouse_button_up(event.button.x, event.button.y, event.button.button);
					break;
					
				case SDL_MOUSEBUTTONDOWN:
					mouse_button_down(event.button.x, event.button.y, event.button.button);
					break;*/

				case SDL_KEYDOWN:
					// event.key.repeat== 0 correspond au 1er appui de la touche; si on ne fait pas ca key_down est déclenché plein
					// de fois, tant que la touche est enfoncée
					if (event.key.repeat== 0) {
						key_down(event.key.keysym.sym, now);
					}
					break;
					
				case SDL_KEYUP:
					key_up(event.key.keysym.sym);
					break;

				case SDL_JOYBUTTONDOWN:
					joystick_down(event.jbutton.button);
					break;

				case SDL_JOYBUTTONUP:
					joystick_up(event.jbutton.button);
					break;

				 case SDL_JOYAXISMOTION:
				 	joystick_axis(event.jaxis.axis, event.jaxis.value);
					break;

				// utilisé ?
				//case SDL_JOYHATMOTION:
				//	printf("The hat with index %d was moved to position %d.\n", event.jhat.hat, event.jhat.value);
				//	break;

				case SDL_QUIT:
					done= 1;
					break;
					
				default:
					break;
			}
		}
		idle(now);
	}
}


void clean() {
	delete racing;
	delete input_state;
	delete screengl;

	Mix_CloseAudio();
	SDL_GL_DeleteContext(main_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


int main() {
	init();
	main_loop();
	clean();

	/*std::vector<int> v{1, 45, 3, 8, 2};
	std::sort(v.begin(), v.end(), [](int a, int b) {
		if (a> b) {
			return 0;
		}
		return 1;
	});
	for (auto x : v) {
		std::cout << x << "\n";
	}*/

	return 0;
}
