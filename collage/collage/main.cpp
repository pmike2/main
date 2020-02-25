/*

*/


#include "CoreFoundation/CoreFoundation.h"

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <string>
#include <sys/time.h>
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

#include <ft2build.h>
#include FT_FREETYPE_H

#include "portaudio.h"

#ifdef WIN32
#include <windows.h>
#if PA_USE_ASIO
#include "pa_asio.h"
#endif
#endif

#include <fftw3.h>

#include "gl_utils.h"
#include "utile.h"
#include "font.h"
#include "pa_utils.h"
#include "constantes.h"
#include "input_state.h"
#include "audio_utils.h"


using namespace std;



// ---------------------------------------------------------------------------------------
SDL_Window * window= NULL;
SDL_GLContext main_context;
InputState * input_state;
ScreenGL * screengl;

int done= 0;

unsigned int val_fps, compt_fps;
unsigned int tikfps1, tikfps2, tikanim1, tikanim2;

GLuint prog_2d, prog_font;
GLuint g_vao;

Font * arial_font;

int idx_device_input= -1;
int idx_device_output= -1;
PaStream * stream;
PaError err;

AudioProjectGL * audio_project;

// ---------------------------------------------------------------------------------------
// callback PortAudio
int pa_callback(const void * input, void * output, unsigned long sample_count, const PaStreamCallbackTimeInfo * time_info, PaStreamCallbackFlags status_flags, void * user_data) {
	//Audio * data= (Audio *)user_data;
	float * in= (float *)input;
	float * out= (float *)output;

	/*
	cout << fixed;
	cout << setprecision(6);
	cout << time_info->currentTime << ";" << time_info->inputBufferAdcTime << ";" << time_info->outputBufferDacTime << endl;
	*/

	// pour ne pas avoir de glitchs
	for (unsigned int i=0; i<sample_count; ++i) {
		out[2* i+ 0]= 0.0f;
		out[2* i+ 1]= 0.0f;
	}

	return 0;
}


// ---------------------------------------------------------------------------------------
void mouse_motion(int x, int y, int xrel, int yrel) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, xrel, yrel, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

 	if (audio_project->mouse_motion(input_state)) {
		return;
	}
}


void mouse_button_up(unsigned int x, unsigned int y) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

 	if (audio_project->mouse_button_up(input_state)) {
		return;
	}
}


void mouse_button_down(unsigned int x, unsigned int y, unsigned short button) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);


 	if (audio_project->mouse_button_down(input_state)) {
		return;
	}
}


void key_down(SDL_Keycode key) {
	input_state->key_down(key);

	if (key== SDLK_ESCAPE) {
		done= true;
	}

 	if (audio_project->key_down(input_state, key)) {
		//return;
	}
}


void key_up(SDL_Keycode key) {
	input_state->key_up(key);

}


void drag_drop(string file_path) {
	if (audio_project->drag_drop(file_path)) {
		return;
	}
}


// ---------------------------------------------------------------------------------------
void init() {
	srand(time(NULL));
	
	SDL_Init(SDL_INIT_EVERYTHING);
	//IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF);
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	window= SDL_CreateWindow("collage", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	main_context= SDL_GL_CreateContext(window);

	// drag and drop
	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

	cout << "OpenGL version=" << glGetString(GL_VERSION) << endl;
	/*int x= 0;
	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &x);
	cout << x << endl;*/

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
	arial_font= new Font(prog_font, "../../fonts/Arial.ttf", 24, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
	input_state= new InputState();
	screengl= new ScreenGL(MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, GL_WIDTH, GL_HEIGHT);
	audio_project= new AudioProjectGL(prog_2d, arial_font, screengl);
	drag_drop("../data/record_afx.wav");
	drag_drop("../data/record_autechre.wav");
	drag_drop("../data/record_beat_simple.wav");

	// ------------------------------------------------------------------------
	err= Pa_Initialize();
	if (err!= paNoError) {
		printf("ERROR: Pa_Initialize returned 0x%x\n", err);
	}
	//printf("PortAudio version number = %d\nPortAudio version text = '%s'\n", Pa_GetVersion(), Pa_GetVersionText());
	
	//const PaDeviceInfo *deviceInfo= Pa_GetDeviceInfo(idx_device_output);
	//printf("%i channels\n", deviceInfo->maxInputChannels);
	
	PaStreamParameters inputParameters;
	bzero(&inputParameters, sizeof(inputParameters));
	inputParameters.device= idx_device_input;
	inputParameters.channelCount= 2;
	inputParameters.sampleFormat= paFloat32;
	inputParameters.suggestedLatency= Pa_GetDeviceInfo(idx_device_input)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo= NULL;

	PaStreamParameters outputParameters;
	bzero(&outputParameters, sizeof(outputParameters));
	outputParameters.device= idx_device_output;
	outputParameters.channelCount= 2;
	outputParameters.sampleFormat= paFloat32;
	outputParameters.suggestedLatency= Pa_GetDeviceInfo(idx_device_output)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo= NULL;
	
	err= Pa_OpenStream(&stream, &inputParameters, &outputParameters, (double)(SAMPLE_RATE_PLAYBACK), (unsigned long)(SAMPLES_PER_BUFFER_PLAYBACK), paNoFlag, pa_callback, NULL);
	//err= Pa_OpenDefaultStream(&stream, 2, 2, paFloat32, (double)(SAMPLE_RATE_PLAYBACK), (unsigned long)(SAMPLES_PER_BUFFER_PLAYBACK)), pa_callback, audio);
	
	if (err!= paNoError) {
		printf("ERROR: Pa_OpenStream returned 0x%x\n", err);
	}
	
	Pa_StartStream(stream);
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
	
	glClearColor(MAIN_BCK[0], MAIN_BCK[1], MAIN_BCK[2], MAIN_BCK[3]);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);

	audio_project->draw();

	show_infos();

	SDL_GL_SwapWindow(window);
}


void anim() {
	tikanim2= SDL_GetTicks();
	if (tikanim2- tikanim1< DELTA_ANIM)
		return;

	tikanim1= SDL_GetTicks();

	// mise a jour de l'audio courant en lisant dans la queue écrite par le callback
	PaTime current_time= Pa_GetStreamTime(stream);
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
					
				case SDL_DROPFILE:
					drag_drop(string(event.drop.file));
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
	err= Pa_StopStream(stream);
	if (err!= paNoError) {
		printf("ERROR: Pa_StopStream returned 0x%x\n", err);
	}
	Pa_CloseStream(stream);
	Pa_Terminate();

	delete arial_font;
	delete input_state;
	delete audio_project;

	SDL_GL_DeleteContext(main_context);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
}



// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------

/*
taf
	idx_device_input= 2;
	idx_device_output= 4;
	
home
	idx_device_input= 2;
	idx_device_output= 1;
*/

int main(int argc, char *argv[]) {
	// sans argument on liste les périphériques audio
	if (argc== 1) {
		err= Pa_Initialize();
		if (err!= paNoError) {
			printf("ERROR: Pa_Initialize returned 0x%x\n", err);
		}
		list_devices();
	}
	else if (argc== 3) {
		sscanf(argv[1], "%d", &idx_device_input);
		sscanf(argv[2], "%d", &idx_device_output);
		
		init();
		main_loop();
		clean();
	}
	else {
		cout << "erreur parcours arguments" << endl;
	}

	return 0;
}

