#include "audio_utils.h"

using namespace std;


StereoSample::StereoSample() : _data(0), _n_samples(0) {

}


StereoSample::~StereoSample() {
	delete[] _data;
}


void StereoSample::load_from_file(std::string file_path) {
	SF_INFO info;
	SNDFILE * sound_file= sf_open(file_path.c_str(), SFM_READ, &info);
	
	if (!sound_file) {
		cerr << "ERREUR sf_open : " << file_path << endl;
		return;
	}
	if ((info.channels!= 1) && (info.channels!= 2)) {
		cerr << "ERREUR n_channels = " << info.channels << endl;
		return;
	}

	float buff[info.channels* info.frames];
	sf_read_float(sound_file, buff, info.channels* info.frames);
	sf_close(sound_file);

	if (_data!= 0) {
		delete[] _data;
	}

	_n_samples= info.frames;
	_data= new float[2* _n_samples];
	
	for (unsigned long i=0; i<info.frames; ++i) {
		if (info.channels== 1) {
			_data[2* i+ 0]= buff[i];
			_data[2* i+ 1]= buff[i];
		}
		else {
			_data[2* i+ 0]= buff[2* i+ 0];
			_data[2* i+ 1]= buff[2* i+ 1];
		}
	}
}


// -------------------------------------------------------------------------------------------------------
StereoSampleGL::StereoSampleGL() {

}


