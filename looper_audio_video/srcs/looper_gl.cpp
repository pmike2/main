#include "looper_gl.h"


using namespace std;


LooperGL::LooperGL() {

}


LooperGL::LooperGL(GLuint prog_2d, GLuint prog_font, ScreenGL * screengl) :
	_prog_2d(prog_2d), _prog_font(prog_font), _n_triangles(0), _tap(), _ratio_numerator(0), _screengl(screengl) 
{
	_input_state= new InputState();
	_font= new Font(_prog_font, "../../fonts/Arial.ttf", 24, _screengl->_screen_width, _screengl->_screen_height);

	glUseProgram(_prog_2d);
	_camera2clip_loc= glGetUniformLocation(_prog_2d, "camera2clip_matrix");
	_position_loc= glGetAttribLocation(_prog_2d, "position_in");
	_color_loc= glGetAttribLocation(_prog_2d, "color_in");
	glUseProgram(0);

	_camera2clip= glm::ortho(-_screengl->_gl_width* 0.5f, _screengl->_gl_width* 0.5f, -_screengl->_gl_height* 0.5f, _screengl->_gl_height* 0.5f, -1.0f, 1.0f);

	glGenBuffers(1, &_vbo);

	update_vbo();
}


LooperGL::~LooperGL() {
	
}


bool LooperGL::event_key(SDL_Keycode key) {
	if ((key>= 97) && (key<=122)) {
		return true;
	}
	return false;
}


void LooperGL::key_down(SDL_Keycode key) {
	//cout << "key down : " << key << "\n";
	
	if (_input_state->get_key(key)) {
		return;
	}
	
	_input_state->key_down(key);

	SDL_Keymod key_mod= SDL_GetModState();
	if (KMOD_LSHIFT & key_mod) {
		for (unsigned int i=SDLK_KP_1; i<=SDLK_KP_9; ++i) {
			if (_input_state->get_key(i)) {
				if (_ratio_numerator> 0) {
					unsigned int ratio_denominator= i- SDLK_KP_1+ 1;
					if (_current_track!= _tracks[1]) {
						_current_track->set_ratio_to_master_track(_tracks[1], ratio_type(_ratio_numerator, ratio_denominator));
					}
					_ratio_numerator= 0;
				}
				else {
					_ratio_numerator= i- SDLK_KP_1+ 1;
				}
				break;
			}
		}

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
			//clear();
			_current_track->clear();
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
		return;
	}

	if (event_key(key)) {
		amplitude_type amplitude= 1.0f;
		for (unsigned int i=SDLK_KP_1; i<=SDLK_KP_9; ++i) {
			if (_input_state->get_key(i)) {
				amplitude= (amplitude_type)(i- SDLK_KP_1+ 1)/ 9.0f;
				break;
			}
		}
		note_on(key, amplitude);
		return;
	}
	
}


void LooperGL::key_up(SDL_Keycode key) {
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


void LooperGL::update_vbo() {
	_n_triangles= 1;

	float vertices[_n_triangles* 3* 7];
	vertices[0]= 0.0f;
	vertices[1]= 0.0f;
	vertices[2]= 0.0f;
	vertices[3]= 1.0f;
	vertices[4]= 0.0f;
	vertices[5]= 0.0f;
	vertices[6]= 1.0f;

	vertices[7]= 1.0f;
	vertices[8]= 0.0f;
	vertices[9]= 0.0f;
	vertices[10]= 0.0f;
	vertices[11]= 1.0f;
	vertices[12]= 0.0f;
	vertices[13]= 1.0f;

	vertices[14]= 0.5f;
	vertices[15]= 0.5f;
	vertices[16]= 0.0f;
	vertices[17]= 0.0f;
	vertices[18]= 0.0f;
	vertices[19]= 1.0f;
	vertices[20]= 1.0f;


	
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, _n_triangles* 3* 7* sizeof(float), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void LooperGL::draw() {
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, _screengl->_screen_width, _screengl->_screen_height);
	
	glUseProgram(_prog_2d);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);

	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, glm::value_ptr(_camera2clip));
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_color_loc);

	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)0);
	glVertexAttribPointer(_color_loc, 4, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_triangles* 3);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void LooperGL::tap_tempo() {
	chrono::system_clock::time_point t_now= chrono::system_clock::now();
	time_type tap_diff= t_now- _tap;
	if (tap_diff< chrono::milliseconds(5000)) {
		set_master_track_duration(tap_diff);
	}
	_tap= t_now;
}
