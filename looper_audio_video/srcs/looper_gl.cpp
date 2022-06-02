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


string bool2onoff(bool b) {
	if (b) {
		return "ON";
	}
	return "OFF";
}


LooperGL::LooperGL() {

}


LooperGL::LooperGL(GLuint prog_2d, GLuint prog_font, ScreenGL * screengl) :
	_prog_2d(prog_2d), _prog_font(prog_font), _tap(), _ratio_numerator(0), _screengl(screengl), _current_track_changed(false)/*, _insert_idx_track(-1), _insert_idx_event(-1)*/
{
	_input_state= new InputState();

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

	_font= new Font(_prog_font, "../../fonts/Arial.ttf", 96, _screengl, 1+ 2* N_TRACKS);
	update_text_general();
	for (unsigned int idx_track=0; idx_track<N_TRACKS; ++idx_track) {
		update_text_track_info(idx_track);
		update_text_track_data(idx_track);
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
						update_text_track_info(idx_track);
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
			update_text_track_info(idx_track);
		}
		else if (key== SDLK_h) {
			_current_track->_hold= !_current_track->_hold;
			update_text_track_info(idx_track);
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
			_current_track->clear();
			_current_track_changed= true;
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
			update_vbo_track_info(idx_track);
			update_text_track_info(idx_track);
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
		_current_track_changed= true;
		return;
	}
	
}


void LooperGL::key_up(SDL_Keycode key) {
	//cout << "key up : " << key << "\n";

	unsigned int idx_track= get_current_track_index();

	_input_state->key_up(key);

	SDL_Keymod key_mod= SDL_GetModState();
	if (KMOD_CTRL && key_mod) {
		return;
	}

	if (event_key(key)) {
		note_off();
		_current_track_changed= false;
		update_text_track_data(idx_track);
		return;
	}
}


void LooperGL::tap_tempo() {
	chrono::system_clock::time_point t_now= chrono::system_clock::now();
	time_type tap_diff= t_now- _tap;
	if (tap_diff< chrono::milliseconds(5000)) {
		set_master_track_duration(tap_diff);
		update_text_general();
	}
	_tap= t_now;
}


