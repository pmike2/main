
#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <string>
#include <vector>
#include <map>

#include <OpenGL/gl3.h>



class ScreenGL {
public:
	ScreenGL();
	ScreenGL(int screen_width, int screen_height, float gl_width, float gl_height);
	~ScreenGL();
	void screen2gl(int i, int j, float & x, float & y);
	void gl2screen(float x, float y, int & i, int & j);


	int _screen_width, _screen_height;
	float _gl_width, _gl_height;
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
// sert à la skybox
//unsigned int load_cube_map(std::vector<std::string> faces);
GLuint create_prog(std::string vs_path, std::string fs_path, bool check=true);
void set_subwindow(const float bkgnd_color[4], int x, int y, int w, int h);

#endif
