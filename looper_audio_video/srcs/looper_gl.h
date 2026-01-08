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


const uint N_VERTICES_PER_RECTANGLE= 6;
const uint N_FLOATS_PER_VERTEX= 7;
const float GENERAL_INFO_WIDTH= 0.1f;
const float TRACK_INFO_WIDTH= 0.1f;
const float EPS= 0.01f;


uint add_rectangle(float * vertices, uint idx, RectangleGL & rect);
std::string bool2onoff(bool b);


class LooperGL : public Sequence {
public:
	LooperGL();
	LooperGL(GLuint prog_2d, GLuint prog_font, ScreenGL * screengl);
	~LooperGL();
	bool event_key(SDL_Keycode key);
	void key_down(SDL_Keycode key);
	void key_up(SDL_Keycode key);
	void tap_tempo();
	void update_vbo_general();
	void update_vbo_time();
	void update_vbo_track_info(uint idx_track);
	void update_vbo_track_data(uint idx_track);
	void update_text_general();
	void update_text_track_info(uint idx_track);
	void update_text_track_data(uint idx_track);
	void draw();
	glm::vec3 get_color(SDL_Keycode key);
	float get_track_y(uint idx_track);
	float get_track_h();
	float get_event_x(uint idx_track, uint idx_event);
	float get_event_w(uint idx_track, uint idx_event, bool until_now);
	float get_track_now(uint idx_track);
	void get_general_rectangle(RectangleGL & rect);
	void get_timeline_rectangle(RectangleGL & rect, uint idx_track);
	void get_track_info_rectangle(RectangleGL & rect, uint idx_track);
	void get_quantize_rectangle(RectangleGL & rect, uint idx_track, uint idx_quantize);
	void get_track_data_rectangle(RectangleGL & rect, uint idx_track);
	void get_event_rectangle(RectangleGL & rect, uint idx_track, uint idx_event, bool until_now);


	InputState * _input_state;
	Font * _font;
	ScreenGL * _screengl;
	GLuint _prog_2d, _prog_font;
	GLint _camera2clip_loc, _position_loc, _color_loc;
	glm::mat4 _camera2clip;
	GLuint _vbos[2+ 2* N_TRACKS];
	uint _n_rectangles[2+ 2* N_TRACKS];
	std::map<key_type, glm::vec3> _event_colors;
	std::chrono::system_clock::time_point _tap;
	uint _ratio_numerator;
	bool _current_track_changed;
	bool _screen_save;
};

#endif

