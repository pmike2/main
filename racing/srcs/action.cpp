#include <fstream>
#include <sstream>

#include "json.hpp"

#include "utile.h"
#include "gl_utils.h"
#include "action.h"


using json = nlohmann::json;


// ActionTexture -----------------------------------------------------------------
ActionTexture::ActionTexture() {

}


ActionTexture::ActionTexture(std::string texture_path, unsigned int n_ms) : _texture_path(texture_path), _n_ms(n_ms) {
	std::string bump= splitext(_texture_path).first+ "_bump.png";
	if (file_exists(bump)) {
		_texture_path_bump= bump;
	}
	else {
		// si la version bump n'existe pas on sautera dans fill_texture_array l'indice correspondant
		_texture_path_bump= NO_PNG;
	}
}


ActionTexture::~ActionTexture() {

}


// ActionForce --------------------------------------------------------------------
ActionForce::ActionForce() {

}


ActionForce::ActionForce(ActionForceType type, pt_type force, number torque, unsigned int n_ms) :
	_type(type), _force(force), _torque(torque), _n_ms(n_ms) 
{

}


ActionForce::~ActionForce() {

}


// Action -------------------------------------------------------------------------
Action::Action() {

}


Action::~Action() {
	for (auto tex : _textures) {
		delete tex;
	}
	_textures.clear();
	for (auto force : _forces) {
		delete force;
	}
	_forces.clear();
}


// ActionSequence -----------------------------------------------------------------
ActionSequence::ActionSequence() {

}


ActionSequence::~ActionSequence() {
	
}


// Actiontransition ---------------------------------------------------------------
SequenceTransition::SequenceTransition() {

}


SequenceTransition::SequenceTransition(std::string from, std::string to, unsigned int n_ms) : _from(from), _to(to), _n_ms(n_ms) {

}


SequenceTransition::~SequenceTransition() {

}


// ActionableObjectModel ------------------------------------------------------------
ActionableObjectModel::ActionableObjectModel() {

}


ActionableObjectModel::ActionableObjectModel(std::string json_path) {
	load(json_path);
}


ActionableObjectModel::~ActionableObjectModel() {
	for (auto transition : _transitions) {
		delete transition;
	}
	_transitions.clear();
	for (auto seq : _sequences) {
		delete seq.second;
	}
	_sequences.clear();
}


void ActionableObjectModel::load(std::string json_path) {
	_json_path= json_path;
	_name= basename(_json_path);

	std::ifstream ifs(_json_path);
	json js= json::parse(ifs);
	ifs.close();

	std::map<std::string, Action *> actions;
	if (js["actions"]!= nullptr) {
		for (json::iterator it = js["actions"].begin(); it != js["actions"].end(); ++it) {
			auto & action_name= it.key();
			auto & l_ac= it.value();
			actions[action_name]= new Action();
			for (auto ac : l_ac) {
				if (ac["texture"]!= nullptr) {
					std::string texture_rel_path= ac["texture"];
					std::string texture_path= dirname(_json_path)+ "/textures/"+ texture_rel_path;
					unsigned int n_ms= 0;
					if (ac["n_ms"]!= nullptr) {
						n_ms= ac["n_ms"];
					}
					actions[action_name]->_textures.push_back(new ActionTexture(texture_path, n_ms));
				}
				else if (ac["force"]!= nullptr) {
					pt_type force= pt_type(ac["force"][0], ac["force"][1]);
					unsigned int n_ms= 0;
					if (ac["n_ms"]!= nullptr) {
						n_ms= ac["n_ms"];
					}
					actions[action_name]->_forces.push_back(new ActionForce(TRANSLATION, force, 0.0, n_ms));
				}
				else if (ac["torque"]!= nullptr) {
					number torque= ac["torque"];
					unsigned int n_ms= 0;
					if (ac["n_ms"]!= nullptr) {
						n_ms= ac["n_ms"];
					}
					actions[action_name]->_forces.push_back(new ActionForce(ROTATION, pt_type(0.0), torque, n_ms));
				}
			}

			if (actions[action_name]->_textures.empty()) {
				std::string texture_path= dirname(_json_path)+ "/textures/"+ _name+ ".png";
				actions[action_name]->_textures.push_back(new ActionTexture(texture_path, 0));
			}
		}
	}
	else {
		actions[MAIN_ACTION_NAME]= new Action();
		std::string texture_path= dirname(_json_path)+ "/textures/"+ _name+ ".png";
		actions[MAIN_ACTION_NAME]->_textures.push_back(new ActionTexture(texture_path, 0));
	}

	if (js["sequences"]!= nullptr) {
		for (json::iterator it = js["sequences"].begin(); it != js["sequences"].end(); ++it) {
			auto & sequence_name= it.key();
			auto & l_actions= it.value();
			_sequences[sequence_name]= new ActionSequence();
			for (auto ac : l_actions) {
				_sequences[sequence_name]->_actions.push_back(std::make_pair(actions[ac[0]], ac[1]));
			}
		}
	}
	else {
		_sequences[MAIN_SEQUENCE_NAME]= new ActionSequence();
		_sequences[MAIN_SEQUENCE_NAME]->_actions.push_back(std::make_pair(actions.begin()->second, -1));
	}

	if (js["transitions"]!= nullptr) {
		for (auto transition : js["transitions"]) {
			_transitions.push_back(new SequenceTransition(transition["from"], transition["to"], transition["n_ms"]));
		}
	}
}


