#include <fstream>
#include <sstream>
#include <iostream>

#include "json.hpp"

#include "utile.h"
#include "gl_utils.h"
#include "action.h"


using json = nlohmann::json;


// ActionTexture -----------------------------------------------------------------
ActionTexture::ActionTexture() {

}


ActionTexture::ActionTexture(std::string texture_path, int n_ms) : _texture_path(texture_path), _n_ms(n_ms) {
	std::string bump= splitext(_texture_path).first+ "_bump.png";
	if (file_exists(bump)) {
		_texture_path_bump= bump;
	}
	else {
		// si la version bump n'existe pas on sautera dans fill_texture_array l'indice correspondant
		_texture_path_bump= NO_PNG;
	}
}


ActionTexture::ActionTexture(std::string texture_path, int n_ms, float texture_idx) : ActionTexture(texture_path, n_ms) {
	_texture_idx= texture_idx;
}


ActionTexture::~ActionTexture() {

}


std::ostream & operator << (std::ostream & os, const ActionTexture & tex) {
	os << "texture_path=" << tex._texture_path << " ; texture_idx=" << tex._texture_idx << " ; n_ms=" << tex._n_ms;
	return os;
}


// ActionForce --------------------------------------------------------------------
ActionForce::ActionForce() {

}


ActionForce::ActionForce(ActionForceType type, pt_type force, number torque, int n_ms) :
	_type(type), _force(force), _torque(torque), _n_ms(n_ms) 
{

}


ActionForce::~ActionForce() {

}


