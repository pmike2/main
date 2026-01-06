#include <queue>
#include <fstream>
#include <tuple>
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "json.hpp"

#include "utile.h"
#include "shapefile.h"

#include "map.h"


using json = nlohmann::json;


uint Map::_next_unit_id = 1;

// ----------------------------------------------------------------------------------------
/*Obstacle::Obstacle() {

}


Obstacle::Obstacle(OBSTACLE_TYPE type, const std::vector<pt_2d> & pts) : _type(type) {
	_polygon = new Polygon2D(pts);
	_polygon->update_all();
}


Obstacle::Obstacle(OBSTACLE_TYPE type, Polygon2D * polygon) : _type(type), _polygon(polygon) {
	_polygon->update_all();
}


Obstacle::~Obstacle() {
	delete _polygon;
}*/


Map::Map() {

}


Map::Map(std::string unit_types_dir, std::string elements_dir, pt_2d origin, pt_2d size, pt_2d path_resolution, pt_2d elevation_resolution, time_point t) :
	_origin(origin), _size(size), _paused(false), _path_find_thr_active(false)
{
	uint n_ligs_path = uint(_size.y / path_resolution.y) + 1;
	uint n_cols_path = uint(_size.x / path_resolution.x) + 1;

	uint n_ligs_elevation = uint(_size.y / elevation_resolution.y) + 1;
	uint n_cols_elevation = uint(_size.x / elevation_resolution.x) + 1;

	_elevation = new Elevation(_origin, _size, n_ligs_elevation, n_cols_elevation);

	_path_finder = new PathFinder(_origin, _size, n_ligs_path, n_cols_path);

	std::vector<std::string> jsons_paths = list_files(unit_types_dir, "json");
	for (auto & json_path : jsons_paths) {
		UnitType * unit_type = new UnitType(json_path);
		_unit_types[basename(json_path)] = unit_type;
		_path_finder->add_unit_type(unit_type);
	}

	_elements = new Elements(elements_dir);

	sync2elevation();
}


Map::~Map() {
	clear();

	delete _elements;
	
	delete _path_finder;

	for (auto & ut : _unit_types) {
		delete ut.second;
	}
	_unit_types.clear();

	delete _elevation;
}


void Map::add_unit(std::string type_name, pt_2d pos, time_point t) {
	if (pos.x < _origin.x || pos.y < _origin.y || pos.x >= _origin.x + _size.x || pos.y >= _origin.y + _size.y) {
		std::cerr << "Map::add_unit hors Elevation\n";
		return;
	}

	pt_3d pt3d(pos.x, pos.y, _elevation->get_alti(pos));
	Unit * unit = new Unit(_unit_types[type_name], pt3d, t);
	unit->_id = _next_unit_id++;
	_units.push_back(unit);

	add_waiting_unit_to_position_grids(unit);
}


void Map::add_static_element(std::string element_name, pt_3d pos, pt_3d size) {
	AABB * element_aabb;
	if (element_name == "stone") {
		Stone * stone = _elements->add_stone(pos, size);
		element_aabb = stone->_aabb;
	}
	else {
		Tree * tree = _elements->add_tree(element_name, pos, size);
		element_aabb = tree->_aabb;
	}

	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;
		//AABB_2D * aabb = new AABB_2D(pt_2d(element_aabb->_vmin - 0.5 * unit_type->_size), pt_2d(element_aabb->size() + unit_type->_size));
		AABB_2D * aabb = new AABB_2D(pt_2d(element_aabb->_vmin), pt_2d(element_aabb->size()));
		aabb->buffer(unit_type->buffer_size());

		std::vector<std::pair<uint, uint> > edges = _path_finder->aabb_intersection(aabb);
		for (auto & e : edges) {
			GraphEdge & edge = _path_finder->_vertices[e.first]._edges[e.second];
			EdgeData * data = (EdgeData *)(edge._data);
			data->_type[unit_type] = OBSTACLE;
		}
	}
}


