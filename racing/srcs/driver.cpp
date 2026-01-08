#include <fstream>
#include <sstream>
#include <iostream>

#include "json.hpp"

#include "utile.h"
#include "driver.h"


using json = nlohmann::json;

// ExpressionTexture ------------------------
ExpressionTexture::ExpressionTexture() {

}


ExpressionTexture::ExpressionTexture(std::string texture_path, uint n_ms) : _texture_path(texture_path), _n_ms(n_ms) {
	
}


ExpressionTexture::~ExpressionTexture() {
	
}


// Expression ------------------------------
Expression::Expression() {

}


Expression::~Expression() {
	for (auto t : _textures) {
		delete t;
	}
	_textures.clear();
}


// Driver ---------------------------------
Driver::Driver() {

}


Driver::Driver(std::string json_path) : _current_expression_name(NORMAL_EXPRESSION), _next_expression_name(NORMAL_EXPRESSION),
	_current_expression_texture_idx(0), _angry(false), _happy(false), _tired(false), _sad(false) {
	_json_path= json_path;
	_name= basename(_json_path);

	std::ifstream ifs(_json_path);
	json js= json::parse(ifs);
	ifs.close();

	for (json::iterator it = js["expressions"].begin(); it != js["expressions"].end(); ++it) {
		auto & expression_name= it.key();
		auto & l_tex= it.value();
		_expressions[expression_name]= new Expression();
		for (auto tex : l_tex) {
			std::string texture_rel_path= tex["texture"];
			std::string texture_path= dirname(_json_path)+ "/textures/"+ texture_rel_path;
			uint n_ms= 0;
			if (tex["n_ms"]!= nullptr) {
				n_ms= tex["n_ms"];
			}
			_expressions[expression_name]->_textures.push_back(new ExpressionTexture(texture_path, n_ms));
		}
	}
}


Driver::~Driver() {
	for (auto exp : _expressions) {
		delete exp.second;
	}
	_expressions.clear();
}


void Driver::reinit(time_point t, bool set_normal_expression) {
	_angry= _happy= _tired= _sad= false;
	if (set_normal_expression) {
		_current_expression_name= _next_expression_name= NORMAL_EXPRESSION;
	}
}


void Driver::set_current_expression(std::string expression_name, time_point t) {
	if (_current_expression_name== expression_name) {
		_next_expression_name= _current_expression_name;
		return;
	}
	if (_next_expression_name== expression_name) {
		return;
	}
	_next_expression_name= expression_name;
	_last_change_expression_t= t;
}


void Driver::anim(time_point t) {
	if (_next_expression_name!= _current_expression_name) {
		auto dt_change= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_change_expression_t).count();
		if ((_next_expression_name== NORMAL_EXPRESSION && dt_change> EXPRESSION_CHANGE_TO_NORMAL_MS)
			|| (_next_expression_name!= NORMAL_EXPRESSION && dt_change> EXPRESSION_CHANGE_TO_OTHERS_MS)) {
			_current_expression_name= _next_expression_name;
			_last_anim_expression_t= t;
			_current_expression_texture_idx= 0;
		}
	}
	
	auto dt_anim= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_anim_expression_t).count();
	if (dt_anim> _expressions[_current_expression_name]->_textures[_current_expression_texture_idx]->_n_ms) {
		_last_anim_expression_t= t;
		_current_expression_texture_idx++;
		if (_current_expression_texture_idx>= _expressions[_current_expression_name]->_textures.size()) {
			_current_expression_texture_idx= 0;
		}
	}

	// gestion priorité
	if (_tired) {
		set_current_expression(TIRED_EXPRESSION, t);
	}
	else if (_angry) {
		set_current_expression(ANGRY_EXPRESSION, t);
	}
	else if (_happy) {
		set_current_expression(HAPPY_EXPRESSION, t);
	}
	else if (_sad) {
		set_current_expression(SAD_EXPRESSION, t);
	}
	else {
		set_current_expression(NORMAL_EXPRESSION, t);
	}
}
