#ifndef TEST_OBJ_ANIM_H
#define TEST_OBJ_ANIM_H

#include <string>
#include <vector>
#include <map>
#include <filesystem>

#include "obj_parser.h"
#include "gl_draw.h"
#include "repere.h"


const glm::vec3 LIGHT_POSITION(10.0f, 10.0f, 30.0f);
const glm::vec3 LIGHT_COLOR(1.0f);

const uint N_MAX_MATRICES = 100;


struct AnimatedObjBone {
	AnimatedObjBone();
	AnimatedObjBone(std::string name);
	~AnimatedObjBone();


	std::string _name;
};


struct AnimatedObjTransform {
	AnimatedObjTransform();
	~AnimatedObjTransform();

	
	glm::mat4 _mat;
	uint _idx;
};


struct AnimatedObjFrame {
	AnimatedObjFrame();
	~AnimatedObjFrame();


	std::map<AnimatedObjBone *, AnimatedObjTransform *> _transforms;
};


struct AnimatedObjAction {
	AnimatedObjAction();
	AnimatedObjAction(std::string name);
	~AnimatedObjAction();


	std::string _name;
	std::vector<AnimatedObjFrame *> _frames;
};


struct AnimatedObj {
	AnimatedObj();
	AnimatedObj(std::string json_path);
	~AnimatedObj();
	void update_matrices();
	void update_data();
	void anim();


	ObjData * _obj_data;
	float * _data;
	uint _n_attrs_per_pts;
	uint _n_pts;
	std::map<std::string, AnimatedObjAction *> _actions;
	std::map<std::string, AnimatedObjBone *> _bones;
	std::map<std::string, AnimatedObjBone *> _obj2bone;
	std::string _current_action;
	uint _current_frame;
	//glm::mat4 _matrices[N_MAX_MATRICES];
	float _matrices[N_MAX_MATRICES * 16];
};


struct TestObjAnim {
	TestObjAnim();
	TestObjAnim(GLDrawManager * gl_draw_manager, ViewSystem * view_system);
	~TestObjAnim();
	void anim();
	void update();
	void draw();
	bool key_down(InputState * input_state, SDL_Keycode key, time_point t);

	
	GLDrawManager * _gl_draw_manager;
	ViewSystem * _view_system;
	//ObjData * _obj_data;
	AnimatedObj * _animated_obj;
};


#endif
