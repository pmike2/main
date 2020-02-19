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



const float GL_WIDTH= 20.0f;
const float GL_HEIGHT= 20.0f;


class StereoSample {
public:
	StereoSample();
	~StereoSample();
	void load_from_file(std::string file_path);


	unsigned long _n_samples;
	float * _data_left;
	float * _data_right;
};


class StereoSampleGL {
public:
	StereoSampleGL();
	StereoSampleGL(GLuint prog_draw_2d, StereoSample * ss, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int screen_w, unsigned int screen_h);
	~StereoSampleGL();
	void draw();
	void update_data();
	void zoom_all();
	bool mouse_motion(InputState * input_state);
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool key_down(InputState * input_state, SDL_Keycode key);


	GLuint _prog_draw;
	GLint _camera2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffer_left, _buffer_right, _buffer_background, _buffer_lines;
	float _camera2clip[16];

	StereoSample * _ss;

	float _x, _y, _w, _h;
	float _first_sample, _interval_sample;
	unsigned long _n_samples;
	bool _mouse_down;
};

#endif
