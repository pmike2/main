#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <thread>

#include "sndfile.h"
#include "portaudio.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>


using namespace std;




vector<string> SAMPLES_PATHS {
	"/Volumes/Vol-Users/Users/pmbeau2/Desktop/samples/bcl1.wav",
};


const double SAMPLE_RATE_INIT= 44100.0;
const unsigned int FRAMES_PER_BUFFER_INIT= 512;


const unsigned int SCREEN_WIDTH= 100;
const unsigned int SCREEN_HEIGHT= 100;
const string SCREEN_TITLE= "SEQ";


const float MAIN_IHM_BG_WIDTH= 650.0f;
const float MAIN_IHM_BG_HEIGHT= 550.0f;
const float MAIN_IHM_PARENT_OFFX= 5.0f;
const float MAIN_IHM_PARENT_OFFY= 5.0f;
const sf::Color MAIN_IHM_BG_COLOR= sf::Color(20, 20, 40);


static int callback(const void *input, void *output, unsigned long frame_count, const PaStreamCallbackTimeInfo *time_info, PaStreamCallbackFlags status_flags, void *user_data);


// ENGINE --------------------------------------------------------------------------------
struct Sample {
	string _path;
	SF_INFO _info;
	float* _buff;
	
	Sample(string path) :
		_path(path)
	{
		boost::filesystem::path file_path(_path);
		if (!boost::filesystem::exists(file_path)) {
			cerr << "fichier n'existe pas" << endl;
			return;
		}
		
		SNDFILE* file= sf_open(path.c_str(), SFM_READ, &_info);
		if (!file) {
			cerr << "sf_open erreur" << endl;
			return;
		}
		
		_buff= new float[_info.channels* _info.frames];
		sf_read_float(file, _buff, _info.channels* _info.frames);
		sf_close(file);
	}
	
	
	void clean() {
		delete[] _buff;
	}
};



struct Engine {
	double _sample_rate;
	unsigned int _frames_per_buffer;
	vector<Sample> _samples;
	PaStream *_stream;
	bool _is_playing;
	double _bpm;
	int (*_callback)(const void*, void*, unsigned long, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*);
	
	Engine() :
		_sample_rate(SAMPLE_RATE_INIT),
		_frames_per_buffer(FRAMES_PER_BUFFER_INIT),
		_is_playing(false),
		_bpm(BPM_INIT)
	{
		for (auto &it : SAMPLES_PATHS) {
			Sample sample(it);
			sample.init_markers(DEFAULT_TDP);
			_samples.push_back(sample);
		}
	}
	
	void init_pa(int (*callback)(const void*, void*, unsigned long, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*)) {
		PaError error;

		_callback= callback;

		error= Pa_Initialize();
		if (error!= paNoError)
			cerr << "Pa_Initialize erreur" << endl;
		
		//error= Pa_OpenDefaultStream(&_stream, 0, 2, paFloat32, _sample_rate, paFramesPerBufferUnspecified, _callback, this);
		error= Pa_OpenDefaultStream(&_stream, 0, 2, paFloat32, _sample_rate, _frames_per_buffer, _callback, this);
		if (error!= paNoError)
			cerr << "Pa_OpenDefaultStream erreur" << endl;
	}
	
	void pause() {
		PaError error;
		
		_is_playing= false;
		error= Pa_StopStream(_stream);
		if (error!= paNoError)
			cerr << "Pa_StopStream erreur" << endl;
	}
	
	void resume() {
		PaError error;
		
		_is_playing= true;
		error= Pa_StartStream(_stream);
		if (error!= paNoError)
			cerr << "Pa_StartStream erreur" << endl;
	}
	
	void clean() {
		PaError error;
		
		for (auto& it_sample : _samples)
			it_sample.clean();

		error= Pa_CloseStream(_stream);
		if (error!= paNoError)
			cerr << "Pa_CloseStream erreur" << endl;
	
		error= Pa_Terminate();
		if (error!= paNoError)
			cerr << "Pa_Terminate erreur" << endl;
	}
	
};



struct IHM {
	unsigned int _screen_width;
	unsigned int _screen_height;
	sf::RenderWindow _window;
	MainIHM _main;
	
	IHM() :
		_screen_width(SCREEN_WIDTH),
		_screen_height(SCREEN_HEIGHT),
		_window(sf::VideoMode(_screen_width, _screen_height), SCREEN_TITLE) 
	{
	}
	
