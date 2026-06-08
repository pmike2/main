#include "path_finder.h"


uint PathFinderTest::_next_gmo_id = 1;


// GridMovingObject ------------------------------------------------------------------------------------
GridMovingObject::GridMovingObject() {

}


GridMovingObject::GridMovingObject(pt_2d pos, pt_2d size, number speed) :
	_speed(speed), _selected(false), _idx_path(0), _status(GMO_IDLE)
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
		vertex._data = new VertexData();
		VertexData * vertex_data = (VertexData *)(vertex._data);
		vertex_data->_id_gmo = 0;
		vertex_data->_obstacle = VERTEX_NO_OBSTACLE;

		_it_e= _it_v->second._edges.begin();
		while (_it_e!= _it_v->second._edges.end()) {
			GraphEdge & edge = get_edge(_it_v->first, _it_e->first);
			edge._data = new EdgeData();
			EdgeData * edge_data = (EdgeData *)(edge._data);
			edge_data->_cost = SOFT_EDGE;
			_it_e++;
		}
		_it_v++;
	}
}


PathFinder::~PathFinder() {

}


VertexData * PathFinder::get_vertex_data(uint id_vertex) {
	GraphVertex & vertex = get_vertex(id_vertex);
	return (VertexData *)(vertex._data);
}


EdgeData * PathFinder::get_edge_data(uint from , uint to) {
	GraphEdge & edge = get_edge(from, to);
	return (EdgeData *)(edge._data);
}


number PathFinder::cost(uint id_gmo, uint n_grid_size, uint from, uint to) {
	EdgeData * edge_data = get_edge_data(from, to);
	number result = edge_data->_cost;

	std::pair<int_pair, int_pair> clmm = col_lig_min_max(to, n_grid_size);
	//std::cout << clmm.first.first << " -> " << clmm.second.first << " ; " << clmm.first.second << " ->" << clmm.second.second << "\n";
	bool occupied = false;
	for (int col=clmm.first.first; col<=clmm.second.first; ++col) {
		for (int lig=clmm.first.second; lig<=clmm.second.second; ++lig) {
			VertexData * data = get_vertex_data(col_lig2id(col, lig));
			if ((data->_id_gmo > 0 && data->_id_gmo != id_gmo) || (data->_obstacle == VERTEX_OBSTACLE)) {
				occupied = true;
				break;
			}
		}
		if (occupied) {
			break;
		}
	}

	if (occupied) {
		result += OCCUPIED_EDGE;
	}
	
	/*VertexData * vfrom_data = get_vertex_data(from);
	VertexData * vto_data = get_vertex_data(to);
	
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


void PathFinder::path_find(uint id_gmo, uint n_grid_size, uint start, uint goal, std::vector<uint> & path) {
	bool verbose = true;

	auto frontier_cmp = [](std::pair<uint, number> x, std::pair<uint, number> y) { return x.second > y.second; };
	std::priority_queue< std::pair<uint, number>, std::vector<std::pair<uint, number> >, decltype(frontier_cmp) > frontier(frontier_cmp);
	std::unordered_map<uint, uint> came_from;
	std::unordered_map<uint, number> cost_so_far;

	if (verbose) {
		std::cout << "path_find : id = " << id_gmo << " ; start = " << start << " ; goal = " << goal << "\n";
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
			VertexData * vertex_data = get_vertex_data(next);
			/*if ((vertex_data->_id_gmo > 0 && vertex_data->_id_gmo != id_gmo) || vertex_data->_obstacle == VERTEX_OBSTACLE) {
				continue;
			}*/

			number new_cost = cost_so_far[current] + cost(id_gmo, n_grid_size, current, next);
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

	path.clear();
	for (auto & i : path_tmp) {
		if (cost_so_far[i] >= OCCUPIED_EDGE) {
			break;
		}
		path.push_back(i);
	}

	/*for (auto i : path) {
		std::cout << i << " ; " << cost_so_far[i] << "\n";
	}*/

	if (verbose) {
		std::cout << "path_find end ; path size = " << path.size() << "\n";
	}
}