void LooperGL::update_vbo_general() {
	unsigned int idx_vbo= 0;

	_n_rectangles[idx_vbo]= 1;
	
	float vertices[_n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX];
	RectangleGL rect_general;
	get_general_rectangle(rect_general);
	add_rectangle(vertices, 0, rect_general);
	
	glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_vbo]);
	glBufferData(GL_ARRAY_BUFFER, _n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX* sizeof(float), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void LooperGL::update_vbo_time() {
	unsigned int idx_vbo= 1;

	_n_rectangles[idx_vbo]= N_TRACKS;
	
	float vertices[_n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX];
	unsigned int idx= 0;
	for (unsigned int idx_track=0; idx_track<N_TRACKS; ++idx_track) {
		RectangleGL rect_timeline;
		get_timeline_rectangle(rect_timeline, idx_track);
		idx= add_rectangle(vertices, idx, rect_timeline);
	}
	glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_vbo]);
	glBufferData(GL_ARRAY_BUFFER, _n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX* sizeof(float), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void LooperGL::update_vbo_track_info(unsigned int idx_track) {
	unsigned int idx_vbo= 2+ idx_track;

	_n_rectangles[idx_vbo]= 1+ _tracks[idx_track]->_quantize;

	float vertices[_n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX];

	RectangleGL rect_track_info;
	get_track_info_rectangle(rect_track_info, idx_track);

	unsigned int idx= 0;
	idx= add_rectangle(vertices, idx, rect_track_info);

	if (_tracks[idx_track]->_quantize> 0) {
		for (unsigned int idx_quantize=0; idx_quantize<_tracks[idx_track]->_quantize; ++idx_quantize) {
			RectangleGL rect_quantize;
			get_quantize_rectangle(rect_quantize, idx_track, idx_quantize);
			idx= add_rectangle(vertices, idx, rect_quantize);
		}
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_vbo]);
	glBufferData(GL_ARRAY_BUFFER, _n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX* sizeof(float), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void LooperGL::update_vbo_track_data(unsigned int idx_track) {
	unsigned int idx_vbo= 2+ N_TRACKS+ idx_track;

	_n_rectangles[idx_vbo]= 1+ N_MAX_EVENTS;

	float vertices[_n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX];

	RectangleGL rect_track_data;
	get_track_data_rectangle(rect_track_data, idx_track);

	unsigned int idx= 0;
	idx= add_rectangle(vertices, idx, rect_track_data);
	for (unsigned int idx_event=0; idx_event<N_MAX_EVENTS; ++idx_event) {
		RectangleGL rect_event;
		get_event_rectangle(rect_event, idx_track, idx_event, false);
		idx= add_rectangle(vertices, idx, rect_event);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, _vbos[idx_vbo]);
	glBufferData(GL_ARRAY_BUFFER, _n_rectangles[idx_vbo]* N_VERTICES_PER_RECTANGLE* N_FLOATS_PER_VERTEX* sizeof(float), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void LooperGL::update_text_general() {
	RectangleGL rect_general;
	get_general_rectangle(rect_general);

	vector<Text> texts;
	texts.push_back(Text(to_string(int(get_bpm()))+ " BPM", glm::vec2(rect_general._x, rect_general._y+ rect_general._h- 1.0f), 0.005f, glm::vec4(1.0f, 0.1f, 0.1f, 0.7f)));
	texts.push_back(Text("SHIFT+", glm::vec2(rect_general._x, rect_general._y+ rect_general._h- 2.0f), 0.004f, glm::vec4(1.0f, 1.0f, 1.0f, 0.7f)));
	vector<string> shift_cmds= {"up:prev trk", "down:nxt trk", "1a9x2:ratio", "space:play", "r:record", "t:taptempo", "y:yank", "h:hold", "c:clear", "q:quantize", "d:debug"};
	for (unsigned int i=0; i<shift_cmds.size(); ++i) {
		texts.push_back(Text(shift_cmds[i], glm::vec2(rect_general._x, rect_general._y+ rect_general._h- 2.5f- 0.5f* (float)(i)), 0.004f, glm::vec4(1.0f, 1.0f, 1.0f, 0.7f)));
	}
	texts.push_back(Text("NOSHIFT+", glm::vec2(rect_general._x, rect_general._y+ 0.5f), 0.004f, glm::vec4(1.0f, 1.0f, 1.0f, 0.7f)));
	texts.push_back(Text("1a9:amp", glm::vec2(rect_general._x, rect_general._y), 0.004f, glm::vec4(1.0f, 1.0f, 1.0f, 0.7f)));
	_font->set_text_group(0, texts);
}


void LooperGL::update_text_track_info(unsigned int idx_track) {
	RectangleGL rect_track_info;
	get_track_info_rectangle(rect_track_info, idx_track);

	vector<Text> texts;
	texts.push_back(Text(to_string(idx_track), glm::vec2(rect_track_info._x+ 0.1f, rect_track_info._y+ rect_track_info._h- 0.4f), 0.005f, glm::vec4(1.0f, 1.0f, 1.0f, 0.7f)));
	float scale= 0.003f;
	texts.push_back(Text("Q "+ to_string(_tracks[idx_track]->_quantize), glm::vec2(rect_track_info._x+ 0.1f, rect_track_info._y+ rect_track_info._h- 0.6f), scale, glm::vec4(1.0f, 0.1f, 0.1f, 0.7f)));
	texts.push_back(Text("H "+ bool2onoff(_tracks[idx_track]->_hold), glm::vec2(rect_track_info._x+ 0.1f, rect_track_info._y+ rect_track_info._h- 0.8f), scale, glm::vec4(1.0f, 0.1f, 0.1f, 0.7f)));
	texts.push_back(Text("Y "+ bool2onoff(_tracks[idx_track]->_repeat), glm::vec2(rect_track_info._x+ 0.1f, rect_track_info._y+ rect_track_info._h- 1.0f), scale, glm::vec4(1.0f, 0.1f, 0.1f, 0.7f)));
	texts.push_back(Text("R "+ to_string(_tracks[idx_track]->_ratio_to_master_track.first)+ "/"+ to_string(_tracks[idx_track]->_ratio_to_master_track.second), glm::vec2(rect_track_info._x+ 0.1f, rect_track_info._y+ rect_track_info._h- 1.2f), scale, glm::vec4(1.0f, 0.1f, 0.1f, 0.7f)));
	_font->set_text_group(1+ idx_track, texts);
}


void LooperGL::update_text_track_data(unsigned int idx_track) {
	vector<Text> texts;
	for (unsigned int idx_event=0; idx_event<N_MAX_EVENTS; ++idx_event) {
		if (_tracks[idx_track]->_events[idx_event]->is_null()) {
			continue;
		}
		RectangleGL rect_event;
		get_event_rectangle(rect_event, idx_track, idx_event, false);
		texts.push_back(Text(string(1, _tracks[idx_track]->_events[idx_event]->_data._key), glm::vec2(rect_event._x+ rect_event._w* 0.5f- 0.1f, rect_event._y+ rect_event._h* 0.5f- 0.1f), 0.005f, glm::vec4(1.0f, 1.0f, 1.0f, 0.7f)));
	}
	_font->set_text_group(1+ N_TRACKS+ idx_track, texts);
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

	_font->draw();
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


float LooperGL::get_event_x(unsigned int idx_track, unsigned int idx_event) {
	return (-0.5f+ GENERAL_INFO_WIDTH+ TRACK_INFO_WIDTH)* _screengl->_gl_width+ _screengl->_gl_width* (1.0f- GENERAL_INFO_WIDTH- TRACK_INFO_WIDTH)* (float)(time_ms(_tracks[idx_track]->_events[idx_event]->_data._t_start))/ (float)(time_ms(_tracks[idx_track]->_duration));
}


float LooperGL::get_event_w(unsigned int idx_track, unsigned int idx_event, bool until_now) {
	if (until_now) {
		time_type now_time= now();
		return _screengl->_gl_width* (1.0f- GENERAL_INFO_WIDTH- TRACK_INFO_WIDTH)* (float)(time_ms(_tracks[idx_track]->get_relative_t(now_time))- time_ms(_tracks[idx_track]->_events[idx_event]->_data._t_start))/ (float)(time_ms(_tracks[idx_track]->_duration));
	}
	else {
		return _screengl->_gl_width* (1.0f- GENERAL_INFO_WIDTH- TRACK_INFO_WIDTH)* (float)(time_ms(_tracks[idx_track]->_events[idx_event]->_data._t_end)- time_ms(_tracks[idx_track]->_events[idx_event]->_data._t_start))/ (float)(time_ms(_tracks[idx_track]->_duration));
	}
}


float LooperGL::get_track_now(unsigned int idx_track) {
	time_type now_time= now();
	return (-0.5f+ GENERAL_INFO_WIDTH+ TRACK_INFO_WIDTH)* _screengl->_gl_width+ _screengl->_gl_width* (1.0f- GENERAL_INFO_WIDTH- TRACK_INFO_WIDTH)* (float)(time_ms(_tracks[idx_track]->get_relative_t(now_time)))/ (float)(time_ms(_tracks[idx_track]->_duration));
}


void LooperGL::get_general_rectangle(RectangleGL & rect) {
	rect._x= -0.5f* _screengl->_gl_width;
	rect._y= -0.5f* _screengl->_gl_height;
	rect._z= 0.0f;
	rect._w= GENERAL_INFO_WIDTH* _screengl->_gl_width;
	rect._h= _screengl->_gl_height;
	rect._r= 0.3f;
	rect._g= 0.1f;
	rect._b= 0.1f;
	rect._a= 0.5f;
}


void LooperGL::get_timeline_rectangle(RectangleGL & rect, unsigned int idx_track) {
	rect._x= get_track_now(idx_track);
	rect._y= get_track_y(idx_track)+ EPS;
	rect._z= 0.0f;
	rect._w= 0.05f;
	rect._h= get_track_h();
	rect._r= 1.0f;
	rect._g= 1.0f;
	rect._b= 1.0f;
	rect._a= 0.8f;
}


void LooperGL::get_track_info_rectangle(RectangleGL & rect, unsigned int idx_track) {
	unsigned int current_idx_track= get_current_track_index();

	rect._x= (-0.5f+ GENERAL_INFO_WIDTH)* _screengl->_gl_width+ EPS;
	rect._y= get_track_y(idx_track)+ EPS;
	rect._z= 0.0f;
	rect._w= 0.1f* _screengl->_gl_width- 2.0f* EPS;
	rect._h= get_track_h()- 2.0f* EPS;

	if (idx_track== current_idx_track) {
		rect._r= 0.0f;
		rect._g= 0.5f;
		rect._b= 0.0f;
		rect._a= 0.8f;
	}
	else {
		rect._r= 0.0f;
		rect._g= 0.5f;
		rect._b= 0.0f;
		rect._a= 0.5f;
	}
}


void LooperGL::get_quantize_rectangle(RectangleGL & rect, unsigned int idx_track, unsigned int idx_quantize) {
	rect._x= (-0.5f+ GENERAL_INFO_WIDTH+ TRACK_INFO_WIDTH)* _screengl->_gl_width+ EPS+ (float)(idx_quantize)* _screengl->_gl_width* (1.0f- GENERAL_INFO_WIDTH- TRACK_INFO_WIDTH)/ (float)(_tracks[idx_track]->_quantize);
	rect._y= get_track_y(idx_track)+ EPS;
	rect._z= 0.0f;
	rect._w= 0.05f;
	rect._h= get_track_h()- 2.0f* EPS;
	rect._r= 1.0f;
	rect._g= 1.0f;
	rect._b= 1.0f;
	rect._a= 0.5f;
}


void LooperGL::get_track_data_rectangle(RectangleGL & rect, unsigned int idx_track) {
	rect._x= (-0.5f+ GENERAL_INFO_WIDTH+ TRACK_INFO_WIDTH)* _screengl->_gl_width+ EPS;
	rect._y= get_track_y(idx_track)+ EPS;
	rect._z= 0.0f;
	rect._w= (1.0f- GENERAL_INFO_WIDTH- TRACK_INFO_WIDTH)* _screengl->_gl_width- 2.0f* EPS;
	rect._h= get_track_h()- 2.0f* EPS;
	rect._r= 0.2f;
	rect._g= 0.3f;
	rect._b= 0.5f;
	rect._a= 0.5f;
}


void LooperGL::get_event_rectangle(RectangleGL & rect, unsigned int idx_track, unsigned int idx_event, bool until_now) {
	glm::vec3 event_color= get_color(_tracks[idx_track]->_events[idx_event]->_data._key);
	float a= _tracks[idx_track]->_events[idx_event]->_data._amplitude;

	rect._y= get_track_y(idx_track);
	rect._z= 0.0f;
	rect._h= get_track_h();
	rect._r= event_color.x;
	rect._g= event_color.y;
	rect._b= event_color.z;
	rect._a= a;

	if (!_tracks[idx_track]->_events[idx_event]->is_null()) {
		rect._x= get_event_x(idx_track, idx_event);
		rect._w= get_event_w(idx_track, idx_event, until_now);
	}
	else {
		rect._x= 1e7f;
		rect._w= 0.0f;
	}
}
