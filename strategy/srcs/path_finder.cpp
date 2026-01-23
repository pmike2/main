#include "path_finder.h"


PathFinder::PathFinder() {
	
}

PathFinder::PathFinder(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols) : 
	GraphGrid(origin, size, n_ligs, n_cols), _verbose(true), _computing(false)
{
	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		_it_e= _it_v->second._edges.begin();
		while (_it_e!= _it_v->second._edges.end()) {
			GraphEdge & edge = _vertices[_it_v->first]._edges[_it_e->first];
			edge._data = new EdgeData();
			_it_e++;
		}
		_it_v++;
	}

	_path = new UnitPath();
}


PathFinder::~PathFinder() {
	// TODO : détruire les edge._data ici
	/*delete _elevation_grid;
	delete _units_position_grid;
	delete _terrain_grid;*/

	delete _path;
}


void PathFinder::add_unit_type(UnitType * unit_type) {
	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		_it_e= _it_v->second._edges.begin();
		while (_it_e!= _it_v->second._edges.end()) {
			GraphEdge & edge = _vertices[_it_v->first]._edges[_it_e->first];
			EdgeData * edge_data = (EdgeData *)(edge._data);
			edge_data->_delta_elevation[unit_type] = 0.0;
			edge_data->_type[unit_type] = TERRAIN_UNKNOWN;
			_it_e++;
		}
		_it_v++;
	}
}


number PathFinder::elevation_weight(UnitType * unit_type, uint i, uint j) {
	GraphEdge edge = _vertices[i]._edges[j];
	EdgeData * data = (EdgeData *)(edge._data);
	return data->_delta_elevation[unit_type];
}


number PathFinder::units_position_weight(UnitType * unit_type, uint unit_id, uint i, uint j) {
	GraphEdge edge = _vertices[i]._edges[j];
	EdgeData * data = (EdgeData *)(edge._data);
	if (data->_ids[unit_type].empty() || (data->_ids[unit_type].size() == 1 && data->_ids[unit_type][0] == unit_id)) {
		return DEFAULT_EDGE_WEIGHT;
	}
	return MAX_UNIT_MOVING_WEIGHT;
}


number PathFinder::terrain_weight(UnitType * unit_type, uint i, uint j) {
	GraphEdge edge = _vertices[i]._edges[j];
	EdgeData * data = (EdgeData *)(edge._data);
	if (data->_type[unit_type] == TERRAIN_UNKNOWN) {
		std::cerr << "PathFinder::terrain_weight UNKNOWN\n";
		return 0.0;
	}
	return unit_type->_terrain_weights[data->_type[unit_type]];
}


number PathFinder::cost(UnitType * unit_type, uint unit_id, uint i, uint j) {
	number result = 0.0;
	if (!unit_type->_floats) {
		result += elevation_weight(unit_type, i, j);
	}
	result += units_position_weight(unit_type, unit_id, i, j);
	result += terrain_weight(unit_type, i, j);
	return result;
}


number PathFinder::heuristic(uint i, uint j) {
	return glm::distance(_vertices[i]._pos, _vertices[j]._pos);
}


number PathFinder::line_of_sight_max_weight(UnitType * unit_type, uint unit_id, pt_2d pt1, pt_2d pt2) {
	//std::vector<uint_pair> edges = edges_intersecting_segment(pt_2d(pt1), pt_2d(pt2));

	const number LOS_SEGMENT_WIDTH = 0.5;
	BBox_2D * bbox = new BBox_2D(LOS_SEGMENT_WIDTH, pt1, pt2);
	std::vector<uint_pair> edges = edges_intersecting_bbox(bbox);
	delete bbox;

	number result = 0.0;
	for (auto edge : edges) {
		pt_2d v1 = pt_2d(_vertices[edge.first]._pos);
		pt_2d v2 = pt_2d(_vertices[edge.second]._pos);
		number weight = cost(unit_type, unit_id, edge.first, edge.second);
		if (glm::dot(v2 - v1, pt2 - pt1) >= 0.0 && weight > result) {
			result = weight;
		}
	}
	return result;
}