void Map::add_river(pt_2d src) {
	River * river = new River(_elevation, src);
	_rivers.push_back(river);

	/*for (auto & triangle : river->_triangles) {
		std::vector<pt_3d> pts = {std::get<0>(triangle), std::get<1>(triangle), std::get<2>(triangle)};

	}*/

	Polygon2D * polygon = _elevation->ids2polygon(river->_id_nodes);

	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;

		std::vector<uint> id_nodes_buffered;

		AABB_2D * aabb = polygon->_aabb->buffered(unit_type->buffer_size());
		std::vector<uint> id_nodes_buffered_aabb = _elevation->vertices_in_aabb(aabb);
		delete aabb;
		for (auto & id : id_nodes_buffered_aabb) {
			if (distance_poly_pt(polygon, _elevation->id2pt_2d(id), NULL) < unit_type->buffer_size()) {
				id_nodes_buffered.push_back(id);
			}
		}
		Polygon2D * polygon_buffered = _elevation->ids2polygon(id_nodes_buffered);

		std::vector<std::pair<uint, uint> > edges = _path_finder->polygon_intersection(polygon_buffered);
		for (auto & edge : edges) {
			uint n_vertices_in_polygon = 0;
			if (is_pt_inside_poly(pt_2d(_path_finder->_vertices[edge.first]._pos), polygon_buffered)) {
				n_vertices_in_polygon++;
			}
			if (is_pt_inside_poly(pt_2d(_path_finder->_vertices[edge.second]._pos), polygon_buffered)) {
				n_vertices_in_polygon++;
			}
			
			GraphEdge & e = _path_finder->_vertices[edge.first]._edges[edge.second];
			EdgeData * data = (EdgeData *)(e._data);
			if (n_vertices_in_polygon > 0) {
				data->_type[unit_type] = RIVER;
			}
		}

		delete polygon_buffered;
	}

	delete polygon;
}


void Map::add_lake(pt_2d src) {
	Lake * lake = new Lake(_elevation, src);
	if (!lake->_valid) {
		delete lake;
		return;
	}

	_lakes.push_back(lake);

	Polygon2D * polygon = _elevation->ids2polygon(lake->_id_nodes);

	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;

		std::vector<uint> id_nodes_buffered;

		if (unit_type->_terrain_weights[LAKE] > MAX_UNIT_MOVING_WEIGHT) {
			AABB_2D * aabb = polygon->_aabb->buffered(unit_type->buffer_size());
			std::vector<uint> id_nodes_buffered_aabb = _elevation->vertices_in_aabb(aabb);
			delete aabb;
			for (auto & id : id_nodes_buffered_aabb) {
				if (distance_poly_pt(polygon, _elevation->id2pt_2d(id), NULL) < unit_type->buffer_size()) {
					id_nodes_buffered.push_back(id);
				}
			}
		}
		else {
			AABB_2D * aabb = polygon->_aabb->buffered(-1.0 * unit_type->buffer_size());
			std::vector<uint> id_nodes_buffered_aabb = _elevation->vertices_in_aabb(aabb);
			delete aabb;
			for (auto & id : id_nodes_buffered_aabb) {
				//if (distance_poly_pt(polygon, _elevation->id2pt_2d(id), NULL) < 0.01) {
				// PAS GENIAL ...
				if (is_pt_inside_poly(_elevation->id2pt_2d(id), polygon)) {
					id_nodes_buffered.push_back(id);
				}
			}
		}
		Polygon2D * polygon_buffered = _elevation->ids2polygon(id_nodes_buffered);

		std::vector<std::pair<uint, uint> > edges = _path_finder->polygon_intersection(polygon_buffered);
		for (auto & edge : edges) {
			uint n_vertices_in_polygon = 0;
			if (is_pt_inside_poly(pt_2d(_path_finder->_vertices[edge.first]._pos), polygon_buffered)) {
				n_vertices_in_polygon++;
			}
			if (is_pt_inside_poly(pt_2d(_path_finder->_vertices[edge.second]._pos), polygon_buffered)) {
				n_vertices_in_polygon++;
			}
			
			GraphEdge & e = _path_finder->_vertices[edge.first]._edges[edge.second];
			EdgeData * data = (EdgeData *)(e._data);
			if (n_vertices_in_polygon == 2) {
				data->_type[unit_type] = LAKE;
			}
			else if (n_vertices_in_polygon == 1) {
				data->_type[unit_type] = LAKE_COAST;
			}
		}

		delete polygon_buffered;
	}

	delete polygon;

	sync2elevation();
}


// maj des altis des vertices de la grille
void Map::update_alti_grid() {
	_path_finder->_it_v= _path_finder->_vertices.begin();
	while (_path_finder->_it_v!= _path_finder->_vertices.end()) {
		pt_3d & pt= _path_finder->_it_v->second._pos;
		pt.z = _elevation->get_alti(pt);
		_path_finder->_it_v++;
	}
}


