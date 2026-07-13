
#ifndef TEST_OBJ_ANIM_H
#define TEST_OBJ_ANIM_H

#include <string>
#include <vector>
#include <filesystem>
#include <iterator>

#include "json.hpp"

#include "obj_parser.h"
#include "gl_draw.h"
#include "repere.h"
#include "bbox.h"
#include "typedefs.h"

#include "animated_obj.h"


const glm::vec3 LIGHT_POSITION(0.0f, 0.0f, 0.0f);
const glm::vec3 LIGHT_COLOR(1.0f);


struct TestObjAnim {
	TestObjAnim();
	TestObjAnim(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t);
	~TestObjAnim();
	AnimatedObjModel * get_model(std::string model_name);

	void anim(time_point t);

	void update_static_buffer(AnimatedObjModel * model);
	void update_dynamic_buffer(AnimatedObjModel * model);

	void draw();
	bool key_down(InputState * input_state, SDL_Keycode key, time_point t);

	
	GLDrawManager * _gl_draw_manager;
	ViewSystem * _view_system;
	std::vector<AnimatedObjModel *> _models;
	std::vector<AnimatedObjInstance *> _instances;
	bool _paused;
};


#endif
