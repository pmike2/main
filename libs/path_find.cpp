#include "path_find.h"


uint PathFinder::_next_gmo_id = 1;


GridMovingObjectType::GridMovingObjectType() {

}


GridMovingObjectType::GridMovingObjectType(std::string name, std::map<GRID_VERTEX_TYPE, number> vertex_cost, std::map<GRID_EDGE_TYPE, number> edge_cost) :
	_name(name), _vertex_cost(vertex_cost), _edge_cost(edge_cost)
{

}


GridMovingObjectType::~GridMovingObjectType() {

}


// GridMovingObject ------------------------------------------------------------------------------------
GridMovingObject::GridMovingObject() {

}


GridMovingObject::GridMovingObject(GridMovingObjectType * type, pt_2d pos, pt_2d size, number speed) :
	_type(type), _speed(speed), _selected(false), _idx_path(0), _status(GMO_IDLE)
{
	_aabb = new AABB_2D(pos, size);
}


GridMovingObject::~GridMovingObject() {
	delete _aabb;
}


// PathFinderInput -------------------------------------------------------------------------------------
PathFinderInput::PathFinderInput() {

}


PathFinderInput::PathFinderInput(GridMovingObject * gmo, uint goal) : _gmo(gmo), _goal(goal) {

}


PathFinderInput::~PathFinderInput() {

}


// PathFinder -------------------------------------------------------------------------------------------
PathFinder::PathFinder() {

}


PathFinder::PathFinder(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols, time_point t) : GraphGrid(origin, size, n_ligs, n_cols), _t_last_path_find(t) {
	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		GraphVertex & vertex = get_vertex(_it_v->first);
		vertex._data = new PathFinderVertexData();
		PathFinderVertexData * vertex_data = (PathFinderVertexData *)(vertex._data);
		vertex_data->_id_gmo = 0;
		vertex_data->_type = GRID_VERTEX_LAND;

		_it_e= _it_v->second._edges.begin();
		while (_it_e!= _it_v->second._edges.end()) {
			GraphEdge & edge = get_edge(_it_v->first, _it_e->first);
			edge._data = new PathFinderEdgeData();
			PathFinderEdgeData * edge_data = (PathFinderEdgeData *)(edge._data);
			edge_data->_type = GRID_EDGE_FLAT;
			_it_e++;
		}
		_it_v++;
	}

	_gmo_types.push_back(new GridMovingObjectType("infantery", 
		std::map<GRID_VERTEX_TYPE, number>{{GRID_VERTEX_LAND, 0.0}, {GRID_VERTEX_WATER, 1000.0}, {GRID_VERTEX_LAND_OBSTACLE, 1000.0}},
		std::map<GRID_EDGE_TYPE, number>{{GRID_EDGE_FLAT, 10.0}, {GRID_EDGE_SOFT_DOWN, 5.0}, {GRID_EDGE_SOFT_UP, 20.0}, {GRID_EDGE_HARD_DOWN, 1000.0}, {GRID_EDGE_HARD_UP, 1000.0}}));

	_gmo_types.push_back(new GridMovingObjectType("boat", 
		std::map<GRID_VERTEX_TYPE, number>{{GRID_VERTEX_LAND, 1000.0}, {GRID_VERTEX_WATER, 0.0}, {GRID_VERTEX_LAND_OBSTACLE, 1000.0}},
		std::map<GRID_EDGE_TYPE, number>{{GRID_EDGE_FLAT, 10.0}, {GRID_EDGE_SOFT_DOWN, 100.0}, {GRID_EDGE_SOFT_UP, 100.0}, {GRID_EDGE_HARD_DOWN, 1000.0}, {GRID_EDGE_HARD_UP, 1000.0}}));
}


PathFinder::~PathFinder() {
	_inputs = std::queue<PathFinderInput *>();
	for (auto & gmo : _gmos) {
		delete gmo;
	}
	_gmos.clear();
	for (auto & t : _gmo_types) {
		delete t;
	}
	_gmo_types.clear();
}


PathFinderVertexData * PathFinder::get_vertex_data(uint id_vertex) {
	GraphVertex & vertex = get_vertex(id_vertex);
	return (PathFinderVertexData *)(vertex._data);
}


PathFinderEdgeData * PathFinder::get_edge_data(uint from , uint to) {
	GraphEdge & edge = get_edge(from, to);
	return (PathFinderEdgeData *)(edge._data);
}


