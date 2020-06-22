#include "audio_utils.h"

using namespace std;


StereoSample::StereoSample() : _data_left(0), _data_right(0), _n_samples(0) {

}


StereoSample::~StereoSample() {
	delete[] _data_left;
	delete[] _data_right;
}


void StereoSample::load_from_file(string file_path) {
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

	if (_data_left!= 0) {
		delete[] _data_left;
	}

	if (_data_right!= 0) {
		delete[] _data_right;
	}

	_n_samples= info.frames;
	_data_left= new float[_n_samples];
	_data_right= new float[_n_samples];
	
	for (unsigned long i=0; i<info.frames; ++i) {
		if (info.channels== 1) {
			_data_left[i]= buff[i];
			_data_right[i]= buff[i];
		}
		else {
			_data_left[i]= buff[2* i+ 0];
			_data_right[i]= buff[2* i+ 1];
		}
	}
}


// -------------------------------------------------------------------------------------------------------
AudioMark::AudioMark() {

}


AudioMark::AudioMark(unsigned long track_sample, int idx_sample, unsigned long sample_sample) : _track_sample(track_sample), _idx_sample(idx_sample), _sample_sample(sample_sample) {

}


AudioMark::~AudioMark() {

}


// -------------------------------------------------------------------------------------------------------
StereoTrack::StereoTrack() {

}


StereoTrack::~StereoTrack() {
	for (auto mark : _marks) {
		delete mark;
	}
	_marks.clear();
}


void StereoTrack::add_mark(unsigned long track_sample, int idx_sample, unsigned long sample_sample) {
	AudioMark * mark= new AudioMark(track_sample, idx_sample, sample_sample);
	_marks.push_back(mark);
}


// -------------------------------------------------------------------------------------------------------
AudioProject::AudioProject() {

}


AudioProject::~AudioProject() {
	for (auto ss : _samples) {
		delete ss;
	}
	_samples.clear();

	for (auto track : _tracks) {
		delete track;
	}
	_tracks.clear();
}


void AudioProject::add_sample(string file_path) {
	StereoSample * ss= new StereoSample();
	ss->load_from_file(file_path);
	_samples.push_back(ss);
}


void AudioProject::add_track() {
	StereoTrack * track= new StereoTrack();
	_tracks.push_back(track);
}


// -------------------------------------------------------------------------------------------------------
StereoSampleGL::StereoSampleGL() {

}


