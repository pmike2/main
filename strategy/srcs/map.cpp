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
#include "thread.h"

#include "map.h"


using json = nlohmann::json;


uint Map::_next_unit_id = 1;


Map::Map() {

}


Map::Map(std::string unit_types_dir, std::string ammo_types_dir, std::string elements_dir, pt_2d origin, pt_2d size, pt_2d path_resolution, pt_2d elevation_resolution, pt_2d fow_resolution, time_point t) {
	uint n_ligs_path = uint(size.y / path_resolution.y) + 1;
	uint n_cols_path = uint(size.x / path_resolution.x) + 1;

	uint n_ligs_elevation = uint(size.y / elevation_resolution.y) + 1;
	uint n_cols_elevation = uint(size.x / elevation_resolution.x) + 1;

	_elevation = new Elevation(origin, size, n_ligs_elevation, n_cols_elevation);

	_path_finder = new PathFinder(origin, size, n_ligs_path, n_cols_path);

	std::vector<std::string> unit_type_json_paths = list_files(unit_types_dir, "json");
	for (auto & json_path : unit_type_json_paths) {
		UnitType * unit_type = new UnitType(json_path);
		_unit_types[str2unit_type(str_to_upper(basename(json_path)))] = unit_type;
		_path_finder->add_unit_type(unit_type);
	}
	//std::cout << *_unit_types[TANK]->_obj_data << "\n";

	std::vector<std::string> ammo_type_json_paths = list_files(ammo_types_dir, "json");
	for (auto & json_path : ammo_type_json_paths) {
		AmmoType * ammo_type = new AmmoType(json_path);
		_ammo_types[ammo_type->_name] = ammo_type;
	}

	for (auto & unit_type : _unit_types) {
		unit_type.second->_ammo_type = _ammo_types[unit_type.second->_ammo_type_str];
	}

	_elements = new Elements(elements_dir + "/tree_species", elements_dir + "/stone_species", _elevation);

	_teams.push_back(new Team("Team1", glm::vec3(1.0f, 0.0f, 0.0f), _elevation, fow_resolution));
	_teams.push_back(new Team("Team2", glm::vec3(0.0f, 0.0f, 1.0f), _elevation, fow_resolution));

	sync2elevation();

	_path_find_thr_running = true;
	_path_find_thr= std::thread(&Map::path_find, this);
	_path_finder_computing = false;

	for (auto & team : _teams) {
		while (true) {
			pt_2d position = rand_pt_2d(origin, origin + size);
			std::cout << glm_to_string(position) << "\n";
			if (add_unit_check(INFANTERY, position)) {
				add_unit(team, INFANTERY, position);
				break;
			}
		}
	}
}


Map::~Map() {
	clear();

	for (auto & team : _teams) {
		delete team;
	}

	delete _elements;

	delete _path_finder;

	for (auto & ut : _unit_types) {
		delete ut.second;
	}
	_unit_types.clear();

	for (auto & at : _ammo_types) {
		delete at.second;
	}
	_ammo_types.clear();

	delete _elevation;

	_path_find_thr_running = false;
	_path_find_thr.join();
}


bool Map::add_unit_check(UNIT_TYPE type, pt_2d pos) {
	if (pos.x < _elevation->_origin.x || pos.y < _elevation->_origin.y || pos.x >= _elevation->_origin.x + _elevation->_size.x || pos.y >= _elevation->_origin.y + _elevation->_size.y) {
		return false;
	}

	/*AABB_2D * aabb = _unit_types[type]->_obj_data->_aabb->aabb2d();
	aabb->translate(pos);
	std::vector<uint_pair> edges = _path_finder->edges_intersecting_aabb(aabb);
	delete aabb;*/

	std::vector<uint_pair> edges = _path_finder->edges_in_cell_containing_pt(pos, true);

	for (auto & e : edges) {
		GraphEdge edge = _path_finder->get_edge(e);
		EdgeData * data = (EdgeData *)(edge._data);
		if (!data->_ids[_unit_types[type]].empty()) {
			return false;
		}
		TERRAIN_TYPE terrain = data->_type[_unit_types[type]];
		//std::cout << terrain_type2str(terrain) << "\n";
		if (_unit_types[type]->_terrain_weights[terrain] > MAX_UNIT_MOVING_WEIGHT) {
			return false;
		}

		// TODO : ajouter une contrainte d'élévation ?
	}
	
	return true;
}