void Map::update_alti_path(Unit * unit) {
	for (auto & pt : unit->_path->_pts) {
		pt.z = _elevation->get_alti(pt);
	}
}


void Map::update_elevation_grids() {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;

		//number max_weight = -1e-5;
		//number min_weight = 1e-5;

		_path_finder->_it_v= _path_finder->_vertices.begin();
		while (_path_finder->_it_v!= _path_finder->_vertices.end()) {
			_path_finder->_it_e= _path_finder->_it_v->second._edges.begin();
			while (_path_finder->_it_e!= _path_finder->_it_v->second._edges.end()) {
				GraphEdge & edge = _path_finder->_vertices[_path_finder->_it_v->first]._edges[_path_finder->_it_e->first];
				pt_3d & pt_begin= _path_finder->_it_v->second._pos;
				pt_3d & pt_end= _path_finder->_vertices[_path_finder->_it_e->first]._pos;
				EdgeData * data = (EdgeData *)(edge._data);
				data->_delta_elevation[unit_type] = unit_type->elevation_coeff(pt_end.z - pt_begin.z);

				/*if (pt_begin.z < 0.01 && pt_end.z < 0.01) {
					edge._weight += type_grid.first->_weights[WATER];
				}
				else {
					edge._weight += type_grid.first->_weights[GROUND];
				}*/
				
				/*for (auto & obstacle : _buffered_obstacles[type_grid.first]) {
					if (segment_intersects_poly(pt_begin, pt_end, obstacle->_polygon, NULL)) {
						edge._weight += type_grid.first->_weights[obstacle->_type];
						break;
					}
				}*/

				/*if (edge._weight > max_weight) {
					max_weight = edge._weight;
				}
				if (edge._weight < min_weight) {
					min_weight = edge._weight;
				}*/

				_path_finder->_it_e++;
			}
			_path_finder->_it_v++;
		}

		//std::cout << type_grid.first->_name << " ; min_w=" << min_weight << " ; max_w=" << max_weight << "\n";
	}
}


void Map::update_terrain_grids_with_elevation(BBox_2D * bbox) {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;

		std::vector<std::pair<uint, uint> > edges;
		if (bbox != NULL) {
			edges = _path_finder->bbox_intersection(bbox);
		}

		_path_finder->_it_v= _path_finder->_vertices.begin();
		while (_path_finder->_it_v!= _path_finder->_vertices.end()) {
			_path_finder->_it_e= _path_finder->_it_v->second._edges.begin();
			while (_path_finder->_it_e!= _path_finder->_it_v->second._edges.end()) {
				if (bbox != NULL && std::find(edges.begin(), edges.end(), std::make_pair(_path_finder->_it_v->first, _path_finder->_it_e->first)) == edges.end()) {
					continue;
				}

				GraphEdge & edge = _path_finder->_vertices[_path_finder->_it_v->first]._edges[_path_finder->_it_e->first];
				pt_3d & pt_begin = _path_finder->_it_v->second._pos;
				pt_3d & pt_end = _path_finder->_vertices[_path_finder->_it_e->first]._pos;
				EdgeData * data = (EdgeData *)(edge._data);

				if (pt_begin.z < 0.01 && pt_end.z < 0.01) {
					data->_type[unit_type] = SEA;
				}
				else if ((pt_begin.z < 0.01 && pt_end.z > 0.01) || (pt_begin.z > 0.01 && pt_end.z < 0.01)) {
					data->_type[unit_type] = SEA_COAST;
				}
				else if (data->_type[unit_type] == UNKNOWN || data->_type[unit_type] == SEA || data->_type[unit_type] == SEA_COAST) {
					data->_type[unit_type] = GROUND;
				}

				_path_finder->_it_e++;
			}
			_path_finder->_it_v++;
		}
	}
}


void Map::sync2elevation() {
	update_alti_grid();
	update_elevation_grids();
	update_terrain_grids_with_elevation();
}


void Map::clear_units_position_grids() {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;

		_path_finder->_it_v= _path_finder->_vertices.begin();
		while (_path_finder->_it_v!= _path_finder->_vertices.end()) {
			_path_finder->_it_e = _path_finder->_vertices[_path_finder->_it_v->first]._edges.begin();
			while (_path_finder->_it_e != _path_finder->_vertices[_path_finder->_it_v->first]._edges.end()) {
				EdgeData * data = (EdgeData * )(_path_finder->_vertices[_path_finder->_it_v->first]._edges[_path_finder->_it_e->first]._data);
				data->_ids[unit_type].clear();
				_path_finder->_it_e++;
			}
			_path_finder->_it_v++;
		}
	}
}


