
#include "json.hpp"

#include "utile.h"

#include "barrier_type.h"

using json = nlohmann::json;


BarrierType::BarrierType() {

}


BarrierType::BarrierType(fs json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	_name = json_path.stem();
	_life_init = js["life_init"];
	
	_obj_data = new ObjData(js["obj"]);
	_obj_data->update_data(std::vector<OBJDATA_DATA_ITEM>{OBJDATA_VERTEX, OBJDATA_NORMAL, OBJDATA_DIFFUSE_COLOR});
}


BarrierType::~BarrierType() {
	delete _obj_data;
}


