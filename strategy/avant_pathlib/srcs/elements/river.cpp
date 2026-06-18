#include "river.h"


River::River() {

}


River::River(Elevation * elevation, pt_2d src) : Element(elevation, src), _valid(true) {
	_type = ELEMENT_RIVER;
	
	if (!_elevation->in_boundaries(src)) {
		std::cerr << "River impossible src hors grille\n";
		_valid = false;
		return;
	}

	if (_elevation->get_alti(src) < 0.1) {
		std::cerr << "River impossible src alti < eps\n";
		_valid = false;
		return;
	}

	uint id_src = _elevation->pt2id(src);
	std::vector<uint> lowest_gradient_id_nodes = _elevation->lowest_gradient(id_src);
	
	_id_nodes = _elevation->buffered_ids(lowest_gradient_id_nodes, 2);

	_triangles = _elevation->ids2triangles(_id_nodes);

	// on ne conserve que les id_nodes qui appartiennent à un triangle
	_id_nodes = _elevation->triangles2ids(_triangles);

	_polygon = _elevation->ids2polygon(_id_nodes);

	pt_3d lp = lowest_pt();
	pt_3d hp = highest_pt();
	AABB * aabb = new AABB(pt_3d(_polygon->_aabb->_pos.x, _polygon->_aabb->_pos.y, lp.z),
		pt_3d(_polygon->_aabb->_pos.x + _polygon->_aabb->_size.x, _polygon->_aabb->_pos.y + _polygon->_aabb->_size.y, hp.z));
	_bbox->set_aabb(aabb);
	delete aabb;

	/*number EPS = 0.5;
	for (auto & id : _id_nodes) {
		_elevation->set_alti(id, _elevation->get_alti(id) - EPS);
		_elevation->update_normal(id);
	}
	_elevation->update_data(_polygon->_aabb);*/

	_n_pts = 3 * _triangles.size();
	uint n_attrs_per_pts = 12;
	_data = new float[_n_pts * n_attrs_per_pts];
	update_data();
}


River::~River() {
	delete _data;
	delete _polygon;
}


void River::update_data() {
	const glm::vec4 RIVER_COLOR(0.6, 0.8, 0.9, 0.6);
	float * ptr = _data;
	for (auto & triangle : _triangles) {
		std::vector<uint> ids = {std::get<0>(triangle), std::get<1>(triangle), std::get<2>(triangle)};
		
		for (uint i=0; i<3; ++i) {
			pt_2d pt = _elevation->id2pt_2d(ids[i]);
			pt_3d normal = _elevation->get_normal(ids[i]);
			pt_2d direction = _elevation->id2pt_2d(_elevation->lowest_neighbor(ids[i])) - pt;
			
			ptr[0] = float(pt.x);
			ptr[1] = float(pt.y);
			ptr[2] = _elevation->get_alti(pt) + 0.01;
			ptr[3] = RIVER_COLOR.r;
			ptr[4] = RIVER_COLOR.g;
			ptr[5] = RIVER_COLOR.b;
			ptr[6] = RIVER_COLOR.a;
			ptr[7] = float(normal.x);
			ptr[8] = float(normal.y);
			ptr[9] = float(normal.z);
			ptr[10] = float(direction.x);
			ptr[11] = float(direction.y);
			ptr += 12;
		}
	}
}


pt_3d River::lowest_pt() {
	number lowest_alti = 1e9;
	pt_3d result(0.0);
	for (auto & id : _id_nodes) {
		if (_elevation->get_alti(id) < lowest_alti) {
			lowest_alti = _elevation->get_alti(id);
			result = _elevation->id2pt_3d(id);
		}
	}
	return result;
}


pt_3d River::highest_pt() {
	number highest_alti = -1e9;
	pt_3d result(0.0);
	for (auto & id : _id_nodes) {
		if (_elevation->get_alti(id) > highest_alti) {
			highest_alti = _elevation->get_alti(id);
			result = _elevation->id2pt_3d(id);
		}
	}
	return result;
}


json River::get_json() {
	json result;
	
	result["type"] = element_type2str(_type);
	result["position"] = json::array();
	result["position"].push_back(_position.x);
	result["position"].push_back(_position.y);
	//result["position"].push_back(_position.z);

	return result;
}