std::vector<std::pair<uint, uint> > Map::waiting_unit_positions_edges(Unit * unit, UnitType * unit_type) {
	AABB_2D * aabb = new AABB_2D(pt_2d(unit->_aabb->_vmin - 0.5 * unit_type->_size), pt_2d(unit->_aabb->size() + unit_type->_size));
	std::vector<std::pair<uint, uint> > edges = _path_finder->aabb_intersection(aabb);
	delete aabb;

	return edges;
}


std::vector<std::pair<uint, uint> > Map::moving_unit_positions_edges(Unit * unit, UnitType * unit_type, bool all) {
	AABB_2D * aabb_unit = unit->_aabb->aabb2d();

	std::vector<std::pair<uint, uint> > edges;
	bool intersection_happened = false;

	AABB_2D * aabb_start = new AABB_2D(unit->_path->_start - 0.5 * pt_2d(unit_type->_size), pt_2d(unit->_aabb->size() + unit_type->_size));
	if (!all && !intersection_happened) {
		if (aabb2d_intersects_aabb2d(aabb_unit, aabb_start)) {
			intersection_happened = true;
		}
	}
	if (all || !intersection_happened) {
		std::vector<std::pair<uint, uint> > start_edges = _path_finder->aabb_intersection(aabb_start);
		edges.insert(edges.end(), start_edges.begin(), start_edges.end());
	}
	delete aabb_start;
	
	for (uint i=0; i<unit->_path->_bboxs.size(); ++i) {
		BBox_2D * buffered_bbox = unit->_path->_bboxs[i]->buffered(unit_type->buffer_size());
		if (!all && !intersection_happened) {
			if (aabb2d_intersects_aabb2d(aabb_unit, buffered_bbox->_aabb)) {
				intersection_happened = true;
			}
		}
		if (all || !intersection_happened) {
			std::vector<std::pair<uint, uint> > path_edges = _path_finder->bbox_intersection(buffered_bbox);
			edges.insert(edges.end(), path_edges.begin(), path_edges.end());
		}
		delete buffered_bbox;
	}

	AABB_2D * aabb_goal = new AABB_2D(unit->_path->_goal - pt_2d(unit->_type->_size- 0.5 * unit_type->_size), pt_2d(unit->_type->_size + unit_type->_size));
	if (!all && !intersection_happened) {
		if (aabb2d_intersects_aabb2d(aabb_unit, aabb_goal)) {
			intersection_happened = true;
		}
	}
	if (all || !intersection_happened) {
		std::vector<std::pair<uint, uint> > goal_edges = _path_finder->aabb_intersection(aabb_goal);
		edges.insert(edges.end(), goal_edges.begin(), goal_edges.end());
	}
	delete aabb_goal;

	delete aabb_unit;
	
	return edges;
}


void Map::add_waiting_unit_to_position_grids(Unit * unit) {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;

		for (auto & e : waiting_unit_positions_edges(unit, unit_type)) {
			GraphEdge & edge = _path_finder->_vertices[e.first]._edges[e.second];
			EdgeData * data = (EdgeData *)(edge._data);
			if (std::find(data->_ids[unit_type].begin(), data->_ids[unit_type].end(), unit->_id) == data->_ids[unit_type].end()) {
				data->_ids[unit_type].push_back(unit->_id);
			}
		}
	}
}


void Map::remove_waiting_unit_from_position_grids(Unit * unit) {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;

		for (auto & e : waiting_unit_positions_edges(unit, unit_type)) {
			GraphEdge & edge = _path_finder->_vertices[e.first]._edges[e.second];
			EdgeData * data = (EdgeData *)(edge._data);
			if (std::find(data->_ids[unit_type].begin(), data->_ids[unit_type].end(), unit->_id) != data->_ids[unit_type].end()) {
				data->_ids[unit_type].erase(std::find(data->_ids[unit_type].begin(), data->_ids[unit_type].end(), unit->_id));
			}
		}
	}
}


void Map::add_moving_unit_to_position_grids(Unit * unit) {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;

		for (auto & e : moving_unit_positions_edges(unit, unit_type, true)) {
			GraphEdge & edge = _path_finder->_vertices[e.first]._edges[e.second];
			EdgeData * data = (EdgeData *)(edge._data);
			if (std::find(data->_ids[unit_type].begin(), data->_ids[unit_type].end(), unit->_id) == data->_ids[unit_type].end()) {
				data->_ids[unit_type].push_back(unit->_id);
			}
		}
	}
}


