#include <fstream>

#include "json.hpp"

#include "utile.h"
#include "geom_2d.h"

#include "unit_type.h"


using json = nlohmann::json;


UnitType::UnitType() {

}


UnitType::UnitType(std::string json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	_type = str2unit_type(js["type"]);
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

	for (json::iterator it = js["weights"].begin(); it != js["weights"].end(); ++it) {
		TERRAIN_TYPE ot = str2terrain_type(it.key());
		if (ot == TERRAIN_UNKNOWN) {
			continue;
		}
		_terrain_weights[ot] = it.value();
	}

	for (auto & ec : js["delta_elevation_coeffs"]) {
		number ec_min = ec["min"];
		number ec_max = ec["max"];
		number ec_coeff = ec["coeff"];
		UnitElevationCoeff coeff = {ec_min, ec_max, ec_coeff};
		_delta_elevation_coeffs.push_back(coeff);
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


number UnitType::elevation_coeff(number delta_elevation) {
	for (auto & ec : _delta_elevation_coeffs) {
		if (delta_elevation >= ec._elevation_min && delta_elevation <= ec._elevation_max) {
			return ec._coeff;
		}
	}
	std::cerr << "UnitType " << unit_type2str(_type) << " ::elevation_coeff : " << delta_elevation << " non gérée\n";
	return 0.0;
}


pt_2d UnitType::get_size() {
	number size_x = _obj_data->_aabb->_vmax.x - _obj_data->_aabb->_vmin.x;
	number size_y = _obj_data->_aabb->_vmax.y - _obj_data->_aabb->_vmin.y;
	return pt_2d(size_x, size_y);
}


number UnitType::buffer_size() {
	//return 0.5 * norm(pt_2d(std::max(_size.x, _size.y))) + EPS_UNIT_TYPE_BUFFER_SIZE;
	//return 0.5 * norm(pt_2d(std::max(_size.x, _size.y)));
	// sqrt(2) / 2
	//return 0.7071 * std::max(_obj_data->_aabb->size().x, _obj_data->_aabb->size().y);

	// TODO : si trop permissif mettre 0.7071
	pt_2d size = get_size();
	return 0.5 * std::max(size.x, size.y);
}


std::ostream & operator << (std::ostream & os, UnitType & ut) {
	os << "type = " << unit_type2str(ut._type) << " ; velocity = " << ut._max_velocity;
	os << "\nterrain_weights = ";
	for (auto & w : ut._terrain_weights) {
		os << terrain_type2str(w.first) << " -> " << w.second << " ; ";
	}
	os << "\nelevation_coeffs = ";
	for (auto & ec : ut._delta_elevation_coeffs) {
		os << "[elevation_min = " << ec._elevation_min  << " ; elevation_max = " << ec._elevation_max << " ; coeff = " << ec._coeff << "] ; ";
	}
	return os;
}