void PathFinder::anim(time_point t) {
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
	
	uint start = pt2closest_id(input->_gmo->_aabb->center());
	path_find(input->_gmo->_id, input->_gmo->_n_grid_size, start, input->_goal, input->_gmo->_path);
	input->_gmo->_idx_path = 0;
	input->_gmo->_status = GMO_MOVING;
}


// PathFinderTest -------------------------------------------------------------------------------------------
PathFinderTest::PathFinderTest() {

}


PathFinderTest::PathFinderTest(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t) :
	_gl_draw_manager(gl_draw_manager), _view_system(view_system) 
{
	uint n_ligs = uint(GRID_SIZE.y / GRID_RESOLUTION.y) + 1;
	uint n_cols = uint(GRID_SIZE.x / GRID_RESOLUTION.x) + 1;

	_pfi = new PathFinder(GRID_ORIGIN, GRID_SIZE, n_ligs, n_cols, t);

	_font = new Font(_gl_draw_manager, "../../fonts/Silom.ttf", 48, _view_system->_screengl);
	_font->_z = 100.0f; // pour que l'affichage des infos se fassent par dessus le reste

	//randomize_edges();
	update();
}


PathFinderTest::~PathFinderTest() {
	for (auto & gmo : _gmos) {
		delete gmo;
	}
	_gmos.clear();
	delete _pfi;

}


void PathFinderTest::add_gmo(pt_2d pt, number size, number speed) {
	uint id = _pfi->pt2closest_id(pt);
	pt_2d center = _pfi->id2pt_2d(id);
	GridMovingObject * gmo = new GridMovingObject(center - 0.5 * pt_2d(size), pt_2d(size), speed);
	gmo->_id = _next_gmo_id++;
	gmo->_n_grid_size = uint(size / _pfi->_resolution.x);
	_gmos.push_back(gmo);
	update_gmo_grid(gmo);
}


GridMovingObject * PathFinderTest::get_gmo(uint id) {
	for (auto & gmo : _gmos) {
		if (gmo->_id == id) {
			return gmo;
		}
	}
	std::cerr << "PathFinderTest::get_gmo : id = " << id << " introuvable\n";
	return NULL;
}


void PathFinderTest::delete_selected_gmos() {
	_gmos.erase(std::remove_if(_gmos.begin(), _gmos.end(), [](GridMovingObject * gmo) { return gmo->_selected; }), _gmos.end());
}


void PathFinderTest::goto_gmo(GridMovingObject * gmo, uint id_vertex) {
	
	/*uint start = _pfi->pt2closest_id(gmo->_aabb->center());
	_pfi->path_find(gmo->_id, gmo->_n_grid_size, start, id_vertex, gmo->_path);
	gmo->_idx_path = 0;
	gmo->_status = GMO_MOVING;*/

	_pfi->_inputs.push(new PathFinderInput(gmo, id_vertex));

	/*std::cout << "goto : id = " << gmo->_id;
	std::cout << " ; start = " << start << " ; goal = " << goal;
	std::cout << " ; path = ";
	for (auto & id : gmo->_path) {
		std::cout << id << " ; ";
	}
	std::cout << "\n";*/
}


void PathFinderTest::goto_gmo(GridMovingObject * gmo, pt_2d target) {
	uint id_vertex = _pfi->pt2closest_id(target);
	goto_gmo(gmo, id_vertex);
}


void PathFinderTest::goto_selected_gmos(pt_2d target) {
	uint goal = _pfi->pt2closest_id(target);

	for (auto & gmo : _gmos) {
		if (gmo->_selected) {
			goto_gmo(gmo, target);
		}
	}

	update_gmos_path();
}


void PathFinderTest::stop_gmo(GridMovingObject * gmo) {
	gmo->_idx_path = 0;
	gmo->_path.clear();
	gmo->_status = GMO_IDLE;
}


void PathFinderTest::clear_edges() {
	_pfi->_it_v= _pfi->_vertices.begin();
	while (_pfi->_it_v!= _pfi->_vertices.end()) {
		_pfi->_it_e= _pfi->_it_v->second._edges.begin();
		while (_pfi->_it_e!= _pfi->_it_v->second._edges.end()) {
			EdgeData * edge_data = _pfi->get_edge_data(_pfi->_it_v->first, _pfi->_it_e->first);
			edge_data->_cost = SOFT_EDGE;
			_pfi->_it_e++;
		}
		_pfi->_it_v++;
	}

	update_grid_edges();
}


