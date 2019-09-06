/*

PMB 12/2018
Visualisation audio

Basé sur :
http://archive.gamedev.net/archive/reference/programming/features/beatdetection/index.html

pour tester :
/Volumes/Cezanne/Cezanne/bin/ffplay -nodisp -loglevel panic /Volumes/Cezanne/Cezanne/perso_dev/ovh/www/music/bof.mp3

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
#include "spectrum.h"
#include "input_state.h"
#include "light.h"


using namespace std;



// ---------------------------------------------------------------------------------------
SDL_Window * window= NULL;
SDL_GLContext main_context;
InputState * input_state;

int done= 0;

unsigned int val_fps, compt_fps;
unsigned int tikfps1, tikfps2, tikanim1, tikanim2;

GLuint prog_3d, prog_3d_obj, prog_2d, prog_repere, prog_font;
GLuint g_vao;

Font * arial_font;

int idx_device_input= -1;
int idx_device_output= -1;
PaStream * stream;
PaError err;

LightsUBO * lights_ubo;
Audio * audio;
VisuWave * visu_wave;
VisuSpectrum * visu_spectrum;
VisuSimu * visu_simu;
VisuArt * visu_art;


// ---------------------------------------------------------------------------------------
// callback PortAudio
int pa_callback(const void * input, void * output, unsigned long sample_count, const PaStreamCallbackTimeInfo * time_info, PaStreamCallbackFlags status_flags, void * user_data) {
	Audio * data= (Audio *)user_data;
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

	if (data->_mode== AUDIO_STOP) {

	}

	else if (data->_mode== AUDIO_PLAYBACK) {
		// pousse dans une queue l'info que _current_sample_callback sera joué à time_info->outputBufferDacTime
		data->push_event(time_info->outputBufferDacTime);

		// ecriture sur le DAC
		for (unsigned int i=0; i<sample_count; ++i) {
			out[2* i+ 0]= data->_amplitudes[2* data->_current_sample_callback+ 0]* data->_playback_amplitude;
			out[2* i+ 1]= data->_amplitudes[2* data->_current_sample_callback+ 1]* data->_playback_amplitude;

			data->_current_sample_callback++;
			if (data->_current_sample_callback< data->_left_selection)
				data->_current_sample_callback= data->_left_selection;
			if (data->_current_sample_callback>= data->_right_selection)
				data->_current_sample_callback= data->_left_selection;
		}
	}

	else if (data->_mode== AUDIO_RECORD) {
		// la je ne sais pas trop lequel utiliser ; inputBufferAdcTime est dans le passé, currentTime le présent
		data->push_event(time_info->inputBufferAdcTime);
		//data->push_event(time_info->currentTime);

		// lecture du ADC
		for (unsigned int i=0; i<sample_count; i++) {
			float input_left= *in++;
			float input_right= *in++;
			
			data->_amplitudes[2* (data->_current_sample_callback+ i)+ 0]= input_left;
			data->_amplitudes[2* (data->_current_sample_callback+ i)+ 1]= input_right;
		}

		// calcul du spectre pour le bloc courant
		data->compute_spectrum(data->_current_sample_callback);

		data->_current_sample_callback+= SAMPLES_PER_BUFFER;
		if (data->_current_sample_callback< data->_left_selection) {
			data->_current_sample_callback= data->_left_selection;
		}
		if (data->_current_sample_callback>= data->_right_selection) {
			data->_current_sample_callback= data->_left_selection;
		}
	}

	return 0;
}


// ---------------------------------------------------------------------------------------
void mouse_motion(int x, int y, int xrel, int yrel) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, xrel, yrel, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

 	if (visu_spectrum->mouse_motion(input_state)) {
		return;
	}
	
 	if (visu_art->mouse_motion(input_state)) {
		return;
	}
	
 	if (visu_wave->mouse_motion(input_state)) {
		return;
	}

 	if (visu_simu->mouse_motion(input_state)) {
		return;
	}
}


void mouse_button_up(unsigned int x, unsigned int y) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

 	if (visu_spectrum->mouse_button_up(input_state)) {
		//return;
	}
	
 	if (visu_art->mouse_button_up(input_state)) {
		//return;
	}
	
 	if (visu_wave->mouse_button_up(input_state)) {
		//return;
	}

 	if (visu_simu->mouse_button_up(input_state)) {
		//return;
	}
}


void mouse_button_down(unsigned int x, unsigned int y, unsigned short button) {
	unsigned int mouse_state= SDL_GetMouseState(NULL, NULL);
	input_state->update_mouse(x, y, mouse_state & SDL_BUTTON_LMASK, mouse_state & SDL_BUTTON_MMASK, mouse_state & SDL_BUTTON_RMASK);

 	if (visu_spectrum->mouse_button_down(input_state)) {
		return;
	}
	
 	if (visu_art->mouse_button_down(input_state)) {
		return;
	}
	
 	if (visu_wave->mouse_button_down(input_state)) {
		return;
	}

 	if (visu_simu->mouse_button_down(input_state)) {
		return;
	}
}


void key_down(SDL_Keycode key) {
	input_state->key_down(key);

	if (key== SDLK_ESCAPE) {
		done= true;
	}

 	if (visu_spectrum->key_down(input_state, key)) {
		//return;
	}
	
 	if (visu_art->key_down(input_state, key)) {
		//return;
	}
	
 	if (visu_wave->key_down(input_state, key)) {
		//return;
	}
	
 	if (visu_simu->key_down(input_state, key)) {
		//return;
	}
}


void key_up(SDL_Keycode key) {
	input_state->key_up(key);

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
	
	window= SDL_CreateWindow("tmp", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	main_context= SDL_GL_CreateContext(window);

	cout << "OpenGL version=" << glGetString(GL_VERSION) << endl;
	/*int x= 0;
	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &x);
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
	
	SDL_GL_SwapWindow(window);
	
	// --------------------------------------------------------------------------
	/* VAO = vertex array object : tableau d'objets, chaque appel à un objet rappelle un contexte de dessin
	incluant tous les attribute array setup (glVertexAttribArray), buffer objects used for attribute arrays
	et GL_ELEMENT_ARRAY_BUFFER eventuellement
	ici je n'en utilise qu'un pour tout le prog ; à terme peut-être faire plusieurs VAOs
	*/
	
	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);

	prog_3d    = create_prog("../../shaders/vertexshader_3d_basic.txt", "../../shaders/fragmentshader_3d.txt");
	prog_3d_obj= create_prog("../../shaders/vertexshader_3d_obj.txt"  , "../../shaders/fragmentshader_3d_obj.txt");
	prog_2d    = create_prog("../../shaders/vertexshader_2d.txt"      , "../../shaders/fragmentshader_basic.txt");
	prog_repere= create_prog("../../shaders/vertexshader_repere.txt"  , "../../shaders/fragmentshader_basic.txt");
	prog_font  = create_prog("../../shaders/vertexshader_font.txt"    , "../../shaders/fragmentshader_font.txt");

	float eye_direction[]= {0.0f, 0.0f, 1.0f};
	GLuint progs_eye[]= {prog_3d, prog_3d_obj};
	for (unsigned int i=0; i<sizeof(progs_eye)/ sizeof(progs_eye[0]); ++i) {
		GLint eye_direction_loc= glGetUniformLocation(progs_eye[i], "eye_direction");
		glUseProgram(progs_eye[i]);
		glUniform3fv(eye_direction_loc, 1, eye_direction);
		glUseProgram(0);
	}

	check_gl_error(); // verif que les shaders ont bien été compilés - linkés
	
	// --------------------------------------------------------------------------
	lights_ubo= new LightsUBO(prog_3d); // heu ca va marcher ca ???
	lights_ubo->add_light(LIGHT_PARAMS_1, prog_repere, glm::vec3(0.0f, 0.0f, 5000.0f), glm::vec3(0.0f, 0.0f, -1.0f));

	arial_font= new Font(prog_font, "fonts/Arial.ttf", 24, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
	audio= new Audio();
	visu_wave= new VisuWave(prog_2d, audio);
	visu_spectrum= new VisuSpectrum(prog_3d, prog_repere, audio);
	visu_simu= new VisuSimu(prog_2d, audio);
	visu_art= new VisuArt(prog_3d_obj, prog_repere, audio);

	visu_art->load("../configs/visu_simple_1.json");
	//visu_art->randomize();
	//visu_art->save("../configs/test.json");

	// ------------------------------------------------------------------------
	input_state= new InputState();

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
	
	err= Pa_OpenStream(&stream, &inputParameters, &outputParameters, (double)(SAMPLE_RATE), (unsigned long)(SAMPLES_PER_BUFFER), paNoFlag, pa_callback, audio);
	err= Pa_OpenDefaultStream(&stream, 2, 2, paFloat32, (double)(SAMPLE_RATE), (unsigned long)(SAMPLES_PER_BUFFER), pa_callback, audio);
	
	if (err!= paNoError) {
		printf("ERROR: Pa_OpenStream returned 0x%x\n", err);
	}
	
	Pa_StartStream(stream);
}


