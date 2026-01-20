#include <fstream>
#include <sstream>

#include "json.hpp"

#include "stone.h"


using json = nlohmann::json;


// StoneSpecies ---------------------------------------------------------------------------------
StoneSpecies::StoneSpecies() {

}


StoneSpecies::StoneSpecies(std::string json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	_name = js["name"];
	_color[0] = js["color"][0];
	_color[1] = js["color"][1];
	_color[2] = js["color"][2];
	_color[3] = js["color"][3];
	_alti_min = js["alti_min"];
	_alti_max = js["alti_max"];
	_water_dist_min = js["water_dist_min"];
	_water_dist_max = js["water_dist_max"];
	_size_min = pt_3d(js["size_min"][0], js["size_min"][1], js["size_min"][2]);
	_size_max = pt_3d(js["size_max"][0], js["size_max"][1], js["size_max"][2]);
}


StoneSpecies::~StoneSpecies() {

}


// Stone ----------------------------------------------------------------------------------------
Stone::Stone() {

}


Stone::Stone(StoneSpecies * species, Elevation * elevation, pt_2d position) : Element(elevation, position), _species(species) {
	_type = ELEMENT_STONE;
	_hull = new ConvexHull();
	
	pt_3d vmin;
	vmin.x = position.x - rand_number(_species->_size_min.x, _species->_size_max.x);
	vmin.y = position.y - rand_number(_species->_size_min.y, _species->_size_max.y);
	vmin.z = _elevation->get_alti(position);

	pt_3d vmax;
	vmax.x = position.x + rand_number(_species->_size_min.x, _species->_size_max.x);
	vmax.y = position.y + rand_number(_species->_size_min.y, _species->_size_max.y);
	vmax.z = vmin.z + rand_number(_species->_size_min.z, _species->_size_max.z);

	AABB * aabb = new AABB(vmin, vmax);
	_bbox->set_aabb(aabb);
	delete aabb;

	_hull->randomize(STONE_N_POINTS_HULL, vmin, vmax);
	_hull->compute();

	uint n_attrs_per_pts= 10;
	_n_pts = _hull->_faces.size() * 3;
	_data = new float[_n_pts * n_attrs_per_pts];
	update_data();
}


Stone::~Stone() {
	delete _hull;
	delete _data;
}


void Stone::update_data() {
	uint compt = 0;
	for (auto face : _hull->_faces) {
		for (uint i=0; i<3; ++i) {
			Pt * pt = _hull->_pts[face->_idx[i]];
			_data[compt++] = float(pt->_coords.x);
			_data[compt++] = float(pt->_coords.y);
			_data[compt++] = float(pt->_coords.z);
			_data[compt++] = _species->_color[0];
			_data[compt++] = _species->_color[1];
			_data[compt++] = _species->_color[2];
			_data[compt++] = _species->_color[3];
			_data[compt++] = float(face->_normal.x);
			_data[compt++] = float(face->_normal.y);
			_data[compt++] = float(face->_normal.z);
		}
	}
}

