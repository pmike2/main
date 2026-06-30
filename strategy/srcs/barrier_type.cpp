
#include "json.hpp"

#include "utile.h"

#include "barrier_type.h"

using json = nlohmann::json;


BarrierType::BarrierType() {

}


BarrierType::BarrierType(std::string json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	_name = basename(json_path);
	_life_init = js["life_init"];
	
	_obj_data = new ObjData(js["obj"]);
	_obj_data->_use_ambient = false;
	_obj_data->_use_diffuse = true;
	_obj_data->_use_specular = false;
	_obj_data->_use_shininess = false;
	_obj_data->_use_opacity = false;
	_obj_data->update_data();
}


BarrierType::~BarrierType() {
	delete _obj_data;
}


