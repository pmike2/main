#include <queue>

#include "lake.h"


Lake::Lake() {

}


Lake::Lake(Elevation * elevation, pt_2d src) : Element(elevation, src), _valid(true) {
	_type = ELEMENT_LAKE;

	if (!_elevation->in_boundaries(src)) {
		std::cerr << "Lake impossible src hors grille\n";
		_valid = false;
		return;
	}

	uint id_src = _elevation->pt2id(src);
	std::vector<uint> id_nodes_src = _elevation->lowest_gradient(id_src);
	uint id_lowest = id_nodes_src[id_nodes_src.size() - 1];
	if (_elevation->get_alti(id_lowest) <= 0.0) {
		std::cerr << "Lake impossible lowest pt alti <= 0.0\n";
		_valid = false;
		return;
	}

	std::queue<uint> frontier;
	std::vector<uint> checked;
	const uint n_max_nodes = 1000;
	const number alti_step = 0.1;
	const number max_lake_depth = 2.0;
	
	_alti_lake = _elevation->get_alti(id_lowest) + max_lake_depth;

	while (true) {
		_id_nodes.clear();
		frontier = {};
		checked.clear();
		frontier.push(id_lowest);
		checked.push_back(id_lowest);
		bool is_ok = true;
		
		while (!frontier.empty()) {
			uint id = frontier.front();
			frontier.pop();
			number alti = _elevation->get_alti(id);

			if (alti < _alti_lake) {
				_id_nodes.push_back(id);
			}

			if (_id_nodes.size() > n_max_nodes) {
				is_ok = false;
				break;
			}

			std::vector<uint> id_neighbors = _elevation->neighbors(id);

			for (auto & id_n : id_neighbors) {
				if (std::find(checked.begin(), checked.end(), id_n) != checked.end()) {
					continue;
				}
				checked.push_back(id_n);

				number alti_n = _elevation->get_alti(id_n);

				if (alti_n < _alti_lake) {
					frontier.push(id_n);
				}
			}
		}

		if (is_ok) {
			break;
		}

		_alti_lake -= alti_step;
		if (_alti_lake <= _elevation->get_alti(id_lowest)) {
			std::cerr << "Lake impossible pas d'alti trouvée\n";
			_valid = false;
			return;
		}
	}

	_triangles = _elevation->ids2triangles(_id_nodes);

	// on ne conserve que les id_nodes qui appartiennent à un triangle
	_id_nodes = _elevation->triangles2ids(_triangles);

	_polygon = _elevation->ids2polygon(_id_nodes);

	AABB * aabb = new AABB(pt_3d(_polygon->_aabb->_pos.x, _polygon->_aabb->_pos.y, -0.1),
		pt_3d(_polygon->_aabb->_pos.x + _polygon->_aabb->_size.x, _polygon->_aabb->_pos.y + _polygon->_aabb->_size.y, 0.1));
	_bbox->set_aabb(aabb);
	delete aabb;

	/*number EPS = 1.1;
	for (auto & id : _id_nodes) {
		_elevation->set_alti(id, _alti_lake - EPS);
		_elevation->update_normal(id);
	}
	_elevation->update_data(_polygon->_aabb);*/

	//std::cout << _elevation->ids2wkt(_id_nodes) << "\n";
	//std::cout << _elevation->triangles2wkt(_triangles) << "\n";

	_n_pts = 3 * _triangles.size();
	uint n_attrs_per_pts = 7;
	_data = new float[_n_pts * n_attrs_per_pts];

	update_data();
}


Lake::~Lake() {
	delete _data;
	delete _polygon;
}


void Lake::update_data() {
	const glm::vec4 LAKE_COLOR(0.6, 0.8, 0.9, 0.6);
	float * ptr = _data;
	for (auto & triangle : _triangles) {
		std::vector<uint> ids = {std::get<0>(triangle), std::get<1>(triangle), std::get<2>(triangle)};

		for (uint i=0; i<3; ++i) {
			pt_2d pt = _elevation->id2pt_2d(ids[i]);
			ptr[0] = float(pt.x);
			ptr[1] = float(pt.y);
			ptr[2] = float(_alti_lake);
			ptr[3] = LAKE_COLOR.r;
			ptr[4] = LAKE_COLOR.g;
			ptr[5] = LAKE_COLOR.b;
			ptr[6] = LAKE_COLOR.a;
			ptr += 7;
		}
	}

	/*for (int i=0; i<_n_pts * _n_attrs_per_pts; ++i) {
		std::cout << _data[i] << " ; ";
	}
	std::cout << "\n";*/
}

