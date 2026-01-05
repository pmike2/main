#include "path_finder.h"


PathFinder::PathFinder() {
	
}

PathFinder::PathFinder(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols) : _use_line_of_sight(true), _verbose(false) {
	_elevation_grid = new GraphGrid(origin, size, n_ligs, n_cols);
	_units_position_grid = new GraphGrid(origin, size, n_ligs, n_cols);
	_terrain_grid = new GraphGrid(origin, size, n_ligs, n_cols);

	_elevation_grid->_it_v= _elevation_grid->_vertices.begin();
	while (_elevation_grid->_it_v!= _elevation_grid->_vertices.end()) {
		_elevation_grid->_it_e= _elevation_grid->_it_v->second._edges.begin();
		while (_elevation_grid->_it_e!= _elevation_grid->_it_v->second._edges.end()) {
			GraphEdge & edge = _elevation_grid->_vertices[_elevation_grid->_it_v->first]._edges[_elevation_grid->_it_e->first];
			edge._data = new ElevationEdgeData();
			_elevation_grid->_it_e++;
		}
		_elevation_grid->_it_v++;
	}

	_units_position_grid->_it_v= _units_position_grid->_vertices.begin();
	while (_units_position_grid->_it_v!= _units_position_grid->_vertices.end()) {
		_units_position_grid->_it_e= _units_position_grid->_it_v->second._edges.begin();
		while (_units_position_grid->_it_e!= _units_position_grid->_it_v->second._edges.end()) {
			GraphEdge & edge = _units_position_grid->_vertices[_units_position_grid->_it_v->first]._edges[_units_position_grid->_it_e->first];
			edge._data = new UnitsPositionEdgeData();
			_units_position_grid->_it_e++;
		}
		_units_position_grid->_it_v++;
	}

	_terrain_grid->_it_v= _terrain_grid->_vertices.begin();
	while (_terrain_grid->_it_v!= _terrain_grid->_vertices.end()) {
		_terrain_grid->_it_e= _terrain_grid->_it_v->second._edges.begin();
		while (_terrain_grid->_it_e!= _terrain_grid->_it_v->second._edges.end()) {
			GraphEdge & edge = _terrain_grid->_vertices[_terrain_grid->_it_v->first]._edges[_terrain_grid->_it_e->first];
			edge._data = new TerrainEdgeData();
			_terrain_grid->_it_e++;
		}
		_terrain_grid->_it_v++;
	}
}


PathFinder::~PathFinder() {
	// TODO : détruire les edge._data ici
	delete _elevation_grid;
	delete _units_position_grid;
	delete _terrain_grid;
}


number PathFinder::elevation_weight(uint i, uint j) {
	GraphEdge edge = _elevation_grid->_vertices[i]._edges[j];
	ElevationEdgeData * data = (ElevationEdgeData *)(edge._data);
	return data->_delta_elevation;
}


number PathFinder::units_position_weight(Unit * unit, uint i, uint j) {
	GraphEdge edge = _units_position_grid->_vertices[i]._edges[j];
	UnitsPositionEdgeData * data = (UnitsPositionEdgeData *)(edge._data);
	if (data->_ids.empty() || (data->_ids.size() == 1 && data->_ids[0] == unit->_id)) {
		return DEFAULT_EDGE_WEIGHT;
	}
	return MAX_UNIT_MOVING_WEIGHT;
}


number PathFinder::terrain_weight(Unit * unit, uint i, uint j) {
	GraphEdge edge = _terrain_grid->_vertices[i]._edges[j];
	TerrainEdgeData * data = (TerrainEdgeData *)(edge._data);
	if (data->_type == UNKNOWN) {
		std::cerr << "PathFinder::terrain_weight UNKNOWN\n";
		return 0.0;
	}
	return unit->_type->_terrain_weights[data->_type];
}


number PathFinder::cost(Unit * unit, uint i, uint j) {
	number result = 0.0;
	result += elevation_weight(i, j);
	result += units_position_weight(unit, i, j);
	result += terrain_weight(unit, i, j);
	return result;
}


number PathFinder::heuristic(uint i, uint j) {
	return glm::distance(_elevation_grid->_vertices[i]._pos, _elevation_grid->_vertices[j]._pos);
}