Unit * Map::add_unit(Team * team, UNIT_TYPE type, pt_2d pos) {
	if (!add_unit_check(type, pos)) {
		return NULL;
	}

	Unit * unit = team->add_unit(_unit_types[type], _next_unit_id, pos);
	_next_unit_id++;
	add_unit_to_position_grid(unit);

	return unit;
}


void Map::add_river(pt_2d pos) {
	River * river = _elements->add_river(pos);
	if (river == NULL) {
		return;
	}

	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;

		std::vector<uint> id_nodes_buffered;

		AABB_2D * aabb = river->_polygon->_aabb->buffered(unit_type->buffer_size());
		std::vector<uint> id_nodes_buffered_aabb = _elevation->vertices_in_aabb(aabb);
		delete aabb;
		for (auto & id : id_nodes_buffered_aabb) {
			if (distance_poly_pt(river->_polygon, _elevation->id2pt_2d(id), NULL) < unit_type->buffer_size()) {
				id_nodes_buffered.push_back(id);
			}
		}
		Polygon2D * polygon_buffered = _elevation->ids2polygon(id_nodes_buffered);
		if (polygon_buffered == NULL) {
			std::cerr << "Map::add_river polygon_buffered NULL\n";
			continue;
		}

		std::vector<uint_pair> edges = _path_finder->edges_intersecting_polygon(polygon_buffered);
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
				data->_type[unit_type] = TERRAIN_RIVER;
			}
		}

		delete polygon_buffered;
	}

	sync2elevation();
}


void Map::add_lake(pt_2d pos) {
	Lake * lake = _elements->add_lake(pos);
	if (lake == NULL) {
		return;
	}

	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;

		std::vector<uint> id_nodes_buffered;

		if (unit_type->_terrain_weights[TERRAIN_LAKE] > MAX_UNIT_MOVING_WEIGHT) {
			AABB_2D * aabb = lake->_polygon->_aabb->buffered(unit_type->buffer_size());
			std::vector<uint> id_nodes_buffered_aabb = _elevation->vertices_in_aabb(aabb);
			delete aabb;
			for (auto & id : id_nodes_buffered_aabb) {
				if (distance_poly_pt(lake->_polygon, _elevation->id2pt_2d(id), NULL) < unit_type->buffer_size()) {
					id_nodes_buffered.push_back(id);
				}
			}
		}
		else {
			AABB_2D * aabb = lake->_polygon->_aabb->buffered(-1.0 * unit_type->buffer_size());
			if (aabb->_size.x < 0.0 || aabb->_size.y < 0.0) {
				delete aabb;
				continue;
			}

			std::vector<uint> id_nodes_buffered_aabb = _elevation->vertices_in_aabb(aabb);
			delete aabb;
			for (auto & id : id_nodes_buffered_aabb) {
				//if (distance_poly_pt(polygon, _elevation->id2pt_2d(id), NULL) < 0.01) {
				// PAS GENIAL ...
				if (is_pt_inside_poly(_elevation->id2pt_2d(id), lake->_polygon)) {
					id_nodes_buffered.push_back(id);
				}
			}
		}

		Polygon2D * polygon_buffered = _elevation->ids2polygon(id_nodes_buffered);
		if (polygon_buffered == NULL) {
			std::cerr << "Map::add_lake polygon_buffered NULL\n";
			continue;
		}

		std::vector<uint_pair> edges = _path_finder->edges_intersecting_polygon(polygon_buffered);
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
				data->_type[unit_type] = TERRAIN_LAKE;
			}
			else if (n_vertices_in_polygon == 1) {
				data->_type[unit_type] = TERRAIN_LAKE_COAST;
			}
		}

		delete polygon_buffered;
	}

	sync2elevation();
}


