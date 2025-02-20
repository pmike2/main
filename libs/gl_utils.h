
#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <string>
#include <vector>
#include <map>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "utile.h"
#include "typedefs.h"


class ScreenGL {
public:
	ScreenGL();
	ScreenGL(int screen_width, int screen_height, number gl_width, number gl_height);
	~ScreenGL();
	void screen2gl(int i, int j, number & x, number & y);
	pt_type screen2gl(int i, int j);
	void gl2screen(number x, number y, int & i, int & j);


	int _screen_width, _screen_height;
	number _gl_width, _gl_height;
};


class DrawContext {
public:
	DrawContext();
	DrawContext(GLuint prog, GLuint buffer, std::vector<std::string> locs_attrib, std::vector<std::string> locs_uniform);
	~DrawContext();


	GLuint _prog;
	std::map<std::string, GLint> _locs_attrib, _locs_uniform;
	GLuint _buffer;
	unsigned int _n_pts;
	unsigned int _n_attrs_per_pts;
};


void _check_gl_error(const char * file, int line);
#define check_gl_error() _check_gl_error(__FILE__,__LINE__)
void check_gl_program(GLuint prog);
void gl_versions();
void active_uniforms(GLuint prog);
void active_attribs(GLuint prog);
char * load_source(const char * filename);
GLuint load_shader(GLenum type, const char * filename);
GLuint create_prog(std::string vs_path, std::string fs_path, std::string gs_path="");

void set_subwindow(const float bkgnd_color[4], int x, int y, int w, int h);
void export_texture2pgm(std::string pgm_path, unsigned int width, unsigned int height);
void export_texture_array2pgm(std::string pgm_dir_path, unsigned int width, unsigned int height, unsigned int depth);

float * draw_cross(float * data, pt_type center, float size, glm::vec4 color);
float * draw_arrow(float * data, pt_type start, pt_type end, float tip_size, float angle, glm::vec4 color);
float * draw_polygon(float * data, std::vector<pt_type> pts, glm::vec4 color);
float * draw_nothing(float * data, unsigned int n_attrs_per_pts, unsigned int n_pts);

#endif