// ---------------------------------------------------------------------------------------
// gestion multi-fenetre
void subwindow(const float bkgnd_color[4], int x, int y, int w, int h) {
	glClearColor(bkgnd_color[0], bkgnd_color[1], bkgnd_color[2], bkgnd_color[3]);
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, y, w, h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	glViewport(x, y, w, h);
}


// affichage strings opengl
void show_infos() {
	ostringstream font_str;
	font_str.precision(1);
	font_str << fixed;

	float font_scale= 0.6f;
	glm::vec3 font_color= glm::vec3(1.0f, 1.0f, 0.0f);

	font_str.str("");
	font_str << "idx_sample=" << audio->_current_sample << "-" << audio->_current_sample+ SAMPLES_PER_BUFFER << " / " << audio->_n_samples;
	arial_font->draw(font_str.str(), 10.0f, float(MAIN_WIN_HEIGHT)- 15.0f, font_scale, font_color);

	font_str.str("");
	font_str << "idx_block=" << audio->_current_sample/ SAMPLES_PER_BUFFER << " / " << audio->_n_samples/ SAMPLES_PER_BUFFER;
	arial_font->draw(font_str.str(), 10.0f, float(MAIN_WIN_HEIGHT)- 30.0f, font_scale, font_color);

	font_str.str("");
	if (audio->_mode== AUDIO_STOP) {
		font_str << "STOP";
	}
	else if (audio->_mode== AUDIO_PLAYBACK) {
		font_str << "PLAYBACK";
	}
	else if (audio->_mode== AUDIO_RECORD) {
		font_str << "RECORD";
	}
	arial_font->draw(font_str.str(), 10.0f, float(MAIN_WIN_HEIGHT)- 45.0f, font_scale, font_color);

	if (visu_art->_last_idx_signature>= 0) {
		font_str.str("");
		font_str << "signature=" << visu_art->_last_idx_signature << " ; " << audio->_signatures[visu_art->_last_idx_signature]->mass_center();
		arial_font->draw(font_str.str(), 10.0f, float(MAIN_WIN_HEIGHT)- 60.0f, font_scale, font_color);
	}

	font_str.str("");	
	for (unsigned int idx_freq=0; idx_freq<audio->_n_blocks_fft; ++idx_freq) {
		font_str << audio->_blocks[(audio->_current_sample/ SAMPLES_PER_BUFFER)* audio->_n_blocks_fft+ idx_freq]->_instant_energy << " ";
	}
	arial_font->draw(font_str.str(), 10.0f, float(MAIN_WIN_HEIGHT)- 75.0f, font_scale, font_color);
}