void Map::add_trees(std::string species_name, pt_2d pos, uint n_trees, number dispersion) {
	for (uint j=0; j<n_trees; ++j) {
		pt_2d pos_tree = rand_gaussian(pos, pt_2d(dispersion));
		if (!_elevation->in_boundaries(pos_tree)) {
			continue;
		}
		
		Tree * tree = _elements->add_tree(species_name, pos_tree);
		if (tree == NULL) {
			continue;
		}

		//AABB_2D * aabb = tree->_bbox->_aabb->aabb2d();
		//update_terrain_grid_with_aabb(aabb);
		//delete aabb;
		add_element_to_terrain_grid(tree);
	}
}


void Map::add_stones(std::string species_name, pt_2d pos, uint n_stones, number dispersion) {
	for (uint j=0; j<n_stones; ++j) {
		pt_2d pos_stone = rand_gaussian(pos, pt_2d(dispersion));
		if (!_elevation->in_boundaries(pos_stone)) {
			continue;
		}
		Stone * stone = _elements->add_stone(species_name, pos_stone);
		if (stone == NULL) {
			continue;
		}

		//AABB_2D * aabb = stone->_bbox->_aabb->aabb2d();
		//update_terrain_grid_with_aabb(stone->_bbox->_aabb->aabb2d());
		add_element_to_terrain_grid(stone);
		//delete aabb;
	}
}


Unit * Map::get_unit(uint unit_id) {
	for (auto & team : _teams) {
		for (auto & unit : team->_units) {
			if (unit->_id == unit_id) {
				return unit;
			}
		}
	}
	std::cerr << "Map::get_unit : unit_id " << unit_id << " non trouvée.\n";
	return NULL;
}


std::vector<Unit *> Map::get_units_in_aabb(AABB_2D * aabb) {
	std::vector<Unit *> result;
	for (auto & team : _teams) {
		std::vector<Unit *> l_units = team->get_units_in_aabb(aabb);
		result.insert(result.begin(), l_units.begin(), l_units.end());
	}
	return result;
}


Team * Map::get_team(std::string team_name) {
	for (auto & team : _teams) {
		if (team->_name == team_name) {
			return team;
		}
	}
	std::cerr << "Map::get_team : pas de team " << team_name << "\n";
	return NULL;
}


uint Map::get_team_idx(std::string team_name) {
	uint result = 0;
	for (auto & team : _teams) {
		if (team->_name == team_name) {
			return result;
		}
		result++;
	}
	std::cerr << "Map::get_team : pas de team " << team_name << "\n";
	return 0;
}


void Map::remove_units_in_aabb(AABB_2D * aabb) {
	std::vector<Unit *> units = get_units_in_aabb(aabb);
	for (auto & unit : units) {
		unit->_delete = true;
		remove_unit_from_position_grid(unit);
	}
	for (auto & team : _teams) {
		team->clear2delete();
	}
}


// si des éléments se superposent, au niveau grid la suppression ne sera pas nickel...
void Map::remove_elements_in_aabb(AABB_2D * aabb) {
	std::vector<Element *> elements = _elements->get_elements_in_aabb(aabb);
	for (auto & element : elements) {
		element->_delete = true;
		remove_element_from_terrain_grid(element);
	}
	_elements->clear2delete();
}