void PathFinderTest::randomize_edges() {
	_pfi->_it_v= _pfi->_vertices.begin();
	while (_pfi->_it_v!= _pfi->_vertices.end()) {
		_pfi->_it_e= _pfi->_it_v->second._edges.begin();
		while (_pfi->_it_e!= _pfi->_it_v->second._edges.end()) {
			EdgeData * edge_data = _pfi->get_edge_data(_pfi->_it_v->first, _pfi->_it_e->first);
			if (rand_bool()) {
				edge_data->_cost = HARD_EDGE;
			}
			else {
				edge_data->_cost = SOFT_EDGE;
			}
			_pfi->_it_e++;
		}
		_pfi->_it_v++;
	}

	update_grid_edges();
}


void PathFinderTest::update_gmo_grid(GridMovingObject * gmo) {
	for (auto & v : gmo->_vertices) {
		VertexData * vertex_data = _pfi->get_vertex_data(v);
		if (vertex_data->_id_gmo != gmo->_id) {
			std::cerr << "update_gmo_grid error\n";
		}
		vertex_data->_id_gmo = 0;
	}

	std::vector<uint> vertices = _pfi->vertices_in_aabb(gmo->_aabb);
	//vertices.erase(std::remove_if(vertices.begin(), vertices.end(), [](uint v){return }), vertices.end());
	gmo->_vertices.clear();
	for (auto & v : vertices) {
		VertexData * vertex_data = _pfi->get_vertex_data(v);
		if (vertex_data->_id_gmo == 0) {
			vertex_data->_id_gmo = gmo->_id;
			gmo->_vertices.push_back(v);
		}
	}

	//gmo->_vertices = vertices;
}


void PathFinderTest::set_vertex(pt_2d center, number size, VERTEX_STATUS status) {
	AABB_2D * aabb = new AABB_2D(center - 0.5 * pt_2d(size), pt_2d(size));
	std::vector<uint> vertices = _pfi->vertices_in_aabb(aabb);
	delete aabb;
	for (auto & v : vertices) {
		VertexData * vertex_data = _pfi->get_vertex_data(v);
		vertex_data->_obstacle = status;
	}
}


void PathFinderTest::set_edges(pt_2d center, number size, number value) {
	AABB_2D * aabb = new AABB_2D(center - 0.5 * pt_2d(size), pt_2d(size));
	std::vector<uint_pair> edges = _pfi->edges_intersecting_aabb(aabb);
	delete aabb;
	for (auto & e : edges) {
		EdgeData * edge_data = _pfi->get_edge_data(e.first, e.second);
		edge_data->_cost = value;
	}
}


