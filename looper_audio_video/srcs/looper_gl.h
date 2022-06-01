#ifndef LOOPER_GL_H
#define LOOPER_GL_H

#include <iostream>
#include <chrono>
#include <map>

#include <OpenGL/gl3.h>

#include <SDL2/SDL.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "looper.h"
#include "input_state.h"
#include "font.h"
#include "gl_utils.h"


struct RectangleGL {
	float _x;
	float _y;
	float _z;
	float _w;
	float _h;
	float _r;
	float _g;
	float _b;
	float _a;
};


const unsigned int N_VERTICES_PER_RECTANGLE= 6;
const unsigned int N_FLOATS_PER_VERTEX= 7;
const float GENERAL_INFO_WIDTH= 0.1f;
const float TRACK_INFO_WIDTH= 0.1f;
const float EPS= 0.01f;


unsigned int add_rectangle(float * vertices, unsigned int idx, RectangleGL & rect);
std::string bool2onoff(bool b);


class LooperGL : public Sequence {
public:
	LooperGL();
	LooperGL(GLuint prog_2d, GLuint prog_font, ScreenGL * screengl);
	~LooperGL();
	bool event_key(SDL_Keycode key);
	void key_down(SDL_Keycode key);
	void key_up(SDL_Keycode key);
	void update_vbo_general();
	void update_vbo_time();
	void update_vbo_track_info(unsigned int idx_track);
	void update_vbo_track_data(unsigned int idx_track);
	//void update_vbo_insert();
	void update_text_general();
	void update_text_track_info(unsigned int idx_track);
	void draw();
	void tap_tempo();
	glm::vec3 get_color(SDL_Keycode key);
	float get_track_y(unsigned int idx_track);
	float get_track_h();
	float get_event_x(unsigned int idx_track, unsigned int idx_event);
	float get_event_w(unsigned int idx_track, unsigned int idx_event, bool until_now);
	float get_track_now(unsigned int idx_track);
	void get_general_rectangle(RectangleGL & rect);
	void get_timeline_rectangle(RectangleGL & rect, unsigned int idx_track);
	void get_track_info_rectangle(RectangleGL & rect, unsigned int idx_track);
	void get_quantize_rectangle(RectangleGL & rect, unsigned int idx_track, unsigned int idx_quantize);
	void get_track_data_rectangle(RectangleGL & rect, unsigned int idx_track);
	void get_event_rectangle(RectangleGL & rect, unsigned int idx_track, unsigned int idx_event, bool until_now);


	InputState * _input_state;
	Font * _font;
	ScreenGL * _screengl;
	GLuint _prog_2d, _prog_font;
	GLint _camera2clip_loc, _position_loc, _color_loc;
	glm::mat4 _camera2clip;
	GLuint _vbos[2+ 2* N_TRACKS];
	unsigned int _n_rectangles[2+ 2* N_TRACKS];
	std::map<key_type, glm::vec3> _event_colors;
	std::chrono::system_clock::time_point _tap;
	unsigned int _ratio_numerator;
	//int _insert_idx_track;
	//int _insert_idx_event;
	bool _current_track_changed;
};

#endif

