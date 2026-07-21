#ifndef TEST_OBJ_ANIM_H
#define TEST_OBJ_ANIM_H

#include "obj_parser.h"
#include "gl_draw.h"
#include "repere.h"


const glm::vec3 LIGHT_POSITION(100.0f, 0.0f, 0.0f);
const glm::vec3 LIGHT_COLOR(1.0f);


struct TestObj {
	TestObj();
	TestObj(GLDrawManager * gl_draw_manager, ViewSystem * view_system);
	~TestObj();
	void anim();
	void update();
	void draw();
	bool key_down(InputState * input_state, SDL_Keycode key, time_point t);

	
	GLDrawManager * _gl_draw_manager;
	ViewSystem * _view_system;
	ObjData * _obj_data;
	mat_4d _model2world;
	number _angle;
	std::string _current_context_name;
};


#endif