void Map::clear_units() {
	for (auto & team : _teams) {
		team->clear(true);
	}

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


void Map::clear_elements() {
	_elements->clear();

	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;

		_path_finder->_it_v= _path_finder->_vertices.begin();
		while (_path_finder->_it_v!= _path_finder->_vertices.end()) {
			_path_finder->_it_e = _path_finder->_vertices[_path_finder->_it_v->first]._edges.begin();
			while (_path_finder->_it_e != _path_finder->_vertices[_path_finder->_it_v->first]._edges.end()) {
				EdgeData * data = (EdgeData * )(_path_finder->_vertices[_path_finder->_it_v->first]._edges[_path_finder->_it_e->first]._data);
				data->_type[unit_type] = TERRAIN_UNKNOWN;
				_path_finder->_it_e++;
			}
			_path_finder->_it_v++;
		}
	}
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


void Map::update_elevation_grid() {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;

		_path_finder->_it_v= _path_finder->_vertices.begin();
		while (_path_finder->_it_v!= _path_finder->_vertices.end()) {
			_path_finder->_it_e= _path_finder->_it_v->second._edges.begin();
			while (_path_finder->_it_e!= _path_finder->_it_v->second._edges.end()) {
				GraphEdge & edge = _path_finder->_vertices[_path_finder->_it_v->first]._edges[_path_finder->_it_e->first];
				pt_3d & pt_begin= _path_finder->_it_v->second._pos;
				pt_3d & pt_end= _path_finder->_vertices[_path_finder->_it_e->first]._pos;
				EdgeData * data = (EdgeData *)(edge._data);
				data->_delta_elevation[unit_type] = unit_type->elevation_coeff(pt_end.z - pt_begin.z);

				_path_finder->_it_e++;
			}
			_path_finder->_it_v++;
		}
	}
}


void Map::update_terrain_grid_with_elevation() {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;

		_path_finder->_it_v= _path_finder->_vertices.begin();
		while (_path_finder->_it_v!= _path_finder->_vertices.end()) {
			_path_finder->_it_e= _path_finder->_it_v->second._edges.begin();
			while (_path_finder->_it_e!= _path_finder->_it_v->second._edges.end()) {
				GraphEdge & edge = _path_finder->_vertices[_path_finder->_it_v->first]._edges[_path_finder->_it_e->first];
				pt_3d & pt_begin = _path_finder->_it_v->second._pos;
				pt_3d & pt_end = _path_finder->_vertices[_path_finder->_it_e->first]._pos;
				EdgeData * data = (EdgeData *)(edge._data);

				if (pt_begin.z < 0.01 && pt_end.z < 0.01) {
					data->_type[unit_type] = TERRAIN_SEA;
				}
				else if ((pt_begin.z < 0.01 && pt_end.z > 0.01) || (pt_begin.z > 0.01 && pt_end.z < 0.01)) {
					data->_type[unit_type] = TERRAIN_SEA_COAST;
				}
				else if (data->_type[unit_type] == TERRAIN_UNKNOWN || data->_type[unit_type] == TERRAIN_SEA || data->_type[unit_type] == TERRAIN_SEA_COAST) {
					data->_type[unit_type] = TERRAIN_GROUND;
				}

				_path_finder->_it_e++;
			}
			_path_finder->_it_v++;
		}
	}
}


void Map::add_element_to_terrain_grid(Element * element) {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;
		AABB_2D * aabb_buffered = element->_bbox->_aabb->aabb2d()->buffered(unit_type->buffer_size());

		std::vector<uint_pair> edges = _path_finder->edges_intersecting_aabb(aabb_buffered);
		for (auto & e : edges) {
			GraphEdge & edge = _path_finder->_vertices[e.first]._edges[e.second];
			EdgeData * data = (EdgeData *)(edge._data);
			data->_type[unit_type] = TERRAIN_OBSTACLE;
		}
	}
}


void Map::remove_element_from_terrain_grid(Element * element) {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;
		AABB_2D * aabb_buffered = element->_bbox->_aabb->aabb2d()->buffered(unit_type->buffer_size());

		std::vector<uint_pair> edges = _path_finder->edges_intersecting_aabb(aabb_buffered);
		for (auto & e : edges) {
			GraphEdge & edge = _path_finder->_vertices[e.first]._edges[e.second];
			EdgeData * data = (EdgeData *)(edge._data);
			data->_type[unit_type] = TERRAIN_GROUND;
		}
	}
}


void Map::sync2elevation() {
	update_alti_grid();
	update_elevation_grid();
	update_terrain_grid_with_elevation();
}