number PathFinder::cost(GridMovingObject * gmo, uint from, uint to) {
	PathFinderEdgeData * edge_data = get_edge_data(from, to);
	number result = gmo->_type->_edge_cost[edge_data->_type];

	std::pair<int_pair, int_pair> clmm = col_lig_min_max(to, gmo->_n_grid_size);
	//std::cout << clmm.first.first << " -> " << clmm.second.first << " ; " << clmm.first.second << " ->" << clmm.second.second << "\n";
	bool obstacle = false;
	for (int col=clmm.first.first; col<=clmm.second.first; ++col) {
		for (int lig=clmm.first.second; lig<=clmm.second.second; ++lig) {
			PathFinderVertexData * vertex_data = get_vertex_data(col_lig2id(col, lig));
			if ((vertex_data->_id_gmo > 0 && vertex_data->_id_gmo != gmo->_id) || (gmo->_type->_vertex_cost[vertex_data->_type] >= VERTEX_OBSTACLE_THRESH)) {
				obstacle = true;
				break;
			}
		}
		if (obstacle) {
			break;
		}
	}

	if (obstacle) {
		result += VERTEX_OBSTACLE_THRESH;
	}
	
	/*PathFinderVertexData * vfrom_data = get_vertex_data(from);
	PathFinderVertexData * vto_data = get_vertex_data(to);
	
	if (
		(vfrom_data->_id_gmo > 0 && vfrom_data->_id_gmo != id_gmo) ||
		(vto_data->_id_gmo > 0 && vto_data->_id_gmo != id_gmo) ||
		(vfrom_data->_obstacle == VERTEX_OBSTACLE) ||
		(vto_data->_obstacle == VERTEX_OBSTACLE)
	) {
		result += OCCUPIED_EDGE;
	}*/

	return result;
}


number PathFinder::heuristic(uint i, uint j) {
	return glm::distance(_vertices[i]._pos, _vertices[j]._pos);
}


void PathFinder::path_find(PathFinderInput * input) {
	bool verbose = true;

	auto frontier_cmp = [](std::pair<uint, number> x, std::pair<uint, number> y) { return x.second > y.second; };
	std::priority_queue< std::pair<uint, number>, std::vector<std::pair<uint, number> >, decltype(frontier_cmp) > frontier(frontier_cmp);
	std::unordered_map<uint, uint> came_from;
	std::unordered_map<uint, number> cost_so_far;

	GridMovingObject * gmo = input->_gmo;
	uint goal = input->_goal;
	uint start = pt2closest_id(gmo->_aabb->center());

	if (verbose) {
		std::cout << "path_find : id = " << gmo->_id << " ; start = " << start << " ; goal = " << goal << "\n";
	}

	frontier.emplace(start, 0.0);
	came_from[start] = start;
	cost_so_far[start] = 0.0;

	while (!frontier.empty()) {
		uint current = frontier.top().first;
		frontier.pop();

		if (current == goal) {
			break;
		}

		std::vector<uint> nexts = neighbors(current);

		for (auto & next : nexts) {
			PathFinderVertexData * vertex_data = get_vertex_data(next);
			/*if ((vertex_data->_id_gmo > 0 && vertex_data->_id_gmo != id_gmo) || vertex_data->_obstacle == VERTEX_OBSTACLE) {
				continue;
			}*/

			number new_cost = cost_so_far[current] + cost(gmo, current, next);
			if ((!cost_so_far.count(next)) || (new_cost< cost_so_far[next])) {
				cost_so_far[next] = new_cost;
				came_from[next] = current;
				//number priority = new_cost; // dijkstra
				//number priority = heuristic(next, goal); // greedy best first search
				number priority = new_cost + heuristic(next, goal); // A *
				frontier.emplace(next, priority);
			}
		}
	}

	/*if (!came_from.count(goal)) {
		return false;
	}*/

	std::vector<uint> path_tmp;

	uint current = goal;
	while (current != start) {
		path_tmp.push_back(current);
		current = came_from[current];
	}
	path_tmp.push_back(start);
	std::reverse(path_tmp.begin(), path_tmp.end());

	gmo->_path.clear();
	for (auto & i : path_tmp) {
		if (cost_so_far[i] >= VERTEX_OBSTACLE_THRESH) {
			break;
		}
		gmo->_path.push_back(i);
	}

	/*for (auto i : path) {
		std::cout << i << " ; " << cost_so_far[i] << "\n";
	}*/

	if (verbose) {
		std::cout << "path_find end ; path size = " << gmo->_path.size() << "\n";
	}
}


void PathFinder::add_gmo(std::string gmo_type_name, pt_2d pt, number size, number speed) {
	GridMovingObjectType * gmo_type = NULL;
	for (auto & gmot : _gmo_types) {
		if (gmot->_name == gmo_type_name) {
			gmo_type = gmot;
			break;
		}
	}
	if (gmo_type == NULL) {
		std::cerr << "PathFinder::add_gmo : " << gmo_type_name << " non reconnu\n";
		return;
	}

	uint id = pt2closest_id(pt);
	pt_2d center = id2pt_2d(id);
	GridMovingObject * gmo = new GridMovingObject(gmo_type, center - 0.5 * pt_2d(size), pt_2d(size), speed);
	gmo->_id = _next_gmo_id++;
	gmo->_n_grid_size = uint(size / _resolution.x);
	_gmos.push_back(gmo);
	update_gmo_grid(gmo);
}