StereoSampleGL::StereoSampleGL(GLuint prog_draw_2d, StereoSample * ss, ScreenGL * screengl, unsigned int x, unsigned int y, unsigned int w, unsigned int h) :
	 _prog_draw(prog_draw_2d), _first_sample(0), _last_sample(0), _n_samples(0), _interval_sample(INIT_INTERVAL_SAMPLE), _ss(ss), _screengl(screengl), _mouse_down(false),
	 _first_selection(0), _last_selection(0), _is_selection(false)
{
	glGenBuffers(1, &_buffer_left);
	glGenBuffers(1, &_buffer_right);
	glGenBuffers(1, &_buffer_background);
	glGenBuffers(1, &_buffer_lines);
	glGenBuffers(1, &_buffer_selection);

	glUseProgram(_prog_draw);
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_camera2clip_loc= glGetUniformLocation(_prog_draw, "camera2clip_matrix");
	glUseProgram(0);
	
	// on veut X, Y entre -... et +...; Z n'existe pas
	memset(_camera2clip, 0, sizeof(float)* 16);
	glm::mat4 glm_ortho= glm::ortho(-_screengl->_gl_width* 0.5f, _screengl->_gl_width* 0.5f, -_screengl->_gl_height* 0.5f, _screengl->_gl_height* 0.5f, -1.0f, 1.0f);
	memcpy(_camera2clip, glm::value_ptr(glm_ortho), sizeof(float)* 16);

	_screengl->screen2gl(x, y, _x, _y);
	float x_tmp, y_tmp;
	_screengl->screen2gl(x+ w, y- h, x_tmp, y_tmp);
	_w= x_tmp- _x;
	_h= y_tmp- _y;
	
	//cout << _x << " ; " << _y << " ; " << _w << " ; " << _h << "\n";

	// background ----------------------------------------------
	float data_background[6* 6];
	data_background[6* 0]= _x;	    data_background[6* 0+ 1]= _y;
	data_background[6* 1]= _x+ _w;	data_background[6* 1+ 1]= _y;
	data_background[6* 2]= _x+ _w;	data_background[6* 2+ 1]= _y+ _h;
	data_background[6* 3]= _x;		data_background[6* 3+ 1]= _y;
	data_background[6* 4]= _x+ _w;	data_background[6* 4+ 1]= _y+ _h;
	data_background[6* 5]= _x;		data_background[6* 5+ 1]= _y+ _h;
	
	for (unsigned int i=0; i<6; ++i) {
		data_background[6* i+ 2]= BACKGROUND_COLOR[0]; data_background[6* i+ 3]= BACKGROUND_COLOR[1]; data_background[6* i+ 4]= BACKGROUND_COLOR[2]; data_background[6* i+ 5]= BACKGROUND_COLOR[3];
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffer_background);
	glBufferData(GL_ARRAY_BUFFER, 6* 6* sizeof(float), data_background, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// lines --------------------------------------------------
	float data_lines[6* 6];
	data_lines[6* 0]= _x; 	  data_lines[6* 0+ 1]= _y+ 0.75f* _h;
	data_lines[6* 1]= _x+ _w; data_lines[6* 1+ 1]= _y+ 0.75f* _h;
	data_lines[6* 2]= _x; 	  data_lines[6* 2+ 1]= _y+ 0.5f* _h;
	data_lines[6* 3]= _x+ _w; data_lines[6* 3+ 1]= _y+ 0.5f* _h;
	data_lines[6* 4]= _x; 	  data_lines[6* 4+ 1]= _y+ 0.25f* _h;
	data_lines[6* 5]= _x+ _w; data_lines[6* 5+ 1]= _y+ 0.25f* _h;
	
	for (unsigned int i=0; i<6; ++i) {
		data_lines[6* i+ 2]= LINES_COLOR[0]; data_lines[6* i+ 3]= LINES_COLOR[1]; data_lines[6* i+ 4]= LINES_COLOR[2]; data_lines[6* i+ 5]= LINES_COLOR[3];
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffer_lines);
	glBufferData(GL_ARRAY_BUFFER, 6* 6* sizeof(float), data_lines, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// -------------------------------------------------------
	update_data();
}


StereoSampleGL::~StereoSampleGL() {

}


void StereoSampleGL::draw() {
	if (_ss->_n_samples== 0) {
		return;
	}
	
	// background -----------------------------------------------------------------------------------
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer_background);

	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 4, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	// lines -----------------------------------------------------------------------------------
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer_lines);

	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 4, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_LINES, 0, 6);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	// buffer_left -----------------------------------------------------------------------------------
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer_left);

	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 4, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_LINE_STRIP, 0, _n_samples);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	// buffer_right -----------------------------------------------------------------------------------
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer_right);

	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 4, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_LINE_STRIP, 0, _n_samples);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	// selection -----------------------------------------------------------------------------------
	if (_is_selection) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffer_selection);

		glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_diffuse_color_loc);

		glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 4, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(2* sizeof(float)));

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_diffuse_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}
}