void Map::remove_moving_unit_from_position_grids(Unit * unit, bool all) {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;
		
		for (auto & e : moving_unit_positions_edges(unit, unit_type, all)) {
			GraphEdge & edge = _path_finder->_vertices[e.first]._edges[e.second];
			EdgeData * data = (EdgeData *)(edge._data);
			if (std::find(data->_ids[unit_type].begin(), data->_ids[unit_type].end(), unit->_id) != data->_ids[unit_type].end()) {
				data->_ids[unit_type].erase(std::find(data->_ids[unit_type].begin(), data->_ids[unit_type].end(), unit->_id));
			}
		}
	}
}


void Map::path_find(Unit * unit, pt_2d goal) {
	_path_finder->path_find(unit, goal);
}


void Map::clear() {
	/*for (auto & obstacle : _obstacles) {
		delete obstacle;
	}
	_obstacles.clear();
	for (auto & buffered_obstacle : _buffered_obstacles) {
		for (auto & obst : buffered_obstacle.second) {
			delete obst;
		}
		buffered_obstacle.second.clear();
	}*/

	clear_units_position_grids();
	
	for (auto & unit : _units) {
		delete unit;
	}
	_units.clear();

	_elements->clear();
	
	_elevation->set_alti_all(0.0);
	
	sync2elevation();
}


/*void Map::read_shapefile(std::string shp_path, pt_2d origin, pt_2d size, bool reverse_y) {
	std::vector<ShpEntry *> entries;
	read_shp(shp_path, entries);
	for (auto & entry : entries) {
		std::vector<pt_2d> pts;
		for (auto & pt : entry->_polygon->_pts) {
			number x= ((pt.x- origin.x)/ size.x)* _size.x+ _origin.x;
			number y;
			if (reverse_y) {
				y= ((origin.y- pt.y)/ size.y)* _size.y+ _origin.y;
			}
			else {
				y= ((pt.y- origin.y)/ size.y)* _size.y+ _origin.y;
			}
			pts.push_back(pt_2d(x, y));
		}
		OBSTACLE_TYPE obst_type = str2type(entry->_fields["type"]);
		if (obst_type != UNKNOWN) {
			add_obstacle(obst_type, pts);
		}
		delete entry;
	}
}*/


void Map::anim(time_point t) {
	if (_paused) {
		return;
	}

	for (auto & unit : _units) {

		std::mutex mtx;
		mtx.lock();
		if (unit->_status == COMPUTING_PATH) {
			continue;
		}
		mtx.unlock();

		if (unit->_status == COMPUTING_PATH_DONE) {
			_path_find_thr_active = false;
			//unit->_thr.join();
			_path_find_thr.join();
			update_alti_path(unit);
			unit->goto_next_checkpoint(t);
			remove_waiting_unit_from_position_grids(unit);
			add_moving_unit_to_position_grids(unit);
		}
		else if (unit->_status == COMPUTING_PATH_FAILED) {
			_path_find_thr_active = false;
			//unit->_thr.join();
			_path_find_thr.join();
			unit->_instructions.push({unit->_path->_goal, t + std::chrono::milliseconds(1000)});
			unit->stop();
		}
		else if (unit->_status == MOVING) {
			/*AABB * aabb_next = new AABB(*unit->_aabb);
			aabb_next->translate(unit->_velocity);
			for (auto & unit2 : _units) {
				if (unit2 == unit) {
					continue;
				}

				AABB * aabb2_buffered = new AABB(*unit2->_aabb);
				//aabb2_buffered->scale(1.0);
				if (aabb_intersects_aabb(aabb_next, aabb2_buffered)) {
					std::cout << "unit " << unit->_id << " : collision stop\n";
					unit->_instructions.push({unit->_path->destination(), t + std::chrono::milliseconds(2000 + rand_int(0, 2000))});
					unit->stop();
					//add_unit_to_position_grids(unit);
					break;
				}
			}*/

			unit->anim(t);

			if (unit->checkpoint_checked()) {
				if (unit->_path->_idx_path + 1 >= unit->_path->_pts.size()) {
					remove_moving_unit_from_position_grids(unit, true);
					add_waiting_unit_to_position_grids(unit);
					unit->stop();
				}
				else {
					remove_moving_unit_from_position_grids(unit, false);
					unit->_path->_idx_path++;
					unit->goto_next_checkpoint(t);
					/*if (unit->_path->_weights[unit->_path->_idx_path] >= MAX_UNIT_MOVING_WEIGHT) {
						std::cout << "unit " << unit->_id << "stops : path weight\n";
						unit->stop();
					}*/
				}
			}
			
			for (auto & unit2 : _units) {
				if (unit2 == unit) {
					continue;
				}

				if (aabb_intersects_aabb(unit->_aabb, unit2->_aabb)) {
					std::cerr << "intersection unit " << unit->_id << " et " << unit2->_id << "\n";
				}
			}

			pt_3d unit_center = unit->_aabb->bottom_center();
			number alti = _elevation->get_alti(unit_center);
			unit->_aabb->set_z(alti);

			/*if (unit->_status == WAITING) {
				std::cout << "unit " << unit->_id << " waiting\n";
				remove_unit_from_position_grids(unit);
				unit->_path->clear();
				add_unit_to_position_grids(unit);
			}*/
		}

		if (!_path_find_thr_active && !unit->_instructions.empty()) {
			Instruction i = unit->_instructions.front();
			if (i._t <= t) {
				unit->_instructions.pop();

				pt_2d pt = i._destination;
				//std::cout << "unit " << unit->_id << " : goto " << glm_to_string(pt) << "\n";

				unit->stop();
				unit->_status = COMPUTING_PATH;
				//unit->_thr= std::thread(&Map::path_find, this, unit, pt);
				_path_find_thr= std::thread(&Map::path_find, this, unit, pt);
				_path_find_thr_active = true;
			}
		}
	}
}