bool PathFinder::path_find_nodes(UnitType * unit_type, uint unit_id, uint start, uint goal) {
	auto frontier_cmp = [](std::pair<uint, number> x, std::pair<uint, number> y) { return x.second > y.second; };
	std::priority_queue< std::pair<uint, number>, std::vector<std::pair<uint, number> >, decltype(frontier_cmp) > frontier(frontier_cmp);
	std::unordered_map<uint, uint> came_from;
	std::unordered_map<uint, number> cost_so_far;

	frontier.emplace(start, 0.0);
	came_from[start]= start;
	cost_so_far[start]= 0.0;

	while (!frontier.empty()) {
		uint current= frontier.top().first;
		frontier.pop();
		
		if (current== goal) {
			break;
		}

		std::vector<uint> nexts= neighbors(current);
		for (auto & next : nexts) {
			number new_cost= cost_so_far[current]+ cost(unit_type, unit_id, current, next);
			if ((!cost_so_far.count(next)) || (new_cost< cost_so_far[next])) {
				cost_so_far[next]= new_cost;
				came_from[next]= current;
				//number priority= new_cost; // dijkstra
				//number priority= heuristic(next, goal, grid); // greedy best first search
				number priority= new_cost + heuristic(next, goal); // A *
				frontier.emplace(next, priority);
			}
		}
	}

	if (!came_from.count(goal)) {
		return false;
	}

	uint current = goal;
	while (current != start) {
		_path->_nodes.push_back(current);
		current = came_from[current];
	}
	_path->_nodes.push_back(start);
	std::reverse(_path->_nodes.begin(), _path->_nodes.end());

	return true;
}


