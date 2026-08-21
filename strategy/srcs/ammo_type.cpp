#include "json.hpp"

#include "utile.h"

#include "ammo_type.h"

using json = nlohmann::json;


AmmoType::AmmoType() {

}


AmmoType::AmmoType(fs json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	_name = json_path.stem();
	_damage = js["damage"];
	_rate = js["rate"];
	_velocity = js["velocity"];
	_max_distance = js["max_distance"];
	_explosion_radius = js["explosion_radius"];
	_explosion_config_str = js["explosion_config_name"];
	_apogee = js["apogee"];
	_ballistic = js["ballistic"];
	
	_obj_data = new ObjData(js["obj"]);
	_obj_data->update_data(std::vector<OBJDATA_DATA_ITEM>{OBJDATA_VERTEX, OBJDATA_NORMAL, OBJDATA_DIFFUSE_COLOR});
}


AmmoType::~AmmoType() {
	delete _obj_data;
}

