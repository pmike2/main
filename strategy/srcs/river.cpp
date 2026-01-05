#include "river.h"


River::River() {

}


River::River(Elevation * elevation, pt_2d src) : _elevation(elevation), _n_pts(0) {

	std::cout << "river begin\n";
	uint id_src = _elevation->pt2id(src);
	_id_nodes = _elevation->lowest_gradient(id_src);
	
std::cout << "river triangles\n";
	for (uint i=0; i<_id_nodes.size() - 1; ++i) {
		pt_3d pt_begin = _elevation->id2pt_3d(_id_nodes[i]);
		pt_3d pt_end = _elevation->id2pt_3d(_id_nodes[i + 1]);
		number length = glm::length(pt_end - pt_begin);
		pt_3d u = (pt_end - pt_begin) / length;
		pt_3d normal = _elevation->get_normal(_id_nodes[i]);
		pt_3d v = glm::cross(normal, u);
		//number width = rand_number(0.1, 0.5);
		//number width = 0.5;
		number width = 0.1 + 0.4 * number(i) / number(_id_nodes.size() - 2);
		number offset = 0.3;
		pt_3d pt0 = pt_begin - 0.5 * width * v + offset * length * u;
		pt_3d pt1 = pt_begin - 0.5 * width * v + (1.0 - offset) * length * u;
		pt_3d pt2 = pt_begin + 0.5 * width * v + (1.0 - offset) * length * u;
		pt_3d pt3 = pt_begin + 0.5 * width * v + offset * length * u;

		_triangles.push_back(std::make_tuple(pt0, pt1, pt2, normal));
		_triangles.push_back(std::make_tuple(pt0, pt2, pt3, normal));
	}

std::cout << "river jointures\n";
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

	_triangles.insert(_triangles.end(), triangles_jointure.begin(), triangles_jointure.end());

	uint n_attrs_per_pts= 10;
	_n_pts = 3 * _triangles.size();
	_data = new float[_n_pts * n_attrs_per_pts];
	std::cout << "river end\n";
	update_data();
}


River::~River() {
	delete _data;
}


void River::update_data() {
	std::cout << "river update\n";
	const number RIVER_Z_OFFSET = 0.1;
	const glm::vec4 RIVER_COLOR(0.5, 0.7, 0.9, 1.0);
	const glm::vec4 RIVER_COLOR_DEBUG(0.9, 0.3, 0.3, 1.0);
	float * ptr = _data;
	uint debug = 0;
	for (auto & triangle : _triangles) {
		std::vector<pt_3d> pts = {std::get<0>(triangle), std::get<1>(triangle), std::get<2>(triangle)};
		pt_3d normal = std::get<3>(triangle);

		for (uint i=0; i<3; ++i) {
			ptr[0] = float(pts[i].x);
			ptr[1] = float(pts[i].y);
			ptr[2] = float(pts[i].z + RIVER_Z_OFFSET);
			if (debug < (2 * _id_nodes.size() - 1)) {
			ptr[3] = RIVER_COLOR.r;
			ptr[4] = RIVER_COLOR.g;
			ptr[5] = RIVER_COLOR.b;
			ptr[6] = RIVER_COLOR.a;
			}
			else {
			ptr[3] = RIVER_COLOR_DEBUG.r;
			ptr[4] = RIVER_COLOR_DEBUG.g;
			ptr[5] = RIVER_COLOR_DEBUG.b;
			ptr[6] = RIVER_COLOR_DEBUG.a;
			}
			ptr[7] = float(normal.x);
			ptr[8] = float(normal.y);
			ptr[9] = float(normal.z);
			ptr += 10;
		}
		debug++;
	}
	std::cout << "river update end\n";
}

