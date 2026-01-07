#include "river.h"


River::River() {

}


River::River(Elevation * elevation, pt_2d src) : _elevation(elevation), _n_pts(0), _valid(true) {
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
	
	/*for (uint i=0; i<lowest_gradient_id_nodes.size() - 1; ++i) {
		pt_3d pt_begin = _elevation->id2pt_3d(lowest_gradient_id_nodes[i]);
		pt_3d pt_end = _elevation->id2pt_3d(lowest_gradient_id_nodes[i + 1]);
		number length = glm::length(pt_end - pt_begin);
		pt_3d u = (pt_end - pt_begin) / length;
		pt_3d normal = _elevation->get_normal(lowest_gradient_id_nodes[i]);
		pt_3d v = glm::cross(normal, u);
		number width = 1.0 + 1.0 * number(i) / number(lowest_gradient_id_nodes.size() - 2);
		number offset = 0.1;
		pt_3d pt0 = pt_begin - 0.5 * width * v + offset * length * u;
		pt_3d pt1 = pt_begin - 0.5 * width * v + (1.0 - offset) * length * u;
		pt_3d pt2 = pt_begin + 0.5 * width * v + (1.0 - offset) * length * u;
		pt_3d pt3 = pt_begin + 0.5 * width * v + offset * length * u;

		_triangles.push_back(std::make_tuple(pt0, pt1, pt2, normal));
		_triangles.push_back(std::make_tuple(pt0, pt2, pt3, normal));
	}

	std::vector<std::tuple<pt_3d, pt_3d, pt_3d, pt_3d> > triangles_jointure;
	for (uint i=0; i<=(_triangles.size() - 4) / 2; ++i) {
		pt_3d pt0 = std::get<1>(_triangles[2 * i]);
		pt_3d pt1 = std::get<0>(_triangles[2 * i + 3]);
		pt_3d pt2 = std::get<2>(_triangles[2 * i + 3]);
		pt_3d pt3 = std::get<2>(_triangles[2 * i]);
		pt_3d normal = glm::normalize(std::get<3>(_triangles[2 * i]) + std::get<3>(_triangles[2 * i + 3]));

		triangles_jointure.push_back(std::make_tuple(pt0, pt1, pt2, normal));
		triangles_jointure.push_back(std::make_tuple(pt0, pt2, pt3, normal));
	}

	_triangles.insert(_triangles.end(), triangles_jointure.begin(), triangles_jointure.end());*/

	_id_nodes = _elevation->buffered_ids(lowest_gradient_id_nodes, 2);

	_triangles = _elevation->ids2triangles(_id_nodes);

	// on ne conserve que les id_nodes qui appartiennent à un triangle
	_id_nodes = _elevation->triangles2ids(_triangles);

	/*for (auto & triangle : _triangles) {
		std::vector<pt_2d> pts_2d = {pt_2d(std::get<0>(triangle)), pt_2d(std::get<1>(triangle)), pt_2d(std::get<2>(triangle))};
		Polygon2D * polygon = new Polygon2D(pts_2d);
		polygon->update_all();
		
		std::vector<std::pair<uint, uint> > edges = _elevation->polygon_intersection(polygon);
		for (auto & edge : edges) {
			if (std::find(_id_nodes.begin(), _id_nodes.end(), edge.first) == _id_nodes.end()) {
				_id_nodes.push_back(edge.first);
			}
			if (std::find(_id_nodes.begin(), _id_nodes.end(), edge.second) == _id_nodes.end()) {
				_id_nodes.push_back(edge.second);
			}
		}

		delete polygon;
	}*/

	/*number EPS = 0.2;
	for (auto & id : _id_nodes) {
		_elevation->set_alti(id, _elevation->get_alti(id) - EPS);
	}*/

	/*std::vector<pt_2d> pts;
	for (auto & id : _id_nodes) {
		pts.push_back(_elevation->id2pt_2d(id));
	}
	_polygon = new Polygon2D(pts);
	_polygon->update_all();
	_polygon->triangulate();*/

	uint n_attrs_per_pts= 10;
	_n_pts = 3 * _triangles.size();
	//_n_pts = 3 * _polygon->_triangles_idx.size();
	_data = new float[_n_pts * n_attrs_per_pts];
	update_data();
}


River::~River() {
	delete _data;
}


void River::update_data() {
	//const number RIVER_Z_OFFSET = 0.1;
	const glm::vec4 RIVER_COLOR(0.5, 0.7, 0.9, 1.0);
	//const glm::vec4 RIVER_COLOR_DEBUG(0.9, 0.3, 0.3, 1.0);
	float * ptr = _data;
	//uint debug = 0;
	for (auto & triangle : _triangles) {
		std::vector<uint> ids = {std::get<0>(triangle), std::get<1>(triangle), std::get<2>(triangle)};

		//std::vector<pt_3d> pts = {std::get<0>(triangle), std::get<1>(triangle), std::get<2>(triangle)};
		//pt_3d normal = std::get<3>(triangle);

	//for (auto & triangle : _polygon->_triangles_idx) {
		//std::vector<pt_2d> pts = {_polygon->_pts[triangle[0]], _polygon->_pts[triangle[1]], _polygon->_pts[triangle[2]]};
		
		//pt_3d normal(0.0, 0.0, 1.0);
		
		for (uint i=0; i<3; ++i) {
			pt_2d pt = _elevation->id2pt_2d(ids[i]);
			pt_3d normal = _elevation->get_normal(ids[i]);
			
			//ptr[0] = float(pts[i].x);
			//ptr[1] = float(pts[i].y);
			ptr[0] = float(pt.x);
			ptr[1] = float(pt.y);
			ptr[2] = _elevation->get_alti(pt) + 0.01;
			//if (debug < (2 * _id_nodes.size() - 1)) {
			ptr[3] = RIVER_COLOR.r;
			ptr[4] = RIVER_COLOR.g;
			ptr[5] = RIVER_COLOR.b;
			ptr[6] = RIVER_COLOR.a;
			/*}
			else {
			ptr[3] = RIVER_COLOR_DEBUG.r;
			ptr[4] = RIVER_COLOR_DEBUG.g;
			ptr[5] = RIVER_COLOR_DEBUG.b;
			ptr[6] = RIVER_COLOR_DEBUG.a;
			}*/
			ptr[7] = float(normal.x);
			ptr[8] = float(normal.y);
			ptr[9] = float(normal.z);
			ptr += 10;
		}
		//debug++;
	}
}


pt_2d River::lowest_pt() {
	number lowest_alti = 1e9;
	pt_2d result(0.0);
	for (auto & id : _id_nodes) {
		if (_elevation->get_alti(id) < lowest_alti) {
			lowest_alti = _elevation->get_alti(id);
			result = _elevation->id2pt_2d(id);
		}
	}
	return result;
}