bool PathFinder::path_find(UnitType * unit_type, uint unit_id, pt_3d start, pt_3d goal, UNIT_STATUS & unit_status) {

	// init ----------------------------------------------------------------------------
	if (_verbose) {
		std::cout << "unit id " << unit_id << " ---- DEBUT path_find --------------------------------------\n";
	}
	_path->clear();
	_path->_start = start;
	_path->_goal = goal;

	if ((!point_in_aabb2d(_path->_start, _aabb)) || (!point_in_aabb2d(_path->_goal, _aabb))) {
		if (_verbose) {
			std::cout << "unit id " << unit_id << " : PathFinder::path_find : point hors grille.\n";
		}
		_mtx.lock();
		unit_status = COMPUTING_PATH_FAILED;
		_mtx.unlock();
		return false;
	}

	// remplissage unit->_nodes ------------------------------------------------------------
	uint start_id = pt2closest_id(_path->_start);
	uint goal_id = pt2closest_id(_path->_goal);

	bool is_path_ok= path_find_nodes(unit_type, unit_id, start_id, goal_id);
	if (!is_path_ok) {
		if (_verbose) {
			std::cout << "unit id " << unit_id << " : PathFinder::path_find : pas de chemin trouvé.\n";
		}
		_mtx.lock();
		unit_status = COMPUTING_PATH_FAILED;
		_mtx.unlock();
		return false;
	}

	if (_verbose) {
		std::cout << "unit id " << unit_id << " : nodes size = " << _path->_nodes.size() << "\n";
	}

	// raw_path à partir de nodes ---------------------------------------------------------
	std::vector<pt_2d> raw_path;
	raw_path.push_back(start);
	for (uint i=0; i<_path->_nodes.size(); ++i) {
		raw_path.push_back(_vertices[_path->_nodes[i]]._pos);
	}
	raw_path.push_back(goal);

	if (_verbose) {
		std::cout << "unit id " << unit_id << " : raw_path size avant suppressions = " << raw_path.size() << "\n";
	}

	// poids de raw_path -------------------------------------------------------------------
	// TODO : on recalcule les poids avec cost() mais c'était déjà fait dans path_find_nodes ...
	std::vector<number> weights;
	
	// poids de start au 1er node
	std::vector<uint_pair> start_edges = edges_in_cell_containing_pt(_path->_start, true);
	number weight_start = 1e9;
	for (auto & edge : start_edges) {
		number w = cost(unit_type, unit_id, edge.first, edge.second);
		if (w < weight_start) {
			weight_start = w;
		}
	}
	weights.push_back(weight_start);

	// poids de path
	number n_nodes = _path->_nodes.size();
	for (uint i=0; i < n_nodes - 1; ++i) {
		weights.push_back(cost(unit_type, unit_id, _path->_nodes[i], _path->_nodes[i + 1]));
	}

	// poids du dernier node à goal
	std::vector<uint_pair> goal_edges = edges_in_cell_containing_pt(_path->_goal, true);
	number weight_goal = 1e9;
	for (auto & edge : goal_edges) {
		number w = cost(unit_type, unit_id, edge.first, edge.second);
		if (w < weight_goal) {
			weight_goal = w;
		}
	}
	weights.push_back(weight_goal);

	if (_verbose) {
		std::cout << "unit id " << unit_id << " : weights size = " << weights.size() << " ; weights = ";
		for (auto & w : weights) {
			std::cout << w << " ; ";
		}
		std::cout << "\n";
	}

	// suppression des portions de raw_path où poids trop grand --------------------------------------
	for (uint i=0; i<weights.size(); ++i) {
		if (weights[i] >= MAX_UNIT_MOVING_WEIGHT) {
			if (i == 0) {
				raw_path.clear();
				break;
			}
			else {
				raw_path.erase(raw_path.begin() + i, raw_path.end());
				break;
			}
		}
	}

	if (_verbose) {
		std::cout << "unit id " << unit_id << " raw_path size après suppressions =" << raw_path.size() << "\n";
	}

	if (raw_path.size() == 0) {
		if (_verbose) {
			std::cout << "unit id " << unit_id << " raw_path.size() == 0\n";
		}
		_mtx.lock();
		unit_status = COMPUTING_PATH_FAILED;
		_mtx.unlock();
		return false;
	}

	// sans line of sight ----------------------------------------------------------------------------
	for (auto & pt : raw_path) {
		_path->_pts.push_back(pt_3d(pt.x, pt.y, 0.0));
	}
	for (auto & w : weights) {
		_path->_weights.push_back(w);
	}

	if (_verbose) {
		std::cout << "unit id " << unit_id << " _path->_pts.size = " << _path->_pts.size() << "\n";
		std::cout << "unit id " << unit_id << " _path->_weights.size = " << _path->_weights.size() << "\n";
	}

	// avec line of sight ----------------------------------------------------------------------------
	if (_verbose) {
		std::cout << "unit id " << unit_id << " début line of sight\n";
	}

	_path->_pts_los.push_back(pt_3d(raw_path[0].x, raw_path[0].y, 0.0));
	
	uint idx = 1;
	int last = 0;
	
	while (idx < raw_path.size()) {
		number raw_max_weight = weights[last];
		number last_los_weight_ok = weights[last];

		if (_verbose) {
			std::cout << "BEGIN while\n";
			std::cout << "last = " << last << " ; idx = " << idx;
			std::cout << " ; raw_max_weight=" << raw_max_weight;
			std::cout << "\n";
		}

		while (true) {
			idx++;
			
			if (_verbose) {
				std::cout << "idx = " << idx << "\n";
			}

			if (idx >= raw_path.size()) {
				break;
			}
			number los_weight = line_of_sight_max_weight(unit_type, unit_id, raw_path[last], raw_path[idx]);

			if (_verbose) {
				std::cout << "\tlos_weight = " << los_weight << " ; raw_max_weight = " << raw_max_weight << "\n";
			}

			if (los_weight> raw_max_weight) {
				break;
			}
			raw_max_weight = std::max(raw_max_weight, weights[idx]);
			last_los_weight_ok = los_weight;

			if (_verbose) {
				std::cout << "\tlast_los_weight_ok = " << last_los_weight_ok << " ; raw_max_weight = " << raw_max_weight << "\n";
			}
		}

		last = idx - 1;
		
		_path->_pts_los.push_back(pt_3d(raw_path[last].x, raw_path[last].y, 0.0));
		_path->_weights_los.push_back(last_los_weight_ok);

		if (_verbose) {
			std::cout << "END while\n";
			std::cout << "last = " << last << " ; idx = " << idx;
			std::cout << " ; raw_max_weight=" << raw_max_weight;
			std::cout << "\n\n";
		}
	}

	if (_verbose) {
		std::cout << "unit id " << unit_id << " _path->_pts_los.size = " << _path->_pts_los.size() << "\n";
		std::cout << "unit id " << unit_id << " _path->_weights_los.size = " << _path->_weights_los.size() << "\n";
	}

	// construction des path->_bboxs -------------------------------------------------------------
	number r = unit_type->_obj_data->_aabb->_base_radius;
	
	std::vector<pt_4d> segments;
	
	for (int i=0; i<_path->_pts.size() - 1; ++i) {
		pt_2d p1 = pt_2d(_path->_pts[i]);
		pt_2d p2 = pt_2d(_path->_pts[i + 1]);
		segments.push_back(pt_4d(p1.x, p1.y, p2.x, p2.y));
	}

	for (auto & segment : segments) {
		pt_2d p1(segment.x, segment.y);
		pt_2d p2(segment.z, segment.w);
		if (norm2(p1 - p2) < 1e-8) {
			continue;
		}

		pt_2d v = glm::normalize(p2 - p1);
		BBox_2D * bbox = new BBox_2D(2.0 * r, p1 - r * v, p2 + r * v);
		_path->_bboxs.push_back(bbox);
	}

	// fin -----------------------------------------------------------------------------------
	if (_verbose) {
		std::cout << "unit id " << unit_id << " ---- FIN path_find --------------------------------------\n";
	}

	_path->_idx_path = 1;

	_mtx.lock();
	unit_status = COMPUTING_PATH_DONE;
	_mtx.unlock();

	return true;
}


