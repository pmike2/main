#include <queue>

#include "lake.h"


Lake::Lake() {

}


Lake::Lake(Elevation * elevation, pt_2d src) : _elevation(elevation), _n_pts(0), _valid(true) {
	uint id_src = _elevation->pt2id(src);
	std::vector<uint> id_nodes_src = _elevation->lowest_gradient(id_src);
	uint id_lowest = id_nodes_src[id_nodes_src.size() - 1];
	if (_elevation->get_alti(id_lowest) <= 0.0) {
		std::cerr << "Lake impossible lowest pt alti <= 0.0\n";
		_valid = false;
		return;
	}

	// std::pair<uint, uint> col_lig = _elevation->id2col_lig(id_lowest);
	// int size = 1000;
	// std::vector<uint> id_nodes;
	// for (int col = int(col_lig.first) - size; col < int(col_lig.first) + size; ++col) {
	// 	for (int lig = int(col_lig.second) - size; lig < int(col_lig.second) + size; ++lig) {
	// 		if (!_elevation->in_boundaries(col, lig)) {
	// 			continue;
	// 		}

	// 		uint id = _elevation->col_lig2id(uint(col), uint(lig));
	// 		std::vector<uint> l_ids = _elevation->lowest_gradient(id);
	// 		if (l_ids[l_ids.size() - 1] == id_lowest) {
	// 			id_nodes.push_back(id);
	// 		}
	// 	}
	// }

	// for (auto & id : id_nodes) {
	// 	std::vector<uint> id_neighbors = _elevation->get_neighbors(id);
	// 	bool is_in_frontier = false;
	// 	for (auto & id_n : id_neighbors) {
	// 		if (std::find(id_nodes.begin(), id_nodes.end(), id_n) == id_nodes.end()) {
	// 			is_in_frontier = true;
	// 			break;
	// 		}
	// 	}
	// 	if (is_in_frontier) {
	// 		_id_nodes.push_back(id);
	// 	}
	// }

	std::queue<uint> frontier;
	std::vector<uint> checked;
	//std::vector<uint> id_nodes;
	//number current_alti = _elevation->get_alti(id_lowest);
	//number alti_max = 1e9;

	const uint n_max_nodes = 1000;
	const number alti_step = 0.2;
	const number max_lake_depth = 2.0;
	number alti_lake = _elevation->get_alti(id_lowest) + max_lake_depth;

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

			if (alti < alti_lake) {
				_id_nodes.push_back(id);
			}

			if (_id_nodes.size() > n_max_nodes) {
				is_ok = false;
				break;
			}

			std::vector<uint> id_neighbors = _elevation->get_neighbors(id);

			for (auto & id_n : id_neighbors) {
				if (std::find(checked.begin(), checked.end(), id_n) != checked.end()) {
					continue;
				}
				checked.push_back(id_n);


				number alti_n = _elevation->get_alti(id_n);

				if (alti_n < alti_lake) {
					frontier.push(id_n);
				}
			}
		}

		if (is_ok) {
			break;
		}

		alti_lake -= alti_step;
	}

	/*std::cout << "alti_max = " << alti_max << " ; checked.size() = " << checked.size() << "\n";

	checked.clear();
	frontier.push(id_lowest);

	while (!frontier.empty()) {
		uint id = frontier.front();
		frontier.pop();
		number alti = _elevation->get_alti(id);

		std::cout << "alti = " << alti << "\n";

		if (alti > alti_max) {
			continue;
		}

		_id_nodes.push_back(id);

		std::vector<uint> id_neighbors = _elevation->get_neighbors(id);

		for (auto & id_n : id_neighbors) {
			if (std::find(checked.begin(), checked.end(), id_n) != checked.end()) {
				continue;
			}

			number alti_n = _elevation->get_alti(id_n);

			if (alti_n <= alti) {
			}
			else if (std::find(checked.begin(), checked.end(), id_n) == checked.end()) {
				frontier.push(id_n);
				checked.push_back(id_n);
			}
		}
	}*/

	//std::cout << _id_nodes.size() << "\n";

	for (auto & id_node : _id_nodes) {
		std::pair<uint, uint> col_lig = _elevation->id2col_lig(id_node);
		uint id_right = _elevation->col_lig2id(col_lig.first + 1, col_lig.second);
		uint id_top = _elevation->col_lig2id(col_lig.first, col_lig.second + 1);
		uint id_left = _elevation->col_lig2id(col_lig.first - 1, col_lig.second);
		uint id_bottom = _elevation->col_lig2id(col_lig.first, col_lig.second - 1);
		
		if (_elevation->in_boundaries(col_lig.first + 1, col_lig.second) && std::find(_id_nodes.begin(), _id_nodes.end(), id_right) != _id_nodes.end()
		 && _elevation->in_boundaries(col_lig.first, col_lig.second + 1) && std::find(_id_nodes.begin(), _id_nodes.end(), id_top) != _id_nodes.end()) {
			pt_3d pt0 = pt_3d(_elevation->id2pt_2d(id_node), alti_lake);
			pt_3d pt1 = pt_3d(_elevation->id2pt_2d(id_right), alti_lake);
			pt_3d pt2 = pt_3d(_elevation->id2pt_2d(id_top), alti_lake);
			pt_3d normal(0.0, 0.0, 1.0);
			_triangles.push_back(std::make_tuple(pt0, pt1, pt2, normal));
		}

		if (_elevation->in_boundaries(col_lig.first - 1, col_lig.second) && std::find(_id_nodes.begin(), _id_nodes.end(), id_left) != _id_nodes.end()
		 && _elevation->in_boundaries(col_lig.first, col_lig.second - 1) && std::find(_id_nodes.begin(), _id_nodes.end(), id_bottom) != _id_nodes.end()) {
			pt_3d pt0 = pt_3d(_elevation->id2pt_2d(id_node), alti_lake);
			pt_3d pt1 = pt_3d(_elevation->id2pt_2d(id_left), alti_lake);
			pt_3d pt2 = pt_3d(_elevation->id2pt_2d(id_bottom), alti_lake);
			pt_3d normal(0.0, 0.0, 1.0);
			_triangles.push_back(std::make_tuple(pt0, pt1, pt2, normal));
		}
	}

	uint n_attrs_per_pts= 10;
	_n_pts = 3 * _triangles.size();
	_data = new float[_n_pts * n_attrs_per_pts];

	update_data();
}


Lake::~Lake() {
	delete _data;
}


void Lake::update_data() {
	const glm::vec4 LAKE_COLOR(0.8, 0.7, 0.9, 1.0);
	float * ptr = _data;
	for (auto & triangle : _triangles) {
		std::vector<pt_3d> pts = {std::get<0>(triangle), std::get<1>(triangle), std::get<2>(triangle)};
		pt_3d normal = std::get<3>(triangle);

		for (uint i=0; i<3; ++i) {
			ptr[0] = float(pts[i].x);
			ptr[1] = float(pts[i].y);
			ptr[2] = float(pts[i].z);
			ptr[3] = LAKE_COLOR.r;
			ptr[4] = LAKE_COLOR.g;
			ptr[5] = LAKE_COLOR.b;
			ptr[6] = LAKE_COLOR.a;
			ptr[7] = float(normal.x);
			ptr[8] = float(normal.y);
			ptr[9] = float(normal.z);
			ptr += 10;
		}
	}
}

