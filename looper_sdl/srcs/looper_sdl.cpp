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

	if ((_current_event) && (_current_event->_hold)) {
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
	_input_state->key_up(key);
}


void LooperSDL::draw() {
	// background color
	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
	SDL_RenderClear(_renderer);

	for (unsigned int i=0; i<N_MAX_TRACKS; ++i) {
		for (unsigned int j=0; j<N_MAX_EVENTS; ++j) {
			if (!_tracks[i]->_events[j]->is_null()) {
				float start= (float)(time_ms(_tracks[i]->_events[j]->_t_start))/ (float)(time_ms(_tracks[i]->_duration));
				float end= (float)(time_ms(_tracks[i]->_events[j]->_t_end))/ (float)(time_ms(_tracks[i]->_duration));
				float y= (float)(i)/ (float)(N_MAX_TRACKS);
				
				// Creat a rect
				SDL_Rect r;
				r.x = (int)((float)(_screen_width)* start);
				r.y = (int)((float)(_screen_height)* y);
				r.w = (int)((float)(_screen_width)* (end- start));
				r.h = 10;

				Color c= get_color(_tracks[i]->_events[j]->_key);

				// Set render color
				SDL_SetRenderDrawColor(_renderer, c._r, c._g, c._b, 255);
				// Render rect
				SDL_RenderFillRect(_renderer, &r);
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