void StereoSampleGL::update_data() {
	if (_ss->_n_samples== 0) {
		return;
	}
	
	if (_first_sample>= _ss->_n_samples) {
		_first_sample= _ss->_n_samples- 1;
	}
	
	_last_sample= _first_sample+ (unsigned long)(round(_w/ _interval_sample));

	if (_last_sample>= _ss->_n_samples) {
		_last_sample= _ss->_n_samples- 1;
	}

	_n_samples= _last_sample- _first_sample;

	//cout << "_interval_sample=" << _interval_sample << " ; _first_sample=" << _first_sample << " ; _last_sample=" << _last_sample << " ; _n_samples=" << _n_samples << "\n";

	if (_n_samples<= 0) {
		_n_samples= 0;
		return;
	}

	// left channel ------------------------------------------------
	float * _data_left= new float[_n_samples* 6];

	for (unsigned long i=0; i<_n_samples; ++i) {
		float x= _x+ _interval_sample* (float)(i);
		float y= _y+ 0.75f* _h;
		y+= _ss->_data_left[_first_sample+ i]* _h* 0.25f;
		
		_data_left[6* i+ 0]= x;
		_data_left[6* i+ 1]= y;
		_data_left[6* i+ 2]= WAVE_COLOR[0];
		_data_left[6* i+ 3]= WAVE_COLOR[1];
		_data_left[6* i+ 4]= WAVE_COLOR[2];
		_data_left[6* i+ 5]= WAVE_COLOR[3];
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffer_left);
	glBufferData(GL_ARRAY_BUFFER, _n_samples* 6* sizeof(float), _data_left, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// right channel ------------------------------------------------
	float * _data_right= new float[_n_samples* 6];

	for (unsigned long i=0; i<_n_samples; ++i) {
		float x= _x+ _interval_sample* (float)(i);
		float y= _y+ 0.25f* _h;
		y+= _ss->_data_right[_first_sample+ i]* _h* 0.25f;
		
		_data_right[6* i+ 0]= x;
		_data_right[6* i+ 1]= y;
		_data_right[6* i+ 2]= WAVE_COLOR[0];
		_data_right[6* i+ 3]= WAVE_COLOR[1];
		_data_right[6* i+ 4]= WAVE_COLOR[2];
		_data_right[6* i+ 5]= WAVE_COLOR[3];
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffer_right);
	glBufferData(GL_ARRAY_BUFFER, _n_samples* 6* sizeof(float), _data_right, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void StereoSampleGL::update_selection() {
	//cout << "_is_selection=" << _is_selection << " ; _first_selection=" << _first_selection << " ; _last_selection=" << _last_selection << "\n";

	if (!_is_selection) {
		return;
	}

	if (_first_selection== _last_selection) {
		_is_selection= false;
		return;
	}

	float x1, x2;

	if (_first_selection< _last_selection) {
		if (_first_selection< _first_sample) {
			x1= _x;
		}
		else if (_first_selection> _last_sample) {
			x1= _x+ _w;
		}
		else {
			x1= _x+ (float)(_first_selection- _first_sample)* _interval_sample;
		}

		if (_last_selection< _first_sample) {
			x2= _x;
		}
		else if (_last_selection> _last_sample) {
			x2= _x+ _w;
		}
		else {
			x2= _x+ (float)(_last_selection- _first_sample)* _interval_sample;
		}
	}
	else {
		if (_first_selection< _first_sample) {
			x2= _x;
		}
		else if (_first_selection> _last_sample) {
			x2= _x+ _w;
		}
		else {
			x2= _x+ (float)(_first_selection- _first_sample)* _interval_sample;
		}

		if (_last_selection< _first_sample) {
			x1= _x;
		}
		else if (_last_selection> _last_sample) {
			x1= _x+ _w;
		}
		else {
			x1= _x+ (float)(_last_selection- _first_sample)* _interval_sample;
		}
	}

	float data_selection[6* 6];
	data_selection[6* 0+ 0]= x1; data_selection[6* 0+ 1]= _y;
	data_selection[6* 1+ 0]= x2; data_selection[6* 1+ 1]= _y;
	data_selection[6* 2+ 0]= x2; data_selection[6* 2+ 1]= _y+ _h;
	data_selection[6* 3+ 0]= x1; data_selection[6* 3+ 1]= _y;
	data_selection[6* 4+ 0]= x2; data_selection[6* 4+ 1]= _y+ _h;
	data_selection[6* 5+ 0]= x1; data_selection[6* 5+ 1]= _y+ _h;

	for (unsigned int i=0; i<6; ++i) {
		data_selection[6* i+ 2]= SELECTION_COLOR[0]; data_selection[6* i+ 3]= SELECTION_COLOR[1]; data_selection[6* i+ 4]= SELECTION_COLOR[2]; data_selection[6* i+ 5]= SELECTION_COLOR[3];
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffer_selection);
	glBufferData(GL_ARRAY_BUFFER, 6* 6* sizeof(float), data_selection, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void StereoSampleGL::zoom_all() {
	_interval_sample= _w/ (float)(_ss->_n_samples);
	update_data();
}


bool StereoSampleGL::mouse_motion(InputState * input_state) {
	float x, y;
	_screengl->screen2gl(input_state->_x, input_state->_y, x, y);

	if (_mouse_down) {
		if (input_state->_left_mouse) {
			_last_selection= _first_sample+ (unsigned long)((x- _x)/ _interval_sample);
			_is_selection= true;
			update_selection();
			return true;
		}
		else if (input_state->_middle_mouse) {
			
		}
		else if (input_state->_right_mouse) {
		}
	}
	return false;
}


bool StereoSampleGL::mouse_button_down(InputState * input_state) {
	float x, y;
	_screengl->screen2gl(input_state->_x, input_state->_y, x, y);
	//cout << x << " ; " << y << "\n";

	if (_mouse_down) {
		return false;
	}

	if ((x>=_x) && (x< _x+ _w) && (y>=_y) && (y< _y+ _h)) {
		_mouse_down= true;
		if (input_state->_left_mouse) {
			_first_selection= _first_sample+ (unsigned long)((x- _x)/ _interval_sample);
			_last_selection= _first_selection;
			_is_selection= false;
			//cout << _first_selection << "\n";
			//update_selection();
		}
		return true;
	}
	return false;
}


bool StereoSampleGL::mouse_button_up(InputState * input_state) {
	_mouse_down= false;
	return true;
}


bool StereoSampleGL::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_UP) {
		_interval_sample*= 1.0f+ ZOOM_FACTOR;
		if (_interval_sample> MAX_INTERVAL_SAMPLE) {
			_interval_sample= MAX_INTERVAL_SAMPLE;
		}
		else {
			if (_last_sample> (unsigned long)(_w* 0.5f/ _interval_sample)) {
				// on veut zoomer sur le centre de la fenetre, il faut donc modifier _first_sample
				unsigned long offset= (unsigned long)(_w* 0.5f* ZOOM_FACTOR/ _interval_sample);
				_first_sample+= offset;
				if (_first_sample>= _ss->_n_samples) {
					_first_sample= _ss->_n_samples- 1;
				}
			}
		}
		update_data();
		update_selection();
		return true;
	}
	else if (key== SDLK_DOWN) {
		_interval_sample*= 1.0f- ZOOM_FACTOR;
		if (_interval_sample< MIN_INTERVAL_SAMPLE) {
			_interval_sample= MIN_INTERVAL_SAMPLE;
		}
		else {
			unsigned long offset= (unsigned long)(_w* 0.5f* ZOOM_FACTOR/ _interval_sample);
			if (_first_sample>= offset) {
				_first_sample-= offset;
			}
			else {
				_first_sample= 0;
			}
		}
		update_data();
		update_selection();
		return true;
	}
	else if (key== SDLK_LEFT) {
		unsigned long step= (unsigned long)(MOVE_FACTOR/ _interval_sample);
		if (_first_sample>= step) {
			_first_sample-= step;
		}
		else {
			_first_sample= 0;
		}
		update_data();
		update_selection();
		return true;
	}
	else if (key== SDLK_RIGHT) {
		unsigned long step= (unsigned long)(MOVE_FACTOR/ _interval_sample);
		_first_sample+= step;
		if (_first_sample>= _ss->_n_samples) {
			_first_sample= _ss->_n_samples- 1;
		}
		update_data();
		update_selection();
		return true;
	}
	
	return false;
}


// ----------------------------------------------------------------------------------------
AudioProjectGL::AudioProjectGL() {

}


AudioProjectGL::AudioProjectGL(GLuint prog_draw_2d, ScreenGL * screengl) : _prog_draw(prog_draw_2d), _screengl(screengl) {
	_audio_project= new AudioProject();
}


AudioProjectGL::~AudioProjectGL() {
	for (auto sample : _samples) {
		delete sample;
	}
	_samples.clear();
	delete _audio_project;
}


void AudioProjectGL::add_sample(string file_path) {
	_audio_project->add_sample(file_path);
	StereoSampleGL * ssgl= new StereoSampleGL(_prog_draw, _audio_project->_samples.back(), _screengl, 10, _audio_project->_samples.size()* 150, 1000, 100);
	_samples.push_back(ssgl);
}


void AudioProjectGL::add_track() {
	_audio_project->add_track();
}


void AudioProjectGL::draw() {
	for (auto sample : _samples) {
		sample->draw();
	}
}


bool AudioProjectGL::mouse_motion(InputState * input_state) {
	for (auto sample : _samples) {
		if (sample->mouse_motion(input_state)) {
			return true;
		}
	}
	return false;
}


bool AudioProjectGL::mouse_button_down(InputState * input_state) {
	for (auto sample : _samples) {
		if (sample->mouse_button_down(input_state)) {
			return true;
		}
	}
	return false;
}


bool AudioProjectGL::mouse_button_up(InputState * input_state) {
	for (auto sample : _samples) {
		if (sample->mouse_button_up(input_state)) {
			return true;
		}
	}
	return false;
}


bool AudioProjectGL::key_down(InputState * input_state, SDL_Keycode key) {
	for (auto sample : _samples) {
		if (sample->key_down(input_state, key)) {
			//return true;
		}
	}
	return false;
}


bool AudioProjectGL::drag_drop(string file_path) {
	add_sample(file_path);

	return true;
}
