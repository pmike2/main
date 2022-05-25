#include "looper_gl.h"
#include "utile.h"


using namespace std;


unsigned int add_rectangle(float * vertices, unsigned int idx, RectangleGL & rect) {
	vertices[idx+ 0]= rect._x;
	vertices[idx+ 1]= rect._y;

	vertices[idx+ 7]= rect._x+ rect._w;
	vertices[idx+ 8]= rect._y;

	vertices[idx+ 14]= rect._x;
	vertices[idx+ 15]= rect._y+ rect._h;

	vertices[idx+ 21]= rect._x+ rect._w;
	vertices[idx+ 22]= rect._y;

	vertices[idx+ 28]= rect._x+ rect._w;
	vertices[idx+ 29]= rect._y+ rect._h;

	vertices[idx+ 35]= rect._x;
	vertices[idx+ 36]= rect._y+ rect._h;

	for (unsigned int i=0; i<6; ++i) {
		vertices[idx+ i* 7+ 2]= rect._z;
		vertices[idx+ i* 7+ 3]= rect._r;
		vertices[idx+ i* 7+ 4]= rect._g;
		vertices[idx+ i* 7+ 5]= rect._b;
		vertices[idx+ i* 7+ 6]= rect._a;
	}

	return idx+ 42;
}


LooperGL::LooperGL() {

}


LooperGL::LooperGL(GLuint prog_2d, GLuint prog_font, ScreenGL * screengl) :
	_prog_2d(prog_2d), _prog_font(prog_font), _tap(), _ratio_numerator(0), _screengl(screengl), _insert_idx_track(-1), _insert_idx_event(-1)
{
	_input_state= new InputState();
	_font= new Font(_prog_font, "../../fonts/Arial.ttf", 24, _screengl->_screen_width, _screengl->_screen_height);

	glUseProgram(_prog_2d);
	_camera2clip_loc= glGetUniformLocation(_prog_2d, "camera2clip_matrix");
	_position_loc= glGetAttribLocation(_prog_2d, "position_in");
	_color_loc= glGetAttribLocation(_prog_2d, "color_in");
	glUseProgram(0);

	_camera2clip= glm::ortho(-_screengl->_gl_width* 0.5f, _screengl->_gl_width* 0.5f, -_screengl->_gl_height* 0.5f, _screengl->_gl_height* 0.5f, -1.0f, 1.0f);

	glGenBuffers(2+ 2* N_TRACKS, _vbos);

	update_vbo_general();
	update_vbo_time();
	for (unsigned int idx_track=0; idx_track<N_TRACKS; ++idx_track) {
		update_vbo_track_info(idx_track);
		update_vbo_track_data(idx_track);
	}
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

	unsigned int idx_track= get_current_track_index();

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
					if (idx_track!= 1) {
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
			update_vbo_track_info(idx_track);
			update_vbo_track_info(get_current_track_index());
		}
		else if (key== SDLK_UP) {
			set_previous_track();
			update_vbo_track_info(idx_track);
			update_vbo_track_info(get_current_track_index());
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
		_insert_idx_track= idx_track;
		_insert_idx_event= _current_track->get_event_idx(_current_track->_inserted_event);
		return;
	}
	
}


void LooperGL::key_up(SDL_Keycode key) {
	//cout << "key up : " << key << "\n";

	//unsigned int idx_track= get_current_track_index();

	_input_state->key_up(key);

	SDL_Keymod key_mod= SDL_GetModState();
	if (KMOD_CTRL && key_mod) {
		return;
	}

	if (event_key(key)) {
		note_off();
		_insert_idx_track= -1;
		_insert_idx_event= -1;
		return;
	}
}


