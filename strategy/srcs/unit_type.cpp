#include <fstream>

#include "json.hpp"

#include "utile.h"
#include "geom_2d.h"

#include "unit_type.h"


using json = nlohmann::json;


UnitType::UnitType() {

}


UnitType::UnitType(std::string json_path) :
	GridMovingObjectType()
{
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	_name = js["type"];
	_max_velocity = js["max_velocity"];
	_life_init = js["life_init"];
	_regen = js["regen"];
	_shooting_rate = js["shooting_rate"];
	_vision_distance = js["vision_distance"];
	_vision_angle = js["vision_angle"];
	_creation_duration = js["creation_duration"];
	_floats = js["floats"];
	_flies = js["flies"];
	_ammo_type_str = js["ammo"];

	for (json::iterator it = js["vertex_cost"].begin(); it != js["vertex_cost"].end(); ++it) {
		std::string name = it.key();
		_vertex_cost[name] = it.value();
	}

	for (json::iterator it = js["edge_cost"].begin(); it != js["edge_cost"].end(); ++it) {
		std::string name = it.key();
		_edge_cost[name] = it.value();
	}

	_obj_data = new ObjData(js["obj"]);
	_obj_data->_use_ambient = false;
	_obj_data->_use_diffuse = true;
	_obj_data->_use_specular = false;
	_obj_data->_use_shininess = false;
	_obj_data->_use_opacity = false;
	_obj_data->update_data();
}


UnitType::~UnitType() {
	delete _obj_data;
}


pt_2d UnitType::get_max_square_size() {
	return pt_2d(std::max(_obj_data->_aabb->_vmax.x - _obj_data->_aabb->_vmin.x, _obj_data->_aabb->_vmax.y - _obj_data->_aabb->_vmin.y));
}


/*pt_2d UnitType::get_size() {
	number size_x = _obj_data->_aabb->_vmax.x - _obj_data->_aabb->_vmin.x;
	number size_y = _obj_data->_aabb->_vmax.y - _obj_data->_aabb->_vmin.y;
	return pt_2d(size_x, size_y);
}


number UnitType::buffer_size() {
	// TODO : si trop permissif mettre 0.7071
	pt_2d size = get_size();
	return 0.5 * std::max(size.x, size.y);
}*/


std::ostream & operator << (std::ostream & os, UnitType & ut) {
	os << "name = " << ut._name;
	return os;
}
