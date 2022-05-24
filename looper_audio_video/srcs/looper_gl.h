#ifndef LOOPER_GL_H
#define LOOPER_GL_H

#include <iostream>
#include <chrono>

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


class LooperGL : public Sequence {
public:
	LooperGL();
	LooperGL(GLuint prog_2d, GLuint prog_font, ScreenGL * screengl);
	~LooperGL();
	bool event_key(SDL_Keycode key);
	void key_down(SDL_Keycode key);
	void key_up(SDL_Keycode key);
	void update_vbo();
	void draw();
	void tap_tempo();


	InputState * _input_state;
	std::chrono::system_clock::time_point _tap;
	unsigned int _ratio_numerator;
	Font * _font;
	ScreenGL * _screengl;
	GLuint _prog_2d, _prog_font;
	GLint _camera2clip_loc, _position_loc, _color_loc;
	glm::mat4 _camera2clip;
	GLuint _vbo;
	unsigned int _n_triangles;
};

#endif

