/*
sur les histoires de matrices :
https://blender.stackexchange.com/questions/44637/how-can-i-manually-calculate-bpy-types-posebone-matrix-using-blenders-python-ap/121495#121495
*/


#ifndef TEST_OBJ_ANIM_H
#define TEST_OBJ_ANIM_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <filesystem>

#include "json.hpp"

#include "obj_parser.h"
#include "gl_draw.h"
#include "repere.h"
#include "bbox.h"


using json = nlohmann::json;


enum ANIMATED_MODEL_MODE {ANIMATED_MODEL_RIGID, ANIMATED_MODEL_WEIGHT};


const glm::vec3 LIGHT_POSITION(10.0f, 10.0f, 30.0f);
const glm::vec3 LIGHT_COLOR(1.0f);

//const uint N_MAX_MATRICES = 100;


mat_4d parse_js_matrix(json js);


struct AnimatedObjBone {
	AnimatedObjBone();
	AnimatedObjBone(std::string name, mat_4d mat_local, std::string parent_name = "");
	~AnimatedObjBone();
	friend std::ostream & operator << (std::ostream & os, AnimatedObjBone & bone);


	mat_4d _mat_local;
	std::string _name;
	AnimatedObjBone * _parent;
	std::string _parent_name;
	std::unordered_map<uint, number> _weights; // non utilisé dans le cas ANIMATED_MODEL_RIGID
};


struct AnimatedObjTransform {
	AnimatedObjTransform();
	~AnimatedObjTransform();
	friend std::ostream & operator << (std::ostream & os, AnimatedObjTransform & transform);

	
	mat_4d _mat;
	uint _idx;
};


struct AnimatedObjFrame {
	AnimatedObjFrame();
	~AnimatedObjFrame();
	friend std::ostream & operator << (std::ostream & os, AnimatedObjFrame & frame);


	std::unordered_map<AnimatedObjBone *, AnimatedObjTransform *> _transforms;
};


struct AnimatedObjAction {
	AnimatedObjAction();
	AnimatedObjAction(std::string name);
	~AnimatedObjAction();
	friend std::ostream & operator << (std::ostream & os, AnimatedObjAction & action);


	std::string _name;
	std::vector<AnimatedObjFrame *> _frames;
};


struct AnimatedObjModel {
	AnimatedObjModel();
	AnimatedObjModel(std::string json_path);
	~AnimatedObjModel();
	void update_matrices();
	void update_vertices();
	std::vector<AnimatedObjBone *> bones_influencing_vertex(uint id_vertex);
	friend std::ostream & operator << (std::ostream & os, AnimatedObjModel & obj);


	ANIMATED_MODEL_MODE _mode;
	ObjData * _obj_data;
	float * _matrices;
	uint _n_matrices;
	std::unordered_map<std::string, AnimatedObjBone *> _bones;
	std::unordered_map<std::string, AnimatedObjAction *> _actions;
	std::unordered_map<std::string, AnimatedObjBone *> _obj2bone; // non utilisé dans le cas ANIMATED_MODEL_WEIGHT
	float * _vertex2weight;
	AnimatedObjBone ** _vertex2bone;
};


struct AnimatedObjInstance : public InstancePosRot {
	AnimatedObjInstance();
	AnimatedObjInstance(AnimatedObjModel * model, pt_3d pos, time_point t, quat q = quat(1.0, 0.0, 0.0, 0.0));
	~AnimatedObjInstance();
	void anim(time_point t);


	AnimatedObjModel * _model;
	std::string _current_action;
	uint _current_frame;
	time_point _last_anim_t;
};


struct TestObjAnim {
	TestObjAnim();
	TestObjAnim(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t);
	~TestObjAnim();
	void anim(time_point t);
	void update();
	void draw();
	bool key_down(InputState * input_state, SDL_Keycode key, time_point t);

	
	GLDrawManager * _gl_draw_manager;
	ViewSystem * _view_system;
	std::unordered_map<std::string, AnimatedObjModel *> _models;
	std::vector<AnimatedObjInstance *> _instances;
	bool _paused;
};


#endif
