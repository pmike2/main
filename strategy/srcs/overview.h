#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <OpenGL/gl3.h>

#include "typedefs.h"
#include "gl_draw.h"
#include "bbox_2d.h"
#include "repere.h"
#include "gl_utils.h"


#include "const.h"


struct OverView {
	OverView();
	OverView(GLDrawManager * gl_draw_manager);
	~OverView();
	void start_draw_in_texture();
	void end_draw_in_texture();
	void draw(ViewSystem * view_system);


	GLDrawManager * _gl_draw_manager;
	ViewSystem * _view_system;
	ScreenGL * _screengl;
	uint _tex_width, _tex_height;
	GLuint _framebuffer;
	AABB_2D * _aabb;
	//mat_4d _camera2clip;
};


#endif