void LooperGL::update_vbo_general() {
	unsigned int idx_vbo= 0;

	_n_rectangles[idx_vbo]= 1;
	
	RectangleGL general_info= {
		-0.5f* _screengl->_gl_width,
		-0.5f* _screengl->_gl_height,
		0.0f,
		GENERAL_INFO_WIDTH* _screengl->_gl_width,
		_screengl->_gl_height,
		1.0f, 0.0f, 0.0f, 0.5f
	};

	float vertices[_n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX];
	add_rectangle(vertices, 0, general_info);
	
	glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_vbo]);
	glBufferData(GL_ARRAY_BUFFER, _n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX* sizeof(float), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void LooperGL::update_vbo_time() {
	time_type now_time= now();
	
	unsigned int idx_vbo= 1;

	_n_rectangles[idx_vbo]= N_TRACKS;
	
	float vertices[_n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX];
	unsigned int idx= 0;
	RectangleGL timeline;
	for (unsigned int idx_track=0; idx_track<N_TRACKS; ++idx_track) {
		timeline._x= (-0.5f+ GENERAL_INFO_WIDTH+ TRACK_INFO_WIDTH)* _screengl->_gl_width+ _screengl->_gl_width* (1.0f- GENERAL_INFO_WIDTH- TRACK_INFO_WIDTH)* (float)(time_ms(_tracks[idx_track]->get_relative_t(now_time)))/ (float)(time_ms(_tracks[idx_track]->_duration));
		timeline._y= get_track_y(idx_track)+ EPS;
		timeline._z= 0.0f;
		timeline._w= 0.05f;
		timeline._h= get_track_h();
		timeline._r= 1.0f;
		timeline._g= 1.0f;
		timeline._b= 1.0f;
		timeline._a= 1.0f;
		idx= add_rectangle(vertices, idx, timeline);
	}
	glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_vbo]);
	glBufferData(GL_ARRAY_BUFFER, _n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX* sizeof(float), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void LooperGL::update_vbo_track_info(unsigned int idx_track) {
	unsigned int idx_vbo= 2+ idx_track;

	_n_rectangles[idx_vbo]= 1;

	unsigned int current_idx_track= get_current_track_index();

	RectangleGL track_info;
	track_info._x= (-0.5f+ GENERAL_INFO_WIDTH)* _screengl->_gl_width+ EPS;
	track_info._y= get_track_y(idx_track)+ EPS;
	track_info._z= 0.0f;
	track_info._w= 0.1f* _screengl->_gl_width- 2.0f* EPS;
	track_info._h= get_track_h()- 2.0f* EPS;

	if (idx_track== current_idx_track) {
		track_info._r= 0.0f;
		track_info._g= 1.0f;
		track_info._b= 0.0f;
		track_info._a= 0.8f;
	}
	else {
		track_info._r= 0.0f;
		track_info._g= 1.0f;
		track_info._b= 0.0f;
		track_info._a= 0.5f;
	}
	float vertices[_n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX];
	add_rectangle(vertices, 0, track_info);
	
	glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_vbo]);
	glBufferData(GL_ARRAY_BUFFER, _n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX* sizeof(float), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void LooperGL::update_vbo_track_data(unsigned int idx_track) {
	unsigned int idx_vbo= 2+ N_TRACKS+ idx_track;

	RectangleGL track_data;
	track_data._x= (-0.5f+ GENERAL_INFO_WIDTH+ TRACK_INFO_WIDTH)* _screengl->_gl_width+ EPS;
	track_data._y= get_track_y(idx_track)+ EPS;
	track_data._z= 0.0f;
	track_data._w= (1.0f- GENERAL_INFO_WIDTH- TRACK_INFO_WIDTH)* _screengl->_gl_width- 2.0f* EPS;
	track_data._h= get_track_h()- 2.0f* EPS;
	track_data._r= 0.0f;
	track_data._g= 0.0f;
	track_data._b= 1.0f;
	track_data._a= 0.5f;

	RectangleGL events[N_MAX_EVENTS];
	for (unsigned int idx_event=0; idx_event<N_MAX_EVENTS; ++idx_event) {
		if (!_tracks[idx_track]->_events[idx_event]->is_null()) {
			glm::vec3 event_color= get_color(_tracks[idx_track]->_events[idx_event]->_data._key);
			float a= _tracks[idx_track]->_events[idx_event]->_data._amplitude;

			events[idx_event]._x= (-0.5f+ GENERAL_INFO_WIDTH+ TRACK_INFO_WIDTH)* _screengl->_gl_width+ _screengl->_gl_width* (1.0f- GENERAL_INFO_WIDTH- TRACK_INFO_WIDTH)* (float)(time_ms(_tracks[idx_track]->_events[idx_event]->_data._t_start))/ (float)(time_ms(_tracks[idx_track]->_duration));
			events[idx_event]._y= get_track_y(idx_track);
			events[idx_event]._z= 0.0f;
			events[idx_event]._w= _screengl->_gl_width* (1.0f- GENERAL_INFO_WIDTH- TRACK_INFO_WIDTH)* (float)(time_ms(_tracks[idx_track]->_events[idx_event]->_data._t_end)- time_ms(_tracks[idx_track]->_events[idx_event]->_data._t_start))/ (float)(time_ms(_tracks[idx_track]->_duration));
			events[idx_event]._h= get_track_h();
			events[idx_event]._r= event_color.x;
			events[idx_event]._g= event_color.y;
			events[idx_event]._b= event_color.z;
			events[idx_event]._a= a;
		}
		else {
			events[idx_event]._x= events[idx_event]._y= events[idx_event]._z= events[idx_event]._w= events[idx_event]._h= events[idx_event]._r= events[idx_event]._g= events[idx_event]._b= events[idx_event]._a= 0.0f;
		}
	}

	_n_rectangles[idx_vbo]= 1+ N_MAX_EVENTS;
	float vertices[_n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX];
	unsigned int idx= 0;
	idx= add_rectangle(vertices, idx, track_data);
	for (unsigned int idx_event=0; idx_event<N_MAX_EVENTS; ++idx_event) {
		idx= add_rectangle(vertices, idx, events[idx_event]);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_vbo]);
	glBufferData(GL_ARRAY_BUFFER, _n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX* sizeof(float), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void LooperGL::update_vbo_insert() {
	if ((_insert_idx_track< 0) || (_insert_idx_event< 0)) {
		return;
	}

	unsigned int idx_vbo= 2+ N_TRACKS+ _insert_idx_track;
	time_type now_time= now();
	float w= _screengl->_gl_width* (1.0f- GENERAL_INFO_WIDTH- TRACK_INFO_WIDTH)* (float)(time_ms(now_time)- time_ms(_tracks[_insert_idx_track]->_events[_insert_idx_event]->_data._t_start))/ (float)(time_ms(_tracks[_insert_idx_track]->_duration));
	glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_vbo]);
	glBufferSubData(GL_ARRAY_BUFFER, (1+ _insert_idx_event)* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX* sizeof(float), sizeof(float), &w);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void LooperGL::draw() {
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, _screengl->_screen_width, _screengl->_screen_height);
	
	glUseProgram(_prog_2d);
	for (unsigned int idx_vbo=0; idx_vbo<2+ 2* N_TRACKS; ++idx_vbo) {
		glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_vbo]);

		glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, glm::value_ptr(_camera2clip));
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_color_loc);

		glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, N_FLOATS_PER_VERTEX* sizeof(float), (void*)0);
		glVertexAttribPointer(_color_loc, 4, GL_FLOAT, GL_FALSE, N_FLOATS_PER_VERTEX* sizeof(float), (void*)(3* sizeof(float)));

		glDrawArrays(GL_TRIANGLES, 0, _n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glUseProgram(0);

	_font->draw("hello", _screengl->_screen_width-50, _screengl->_screen_height-50, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
}


void LooperGL::tap_tempo() {
	chrono::system_clock::time_point t_now= chrono::system_clock::now();
	time_type tap_diff= t_now- _tap;
	if (tap_diff< chrono::milliseconds(5000)) {
		set_master_track_duration(tap_diff);
	}
	_tap= t_now;
}


glm::vec3 LooperGL::get_color(SDL_Keycode key) {
	if (!_event_colors.count(key)) {
		_event_colors[key]= glm::vec3(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f));
	}
	return _event_colors[key];
}


float LooperGL::get_track_y(unsigned int idx_track) {
	return -0.5f* _screengl->_gl_height+ (N_TRACKS- 1- idx_track)* _screengl->_gl_height/ (float)(N_TRACKS);
}


float LooperGL::get_track_h() {
	return _screengl->_gl_height/ (float)(N_TRACKS);
}

