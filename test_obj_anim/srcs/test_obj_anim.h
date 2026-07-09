/*
sur les histoires de matrices :
https://blender.stackexchange.com/questions/44637/how-can-i-manually-calculate-bpy-types-posebone-matrix-using-blenders-python-ap/121495#121495
*/


#ifndef TEST_OBJ_ANIM_H
#define TEST_OBJ_ANIM_H

#include <string>
#include <vector>
#include <filesystem>

#include "json.hpp"

#include "obj_parser.h"
#include "gl_draw.h"
#include "repere.h"
#include "bbox.h"
#include "typedefs.h"


using json = nlohmann::json;


enum ANIMATED_MODEL_MODE {ANIMATED_MODEL_RIGID, ANIMATED_MODEL_WEIGHT};


const glm::vec3 LIGHT_POSITION(0.0f, 0.0f, 0.0f);
const glm::vec3 LIGHT_COLOR(1.0f);


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
};


struct AnimatedObjTransform {
	AnimatedObjTransform();
	~AnimatedObjTransform();
	friend std::ostream & operator << (std::ostream & os, AnimatedObjTransform & transform);

	
	mat_4d _mat;
	uint _idx;
};


struct AnimatedObjBone2Transform {
	AnimatedObjBone2Transform();
	AnimatedObjBone2Transform(AnimatedObjBone * bone, AnimatedObjTransform * transform);
	~AnimatedObjBone2Transform();

	AnimatedObjBone * _bone;
	AnimatedObjTransform * _transform;
};


struct AnimatedObjFrame {
	AnimatedObjFrame();
	~AnimatedObjFrame();
	AnimatedObjTransform * get_transform(AnimatedObjBone * bone);
	friend std::ostream & operator << (std::ostream & os, AnimatedObjFrame & frame);


	//map<AnimatedObjBone *, AnimatedObjTransform *> _transforms;
	//std::vector<std::pair<AnimatedObjBone *, AnimatedObjTransform *> > _transforms;
	std::vector<AnimatedObjBone2Transform *> _transforms;
};


struct AnimatedObjAction {
	AnimatedObjAction();
	AnimatedObjAction(std::string name);
	~AnimatedObjAction();
	friend std::ostream & operator << (std::ostream & os, AnimatedObjAction & action);


	std::string _name;
	std::vector<AnimatedObjFrame *> _frames;
};


struct AnimatedObjObject {
	AnimatedObjObject();
	AnimatedObjObject(ObjObject * static_object);
	~AnimatedObjObject();


	ObjObject * _static_object;
	AnimatedObjBone ** _bones; // utilisé dans le cas ANIMATED_MODEL_WEIGHT
	number * _weights; // utilisé dans le cas ANIMATED_MODEL_WEIGHT
	AnimatedObjBone * _parent_bone; // utilisé dans le cas ANIMATED_MODEL_RIGID
};


struct AnimatedObjModel {
	AnimatedObjModel();
	AnimatedObjModel(std::string json_path);
	~AnimatedObjModel();
	void update_matrices();
	AnimatedObjObject * get_animated_object(std::string obj_name);
	AnimatedObjAction * get_action(std::string action_name);
	AnimatedObjBone * get_bone(std::string bone_name);
	friend std::ostream & operator << (std::ostream & os, AnimatedObjModel & obj);


	std::string _name;
	ANIMATED_MODEL_MODE _mode;
	ObjData * _obj_data;
	float * _matrices;
	uint _n_matrices;
	//map<std::string, AnimatedObjBone *> _bones;
	std::vector<AnimatedObjBone *> _bones;
	//map<std::string, AnimatedObjAction *> _actions;
	std::vector<AnimatedObjAction *> _actions;
	//map<std::string, AnimatedObjObject *> _objects;
	std::vector<AnimatedObjObject *> _objects;
};


struct AnimatedObjInstance : public InstancePosRot {
	AnimatedObjInstance();
	AnimatedObjInstance(AnimatedObjModel * model, pt_3d pos, time_point t, quat q = quat(1.0, 0.0, 0.0, 0.0));
	~AnimatedObjInstance();
	void anim(time_point t);


	AnimatedObjModel * _model;
	AnimatedObjAction * _current_action;
	AnimatedObjFrame * _current_frame;
	uint _current_frame_idx;
	time_point _last_anim_t;
};


struct TestObjAnim {
	TestObjAnim();
	TestObjAnim(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t);
	~TestObjAnim();
	void anim(time_point t);
	void update(AnimatedObjModel * model);
	void draw();
	bool key_down(InputState * input_state, SDL_Keycode key, time_point t);

	
	GLDrawManager * _gl_draw_manager;
	ViewSystem * _view_system;
	map<std::string, AnimatedObjModel *> _models;
	std::vector<AnimatedObjInstance *> _instances;
	bool _paused;
};


#endif
