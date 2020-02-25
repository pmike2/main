
#ifndef REPERE_H
#define REPERE_H

#include <OpenGL/gl3.h>

#include <iostream>


class Repere{
public:
	Repere();
	Repere(GLuint prog_draw_);
	void draw(float * world2clip);
	
	GLuint buffer;
	float data[36];
	GLint world2clip_loc, position_loc, diffuse_color_loc;
	bool is_active;
	GLuint prog_draw;
};



#endif
