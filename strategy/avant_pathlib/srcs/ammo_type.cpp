#include "json.hpp"

#include "ammo_type.h"

using json = nlohmann::json;


AmmoType::AmmoType() {

}


AmmoType::AmmoType(std::string json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	_name = js["name"];
	_damage = js["damage"];
	_velocity = js["velocity"];
	_max_distance = js["max_distance"];
	
	_obj_data = new ObjData(js["obj"]);
	_obj_data->_use_ambient = false;
	_obj_data->_use_diffuse = true;
	_obj_data->_use_specular = false;
	_obj_data->_use_shininess = false;
	_obj_data->_use_opacity = false;
	_obj_data->update_data();
}


AmmoType::~AmmoType() {
	delete _obj_data;
}

