#include "looper_sdl.h"
#include "utile.h"


using namespace std;


LooperSDL::LooperSDL() {
	
}


LooperSDL::LooperSDL(SDL_Renderer * renderer, int screen_width, int screen_height) : 
	_renderer(renderer), _screen_width(screen_width), _screen_height(screen_height)
{
	_input_state= new InputState();
}


LooperSDL::~LooperSDL() {
	delete _input_state;
}


bool LooperSDL::event_key(SDL_Keycode key) {
	if ((key>= 97) && (key<=122)) {
		return true;
	}
	return false;
}


void LooperSDL::key_down(SDL_Keycode key) {
	//cout << "key down : " << key << "\n";

	SDL_Keymod key_mod= SDL_GetModState();
	if (KMOD_CTRL && key_mod) {
		if (key== SDLK_d) {
			debug();
		}
		else if (key== SDLK_SPACE) {
			toggle_start();
		}
		else if (key== SDLK_r) {
			toggle_record();
		}
		else if (key== SDLK_DOWN) {
			set_next_track();
		}
		else if (key== SDLK_UP) {
			set_previous_track();
		}
		else if (key== SDLK_c) {
			clear();
		}
		else if (key== SDLK_KP_1) {
			set_current_track_duration_ratio(1.0);
		}
		else if (key== SDLK_KP_2) {
			set_current_track_duration_ratio(0.5);
		}
		return;
	}

	if (event_key(key)) {
		if (!_input_state->get_key(key)) {
			unsigned int amplitude= 0; // TODO
			note_on(key, amplitude, true);
		}
		_input_state->key_down(key);
		return;
	}
	
}


void LooperSDL::key_up(SDL_Keycode key) {
	//cout << "key up : " << key << "\n";
	SDL_Keymod key_mod= SDL_GetModState();
	if (KMOD_CTRL && key_mod) {
		return;
	}

	if (event_key(key)) {
		note_off();
		_input_state->key_up(key);
		return;
	}
}


void LooperSDL::draw() {
	// background color
	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
	SDL_RenderClear(_renderer);

	time_type now_time= now();
	unsigned int track_height= _screen_height/ N_MAX_TRACKS;

	for (unsigned int i=1; i<N_MAX_TRACKS; ++i) {
		float y= (float)(i)/ (float)(N_MAX_TRACKS);

		if ((_mode== RUNNING) || (_mode== RECORDING)) {
			float t_track= (float)(time_ms(_tracks[i]->get_relative_t(now_time)))/ (float)(time_ms(_tracks[i]->_duration));
			Color c;
			if (_mode== RUNNING) {
				c._r= 250; c._g= 250; c._b= 200;
			}
			else if (_mode== RECORDING) {
				c._r= 250; c._g= 50; c._b= 50;
			}
			draw_rect((int)((float)(_screen_width)* t_track), (int)((float)(_screen_height)* y), 2, track_height, c);
		}

		if (_tracks[i]== _current_track) {
			Color c;
			c._r= 50; c._g= 50; c._b= 50;
			draw_rect(0, (int)((float)(_screen_height)* y), _screen_width, track_height, c);
		}

		for (unsigned int j=0; j<N_MAX_EVENTS; ++j) {
			if (!_tracks[i]->_events[j]->is_null()) {
				float start= (float)(time_ms(_tracks[i]->_events[j]->_data._t_start))/ (float)(time_ms(_tracks[i]->_duration));
				float end= (float)(time_ms(_tracks[i]->_events[j]->_data._t_end))/ (float)(time_ms(_tracks[i]->_duration));
				
				Color c= get_color(_tracks[i]->_events[j]->_data._key);
				draw_rect((int)((float)(_screen_width)* start), (int)((float)(_screen_height)* y), (int)((float)(_screen_width)* (end- start)), track_height, c);
			}
		}
	}

	// Render to the screen
	SDL_RenderPresent(_renderer);
}


Color LooperSDL::get_color(SDL_Keycode key) {
	if (!_colors.count(key)) {
		Color c;
		c._r= rand_int(100, 200);
		c._g= rand_int(100, 200);
		c._b= rand_int(100, 200);
		_colors[key]= c;
	}
	return _colors[key];
}


void LooperSDL::draw_rect(int x, int y, int w, int h, Color c) {
	SDL_Rect r;
	r.x= x;
	r.y= y;
	r.w= w;
	r.h= h;

	// Set render color
	SDL_SetRenderDrawColor(_renderer, c._r, c._g, c._b, 255);
	// Render rect
	SDL_RenderFillRect(_renderer, &r);
}