std::ostream & operator << (std::ostream & os, const ActionForce & force) {
	if (force._type== TRANSLATION) {
		os << "type=TRANSLATION ; force=" << force._force.x << " ; " << force._force.x;
	}
	else if (force._type== ROTATION) {
		os << "type=ROTATION ; torque=" << force._torque;
	}
	os << " ; n_ms=" << force._n_ms;
	
	return os;
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


std::ostream & operator << (std::ostream & os, const Action & action) {
	os << "name=" << action._name;
	os << " ; textures=";
	for (auto tex : action._textures) {
		os << *tex << " ; ";
	}
	os << " ; forces=";
	for (auto force : action._forces) {
		os << *force << " ; ";
	}
	return os;
}


// ActionSequence -----------------------------------------------------------------
ActionSequence::ActionSequence() {

}


ActionSequence::~ActionSequence() {
	
}


std::ostream & operator << (std::ostream & os, const ActionSequence & sequence) {
	os << "actions=";
	for (auto action : sequence._actions) {
		os << *action.first << " ; " << action.second;
	}
	return os;
}


// Actiontransition ---------------------------------------------------------------
SequenceTransition::SequenceTransition() {

}


SequenceTransition::SequenceTransition(std::string from, std::string to, int n_ms) : _from(from), _to(to), _n_ms(n_ms) {

}


SequenceTransition::~SequenceTransition() {

}


std::ostream & operator << (std::ostream & os, const SequenceTransition & transition) {
	os << "from=" << transition._from << " ; to=" << transition._to << " ; n_ms=" << transition._n_ms;
	return os;
}


// ActionableObjectModel ------------------------------------------------------------
ActionableObjectModel::ActionableObjectModel() {

}


ActionableObjectModel::ActionableObjectModel(std::string json_path) : _flippable(false) {
	load(json_path);
}


ActionableObjectModel::ActionableObjectModel(const ActionableObjectModel & model) {
	_json_path= model._json_path;
	_name= model._name;
	_flippable= model._flippable;
	for (auto transition : model._transitions) {
		SequenceTransition * t= new SequenceTransition(transition->_from, transition->_to, transition->_n_ms);
		_transitions.push_back(t);
	}
	for (auto seq : model._sequences) {
		_sequences[seq.first]= new ActionSequence();
		for (auto action : seq.second->_actions) {
			Action * ac= new Action();
			ac->_name= action.first->_name;
			for (auto texture : action.first->_textures) {
				ac->_textures.push_back(new ActionTexture(texture->_texture_path, texture->_n_ms, texture->_texture_idx));
			}
			for (auto force : action.first->_forces) {
				ac->_forces.push_back(new ActionForce(force->_type, force->_force, force->_torque, force->_n_ms));
			}
			_sequences[seq.first]->_actions.push_back(std::make_pair(ac, action.second));
		}
	}
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

	if (js["flippable"]!= nullptr) {
		_flippable= js["flippable"];
	}

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
					int n_ms= 0;
					if (ac["n_ms"]!= nullptr) {
						n_ms= ac["n_ms"];
					}
					actions[action_name]->_textures.push_back(new ActionTexture(texture_path, n_ms));
				}
				else if (ac["force"]!= nullptr) {
					pt_type force= pt_type(ac["force"][0], ac["force"][1]);
					int n_ms= 0;
					if (ac["n_ms"]!= nullptr) {
						n_ms= ac["n_ms"];
					}
					actions[action_name]->_forces.push_back(new ActionForce(TRANSLATION, force, 0.0, n_ms));
				}
				else if (ac["torque"]!= nullptr) {
					number torque= ac["torque"];
					int n_ms= 0;
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


int ActionableObjectModel::get_transition(std::string from, std::string to) {
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


std::ostream & operator << (std::ostream & os, const ActionableObjectModel & model) {
	os << "json_path = " << model._json_path << " ; name = " << model._name << "\n";
	os << "transitions = [";
	for (auto transition : model._transitions) {
		os << *transition << " | ";
	}
	os << "]\nsequences = [\n";
	for (auto sequence : model._sequences) {
		os << "\t" << sequence.first << " : (" << *sequence.second << ")";
	}
	os << "]\n";
	return os;
}


// ActionableObject -----------------------------------------------------------------
ActionableObject::ActionableObject() {

}


ActionableObject::ActionableObject(ActionableObjectModel * model) :
	_model_init(model),
	_current_action_texture_idx(0), _current_action_force_idx(0),
	_current_sequence_name(MAIN_SEQUENCE_NAME), _next_sequence_name(MAIN_SEQUENCE_NAME),
	_current_action_idx(0), _flipped(false)
{
	_model= new ActionableObjectModel(*_model_init);
}


ActionableObject::~ActionableObject() {
	delete _model;
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

		randomize();
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

		randomize();
	}
}


void ActionableObject::anim_sequence(time_point t) {
	if (_next_sequence_name!= _current_sequence_name) {
		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_sequence_change_t).count();
		if (dt> _model->get_transition(_current_sequence_name, _next_sequence_name)) {
			_last_sequence_change_t= t;
			_current_sequence_name= _next_sequence_name;
			_current_action_idx= 0;

			_last_action_change_t= t;
			_current_action_texture_idx= 0;
			_last_action_texture_t= t;
			_current_action_force_idx= 0;
			_last_action_force_t= t;

			randomize();
		}
	}
}


void ActionableObject::randomize() {
	if (_model->_flippable && rand_int(0, 100)> 80) {
		_flipped= !_flipped;
	}

	for (auto seq : _model->_sequences) {
		std::string seq_key= seq.first;
		ActionSequence * sequence= seq.second;
		ActionSequence * sequence_init= _model_init->_sequences[seq_key];
		for (int idx_action=0; idx_action<sequence->_actions.size(); ++idx_action) {
			Action * action= sequence->_actions[idx_action].first;
			//int n_ms= sequence->_actions[idx_action].second;
			Action * action_init= sequence_init->_actions[idx_action].first;
			int n_ms_init= sequence_init->_actions[idx_action].second;

			if (n_ms_init> 0) {
				int rand_n_ms= (int)(rand_gaussian(number(n_ms_init), 1000));
				if (rand_n_ms< 0) {
					rand_n_ms= 0;
				}
				_model->_sequences[seq_key]->_actions[idx_action].second= rand_n_ms;
			}

			for (int idx_force=0; idx_force<action->_forces.size(); ++idx_force) {
				ActionForce * force= action->_forces[idx_force];
				ActionForce * force_init= action_init->_forces[idx_force];
				if (force->_type== TRANSLATION) {
					_model->_sequences[seq_key]->_actions[idx_action].first->_forces[idx_force]->_force= rand_gaussian(force_init->_force, pt_type(0.1, 0.1));
					if (_flipped) {
						_model->_sequences[seq_key]->_actions[idx_action].first->_forces[idx_force]->_force*= -1.0;
					}
				}
				else if (force->_type== ROTATION) {
					_model->_sequences[seq_key]->_actions[idx_action].first->_forces[idx_force]->_torque= rand_gaussian(force_init->_torque, 1.0);
				}
				if (force_init->_n_ms> 0) {
					int rand_n_ms= (int)(rand_gaussian(number(force_init->_n_ms), 1000));
					if (rand_n_ms< 0) {
						rand_n_ms= 0;
					}
					_model->_sequences[seq_key]->_actions[idx_action].first->_forces[idx_force]->_n_ms= rand_n_ms;
				}
			}
		}
	}
}


std::ostream & operator << (std::ostream & os, const ActionableObject & obj) {
	os << "model_init = [\n" << *obj._model_init << "\n]\n";
	os << "model = [\n" << *obj._model << "\n]\n";
	os << "current_sequence_name = " << obj._current_sequence_name << " ; next_sequence_name = " << obj._next_sequence_name;
	os << " ; current_action_texture_idx = " << obj._current_action_texture_idx;
	os << " ; current_action_force_idx = " << obj._current_action_force_idx;
	os << " ; current_action_idx = " << obj._current_action_idx;
	os << "\n";
	return os;
}