GridMovingObject * PathFinder::get_gmo(uint id) {
	for (auto & gmo : _gmos) {
		if (gmo->_id == id) {
			return gmo;
		}
	}
	std::cerr << "PathFinderTest::get_gmo : id = " << id << " introuvable\n";
	return NULL;
}


void PathFinder::update_gmo_grid(GridMovingObject * gmo) {
	for (auto & v : gmo->_vertices) {
		PathFinderVertexData * vertex_data = get_vertex_data(v);
		if (vertex_data->_id_gmo != gmo->_id) {
			std::cerr << "update_gmo_grid error\n";
		}
		vertex_data->_id_gmo = 0;
	}

	std::vector<uint> vertices = vertices_in_aabb(gmo->_aabb);
	//vertices.erase(std::remove_if(vertices.begin(), vertices.end(), [](uint v){return }), vertices.end());
	gmo->_vertices.clear();
	for (auto & v : vertices) {
		PathFinderVertexData * vertex_data = get_vertex_data(v);
		if (vertex_data->_id_gmo == 0) {
			vertex_data->_id_gmo = gmo->_id;
			gmo->_vertices.push_back(v);
		}
	}

	//gmo->_vertices = vertices;
}


void PathFinder::delete_selected_gmos() {
	_gmos.erase(std::remove_if(_gmos.begin(), _gmos.end(), [](GridMovingObject * gmo) { return gmo->_selected; }), _gmos.end());
}


void PathFinder::goto_gmo(GridMovingObject * gmo, uint id_vertex) {
	
	/*uint start = _pf->pt2closest_id(gmo->_aabb->center());
	_pf->path_find(gmo->_id, gmo->_n_grid_size, start, id_vertex, gmo->_path);
	gmo->_idx_path = 0;
	gmo->_status = GMO_MOVING;*/

	_inputs.push(new PathFinderInput(gmo, id_vertex));

	/*std::cout << "goto : id = " << gmo->_id;
	std::cout << " ; start = " << start << " ; goal = " << goal;
	std::cout << " ; path = ";
	for (auto & id : gmo->_path) {
		std::cout << id << " ; ";
	}
	std::cout << "\n";*/
}


void PathFinder::goto_gmo(GridMovingObject * gmo, pt_2d target) {
	uint id_vertex = pt2closest_id(target);
	goto_gmo(gmo, id_vertex);
}


void PathFinder::goto_selected_gmos(pt_2d target) {
	uint goal = pt2closest_id(target);

	for (auto & gmo : _gmos) {
		if (gmo->_selected) {
			goto_gmo(gmo, target);
		}
	}
}


void PathFinder::stop_gmo(GridMovingObject * gmo) {
	gmo->_idx_path = 0;
	gmo->_path.clear();
	gmo->_status = GMO_IDLE;
}


void PathFinder::clear_edges() {
	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		_it_e= _it_v->second._edges.begin();
		while (_it_e!= _it_v->second._edges.end()) {
			PathFinderEdgeData * edge_data = get_edge_data(_it_v->first, _it_e->first);
			edge_data->_type = GRID_EDGE_FLAT;
			_it_e++;
		}
		_it_v++;
	}
}


void PathFinder::randomize_edges() {
	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		_it_e= _it_v->second._edges.begin();
		while (_it_e!= _it_v->second._edges.end()) {
			PathFinderEdgeData * edge_data = get_edge_data(_it_v->first, _it_e->first);
			int n = rand_int(0, 4);
			if (n == 0) {
				edge_data->_type = GRID_EDGE_FLAT;
			}
			else if (n == 1) {
				edge_data->_type = GRID_EDGE_SOFT_DOWN;
			}
			else if (n == 2) {
				edge_data->_type = GRID_EDGE_SOFT_UP;
			}
			else if (n == 3) {
				edge_data->_type = GRID_EDGE_HARD_DOWN;
			}
			else if (n == 4) {
				edge_data->_type = GRID_EDGE_HARD_UP;
			}
			_it_e++;
		}
		_it_v++;
	}
}


void PathFinder::set_vertex(pt_2d center, number size, GRID_VERTEX_TYPE type) {
	AABB_2D * aabb = new AABB_2D(center - 0.5 * pt_2d(size), pt_2d(size));
	std::vector<uint> vertices = vertices_in_aabb(aabb);
	delete aabb;
	for (auto & v : vertices) {
		PathFinderVertexData * vertex_data = get_vertex_data(v);
		vertex_data->_type = type;
	}
}


