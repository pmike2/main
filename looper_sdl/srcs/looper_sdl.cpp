#include "looper_sdl.h"
#include "utile.h"


using namespace std;


LooperSDL::LooperSDL() {
	
}


LooperSDL::LooperSDL(SDL_Renderer * renderer, int screen_width, int screen_height) : 
	_renderer(renderer), _screen_width(screen_width), _screen_height(screen_height), _tap()
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
	
	if (_input_state->get_key(key)) {
		return;
	}
	
	_input_state->key_down(key);

	SDL_Keymod key_mod= SDL_GetModState();
	if (KMOD_LSHIFT & key_mod) {
		if (key== SDLK_d) {
			debug();
		}
		else if (key== SDLK_SPACE) {
			toggle_start();
		}
		else if (key== SDLK_r) {
			toggle_record();
		}
		else if (key== SDLK_t) {
			tap_tempo();
		}
		else if (key== SDLK_y) {
			_current_track->_repeat= !_current_track->_repeat;
		}
		else if (key== SDLK_h) {
			_current_track->_hold= !_current_track->_hold;
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
		else if (key== SDLK_q) {
			if (_current_track->_quantize== 0) {
				_current_track->_quantize= 1;
			}
			else {
				_current_track->set_quantize(_current_track->_quantize* 2);
				if (_current_track->_quantize>= 128) {
					_current_track->set_quantize(0);
				}
			}
		}
		else if (key== SDLK_KP_1) {
			set_track_duration_ratio(_current_track, 1.0);
		}
		else if (key== SDLK_KP_2) {
			set_track_duration_ratio(_current_track, 0.5);
		}
		return;
	}

	if (event_key(key)) {
		amplitude_type amplitude= 1.0f;
		for (unsigned int i=SDLK_KP_1; i<SDLK_KP_9; ++i) {
			if (_input_state->get_key(i)) {
				amplitude= (amplitude_type)(i- SDLK_KP_1+ 1)/ 9.0f;
				break;
			}
		}
		note_on(key, amplitude);
		return;
	}
	
}


void LooperSDL::key_up(SDL_Keycode key) {
	//cout << "key up : " << key << "\n";

	_input_state->key_up(key);

	SDL_Keymod key_mod= SDL_GetModState();
	if (KMOD_CTRL && key_mod) {
		return;
	}

	if (event_key(key)) {
		note_off();
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
		int y= (int)((float)(_screen_height* i)/ (float)(N_MAX_TRACKS));

		// rect track
		if (_tracks[i]== _current_track) {
			SDL_Color c;
			c.r= 40; c.g= 40; c.b= 40; c.a= 100;
			draw_rect(0, y, _screen_width, track_height, c);
		}

		// events
		for (unsigned int j=0; j<N_MAX_EVENTS; ++j) {
			if (!_tracks[i]->_events[j]->is_null()) {
				int x= (int)((float)(_screen_width* time_ms(_tracks[i]->_events[j]->_data._t_start))/ (float)(time_ms(_tracks[i]->_duration)));
				int w= (int)((float)(_screen_width* (time_ms(_tracks[i]->_events[j]->_data._t_end)- time_ms(_tracks[i]->_events[j]->_data._t_start)))/ (float)(time_ms(_tracks[i]->_duration)));
				
				SDL_Color c= get_color(_tracks[i]->_events[j]->_data._key);
				c.a= int(_tracks[i]->_events[j]->_data._amplitude* 255.0f);
				draw_rect(x, y, w, track_height, c);
			}
		}

		// time cursor
		if ((_mode== RUNNING) || (_mode== RECORDING)) {
			int x= (int)((float)(_screen_width* time_ms(_tracks[i]->get_relative_t(now_time)))/ (float)(time_ms(_tracks[i]->_duration)));
			SDL_Color c;
			if (_mode== RUNNING) {
				c.r= 250; c.g= 250; c.b= 200;
			}
			else if (_mode== RECORDING) {
				c.r= 250; c.g= 50; c.b= 50;
			}
			c.a= 100;
			draw_rect(x, y, 2, track_height, c);
		}

		// repères quantize
		if (_current_track->_quantize) {
			for (unsigned int i=0; i<_current_track->_quantize; ++i) {
				int x= (int)((float)(i* _screen_width)/ (float)(_current_track->_quantize));
				SDL_Color c;
				c.r= 80; c.g= 80; c.b= 80; c.a= 200;
				draw_rect(x, y, 1, track_height, c);
			}
		}
	}

	SDL_RenderPresent(_renderer);
}


SDL_Color LooperSDL::get_color(SDL_Keycode key) {
	if (!_colors.count(key)) {
		SDL_Color c;
		c.r= rand_int(100, 200);
		c.g= rand_int(100, 200);
		c.b= rand_int(100, 200);
		_colors[key]= c;
	}
	return _colors[key];
}


void LooperSDL::draw_rect(int x, int y, int w, int h, SDL_Color c) {
	SDL_Rect r;
	r.x= x;
	r.y= y;
	r.w= w;
	r.h= h;

	// Set render color
	SDL_SetRenderDrawColor(_renderer, c.r, c.g, c.b, c.a);
	// Render rect
	SDL_RenderFillRect(_renderer, &r);
}


void LooperSDL::tap_tempo() {
	chrono::system_clock::time_point t_now= chrono::system_clock::now();
	if (_tap.time_since_epoch().count()!= 0) {
		time_type tap_diff= t_now- _tap;
		set_master_track_duration(tap_diff);
	}
	_tap= t_now;
}