std::vector<uint_pair> Map::waiting_unit_positions_edges(Unit * unit, UnitType * unit_type) {
	BBox_2D * bbox = unit->_bbox->bbox2d();
	BBox_2D * bbox_buffered = bbox->buffered(unit_type->buffer_size());
	std::vector<uint_pair> edges = _path_finder->edges_intersecting_bbox(bbox_buffered);
	delete bbox;
	delete bbox_buffered;

	return edges;
}


void Map::fill_unit_path_edges(Unit * unit) {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;
		std::vector<PathInterval *> intervals = unit->_path->get_intervals();
		
		for (auto & interval : intervals) {
			BBox_2D * buffered_bbox = interval->_bbox->buffered(unit_type->buffer_size());
			std::vector<uint_pair> path_edges = _path_finder->edges_intersecting_bbox(buffered_bbox);
			interval->_edges[unit_type].insert(interval->_edges[unit_type].end(), path_edges.begin(), path_edges.end());
			delete buffered_bbox;
		}
	}
}


void Map::add_unit_to_position_grid(Unit * unit) {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;
		if (unit->_path->empty()) {
			for (auto & e : waiting_unit_positions_edges(unit, unit_type)) {
				GraphEdge & edge = _path_finder->_vertices[e.first]._edges[e.second];
				EdgeData * data = (EdgeData *)(edge._data);
				data->_ids[unit_type].insert(unit->_id);
			}
		}
		else {
			for (auto & interval : unit->_path->get_intervals()) {
				if (!interval->_active) {
					continue;
				}
				for (auto e : interval->_edges[unit_type]) {
					GraphEdge edge = _path_finder->get_edge(e);
					EdgeData * data = (EdgeData *)(edge._data);
					data->_ids[unit_type].insert(unit->_id);
				}
			}
		}
	}
}


void Map::remove_unit_from_position_grid(Unit * unit) {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;
		if (unit->_path->empty()) {
			for (auto & e : waiting_unit_positions_edges(unit, unit_type)) {
				GraphEdge & edge = _path_finder->_vertices[e.first]._edges[e.second];
				EdgeData * data = (EdgeData *)(edge._data);
				data->_ids[unit_type].erase(unit->_id);
			}
		}
		else {
			for (auto & interval : unit->_path->get_intervals()) {
				if (!interval->_active) {
					continue;
				}
				for (auto e : interval->_edges[unit_type]) {
					GraphEdge edge = _path_finder->get_edge(e);
					EdgeData * data = (EdgeData *)(edge._data);
					data->_ids[unit_type].erase(unit->_id);
				}
			}
		}
	}
}


void Map::advance_unit_in_position_grid(Unit * unit) {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;

		for (auto e : unit->_path->get_current_interval()->_edges[unit_type]) {
			GraphEdge edge = _path_finder->get_edge(e);
			EdgeData * data = (EdgeData *)(edge._data);
			data->_ids[unit_type].erase(unit->_id);
		}
	}
	
	unit->_path->next_checkpoint();
	
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;
	
		for (auto e : unit->_path->get_last_active_interval()->_edges[unit_type]) {
			GraphEdge edge = _path_finder->get_edge(e);
			EdgeData * data = (EdgeData *)(edge._data);
			data->_ids[unit_type].insert(unit->_id);
		}
	}
}


void Map::path_find() {
	PathFinderInput * pfi;
	while (_path_find_thr_running) {
		if (_path_finder_computing) {
			continue;
		}
		if (_path_queue_thr_input.next(pfi)) {
			_path_finder_computing = true;
			_path_finder->path_find(pfi, &_path_queue_thr_output);
		}
	}
}


void Map::pause_all_units(bool pause) {
	for (auto & team : _teams) {
		for (auto & unit : team->_units) {
			unit->_paused = pause;
		}
	}
}