void PathFinder::set_edges(pt_2d center, number size, GRID_EDGE_TYPE type) {
	AABB_2D * aabb = new AABB_2D(center - 0.5 * pt_2d(size), pt_2d(size));
	std::vector<uint_pair> edges = edges_intersecting_aabb(aabb);
	delete aabb;
	for (auto & e : edges) {
		PathFinderEdgeData * edge_data = get_edge_data(e.first, e.second);
		edge_data->_type = type;
	}
}


void PathFinder::parse_input_queue(time_point t) {
	if (_inputs.empty()) {
		return;
	}
	
	auto d_last_path_find = std::chrono::duration_cast<std::chrono::milliseconds>(t - _t_last_path_find).count();
	if (d_last_path_find < PATH_FIND_N_MS) {
		return;
	}
	
	_t_last_path_find = t;
	PathFinderInput * input = _inputs.front();
	_inputs.pop();
	
	path_find(input);
	input->_gmo->_idx_path = 0;
	input->_gmo->_status = GMO_MOVING;
}


void PathFinder::anim(time_point t) {
	parse_input_queue(t);

	bool verbose = true;

	for (auto & gmo : _gmos) {
		if (gmo->_status == GMO_IDLE) {
			continue;
		}
		
		else if (gmo->_status == GMO_MOVING) {
			for (uint i=gmo->_idx_path + 1; i<std::min(uint(gmo->_path.size()), gmo->_idx_path + 1 + N_VERTICES_CHECK); ++i) {
				PathFinderVertexData * vertex_data = get_vertex_data(gmo->_path[i]);
				
				if (vertex_data->_id_gmo > 0 && vertex_data->_id_gmo != gmo->_id) {
					GridMovingObject * gmo2 = get_gmo(vertex_data->_id_gmo);
					if (gmo2->_status == GMO_IDLE) {
						PathFinderVertexData * goal_data = get_vertex_data(gmo->_path[gmo->_path.size() - 1]);
						if (goal_data->_id_gmo == gmo2->_id) {
							stop_gmo(gmo);
							if (verbose) {
								std::cout << gmo->_id << " stop a cause de " << gmo2->_id << "\n";
							}
						}
						else {
							goto_gmo(gmo, gmo->_path[gmo->_path.size() - 1]);
							if (verbose) {
								std::cout << gmo->_id << " contourne " << gmo2->_id << "\n";
							}
						}
					}
					else {
						gmo->_status = GMO_WAITING;
						gmo->_t_wait = t;
						if (verbose) {
							std::cout << gmo->_id << " attend a cause de " << gmo2->_id << "\n";
						}
					}
					break;
				}
				
				else if (gmo->_type->_vertex_cost[vertex_data->_type] >= VERTEX_OBSTACLE_THRESH) {
					PathFinderVertexData * goal_data = get_vertex_data(gmo->_path[gmo->_path.size() - 1]);
					if (gmo->_type->_vertex_cost[goal_data->_type] >= VERTEX_OBSTACLE_THRESH) {
						stop_gmo(gmo);
						if (verbose) {
							std::cout << gmo->_id << " stop a cause d'un obstacle sur la destination\n";
						}
					}
					else {
						goto_gmo(gmo, gmo->_path[gmo->_path.size() - 1]);
						if (verbose) {
							std::cout << gmo->_id << " recalcule a cause d'un nouvel obstacle\n";
						}
					}
					break;
				}
			}
		}

		else if (gmo->_status == GMO_WAITING) {
			auto d_wait = std::chrono::duration_cast<std::chrono::milliseconds>(t- gmo->_t_wait).count();
			if (d_wait > WAIT_N_MS) {
				gmo->_status = GMO_MOVING;
				if (verbose) {
					std::cout << gmo->_id << " repart\n";
				}
			}
		}
	}

	for (auto & gmo : _gmos) {
		if (gmo->_status == GMO_MOVING) {
			pt_2d next = id2pt_2d(gmo->_path[gmo->_idx_path + 1]);

			if (glm::distance(gmo->_aabb->center(), next) <= 0.5 * gmo->_speed) {
				gmo->_idx_path++;
				//gmo->_aabb->set_center(next);
				if (gmo->_idx_path == gmo->_path.size() - 1) {
					stop_gmo(gmo);
					if (verbose) {
						std::cout << gmo->_id << " a atteint sa destination\n";
					}
					continue;
				}
				else {
					next = id2pt_2d(gmo->_path[gmo->_idx_path + 1]);
				}
			}

			//pt_2d last = id2pt_2d(gmo->_path[gmo->_idx_path]);
			//pt_2d direction = glm::normalize(next - last);
			pt_2d direction = glm::normalize(next - gmo->_aabb->center());
			gmo->_aabb->translate(gmo->_speed * direction);

			update_gmo_grid(gmo);
		}
	}
}

