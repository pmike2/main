
#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>

#include <OpenGL/gl3.h>

#include <SDL2/SDL_image.h>

#include "utile.h"


class ScreenGL {
public:
	ScreenGL();
	ScreenGL(unsigned int screen_width, unsigned int screen_height, float gl_width, float gl_height);
	~ScreenGL();
	void screen2gl(unsigned int i, unsigned int j, float & x, float & y);
	void gl2screen(float x, float y, unsigned int & i, unsigned int & j);


	unsigned int _screen_width, _screen_height;
	float _gl_width, _gl_height;
};


void _check_gl_error(const char * file, int line);
 #define check_gl_error() _check_gl_error(__FILE__,__LINE__)
void check_gl_program(GLuint prog);
void gl_versions();
void active_uniforms(GLuint prog);
void active_attribs(GLuint prog);
char * load_source(const char * filename);
GLuint load_shader(GLenum type, const char * filename);
// sert à la skybox
unsigned int load_cube_map(std::vector<std::string> faces);
GLuint create_prog(std::string vs_path, std::string fs_path);
void set_subwindow(const float bkgnd_color[4], int x, int y, int w, int h);

#endif
