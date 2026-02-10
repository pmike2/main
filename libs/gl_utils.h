
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
	pt_2d screen2gl(int i, int j);
	void gl2screen(number x, number y, int & i, int & j);


	int _screen_width, _screen_height;
	number _gl_width, _gl_height;
};


void gl_versions();

void set_subwindow(const float bkgnd_color[4], int x, int y, int w, int h);
void export_screen_to_ppm(std::string ppm_path, uint x, uint y, uint width, uint height);

float * draw_cross(float * data, pt_2d center, float size, glm::vec4 color);
float * draw_arrow(float * data, pt_2d start, pt_2d end, float tip_size, float angle, glm::vec4 color);
float * draw_polygon(float * data, std::vector<pt_2d> pts, glm::vec4 color);
float * draw_nothing(float * data, uint n_attrs_per_pts, uint n_pts);

#endif
