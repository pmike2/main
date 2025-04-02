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


ExpressionTexture::ExpressionTexture(std::string texture_path, unsigned int n_ms) : _texture_path(texture_path), _n_ms(n_ms) {
	
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


Driver::Driver(std::string json_path) : _current_expression_name("normal"), _next_expression_name("normal"),
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
			unsigned int n_ms= 0;
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
		_current_expression_name= _next_expression_name= "normal";
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
	/*if (_name== "pmb") {
		std::cout << "current = " << _current_expression_name << " ; next = " << _next_expression_name << " set to " << expression_name << "\n";
	}*/
	_next_expression_name= expression_name;
	_last_change_expression_t= t;
}


void Driver::anim(time_point t) {
	/*if (_name== "pmb") {
		std::cout << "anim ; current = " << _current_expression_name << " ; next = " << _next_expression_name << "\n";
	}*/
	if (_next_expression_name!= _current_expression_name) {
		auto dt_change= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_change_expression_t).count();
		if ((_next_expression_name== "normal" && dt_change> EXPRESSION_CHANGE_TO_NORMAL_MS)
			|| (_next_expression_name!= "normal" && dt_change> EXPRESSION_CHANGE_TO_OTHERS_MS)) {
			/*if (_name== "pmb") {
				std::cout << _current_expression_name << " -> " << _next_expression_name << "\n";
			}*/
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
}
