#ifndef TEST_OBJ_ANIM_H
#define TEST_OBJ_ANIM_H

#include "obj_parser.h"
#include "gl_draw.h"
#include "repere.h"


const glm::vec3 LIGHT_POSITION(10.0f, 10.0f, 30.0f);
const glm::vec3 LIGHT_COLOR(1.0f);


struct TestObj {
	TestObj();
	TestObj(GLDrawManager * gl_draw_manager, ViewSystem * view_system);
	~TestObj();
	void anim();
	void update();
	void draw();

	
	GLDrawManager * _gl_draw_manager;
	ViewSystem * _view_system;
	ObjData * _obj_data;
	mat_4d _model2world;
	number _angle;
};


#endif
