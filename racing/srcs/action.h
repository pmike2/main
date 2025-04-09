#ifndef ACTION_H
#define ACTION_H

#include <string>
#include <vector>

#include "typedefs.h"


enum ActionForceType {TRANSLATION, ROTATION};

const std::string MAIN_SEQUENCE_NAME= "main_sequence";
const std::string CAR_CONTACT_SEQUENCE_NAME= "car_contact_sequence";
const std::string MAIN_ACTION_NAME= "main_action";


class ActionTexture {
public:
	ActionTexture();
	ActionTexture(std::string texture_path, unsigned int n_ms);
	~ActionTexture();


	std::string _texture_path;
	std::string _texture_path_bump;
	float _texture_idx;
	unsigned int _n_ms;
};


class ActionForce {
public:
	ActionForce();
	ActionForce(ActionForceType type, pt_type force, number torque, unsigned int n_ms);
	~ActionForce();


	ActionForceType _type;
	pt_type _force;
	number _torque;
	
	unsigned int _n_ms;
};


class Action {
public:
	Action();
	~Action();


	std::vector<ActionTexture *> _textures;
	std::vector<ActionForce * > _forces;
	std::string _name;
};


class ActionSequence {
public:
	ActionSequence();
	~ActionSequence();


	std::vector<std::pair<Action *, int> > _actions;
};


class SequenceTransition {
public:
	SequenceTransition();
	SequenceTransition(std::string from, std::string to, unsigned int n_ms);
	~SequenceTransition();
	
	
	std::string _from;
	std::string _to;
	unsigned int _n_ms;
};


class ActionableObjectModel {
public:
	ActionableObjectModel();
	ActionableObjectModel(std::string json_path);
	~ActionableObjectModel();
	void load(std::string json_path);
	unsigned int get_transition(std::string from, std::string to);
	std::vector<Action *> get_unduplicated_actions();


	std::string _json_path; // chemin json de config
	std::string _name; // nom
	std::vector<SequenceTransition * > _transitions;
	std::map<std::string, ActionSequence *> _sequences;
};


class ActionableObject {
public:
	ActionableObject();
	ActionableObject(ActionableObjectModel * model);
	~ActionableObject();
	Action * get_current_action();
	void set_current_sequence(std::string sequence_name, time_point t);
	void anim_texture(time_point t);
	void anim_force(time_point t);
	void anim_action(time_point t);
	void anim_sequence(time_point t);


	ActionableObjectModel * _model;

	time_point _last_action_change_t;
	time_point _last_action_texture_t;
	time_point _last_action_force_t;
	time_point _last_sequence_change_t;
	unsigned int _current_action_texture_idx;
	unsigned int _current_action_force_idx;
	unsigned int _current_action_idx;
	std::string _current_sequence_name;
	std::string _next_sequence_name;
};

#endif