number PathFinder::line_of_sight_max_weight(Unit * unit, pt_2d pt1, pt_2d pt2) {
	std::vector<std::pair<uint, uint> > edges = _elevation_grid->segment_intersection(pt_2d(pt1), pt_2d(pt2));
	number result = 0.0;
	for (auto edge : edges) {
		pt_2d v1 = pt_2d(_elevation_grid->_vertices[edge.first]._pos);
		pt_2d v2 = pt_2d(_elevation_grid->_vertices[edge.second]._pos);
		number weight = cost(unit, edge.first, edge.second);
		if (glm::dot(v2 - v1, pt_2d(pt2 - pt1)) >= 0.0 && weight > result) {
			result = weight;
		}
	}
	return result;
}


bool PathFinder::path_find_nodes(Unit * unit, uint start, uint goal) {
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

		std::vector<uint> nexts= _elevation_grid->neighbors(current);
		for (auto & next : nexts) {
			number new_cost= cost_so_far[current]+ cost(unit, current, next);
			if ((!cost_so_far.count(next)) || (new_cost< cost_so_far[next])) {
				cost_so_far[next]= new_cost;
				came_from[next]= current;
				//number priority= new_cost; // dijkstra
				//number priority= heuristic(next, goal, grid); // greedy best first search
				number priority= new_cost + heuristic(next, goal); // A *
				//std::cout << priority << "\n";
				frontier.emplace(next, priority);
			}
		}
	}

	if (!came_from.count(goal)) {
		//cout << "disconnected\n";
		return false;
	}

	//unit->_path->_nodes.clear();
	uint current = goal;
	while (current != start) {
		unit->_path->_nodes.push_back(current);
		current = came_from[current];
	}
	unit->_path->_nodes.push_back(start);
	std::reverse(unit->_path->_nodes.begin(), unit->_path->_nodes.end());

	return true;
}


