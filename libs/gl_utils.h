
#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <string>
#include <vector>
#include <map>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "utile.h"
#include "typedefs.h"


// utilisé dans fill_texture_array lorsque l'on veut sauter des indices
const std::string NO_PNG= "NO_PNG";


class ScreenGL {
public:
	ScreenGL();
	ScreenGL(int screen_width, int screen_height, number gl_width, number gl_height);
	~ScreenGL();
	void screen2gl(int i, int j, number & x, number & y);
	pt_2d screen2gl(int i, int j);
	void gl2screen(number x, number y, int & i, int & j);


	int _screen_width, _screen_height;
	number _gl_width, _gl_height;
};


struct DrawContextAttrib {
	friend std::ostream & operator << (std::ostream & os, const DrawContextAttrib & dca);


	std::string _name;
	GLint _loc;
	uint _size;
	uint _offset;
};


struct DrawContextBuffer {
	friend std::ostream & operator << (std::ostream & os, const DrawContextBuffer & dcb);


	GLuint _id;
	std::vector<DrawContextAttrib> _attribs;
	uint _n_attrs_per_pts;
	bool _is_instanced;
	GLenum _usage;
};


class DrawContext {
public:
	DrawContext();
	DrawContext(GLuint prog, std::vector<std::string> locs_attrib, std::vector<std::string> locs_uniform, GLenum usage = GL_STATIC_DRAW, bool active = true);
	~DrawContext();
	void set_data(float * data, uint idx_buffer = 0);
	void clear_data(uint idx_buffer = 0);
	void activate();
	void deactivate();
	uint data_size(uint idx_buffer = 0);
	void show_data(uint idx_buffer = 0);
	friend std::ostream & operator << (std::ostream & os, const DrawContext & dc);


	GLuint _prog;
	GLuint _vao;
	std::map<std::string, GLint> _locs_uniform;
	std::vector<DrawContextBuffer> _buffers;
	uint _n_pts;
	uint _n_instances;
	bool _active;
};


void _check_gl_error(const char * file, int line);
#define check_gl_error() _check_gl_error(__FILE__,__LINE__)
void check_gl_program(GLuint prog);
void gl_versions();
void active_uniforms(GLuint prog);
void active_attribs(GLuint prog);
char * load_source(const char * filename);
GLuint load_shader(GLenum type, const char * filename);
GLuint create_prog(std::string vs_path, std::string fs_path, std::string gs_path="", bool check_program=true);

void set_subwindow(const float bkgnd_color[4], int x, int y, int w, int h);
void export_texture2pgm(std::string pgm_path, uint width, uint height);
void export_texture_array2pgm(std::string pgm_dir_path, uint width, uint height, uint depth);
void export_screen_to_ppm(std::string ppm_path, uint x, uint y, uint width, uint height);

float * draw_cross(float * data, pt_2d center, float size, glm::vec4 color);
float * draw_arrow(float * data, pt_2d start, pt_2d end, float tip_size, float angle, glm::vec4 color);
float * draw_polygon(float * data, std::vector<pt_2d> pts, glm::vec4 color);
float * draw_nothing(float * data, uint n_attrs_per_pts, uint n_pts);

void fill_texture_array(uint texture_offset, uint texture_idx, uint texture_size, std::vector<std::string> pngs);

#endif
