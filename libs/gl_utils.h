
#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <string>
#include <vector>
#include <map>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#include "utile.h"
#include "typedefs.h"


struct ScreenGL {
	ScreenGL();
	ScreenGL(uint screen_width, uint screen_height, number gl_width, number gl_height);
	~ScreenGL();
	void screen2gl(uint i, uint j, number & x, number & y);
	pt_2d screen2gl(uint i, uint j);
	void gl2screen(number x, number y, uint & i, uint & j);
	friend std::ostream & operator << (std::ostream & os, const ScreenGL & screengl);


	uint _screen_width, _screen_height;
	number _gl_width, _gl_height;
};


struct GLSDL {
	GLSDL();
	GLSDL(std::string window_title, uint width, uint height, bool multisampling);
	~GLSDL();


	SDL_Window * _window;
	SDL_GLContext _main_context;
};

void gl_versions();

void set_subwindow(const float bkgnd_color[4], int x, int y, int w, int h);
void export_screen_to_ppm(std::string ppm_path, uint x, uint y, uint width, uint height);

float * draw_cross(float * data, pt_2d center, float size, glm::vec4 color);
float * draw_arrow(float * data, pt_2d start, pt_2d end, float tip_size, float angle, glm::vec4 color);
float * draw_polygon(float * data, std::vector<pt_2d> pts, glm::vec4 color);
float * draw_nothing(float * data, uint n_attrs_per_pts, uint n_pts);

#endif