void draw() {
	compt_fps++;
	
	if (visu_art->_fullscreen) {
		glClearColor(ART_BCK[0], ART_BCK[1], ART_BCK[2], ART_BCK[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
		lights_ubo->draw(visu_art->_view_system->_world2clip);
		visu_art->draw();
	}
	else {
		glClearColor(MAIN_BCK[0], MAIN_BCK[1], MAIN_BCK[2], MAIN_BCK[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, MAIN_WIN_WIDTH, MAIN_WIN_HEIGHT);
		show_infos();
		
		subwindow(SPECTRUM_BCK, SPECTRUM_WIN_X, SPECTRUM_WIN_Y, SPECTRUM_WIN_WIDTH, SPECTRUM_WIN_HEIGHT);
		visu_spectrum->draw();

		subwindow(WAVE_BCK, WAVE_WIN_X, WAVE_WIN_Y, WAVE_WIN_WIDTH, WAVE_WIN_HEIGHT);
		visu_wave->draw();

		subwindow(SIMU_BCK, SIMU_WIN_X, SIMU_WIN_Y, SIMU_WIN_WIDTH, SIMU_WIN_HEIGHT);
		visu_simu->draw();

		subwindow(ART_BCK, ART_WIN_X, ART_WIN_Y, ART_WIN_WIDTH, ART_WIN_HEIGHT);
		lights_ubo->draw(visu_art->_view_system->_world2clip);
		visu_art->draw();
	}

	SDL_GL_SwapWindow(window);
}


void anim() {
	tikanim2= SDL_GetTicks();
	if (tikanim2- tikanim1< DELTA_ANIM)
		return;

	tikanim1= SDL_GetTicks();

	lights_ubo->anim(visu_art->_view_system->_world2camera);
	
	// mise a jour de l'audio courant en lisant dans la queue écrite par le callback
	PaTime current_time= Pa_GetStreamTime(stream);
	audio->update_current_sample(current_time);
	
	visu_art->anim(current_time, audio->_mode);
	visu_spectrum->anim();

	if ((audio->_mode== AUDIO_PLAYBACK) || (audio->_mode== AUDIO_RECORD)) {
		visu_wave->update_data();
		visu_spectrum->_gl_spectrum->update_data();
		visu_simu->update_data();
	}
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
	delete audio;
	delete visu_wave;
	delete visu_spectrum;
	delete visu_simu;
	delete visu_art;
	delete lights_ubo;
	delete input_state;

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
	idx_device_input= 0;
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