void Map::selected_units_goto(pt_2d pt, time_point t) {
	uint compt = 0;
	for (auto & unit : _units) {
		if (unit->_selected) {
			//unit->_instructions.push({pt, t + std::chrono::milliseconds(500 * compt)});
			unit->_instructions.push({pt, t});
			compt++;
		}
	}
}


void Map::randomize() {
	std::cout << "random begin\n";
	clear();
	
	_elevation->randomize();
	std::cout << "random 1\n";
	
	sync2elevation();

	std::cout << "random 2\n";

	for (uint i=0; i<1000; ++i) {
		std::string element_name = std::vector<std::string>{"tree_test", "stone"}[rand_int(0, 1)];
		pt_3d size;
		if (element_name == "stone") {
			size = rand_pt_3d(0.2, 1.0, 0.2, 1.0, 0.1, 0.5);
		}
		else {
			size = rand_pt_3d(0.2, 1.0, 0.2, 1.0, 1.0, 2.0);
		}
		pt_2d pt = rand_pt(_origin + 0.5 * pt_2d(size.x, size.y)+ 0.1, _origin + _size - 0.5 * pt_2d(size.x, size.y)- 0.1);
		number alti = _elevation->get_alti(pt);
		if (alti > 0.01) {
			//add_static_element(element_name, pt_3d(pt.x, pt.y, alti), size);
		}
	}
	std::cout << "random end\n";
}


void Map::save(std::string json_path) {
	json js;

	js["Elevation"] = json::array();
	for (uint lig = 0; lig<_elevation->_n_ligs; ++lig) {
		for (uint col = 0; col<_elevation->_n_cols; ++col) {
			js["Elevation"].push_back(_elevation->get_alti(col, lig));
		}
	}

	js["units"] = json::array();
	for (auto unit : _units) {
		json entry;
		entry["type"] = unit->_type->_name;
		json position = json::array();
		position.push_back(unit->_aabb->center().x);
		position.push_back(unit->_aabb->center().y);
		//position.push_back(unit->_aabb->center().z);
		entry["position"]= position;
		js["units"].push_back(entry);
	}

	std::ofstream ofs(json_path);
	ofs << js << "\n";
}


void Map::load(std::string json_path, time_point t) {
	clear();

	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	uint idx = 0;
	for (auto & alti : js["Elevation"]) {
		_elevation->set_alti(idx, alti);
		idx++;
	}

	for (auto & unit : js["units"]) {
		add_unit(unit["type"], pt_2d(unit["position"][0], unit["position"][1]), t);
	}

	sync2elevation();
}


std::ostream & operator << (std::ostream & os, Map & map) {
	os << "unit_types = ";
	for (auto & ut : map._unit_types) {
		os << *ut.second << "\n";
	}
	os << "units = ";
	for (auto & unit : map._units) {
		os << *unit << "\n";
	}
	return os;
}

