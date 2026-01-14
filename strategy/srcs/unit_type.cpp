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

	_name = js["name"];
	_size = pt_3d(js["size"][0], js["size"][1], js["size"][2]);
	_max_velocity = js["max_velocity"];
	for (json::iterator it = js["weights"].begin(); it != js["weights"].end(); ++it) {
		TERRAIN_TYPE ot = str2terrain_type(it.key());
		if (ot == UNKNOWN) {
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
	std::cerr << "UnitType " << _name << " ::elevation_coeff : " << delta_elevation << " non gérée\n";
	return 0.0;
}


// demi-taille de la diagonale du + petit carré contenant unit_type->_size (+ epsilon)
number UnitType::buffer_size() {
	//return 0.5 * norm(pt_2d(std::max(_size.x, _size.y))) + EPS_UNIT_TYPE_BUFFER_SIZE;
	//return 0.5 * norm(pt_2d(std::max(_size.x, _size.y)));
	// sqrt(2) / 2
	return 0.7071 * std::max(_size.x, _size.y);
}


std::ostream & operator << (std::ostream & os, UnitType & ut) {
	os << "name = " << ut._name << " ; size = " << glm_to_string(ut._size) << " ; velocity = " << ut._max_velocity;
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