// TODO : a revoir
/*void PathFinder::draw_svg(GraphGrid * grid, Path * path, std::string svg_path) {
	std::ofstream f;
	f.open(svg_path);
	f << "<!DOCTYPE html>\n<html>\n<body>\n";
	f << "<svg width=\"1000\" height=\"1000\" viewbox=\"" << grid->_origin.x << " " << grid->_origin.y << " " << grid->_size.x << " " << grid->_size.y << "\">\n";

	if (path->_pts.size()) {
		for (uint i=0; i<path->_pts.size()- 1; ++i) {
			number x1= grid->_vertices[path->_pts[i]]._pos.x;
			number y1= grid->_vertices[path[i]]._pos.y;
			number x2= _grid->_vertices[path[i+ 1]]._pos.x;
			number y2= _grid->_vertices[path[i+ 1]]._pos.y;
			f << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" stroke=\"red\" stroke-width=\"0.06\" />\n";
		}

		number radius = 0.1;
		std::string color_start = "green";
		number x_start= _grid->_vertices[path[0]]._pos.x;
		number y_start= _grid->_vertices[path[0]]._pos.y;
		f << "<circle cx=\"" << x_start << "\" cy=\"" << y_start << "\" r=\"" << radius << "\" fill=\"" << color_start << "\" />\n";
		std::string color_end = "orange";
		number x_end= _grid->_vertices[path[path.size() - 1]]._pos.x;
		number y_end= _grid->_vertices[path[path.size() - 1]]._pos.y;
		f << "<circle cx=\"" << x_end << "\" cy=\"" << y_end << "\" r=\"" << radius << "\" fill=\"" << color_end << "\" />\n";
	}

	for (auto poly : _polygons) {
		f << "<polygon points=\"";
		for (auto pt : poly->_pts) {
			f << pt.x << "," << pt.y << " ";
		}
		f << "\" fill=\"none\" stroke=\"purple\" stroke-width=\"0.02\" />\n";
	}

	_grid->_it_v= _grid->_vertices.begin();
	while (_grid->_it_v!= _grid->_vertices.end()) {
		number x1= _grid->_it_v->second._pos.x;
		number y1= _grid->_it_v->second._pos.y;
		std::string color= "black";
		number radius= 0.01* _grid->_it_v->second._weight;
		f << "<circle cx=\"" << x1 << "\" cy=\"" << y1 << "\" r=\"" << radius << "\" fill=\"" << color << "\" />\n";
		//f << "<text x=\"" << x1+ 0.15f << "\" y=\"" << y1- 0.15f << "\" fill=\"black\" font-size=\"0.2px\">" << to_string(_grid->_it_v->first) << "</text>\n";

		_grid->_it_e= _grid->_it_v->second._edges.begin();
		while (_grid->_it_e!= _grid->_it_v->second._edges.end()) {
			number x2= _grid->_vertices[_grid->_it_e->first]._pos.x;
			number y2= _grid->_vertices[_grid->_it_e->first]._pos.y;
			f << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" stroke=\"black\" stroke-width=\"0.01\" />\n";
			_grid->_it_e++;
		}

		_grid->_it_v++;
	}
	
	f << "</svg>\n</body>\n</html>\n";
	f.close();
}*/

