#include "looper_sdl.h"
#include "utile.h"


using namespace std;


LooperSDL::LooperSDL() {
	
}


LooperSDL::LooperSDL(SDL_Renderer * renderer, int screen_width, int screen_height) : 
	_current_event(0), _current_event_key(0), _renderer(renderer), _screen_width(screen_width), _screen_height(screen_height)
{
	_input_state= new InputState();
}


LooperSDL::~LooperSDL() {
	delete _input_state;
}


void LooperSDL::key_down(SDL_Keycode key) {
	//cout << "key down : " << key << "\n";
	
	if (key== SDLK_SPACE) {
		debug();
		return;
	}
	else if (key== SDLK_DOWN) {
		set_next_track();
		return;
	}
	else if (key== SDLK_UP) {
		set_previous_track();
		return;
	}
	else if (key== SDLK_a) {
		clear();
		return;
	}
	else if (key== SDLK_KP_1) {
		set_current_track_duration_ratio(1.0);
		return;
	}
	else if (key== SDLK_KP_2) {
		set_current_track_duration_ratio(0.5);
		return;
	}

	if ((_current_event_key!= key) && (_current_event) && (_current_event->_hold)) {
		set_event_end(_current_event);
		_current_event= 0;
	}

	if (!_input_state->get_key(key)) {
		_current_event= insert_event(key, true);
		_current_event_key= key;
	}
	_input_state->key_down(key);
}


void LooperSDL::key_up(SDL_Keycode key) {
	//cout << "key up : " << key << "\n";

	_input_state->key_up(key);

	if (_current_event== 0) {
		return;
	}

	if (key!= _current_event_key) {
		return;
	}

	if (_current_event->_hold) {
		set_event_end(_current_event);
	}
	_current_event= 0;
	_current_event_key= 0;
}


void LooperSDL::draw() {
	// background color
	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
	SDL_RenderClear(_renderer);

	time_type now_time= now();
	unsigned int track_height= _screen_height/ N_MAX_TRACKS;

	for (unsigned int i=0; i<N_MAX_TRACKS; ++i) {
		float y= (float)(i)/ (float)(N_MAX_TRACKS);
		float t_track= (float)(time_ms(_tracks[i]->get_relative_t(now_time)))/ (float)(time_ms(_tracks[i]->_duration));

		if (_tracks[i]== _current_track) {
			Color c;
			c._r= 50; c._g= 50; c._b= 50;
			draw_rect(0, (int)((float)(_screen_height)* y), _screen_width, track_height, c);
		}

		Color c;
		c._r= 250; c._g= 250; c._b= 200;
		draw_rect((int)((float)(_screen_width)* t_track), (int)((float)(_screen_height)* y), 2, track_height, c);

		for (unsigned int j=0; j<N_MAX_EVENTS; ++j) {
			if (!_tracks[i]->_events[j]->is_null()) {
				float start= (float)(time_ms(_tracks[i]->_events[j]->_t_start))/ (float)(time_ms(_tracks[i]->_duration));
				float end= (float)(time_ms(_tracks[i]->_events[j]->_t_end))/ (float)(time_ms(_tracks[i]->_duration));
				
				Color c= get_color(_tracks[i]->_events[j]->_key);
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