void Map::anim(time_point t) {
	UnitPath * unit_path;
	// TODO : ou doit se faire la destruction de unit_path ?
	if (_path_queue_thr_output.next(unit_path)) {
		if (unit_path->_status == UNIT_PATH_COMPUTING_SUCCESS) {
			Unit * unit = get_unit(unit_path->_unit_id);
			if (unit != NULL) {
				remove_unit_from_position_grid(unit);
				unit->_path->copy_path(unit_path);
				fill_unit_path_edges(unit);
				add_unit_to_position_grid(unit);
				unit->update_alti_path();
				unit->set_status(MOVING, t);
			}
		}
		else if (unit_path->_status == UNIT_PATH_COMPUTING_FAILED) {
			
		}
		_path_finder_computing = false;
	}

	for (auto & team : _teams) {
		if (!team->_ia) {
			continue;
		}

		for (auto & unit : team->_units) {
			if (unit->_status == WAITING) {
				//std::cout << "ia : " << unit->_id << "\n";
				
				bool found_target = false;
				for (auto & team2 : _teams) {
					if (team2 == team) {
						continue;
					}
					for (auto & unit2 : team2->_units) {
						if (team->is_target_reachable(unit, unit2)) {
							team->unit_attack(unit, unit2, t);
							found_target = true;
							break;
						}
					}
					if (found_target) {
						break;
					}
				}
			}
		}
	}

	for (auto & team : _teams) {
		for (auto & unit : team->_units) {
			if (unit->_status == MOVING) {
				if (unit->last_checkpoint_checked()) {
					remove_unit_from_position_grid(unit);
					unit->set_status(WAITING, t);
					add_unit_to_position_grid(unit);
				}
				else if (unit->checkpoint_checked()) {
					advance_unit_in_position_grid(unit);
				}
				else {
				}
			}

			unit->anim(t);

			if (!unit->_instructions.empty()) {
				Instruction i = unit->_instructions.front();
				if (i._t <= t) {
					unit->_instructions.pop();
					// TODO : ou doit se faire la destruction de pfi ?
					PathFinderInput * pfi = new PathFinderInput();
					pfi->_unit_type = unit->_type;
					pfi->_unit_id = unit->_id;
					pfi->_start = unit->_position;
					pfi->_goal = i._destination;
					_path_queue_thr_input.push(pfi);
				}
			}
			else if (unit->_status == SHOOTING) {
				_ammos.push_back(new Ammo(unit->_type->_ammo_type, unit->_position, unit->_target->_position));
				unit->set_status(ATTACKING, t);
			}
		}
	}

	collisions(t);

	for (auto & ammo : _ammos) {
		ammo->anim();
		if (ammo->_target_hit) {
			for (auto & team : _teams) {
				for (auto & unit : team->_units) {
					if (pt_in_bbox2d(pt_2d(ammo->_target), unit->_bbox->bbox2d())) {
						unit->hit(ammo, t);
					}
				}
			}
		}
	}

	_ammos.erase(std::remove_if(_ammos.begin(), _ammos.end(), [](Ammo * w) {
		return w->_target_hit;
	}), _ammos.end());


	for (auto & team : _teams) {
		for (auto & unit : team->_units) {
			if (unit->_status == ATTACKING && (unit->_target->_status == DESTROYED || unit->_target->_hit_status == FINAL_HIT)) {
				unit->set_status(WAITING, t);
			}
		}
	}

	for (auto & team : _teams) {
		for (auto & unit : team->_units) {
			if (unit->_status == DESTROYED) {
				unit->_delete = true;
				remove_unit_from_position_grid(unit);
			}
		}
		team->clear2delete();
	}

	for(auto & team : _teams) {
		team->update_fow();
	}
}