unsigned int ActionableObjectModel::get_transition(std::string from, std::string to) {
	for (auto transition : _transitions) {
		if (transition->_from== from && transition->_to== to) {
			return transition->_n_ms;
		}
	}
	return 0;
}


std::vector<Action *> ActionableObjectModel::get_unduplicated_actions() {
	std::vector<Action *> result;
	for (auto sequence : _sequences) {
		for (auto ac : sequence.second->_actions) {
			if (std::find(result.begin(), result.end(), ac.first)== result.end()) {
				result.push_back(ac.first);
			}
		}
	}
	return result;
}


// ActionableObject -----------------------------------------------------------------
ActionableObject::ActionableObject() :
	_current_action_texture_idx(0), _current_action_force_idx(0),
	_current_sequence_name(MAIN_SEQUENCE_NAME), _next_sequence_name(MAIN_SEQUENCE_NAME),
	_current_action_idx(0)
{

}


ActionableObject::ActionableObject(ActionableObjectModel * model) :
	_model(model),
	_current_action_texture_idx(0), _current_action_force_idx(0),
	_current_sequence_name(MAIN_SEQUENCE_NAME), _next_sequence_name(MAIN_SEQUENCE_NAME),
	_current_action_idx(0)
{

}


ActionableObject::~ActionableObject() {
	
}


Action * ActionableObject::get_current_action() {
	return _model->_sequences[_current_sequence_name]->_actions[_current_action_idx].first;
}


void ActionableObject::set_current_sequence(std::string sequence_name, time_point t) {
	if (_next_sequence_name== sequence_name) {
		return;
	}

	else if (_current_sequence_name== sequence_name) {
		_next_sequence_name= _current_sequence_name;
		return;
	}

	else {
		_next_sequence_name= sequence_name;
		_last_sequence_change_t= t;
	}
}


void ActionableObject::anim_texture(time_point t) {
	auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_action_texture_t).count();
	if (dt> get_current_action()->_textures[_current_action_texture_idx]->_n_ms) {
		_last_action_texture_t= t;
		_current_action_texture_idx++;
		if (_current_action_texture_idx>= get_current_action()->_textures.size()) {
			_current_action_texture_idx= 0;
		}
	}
}


void ActionableObject::anim_force(time_point t) {
	Action * action= get_current_action();
	if (!action->_forces.empty()) {
		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_action_force_t).count();
		if (dt> action->_forces[_current_action_force_idx]->_n_ms) {
			_last_action_force_t= t;
			_current_action_force_idx++;
			if (_current_action_force_idx>= action->_forces.size()) {
				_current_action_force_idx= 0;
			}
		}
	}
}


void ActionableObject::anim_action(time_point t) {
	/*if (_model->_name== "start") {
		std::cout << _model->_sequences[_current_sequence_name]->_actions[_current_action_idx].second << "\n";
	}*/

	if (_model->_sequences[_current_sequence_name]->_actions[_current_action_idx].second< 0) {
		return;
	}

	auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_action_change_t).count();
	if (dt> _model->_sequences[_current_sequence_name]->_actions[_current_action_idx].second) {
		_current_action_idx++;
		if (_current_action_idx>= _model->_sequences[_current_sequence_name]->_actions.size()) {
			_current_action_idx= 0;
		}

		_last_action_change_t= t;
		_current_action_texture_idx= 0;
		_last_action_texture_t= t;
		_current_action_force_idx= 0;
		_last_action_force_t= t;
	}
}


void ActionableObject::anim_sequence(time_point t) {
	if (_next_sequence_name!= _current_sequence_name) {
		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_sequence_change_t).count();
		
		/*if (_model->_name== "boost") {
			std::cout << _model->_name << " ; " << _current_sequence_name << " -> " << _next_sequence_name << "\n";
			std::cout << _model->get_transition(_current_sequence_name, _next_sequence_name) << "\n";
		}*/
		if (dt> _model->get_transition(_current_sequence_name, _next_sequence_name)) {
			_last_sequence_change_t= t;
			_current_sequence_name= _next_sequence_name;
			_current_action_idx= 0;

			_last_action_change_t= t;
			_current_action_texture_idx= 0;
			_last_action_texture_t= t;
			_current_action_force_idx= 0;
			_last_action_force_t= t;
		}
	}
}