void PathFinderTest::anim(time_point t) {
	bool verbose = true;

	_pfi->anim(t);

	for (auto & gmo : _gmos) {
		if (gmo->_status == GMO_IDLE) {
			continue;
		}
		
		else if (gmo->_status == GMO_MOVING) {
			for (uint i=gmo->_idx_path + 1; i<std::min(uint(gmo->_path.size()), gmo->_idx_path + 1 + N_VERTICES_CHECK); ++i) {
				VertexData * vertex_data = _pfi->get_vertex_data(gmo->_path[i]);
				
				if (vertex_data->_id_gmo > 0 && vertex_data->_id_gmo != gmo->_id) {
					GridMovingObject * gmo2 = get_gmo(vertex_data->_id_gmo);
					if (gmo2->_status == GMO_IDLE) {
						VertexData * goal_data = _pfi->get_vertex_data(gmo->_path[gmo->_path.size() - 1]);
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
				
				else if (vertex_data->_obstacle == VERTEX_OBSTACLE) {
					VertexData * goal_data = _pfi->get_vertex_data(gmo->_path[gmo->_path.size() - 1]);
					if (goal_data->_obstacle == VERTEX_OBSTACLE) {
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
			pt_2d next = _pfi->id2pt_2d(gmo->_path[gmo->_idx_path + 1]);

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
					next = _pfi->id2pt_2d(gmo->_path[gmo->_idx_path + 1]);
				}
			}

			//pt_2d last = _pfi->id2pt_2d(gmo->_path[gmo->_idx_path]);
			//pt_2d direction = glm::normalize(next - last);
			pt_2d direction = glm::normalize(next - gmo->_aabb->center());
			gmo->_aabb->translate(gmo->_speed * direction);

			update_gmo_grid(gmo);
		}
	}		

	update_select();
	update_gmos();
	update_gmos_path();
	update_font();

	/*if (++_debug > 100) {
		_debug = 0;
		update_grid_edges();
	}*/
}


void PathFinderTest::draw_font() {
	_font->draw_3d(_view_system->_world2clip);
}


void PathFinderTest::draw_grid_centers() {
	GLDrawContext * context = _gl_draw_manager->get_context("grid_centers");
	context->activate();
	context->set_uniform("world2clip_matrix", glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	context->set_uniform("z", Z_CENTER);
	context->draw();
	context->deactivate();
}


void PathFinderTest::draw_grid_edges() {
	GLDrawContext * context = _gl_draw_manager->get_context("grid_edges");
	context->activate();
	context->set_uniform("world2clip_matrix", glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	context->set_uniform("z", Z_EDGE);
	context->draw();
	context->deactivate();
}


void PathFinderTest::draw_gmos() {
	GLDrawContext * context = _gl_draw_manager->get_context("gmos");
	context->activate();
	context->set_uniform("world2clip_matrix", glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	context->set_uniform("z", Z_GMO);
	context->draw();
	context->deactivate();
}


void PathFinderTest::draw_gmos_path() {
	GLDrawContext * context = _gl_draw_manager->get_context("gmos_path");
	context->activate();
	context->set_uniform("world2clip_matrix", glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	context->set_uniform("z", Z_GMO_PATH);
	context->set_uniform("thickness", 6.0f);
	context->set_uniform("viewport_size", glm::value_ptr(glm::vec2(float(_view_system->_screengl->_screen_width), float(_view_system->_screengl->_screen_height))));
	context->draw();
	context->deactivate();
}


void PathFinderTest::draw_select() {
	GLDrawContext * context= _gl_draw_manager->get_context("select");
	context->activate();
	// pour que l'affichage du rectangle de sélection se fassent par dessus le reste
	context->set_uniform("z", -1.0f);
	context->set_uniform("thickness", 7.0f);
	context->set_uniform("viewport_size", glm::value_ptr(glm::vec2(float(_view_system->_screengl->_screen_width), float(_view_system->_screengl->_screen_height))));
	context->draw();
	context->deactivate();
}


void PathFinderTest::draw() {
	glViewport(0, 0, _view_system->_screengl->_screen_width, _view_system->_screengl->_screen_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_view_system->draw();
	draw_grid_centers();
	draw_grid_edges();
	draw_gmos();
	draw_gmos_path();
	draw_select();
	draw_font();
}


void PathFinderTest::update_font() {
	std::vector<Text3D> texts_3d;

	for (auto & gmo : _gmos) {
		texts_3d.push_back(Text3D(std::to_string(gmo->_id), glm::vec3(gmo->_aabb->_pos.x, gmo->_aabb->_pos.y, Z_FONT), 0.01, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
	}

	_font->set_text(texts_3d);
}


void PathFinderTest::update_select() {
	GLDrawContext * context= _gl_draw_manager->get_context("select");

	if (!_view_system->_rect_select->_is_active) {
		context->_active = false;
		return;
	}

	context->_active = true;
	context->_n_pts = 8;

	float * data = new float[context->data_size()];
	pt_2d pt_min = _view_system->_rect_select->_gl_origin;
	pt_2d pt_max = _view_system->_rect_select->_gl_moving;
	pt_2d pts[8] = {
		pt_2d(pt_min.x, pt_min.y), pt_2d(pt_max.x, pt_min.y),
		pt_2d(pt_max.x, pt_min.y), pt_2d(pt_max.x, pt_max.y),
		pt_2d(pt_max.x, pt_max.y), pt_2d(pt_min.x, pt_max.y),
		pt_2d(pt_min.x, pt_max.y), pt_2d(pt_min.x, pt_min.y)
	};
	float * ptr = data;
	for (uint i=0; i<8; ++i) {
		ptr[0] = float(pts[i].x);
		ptr[1] = float(pts[i].y);
		ptr[2] = float(SELECT_COLOR.r);
		ptr[3] = float(SELECT_COLOR.g);
		ptr[4] = float(SELECT_COLOR.b);
		ptr[5] = float(SELECT_COLOR.a);
		ptr += 6;
	}

	context->set_data(data);
	delete[] data;
}


void PathFinderTest::update_grid_centers() {
	GLDrawContext * context= _gl_draw_manager->get_context("grid_centers");

	context->_active = true;
	context->_n_pts = _pfi->_n_ligs * _pfi->_n_cols * 6;

	float * data = new float[context->data_size()];
	float * ptr = data;

	for (uint lig=0; lig<_pfi->_n_ligs; lig++) {
		for (uint col=0; col<_pfi->_n_cols; col++) {
			pt_2d center = _pfi->col_lig2pt_2d(col, lig);

			pt_2d pt1 = center + pt_2d(-1.0 * _pfi->_resolution.x * GRID_CENTER_SIZE_RATIO, -1.0 * _pfi->_resolution.y * GRID_CENTER_SIZE_RATIO);
			pt_2d pt2 = center + pt_2d(1.0 * _pfi->_resolution.x * GRID_CENTER_SIZE_RATIO, -1.0 * _pfi->_resolution.y * GRID_CENTER_SIZE_RATIO);
			pt_2d pt3 = center + pt_2d(1.0 * _pfi->_resolution.x * GRID_CENTER_SIZE_RATIO, 1.0 * _pfi->_resolution.y * GRID_CENTER_SIZE_RATIO);
			pt_2d pt4 = center + pt_2d(-1.0 * _pfi->_resolution.x * GRID_CENTER_SIZE_RATIO, 1.0 * _pfi->_resolution.y * GRID_CENTER_SIZE_RATIO);

			pt_2d pts[6] = {pt1, pt2, pt3, pt1, pt3, pt4};
			
			VertexData * vertex_data = _pfi->get_vertex_data(_pfi->col_lig2id(col, lig));
			glm::vec4 vertex_color;
			if (vertex_data->_obstacle == VERTEX_OBSTACLE) {
				vertex_color = glm::vec4(0.3, 0.3, 0.3, 1.0);
			}
			else if (vertex_data->_id_gmo == 0) {
				vertex_color = glm::vec4(0.0, 1.0, 0.0, 1.0);
			}
			else {
				vertex_color = glm::vec4(1.0, 0.0, 0.0, 1.0);
			}

			for (uint i=0; i<6; ++i) {
				ptr[0] = float(pts[i].x);
				ptr[1] = float(pts[i].y);
				ptr[2] = vertex_color.r;
				ptr[3] = vertex_color.g;
				ptr[4] = vertex_color.b;
				ptr[5] = vertex_color.a;
				ptr += 6;
			}
		}
	}

	context->set_data(data);
	delete[] data;
}


void PathFinderTest::update_grid_edges() {
	GLDrawContext * context= _gl_draw_manager->get_context("grid_edges");

	context->_active = true;

	_pfi->_it_v= _pfi->_vertices.begin();
	while (_pfi->_it_v!= _pfi->_vertices.end()) {
		_pfi->_it_e= _pfi->_it_v->second._edges.begin();
		while (_pfi->_it_e!= _pfi->_it_v->second._edges.end()) {
			context->_n_pts += 2;
			_pfi->_it_e++;
		}
		_pfi->_it_v++;
	}

	float * data = new float[context->data_size()];
	float * ptr = data;

	_pfi->_it_v= _pfi->_vertices.begin();
	while (_pfi->_it_v!= _pfi->_vertices.end()) {
		_pfi->_it_e= _pfi->_it_v->second._edges.begin();
		while (_pfi->_it_e!= _pfi->_it_v->second._edges.end()) {
			glm::vec4 edge_color;
			EdgeData * edge_data = _pfi->get_edge_data(_pfi->_it_v->first, _pfi->_it_e->first);
			if (edge_data->_cost < HARD_EDGE) {
				edge_color = glm::vec4(0.0, 1.0, 0.0, 1.0);
			}
			else {
				edge_color = glm::vec4(1.0, 0.0, 0.0, 1.0);
			}

			pt_3d & p1 = _pfi->_it_v->second._pos;
			pt_3d & p2 = _pfi->_vertices[_pfi->_it_e->first]._pos;
			pt_3d p1b = p1 + (p2 - p1) * 0.1;
			pt_3d p_middle = (p1 + p2) * 0.5 - (p2 - p1) * 0.1;
			
			ptr[0] = float(p1b.x);
			ptr[1] = float(p1b.y);
			ptr[2] = edge_color.r;
			ptr[3] = edge_color.g;
			ptr[4] = edge_color.b;
			ptr[5] = edge_color.a;

			ptr[6] = float(p_middle.x);
			ptr[7] = float(p_middle.y);
			ptr[8] = edge_color.r;
			ptr[9] = edge_color.g;
			ptr[10] = edge_color.b;
			ptr[11] = edge_color.a;

			ptr += 12;
			
			_pfi->_it_e++;
		}
		_pfi->_it_v++;
	}

	context->set_data(data);
	delete[] data;
}


void PathFinderTest::update_gmos() {
	GLDrawContext * context= _gl_draw_manager->get_context("gmos");

	if (_gmos.size() == 0) {
		context->_active = false;
		return;
	}

	context->_active = true;
	context->_n_pts = _gmos.size() * 6;

	float * data = new float[context->data_size()];
	float * ptr = data;

	for (auto & gmo : _gmos) {
		pt_2d pts[6] = {
			gmo->_aabb->_pos, gmo->_aabb->_pos + pt_2d(gmo->_aabb->_size.x, 0.0), gmo->_aabb->_pos + gmo->_aabb->_size,
			gmo->_aabb->_pos, gmo->_aabb->_pos + gmo->_aabb->_size, gmo->_aabb->_pos + pt_2d(0.0, gmo->_aabb->_size.y)
		};
		
		glm::vec4 color;
		float alpha = 0.3;
		if (gmo->_selected) {
			color = glm::vec4(1.0, 1.0, 0.0, alpha);
		}
		else if (gmo->_status == GMO_IDLE) {
			color = glm::vec4(0.0, 0.2, 1.0, alpha);
		}
		else if (gmo->_status == GMO_MOVING) {
			color = glm::vec4(0.0, 0.8, 1.0, alpha);
		}
		else if (gmo->_status == GMO_WAITING) {
			color = glm::vec4(0.8, 0.2, 0.3, alpha);
		}
		
		for (uint i=0; i<6; ++i) {
			ptr[0] = float(pts[i].x);
			ptr[1] = float(pts[i].y);
			ptr[2] = color.r;
			ptr[3] = color.g;
			ptr[4] = color.b;
			ptr[5] = color.a;
			ptr += 6;
		}
	}

	context->set_data(data);
	delete[] data;
}


void PathFinderTest::update_gmos_path() {
	GLDrawContext * context= _gl_draw_manager->get_context("gmos_path");

	if (_gmos.size() == 0) {
		context->_active = false;
		return;
	}

	context->_n_pts = 0;
	for (auto & gmo : _gmos) {
		if (gmo->_path.empty()) {
			continue;
		}
		
		context->_n_pts += 2 * (gmo->_path.size() - 1);
	}
	
	if (context->_n_pts == 0) {
		context->_active = false;
		return;
	}

	context->_active = true;

	float * data = new float[context->data_size()];
	float * ptr = data;

	for (auto & gmo : _gmos) {
		if (gmo->_path.empty()) {
			continue;
		}

		for (uint i=0; i<gmo->_path.size() - 1; ++i) {
			pt_2d pt1 = _pfi->id2pt_2d(gmo->_path[i]);
			pt_2d pt2 = _pfi->id2pt_2d(gmo->_path[i + 1]);
			glm::vec4 color(0.7, 0.8, 0.9, 0.5);

			ptr[0] = float(pt1.x);
			ptr[1] = float(pt1.y);
			ptr[2] = color.r;
			ptr[3] = color.g;
			ptr[4] = color.b;
			ptr[5] = color.a;

			ptr[6] = float(pt2.x);
			ptr[7] = float(pt2.y);
			ptr[8] = color.r;
			ptr[9] = color.g;
			ptr[10] = color.b;
			ptr[11] = color.a;

			ptr += 12;
		}
	}

	context->set_data(data);
	delete[] data;
}


void PathFinderTest::update() {
	update_grid_centers();
	update_grid_edges();
	update_gmos();
	update_gmos_path();
	update_select();
	update_font();
}


bool PathFinderTest::mouse_button_down(InputState * input_state, time_point t) {
	if (input_state->_keys[SDLK_a]) {
		pt_2d pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);
		number size = 1.5;
		number speed = 0.04;
		add_gmo(pt, size, speed);
		update_gmos();
		return true;
	}
	else if (input_state->_keys[SDLK_z]) {
		pt_2d pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);
		number size = 5.0;
		number speed = 0.02;
		add_gmo(pt, size, speed);
		update_gmos();
		return true;
	}
	else if (input_state->_keys[SDLK_g]) {
		pt_2d pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);
		goto_selected_gmos(pt);
	}

	if (_view_system->mouse_button_down(input_state, t)) {
		return true;
	}

	return false;
}


bool PathFinderTest::mouse_button_up(InputState * input_state, time_point t) {
	if (_view_system->mouse_button_up(input_state, t)) {
		//return true;
	}

	if (_view_system->_new_single_selection) {
		_view_system->_new_single_selection= false;
		for (auto & gmo : _gmos) {
			gmo->_selected = false;
			if (_view_system->single_selection_intersects_aabb_2d(gmo->_aabb)) {
				gmo->_selected = true;
			}
		}
	}
	else if (_view_system->_new_rect_selection) {
		_view_system->_new_rect_selection= false;
		for (auto & gmo : _gmos) {
			gmo->_selected = false;
			if (_view_system->intersects_aabb_2d(gmo->_aabb, true)) {
				gmo->_selected = true;
			}
		}
	}

	return false;
}
	

bool PathFinderTest::mouse_motion(InputState * input_state, time_point t) {
	pt_2d pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);

	if (input_state->_keys[SDLK_w]) {
		set_edges(pt, 3.0, SOFT_EDGE);
		return true;
	}
	if (input_state->_keys[SDLK_x]) {
		set_edges(pt, 3.0, HARD_EDGE);
		return true;
	}
	if (input_state->_keys[SDLK_b]) {
		set_vertex(pt, 3.0, VERTEX_NO_OBSTACLE);
		return true;
	}
	if (input_state->_keys[SDLK_n]) {
		set_vertex(pt, 3.0, VERTEX_OBSTACLE);
		return true;
	}

	if (_view_system->mouse_motion(input_state, t)) {
		return true;
	}

	return false;
}


bool PathFinderTest::key_down(InputState * input_state, SDL_Keycode key, time_point t) {
	if (key == SDLK_d) {
		delete_selected_gmos();
		return true;
	}
	if (key == SDLK_c) {
		clear_edges();
		return true;
	}
	if (key == SDLK_r) {
		randomize_edges();
		return true;
	}
	if (key == SDLK_SPACE) {
		update_grid_edges();
		update_grid_centers();
		/*for (auto & gmo : _gmos) {
			std::cout << gmo->_id << " vertices = ";
			for (auto & v : gmo->_vertices) {
				std::cout << v << " ; ";
			}
			std::cout << "\n";
		}*/
		return true;
	}

	if (_view_system->key_down(input_state, key, t)) {
		return true;
	}

	return false;
}


bool PathFinderTest::key_up(InputState * input_state, SDL_Keycode key, time_point t) {
	if (_view_system->key_up(input_state, key, t)) {
		return true;
	}

	return false;
}
