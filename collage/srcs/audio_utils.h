#ifndef AUDIO_UTILS_H
#define AUDIO_UTILS_H

#include <string>
#include <iostream>
#include <cmath>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "sndfile.h"

#include "input_state.h"
#include "gl_utils.h"
#include "font.h"



const float WAVE_COLOR[]= {0.3f, 0.3f, 0.7f, 0.7f};
const float BACKGROUND_COLOR[]= {0.8f, 0.8f, 0.8f, 1.0f};
const float LINES_COLOR[]= {0.2f, 0.2f, 0.2f, 1.0f};
const float SELECTION_COLOR[]= {0.5f, 0.5f, 0.5f, 0.5f};
const float MARK_COLOR[]= {0.5f, 0.7f, 0.7f, 0.5f};

const float ZOOM_FACTOR= 0.05f;
const float MOVE_FACTOR= 0.3f;

const float INIT_INTERVAL_SAMPLE= 0.001f;
const float MIN_INTERVAL_SAMPLE= 0.00001f;
const float MAX_INTERVAL_SAMPLE= 0.01f;

const unsigned int MAX_GL_N_SAMPLES= 16384;

const unsigned int N_BUFFERS= 6;

enum SAMPLE_SELECTION_MODE {NO_SELECTION, SELECTION, MOVING_FIRST, MOVING_LAST};


// ------------------------------------------------------------------------------------
class StereoSample {
public:
	StereoSample();
	~StereoSample();
	void load_from_file(std::string file_path);


	std::string _file_path;
	unsigned long _n_samples;
	float * _data_left;
	float * _data_right;
};


class SamplesPool {
public:
	SamplesPool();
	~SamplesPool();
	void add_sample(std::string file_path);


	std::vector<StereoSample *> _samples;
};


class AudioMark {
public:
	AudioMark();
	AudioMark(unsigned long track_sample, unsigned int idx_sample, unsigned long first_sample, unsigned long last_sample);
	~AudioMark();


	unsigned long _track_sample;
	unsigned int _idx_sample;
	unsigned long _first_sample;
	unsigned long _last_sample;
};


class StereoTrack {
public:
	StereoTrack();
	StereoTrack(SamplesPool * sp);
	~StereoTrack();
	void add_mark(unsigned long track_sample, unsigned int idx_sample, unsigned long first_sample, unsigned long last_sample);
	void update_data();


	SamplesPool * _sp;
	std::vector<AudioMark *> _marks;
	unsigned long _n_samples;
	float * _data_left;
	float * _data_right;
};


class AudioProject {
public:
	AudioProject();
	~AudioProject();
	void add_track();
	void add_sample(std::string file_path);


	SamplesPool * _sp;
	std::vector<StereoTrack *> _tracks;
};


// ------------------------------------------------------------------------------------
class TrackGL {
public:
	TrackGL();
	TrackGL(GLuint prog_draw_2d, StereoTrack * st, ScreenGL * screengl, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
	~TrackGL();
	void draw();
	void update_waveform();
	void update_selection();
	void update_marks();
	void zoom_all();
	bool mouse_motion(InputState * input_state);
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool key_down(InputState * input_state, SDL_Keycode key);
	unsigned long gl2sampleidx(float x);
	float sampleidx2gl(unsigned long idx);
	bool is_inbox(int i, int j);
	void add_mark(unsigned long track_sample, unsigned int idx_sample, unsigned long first_sample, unsigned long last_sample);


	GLuint _prog_draw;
	GLint _camera2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffers[N_BUFFERS];
	float _camera2clip[16];

	StereoTrack * _st;
	ScreenGL * _screengl;

	float _x, _y, _w, _h;
	unsigned long _first_sample, _last_sample;
	float _interval_sample;
	unsigned long _n_samples;
	bool _mouse_down;
	unsigned long _first_selection, _last_selection;
	SAMPLE_SELECTION_MODE _selection_mode;
};


class AudioProjectGL {
public:
	AudioProjectGL();
	AudioProjectGL(GLuint prog_draw_2d, Font * arial_font, ScreenGL * screengl);
	~AudioProjectGL();
	void add_track();
	void add_sample(std::string file_path);
	void draw();
	bool mouse_motion(InputState * input_state);
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool drag_drop(std::string file_path);

	GLuint _prog_draw;
	ScreenGL * _screengl;	
	AudioProject * _audio_project;
	std::vector<TrackGL *> _tracks;
	Font * _arial_font;
	int _copy_idx;
};


#endif