bool PathFinder::path_find(Unit * unit, pt_2d goal) {
	std::mutex mtx;

	unit->_path->clear();
	unit->_path->_start = pt_2d(unit->_aabb->bottom_center());
	unit->_path->_goal = goal;

	if ((!point_in_aabb2d(unit->_path->_start, _elevation_grid->_aabb)) || (!point_in_aabb2d(unit->_path->_goal, _elevation_grid->_aabb))) {
		if (_verbose) {
			std::cerr << "unit id " << unit->_id << " : PathFinder::path_find : point hors grille\n";
		}
		mtx.lock();
		unit->_status = COMPUTING_PATH_FAILED;
		mtx.unlock();
		return false;
	}

	/*std::vector<std::pair<uint, uint> > start_edges = units_position_grid->edges_in_cell_containing_pt(start, true);
	std::vector<uint> start_vertices = units_position_grid->vertices_in_cell_containing_pt(start);
	int start_id = -1;
	for (auto & id_vertex : start_vertices) {
		bool vertex_ok = true;
		GraphVertex vertex = units_position_grid->_vertices[id_vertex];
		units_position_grid->_it_e = vertex._edges.begin();
		while (units_position_grid->_it_e != vertex._edges.end()) {
			GraphEdge edge = units_position_grid->_it_e->second;
			for (auto & id_edge : start_edges) {
				if (
				(id_edge.first == units_position_grid->_it_e->first || id_edge.second == units_position_grid->_it_e->first)
				&& units_position_weight(edge, unit) >= MAX_UNIT_MOVING_WEIGHT
				&& units_position_grid->segment_intersects_edge(start, pt_2d(vertex._pos), id_edge)) {
					vertex_ok = false;
					break;
				}
			}
			if (vertex_ok == false) {
				break;
			}
			units_position_grid->_it_e++;
		}
		if (vertex_ok) {
			start_id = id_vertex;
		}
	}

	if (start_id < 0) {
		std::cerr << "PathFinder::path_find : pas de noeud valide start trouvé\n";
		mtx.lock();
		unit->_status = COMPUTING_PATH_DONE;
		mtx.unlock();
		return false;
	}*/

	//uint start_id = static_grid->pt2id(start);
	uint start_id = _elevation_grid->pt2closest_id(unit->_path->_start);
	//uint goal_id = static_grid->pt2id(goal);
	uint goal_id = _elevation_grid->pt2closest_id(unit->_path->_goal);

	bool is_path_ok= path_find_nodes(unit, start_id, goal_id);
	if (!is_path_ok) {
		if (_verbose) {
			std::cerr << "unit id " << unit->_id << " PathFinder::path_find : pas de chemin trouvé\n";
		}
		mtx.lock();
		unit->_status = COMPUTING_PATH_FAILED;
		mtx.unlock();
		return false;
	}
	//std::cout << unit->_path->_nodes.size() << "\n";

	std::vector<pt_2d> raw_path;
	for (uint i=0; i<unit->_path->_nodes.size(); ++i) {
		raw_path.push_back(_elevation_grid->_vertices[unit->_path->_nodes[i]]._pos);
	}
	raw_path.push_back(goal);
	//std::cout << raw_path.size() << "\n";

	std::vector<number> weights;
	
	// start
	/*number weight_start = 0.0;
	
	std::vector<std::pair<uint, uint> > static_start_edges = _elevation_grid->edges_in_cell_containing_pt(unit->_path->_start);
	number w_static_start = 1e9;
	for (auto & edge : static_start_edges) {
		number w = elevation_weight(edge.first, edge.second);
		if (w < w_static_start) {
			w_static_start = w;
		}
	}
	weight_start += w_static_start;

	std::vector<std::pair<uint, uint> > units_position_start_edges = _units_position_grid->edges_in_cell_containing_pt(unit->_path->_start);
	number w_units_position_start = 1e9;
	for (auto & edge : units_position_start_edges) {
		number w = units_position_weight(unit, edge.first, edge.second);
		if (w < w_units_position_start) {
			w_units_position_start = w;
		}
	}
	weight_start += w_units_position_start;
	weights.push_back(weight_start);*/

	std::vector<std::pair<uint, uint> > start_edges = _elevation_grid->edges_in_cell_containing_pt(unit->_path->_start);
	number weight_start = 1e9;
	for (auto & edge : start_edges) {
		number w = cost(unit, edge.first, edge.second);
		if (w < weight_start) {
			weight_start = w;
		}
	}
	weights.push_back(weight_start);

	// path
	number n_nodes = unit->_path->_nodes.size();
	for (uint i=0; i < n_nodes - 1; ++i) {
		weights.push_back(cost(unit, unit->_path->_nodes[i], unit->_path->_nodes[i + 1]));
	}

	// goal
	/*number weight_goal = 0.0;
	
	std::vector<std::pair<uint, uint> > static_goal_edges = _elevation_grid->edges_in_cell_containing_pt(unit->_path->_goal);
	number w_static_goal = 1e9;
	for (auto & edge : static_goal_edges) {
		number w = elevation_weight(edge.first, edge.second);
		if (w < w_static_goal) {
			w_static_goal = w;
		}
	}
	weight_goal += w_static_goal;

	std::vector<std::pair<uint, uint> > units_position_goal_edges = _units_position_grid->edges_in_cell_containing_pt(unit->_path->_goal);
	number w_units_position_goal = -1e9;
	for (auto & edge : units_position_goal_edges) {
		number w = units_position_weight(unit, edge.first, edge.second);
		if (w > w_units_position_goal) {
			w_units_position_goal = w;
		}
	}
	weight_goal += w_units_position_goal;
	weights.push_back(weight_goal);*/

	std::vector<std::pair<uint, uint> > goal_edges = _elevation_grid->edges_in_cell_containing_pt(unit->_path->_goal);
	number weight_goal = 1e9;
	for (auto & edge : goal_edges) {
		number w = cost(unit, edge.first, edge.second);
		if (w < weight_goal) {
			weight_goal = w;
		}
	}
	weights.push_back(weight_goal);

	/*for (auto & w : weights) {
		std::cout << w << " ; ";
	}
	std::cout << "\n";*/
	//std::cout << "weight_goal = " << weight_goal << "\n";

	for (uint i=0; i<weights.size(); ++i) {
		if (weights[i] >= MAX_UNIT_MOVING_WEIGHT) {
			if (i == 0) {
				raw_path.clear();
				break;
			}
			else {
				raw_path.erase(raw_path.begin() + i - 1, raw_path.end());
				break;
			}
		}
	}
	if (raw_path.size() == 0) {
		if (_verbose) {
			std::cerr << "unit id " << unit->_id << " raw_path.size() == 0\n";
		}
		mtx.lock();
		unit->_status = COMPUTING_PATH_FAILED;
		mtx.unlock();
		return false;
	}

	if (_use_line_of_sight) {
		uint idx = 0;
		uint last = 0;
		//path->_pts.push_back(start);
		//path->_weights.push_back(0.0); // inutilisé mais nécessaire ?
		
		if (_verbose) {
			std::cout << "raw_path.size() = " << raw_path.size() << "\n\n";
		}

		while (idx < raw_path.size()) {
			number raw_max_weight = weights[last];
			number last_los_weight_ok = 0.0;

			if (_verbose) {
				std::cout << "BEGIN while\n";
				std::cout << "last = " << last << " ; idx = " << idx;
				std::cout << " ; raw_max_weight=" << raw_max_weight;
				//std::cout << " ; raw_path[last]=" << glm::to_string(raw_path[last]),
				//std::cout << " ; raw_path[idx]=" << glm::to_string(raw_path[idx]);
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
				number los_weight = line_of_sight_max_weight(unit, raw_path[last], raw_path[idx]);

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
			
			unit->_path->_pts.push_back(pt_3d(raw_path[last].x, raw_path[last].y, 0.0));
			unit->_path->_weights.push_back(last_los_weight_ok);

			if (_verbose) {
				std::cout << "END while\n";
				std::cout << "last = " << last << " ; idx = " << idx;
				//std::cout << " ; raw_path[last]=" << glm::to_string(raw_path[last]) << " ; raw_path[idx]=" << glm::to_string(raw_path[idx]) << "\n";
				std::cout << " ; raw_max_weight=" << raw_max_weight;
				std::cout << "\n\n";
			}
		}
	}
	else {
		for (uint i=0; i<raw_path.size(); ++i) {
			unit->_path->_pts.push_back(pt_3d(raw_path[i].x, raw_path[i].y, 0.0));
			unit->_path->_weights.push_back(weights[i]);
		}
	}

	//number r = unit->_aabb->_base_radius + 0.5 * std::max(unit_type->_size.x, unit_type->_size.y);
	number r = unit->_aabb->_base_radius;
	
	std::vector<pt_4d> segments;
	pt_2d p1 = pt_2d(unit->_aabb->bottom_center());

	pt_2d p2 = pt_2d(unit->_path->_pts[0]);
	segments.push_back(pt_4d(p1.x, p1.y, p2.x, p2.y));
	
	for (int i=0; i<unit->_path->_pts.size() - 1; ++i) {
		pt_2d p1 = pt_2d(unit->_path->_pts[i]);
		pt_2d p2 = pt_2d(unit->_path->_pts[i + 1]);
		segments.push_back(pt_4d(p1.x, p1.y, p2.x, p2.y));
	}

	for (auto & segment : segments) {
		pt_2d p1(segment.x, segment.y);
		pt_2d p2(segment.z, segment.w);
		//std::cout << glm_to_string(p1) << " ; "  << glm_to_string(p2) << "\n";
		if (norm2(p1 - p2) < 1e-8) {
			continue;
		}

		/*pt_2d v = glm::normalize(p2 - p1);
		pt_2d u(v.y, -v.x);
		std::vector<pt_2d> pts = {p1 - r * u - r * v, p1 + r * u - r * v, p2 + r * u + r * v, p2 - r * u + r * v};
		Polygon2D * polygon = new Polygon2D(pts);
		polygon->update_all();
		std::vector<std::pair<uint, uint> > path_edges = grid->polygon_intersection(polygon);
		delete polygon;*/
		
		pt_2d v = glm::normalize(p2 - p1);
		BBox_2D * bbox = new BBox_2D(2.0 * r, p1 - r * v, p2 + r * v);
		//std::cout << *bbox << "\n";
		unit->_path->_bboxs.push_back(bbox);
	}
	//std::cout << *path << "\n";

	mtx.lock();
	unit->_status = COMPUTING_PATH_DONE;
	mtx.unlock();

	return true;
}


// a revoir
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