void Map::collisions(time_point t) {
	bool verbose = true;

	std::vector<Unit *> units;
	for (auto & team : _teams) {
		units.insert(units.end(), team->_units.begin(), team->_units.end());
	}
	if (units.empty()) {
		return;
	}

	for (auto & unit1 : units) {
		if (unit1->_path->empty()) {
			continue;
		}

		bool future_collision = false;

		std::vector<BBox_2D *> bboxs1;
		for (auto & interval : unit1->_path->get_intervals()) {
			if (!interval->_active) {
				continue;
			}
			bboxs1.push_back(interval->_bbox);
		}
		
		for (auto & unit2 : units) {
			if (unit2 == unit1) {
				continue;
			}

			std::vector<BBox_2D *> bboxs2;
			if (unit2->_path->empty()) {
				bboxs2.push_back(unit2->_bbox->bbox2d());
			}
			else {
				for (auto & interval : unit2->_path->get_intervals()) {
					if (!interval->_active) {
						continue;
					}
					bboxs2.push_back(interval->_bbox);
				}
			}

			for (auto & bbox1 : bboxs1) {
				for (auto & bbox2 : bboxs2) {
					//if (aabb2d_intersects_aabb2d(bbox1->_aabb, bbox2->_aabb)) {
					if (bbox2d_intersects_bbox2d(bbox1, bbox2)) {
						future_collision = true;
						break;
					}
				}
				if (future_collision) {
					break;
				}
			}

			if (future_collision) {
				if (verbose) {
					std::cout << "unit " << unit1->_id << " will collide " << unit2->_id << " ; -> stop.\n";
				}
				
				//unit1->_instructions.push({unit1->_path->_goal, t + std::chrono::milliseconds(rand_int(2000, 5000))});
				remove_unit_from_position_grid(unit1);
				unit1->set_status(WAITING, t);
				add_unit_to_position_grid(unit1);
				break;
			}
		}
	}
}


void Map::clear() {
	clear_units();
	clear_elements();

	_elevation->set_alti_all(1.0);
	
	sync2elevation();
}



void Map::randomize() {
	clear();
	
	_elevation->randomize();
	
	sync2elevation();

	for (uint i=0; i<50; ++i) {
		pt_2d pt = rand_gaussian(_elevation->_origin + 0.5 * _elevation->_size, 0.3 * _elevation->_size);
		if (!_elevation->in_boundaries(pt)) {
			continue;
		}
		number alti = _elevation->get_alti(pt);
		for (auto & tree_species : _elements->_tree_species) {
			if (alti > tree_species.second->_alti_min && alti < tree_species.second->_alti_max) {
				add_trees(tree_species.first, pt, 20, 3.0);
				break;
			}
		}
	}

	for (uint i=0; i<50; ++i) {
		pt_2d pt = rand_gaussian(_elevation->_origin + 0.5 * _elevation->_size, 0.3 * _elevation->_size);
		if (!_elevation->in_boundaries(pt)) {
			continue;
		}
		number alti = _elevation->get_alti(pt);
		for (auto & stone_species : _elements->_stone_species) {
			if (alti > stone_species.second->_alti_min && alti < stone_species.second->_alti_max) {
				add_stones(stone_species.first, pt, 20, 3.0);
				break;
			}
		}
	}
}


void Map::save(std::string json_path) {
	json js;

	/*js["Elevation"] = json::array();
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
		position.push_back(unit->_bbox->_aabb->center().x);
		position.push_back(unit->_bbox->_aabb->center().y);
		//position.push_back(unit->_aabb->center().z);
		entry["position"]= position;
		js["units"].push_back(entry);
	}*/

	std::ofstream ofs(json_path);
	ofs << js << "\n";
}


void Map::load(std::string json_path) {
	/*clear();

	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	uint idx = 0;
	for (auto & alti : js["Elevation"]) {
		_elevation->set_alti(idx, alti);
		idx++;
	}

	for (auto & unit : js["units"]) {
		add_unit(unit["type"], pt_2d(unit["position"][0], unit["position"][1]));
	}

	sync2elevation();*/
}


std::ostream & operator << (std::ostream & os, Map & map) {
	os << "unit_types = ";
	for (auto & ut : map._unit_types) {
		os << *ut.second << "\n";
	}
	os << "teams = ";
	for (auto & team : map._teams) {
		os << *team << "\n";
	}
	return os;
}