	void draw() {
		_window.clear();
		_window.draw(_main);
		_window.display();
	}
	
};


// ---------------------------------------------------------------------------------------
struct SEQ {
	Engine _engine;
	IHM _ihm;
	bool _is_mouse_down;
	bool _is_mouse_action_on;
	bool _is_mouse_action_active;
	bool _is_mouse_action_loop_on;
	
	SEQ() : _is_mouse_down(false), _is_mouse_action_on(false), _is_mouse_action_active(false), _is_mouse_action_loop_on(false) {
		_engine.init_pa(&callback);
		
		for (auto & it_sample : _engine._samples)
			_ihm._samples.add_sample(it_sample, _engine._beats.size());
		
	}
	

	
	void loop() {
		while (_ihm._window.isOpen()) {
			sf::Event event;
			sf::Vector2f resized_pos;
			while (_ihm._window.pollEvent(event)) {
				switch (event.type) {
					case sf::Event::Closed:
						quit();
						break;
				
					case sf::Event::KeyPressed:
						keypressed(event.key);
						break;
				
					case sf::Event::MouseButtonPressed:
						/* il faut utiliser mapPixelToCoords (cf https://www.sfml-dev.org/tutorials/2.4/graphics-view.php) pour convertir
						les coordonnées souris en cas de resize de la fenetre */
						resized_pos= _ihm._window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
						//cout << "mapPixelToCoords= " << world_pos.x << " ; " << world_pos.y << endl;
						//cout << "mousebutton= " << event.mouseButton.x << " ; " << event.mouseButton.y << endl;
						
						//mouse_pressed(event.mouseButton.button, event.mouseButton.x, event.mouseButton.y);
						mouse_pressed(event.mouseButton.button, resized_pos.x, resized_pos.y);
						break;
					
					case sf::Event::MouseButtonReleased:
						mouse_released(event.mouseButton.button);
						break;

					case sf::Event::MouseMoved:
						resized_pos= _ihm._window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
						//mouse_move(event.mouseMove.x, event.mouseMove.y);
						mouse_move(resized_pos.x, resized_pos.y);
						break;

					default:
						break;
				}
			}
			
			anim();
			_ihm.draw();
		}
	}
	
	void anim() {
	}

	void keypressed(sf::Event::KeyEvent key_event) {
		switch (key_event.code) {
			case sf::Keyboard::Escape:
				quit();
				break;
				
			case sf::Keyboard::D:
				break;
				
			case sf::Keyboard::Left:
				break;
			
			case sf::Keyboard::Right:
				break;
			
			case sf::Keyboard::Up:
				break;
				
			case sf::Keyboard::Down:
				break;
				
			default:
				break;
		}
	}
	
	void mouse_pressed(sf::Mouse::Button mbutton, float mx, float my) {
		if (mbutton== sf::Mouse::Left) {
			_is_mouse_down= true;
			
		}
	}
	
	void mouse_released(sf::Mouse::Button mbutton) {
		if (mbutton== sf::Mouse::Left) {
			_is_mouse_down= false;
		}
	}
	
	void mouse_move(float mx, float my) {
		// mouse down --------------------------------------------------------------------
		if (_is_mouse_down) {
		}
		// mouse up ----------------------------------------------------------------------
		else {
		}
	}
	
	void quit() {
		_engine.clean();
		_ihm._window.close();
	}
	
	void debug() {
	}
};



// CALLBACK PORTAUDIO --------------------------------------------------------------------
static int callback(const void *input, void *output, unsigned long frame_count, const PaStreamCallbackTimeInfo *time_info, PaStreamCallbackFlags status_flags, void *user_data) {

	Engine *p_engine= (Engine*)user_data;
	float *f_output= (float*) output;
	
	// on écrit en stéréo
	unsigned int data_count= frame_count* 2;
	memset(f_output, 0, sizeof(float)* data_count);
	
	//return paContinue;
	
	for (unsigned int i=0; i<frame_count; ++i) {

			// http://folk.ntnu.no/oyvinbra/delete/Lesson1Panning.html pour plusieurs types de panning
			f_output[2* i]   += amplitude* (1.0f- pan)* 0.5f* val_left;
			f_output[2* i+ 1]+= amplitude* (1.0f+ pan)* 0.5f* val_right;
			
	}
	
	return paContinue;
}


// ---------------------------------------------------------------------------------------
int main() {
	srand(time(NULL));
	SEQ seq;
	seq.loop();
	
	return 0;
}