StereoSampleGL::StereoSampleGL(GLuint prog_draw_2d, StereoSample * ss, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int screen_w, unsigned int screen_h) :
	 _prog_draw(prog_draw_2d), _first_sample(0.0f), _interval_sample(0.01f), _ss(ss), _mouse_down(false), _data(0)
{
	glGenBuffers(1, &_buffer);
	glGenBuffers(1, &_buffer_background);

	glUseProgram(_prog_draw);
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_camera2clip_loc= glGetUniformLocation(_prog_draw, "camera2clip_matrix");
	glUseProgram(0);
	
	// on veut X, Y entre -... et +...; Z n'existe pas
	memset(_camera2clip, 0, sizeof(float)* 16);
	glm::mat4 glm_ortho= glm::ortho(-GL_WIDTH* 0.5f, GL_WIDTH* 0.5f, -GL_HEIGHT* 0.5f, GL_HEIGHT* 0.5f, -1.0f, 1.0f);
	memcpy(_camera2clip, glm::value_ptr(glm_ortho), sizeof(float)* 16);

	_x= ((float)(x)/ (float)(screen_w)- 0.5f)* GL_WIDTH;
	_y= (0.5f- (float)(y)/ (float)(screen_h))* GL_HEIGHT;
	_w= ((float)(w)/ (float)(screen_w))* GL_WIDTH;
	_h= ((float)(h)/ (float)(screen_h))* GL_HEIGHT;
	cout << _x << " ; " << _y << " ; " << _w << " ; " << _h << "\n";

	float data_background[6* 5];
	data_background[5* 0]= _x;	    data_background[5* 0+ 1]= _y;
	data_background[5* 1]= _x+ _w;	data_background[5* 1+ 1]= _y;
	data_background[5* 2]= _x+ _w;	data_background[5* 2+ 1]= _y+ _h;
	data_background[5* 3]= _x;		data_background[5* 3+ 1]= _y;
	data_background[5* 4]= _x+ _w;	data_background[5* 4+ 1]= _y+ _h;
	data_background[5* 5]= _x;		data_background[5* 5+ 1]= _y+ _h;
	
	for (unsigned int i=0; i<6; ++i) {
		data_background[5* i+ 2]= 0.1; data_background[5* i+ 3]= 0.2; data_background[5* i+ 4]= 0.25;
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffer_background);
	glBufferData(GL_ARRAY_BUFFER, 6* 5* sizeof(float), data_background, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	update_data();
}


StereoSampleGL::~StereoSampleGL() {
	delete[] _data;
}


void StereoSampleGL::draw() {
	if (_ss->_n_samples== 0) {
		return;
	}

	// -----------------------------------------------------------------------------------
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer_background);

	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	// -----------------------------------------------------------------------------------
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);

	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

	// left puis right channel
	glDrawArrays(GL_LINE_STRIP, 0, _n_samples);
	glDrawArrays(GL_LINE_STRIP, _n_samples, _n_samples);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void StereoSampleGL::update_data() {
	if (_ss->_n_samples== 0) {
		return;
	}

	if (_data!= 0) {
		delete[] _data;
	}

	if (_first_sample< 0.0f) {
		_first_sample= 0.0f;
	}
	
	unsigned long first_idx= (long)(ceil(_first_sample));
	unsigned long last_idx= (long)(round(_first_sample+ _w/ _interval_sample));

	if (last_idx>= _ss->_n_samples) {
		last_idx= _ss->_n_samples- 1;
	}

	_n_samples= last_idx- first_idx;
	_data= new float[_n_samples* 2* 5]; // stereo

	// stereo
	for (unsigned long j=0; j<2; ++j)
		for (unsigned long i=first_idx; i<last_idx; ++i) {
			unsigned long idx_data= 5* (i+ j* _n_samples);
			float x= _x+ _interval_sample* (float)(i);
			float y= _y+ (-(float)(j)* 0.5f+ 0.25f)* _h;
			// affichage max == * 0.25f, mais illisible, donc on met moins
			y+= _ss->_data[2* i+ j]* _h* 0.15f;
			
			_data[idx_data+ 0]= x;
			_data[idx_data+ 1]= y;
			_data[idx_data+ 2]= 1.0f;
			_data[idx_data+ 3]= 1.0f;
			_data[idx_data+ 4]= 1.0f;
		}

	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, _n_samples* 5* 2* sizeof(float), _data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool StereoSampleGL::mouse_motion(InputState * input_state) {
	/*if (_mouse_down) {
		if (input_state->_left_mouse) {
			float xx= 2.0f* (float(input_state->_x- WAVE_WIN_X)/ float(WAVE_WIN_WIDTH))- 1.0f;
			_audio->_right_selection= _sample_center+ (long)(xx* (float)(_sample_width));
			if (_audio->_right_selection< _audio->_left_selection) {
				_audio->_right_selection= _audio->_left_selection;
			}
			if (_audio->_right_selection> _audio->_n_samples)
				_audio->_right_selection= _audio->_n_samples;
			update_data();
			return true;
		}
		else if (input_state->_middle_mouse) {
			
		}
		else if (input_state->_right_mouse) {
			_sample_width+= (long)((float)(input_state->_yrel)* 600.0f);
			if (_sample_width< WAVE_SAMPLE_WIDTH_MIN)
				_sample_width= WAVE_SAMPLE_WIDTH_MIN;
			if (_sample_width> WAVE_SAMPLE_WIDTH_MAX)
				_sample_width= WAVE_SAMPLE_WIDTH_MAX;

			_sample_center-= (long)((float)(input_state->_xrel)* (float)(_sample_width)* 0.005f);
			update_data();
			return true;
		}
	}*/
	return false;
}


bool StereoSampleGL::mouse_button_down(InputState * input_state) {
	/*if ((input_state->_x>= WAVE_WIN_X) && (input_state->_x< WAVE_WIN_X+ WAVE_WIN_WIDTH) &&
		(input_state->_y>= MAIN_WIN_HEIGHT- WAVE_WIN_Y- WAVE_WIN_HEIGHT) && (input_state->_y< MAIN_WIN_HEIGHT- WAVE_WIN_Y)) {
		_mouse_down= true;
		if (input_state->_left_mouse) {
			float xx= 2.0f* (float(input_state->_x- WAVE_WIN_X)/ float(WAVE_WIN_WIDTH))- 1.0f;
			_audio->_left_selection= _sample_center+ (long)(xx* (float)(_sample_width));
			if (_audio->_left_selection< 0)
				_audio->_left_selection= 0;
			if (_audio->_left_selection> _audio->_n_samples)
				_audio->_left_selection= _audio->_n_samples;
			_audio->_right_selection= _audio->_left_selection;
			update_data();
		}
		return true;
	}*/
	return false;
}


bool StereoSampleGL::mouse_button_up(InputState * input_state) {
	/*if (_audio->_left_selection== _audio->_right_selection) {
		_audio->_left_selection= 0;
		_audio->_right_selection= _audio->_n_samples;
		update_data();
	}
	_mouse_down= false;*/
	return true;
}


bool StereoSampleGL::key_down(InputState * input_state, SDL_Keycode key) {
	/*if (key== SDLK_LEFT) {
		if (_audio->_mode== AUDIO_STOP) {
			if (_audio->_current_sample- SAMPLES_PER_BUFFER>= 0) {
				update_data();
			}
		}
		return true;
	}
	else if (key== SDLK_RIGHT) {
		if (_audio->_mode== AUDIO_STOP) {
			if (_audio->_current_sample+ SAMPLES_PER_BUFFER< _audio->_n_samples) {
				update_data();
			}
		}
		return true;
	}*/
	return false;
}
