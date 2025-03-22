#include <fstream>
#include <sstream>

#include "json.hpp"

#include "utile.h"
#include "material.h"


using json = nlohmann::json;


Material::Material() {

}


Material::Material(std::string json_path) {
	_json_path= json_path;
	_name= basename(_json_path);

	std::ifstream ifs(_json_path);
	json js= json::parse(ifs);
	ifs.close();

	_solid= js["solid"];
	
	if (js["density"]!= nullptr) {
		_density= js["density"];
	}
	else {
		_density= 0.0;
	}

	if (js["restitution"]!= nullptr) {
		_restitution= js["restitution"];
	}
	else {
		_density= 0.0;
	}

	if (js["linear_friction"]!= nullptr) {
		_linear_friction= js["linear_friction"];
	}
	else {
		_linear_friction= 0.0;
	}

	if (js["angular_friction"]!= nullptr) {
		_angular_friction= js["angular_friction"];
	}
	else {
		_angular_friction= 0.0;
	}
}



Material::~Material() {
	
}

