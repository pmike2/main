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

	if (js["bumpable"]!= nullptr) {
		_bumpable= js["bumpable"];
	}
	else {
		_bumpable= false;
	}
	
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
		_restitution= 0.0;
	}

	if (js["collision_thrust"]!= nullptr) {
		_collision_thrust= js["collision_thrust"];
	}
	else {
		_collision_thrust= 100.0; // car->_thrust non bridé
	}

	if (js["linear_friction"]!= nullptr) {
		_linear_friction= js["linear_friction"];
	}
	else {
		_linear_friction= 1.0;
	}

	if (js["angular_friction"]!= nullptr) {
		_angular_friction= js["angular_friction"];
	}
	else {
		_angular_friction= 1.0;
	}

	if (js["slippery"]!= nullptr) {
		_slippery= js["slippery"];
	}
	else {
		_slippery= 1.0;
	}

	if (js["surface_change_ms"]!= nullptr) {
		_surface_change_ms= js["surface_change_ms"];
	}
	else {
		_surface_change_ms= 0;
	}

	if (js["damage"]!= nullptr) {
		_damage= js["damage"];
	}
	else {
		_damage= 0.0;
	}

	_tire_track_texture_idx= 0.0; // sera éventuellement renseigné dans racing.cpp
}



Material::~Material() {
	
}

