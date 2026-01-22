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


Map::Map() {

}


Map::Map(std::string unit_types_dir, std::string elements_dir, pt_2d origin, pt_2d size, pt_2d path_resolution, pt_2d elevation_resolution, time_point t) {
	uint n_ligs_path = uint(size.y / path_resolution.y) + 1;
	uint n_cols_path = uint(size.x / path_resolution.x) + 1;

	uint n_ligs_elevation = uint(size.y / elevation_resolution.y) + 1;
	uint n_cols_elevation = uint(size.x / elevation_resolution.x) + 1;

	_elevation = new Elevation(origin, size, n_ligs_elevation, n_cols_elevation);

	_path_finder = new PathFinder(origin, size, n_ligs_path, n_cols_path);

	std::vector<std::string> jsons_paths = list_files(unit_types_dir, "json");
	for (auto & json_path : jsons_paths) {
		UnitType * unit_type = new UnitType(json_path);
		_unit_types[str2unit_type(str_to_upper(basename(json_path)))] = unit_type;
		_path_finder->add_unit_type(unit_type);
		//_unit_groups[unit_type] = new UnitGroup();
	}
	//std::cout << *_unit_types["infantery"]->_obj_data << "\n";

	_elements = new Elements(elements_dir + "/tree_species", elements_dir + "/stone_species", _elevation);

	_teams.push_back(new Team("Team 1", _elevation));
	_teams.push_back(new Team("Team 2", _elevation));

	sync2elevation();
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

	delete _elevation;
}


void Map::add_unit(Team * team, UNIT_TYPE type, pt_2d pos) {
	Unit * unit = team->add_unit(_unit_types[type], _next_unit_id, pos);
	if (unit == NULL) {
		return;
	}
	_next_unit_id++;
	add_waiting_unit_to_position_grid(unit);
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

		AABB_2D * aabb = tree->_bbox->_aabb->aabb2d();
		update_terrain_grid_with_aabb(aabb);
		delete aabb;
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

		AABB_2D * aabb = stone->_bbox->_aabb->aabb2d();
		update_terrain_grid_with_aabb(stone->_bbox->_aabb->aabb2d());
		delete aabb;
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


// TODO : ne fonctionne pas on ne voit pas le path ...
void Map::update_alti_path(Unit * unit) {
	for (auto & pt : unit->_path->_pts) {
		pt.z = _elevation->get_alti(pt);
		if (pt.z < 0.0 && unit->_type->_floats) {
			pt.z = 0.01;
		}
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

		/*std::vector<uint_pair> edges;
		if (bbox != NULL) {
			edges = _path_finder->edges_intersecting_bbox(bbox);
		}*/

		_path_finder->_it_v= _path_finder->_vertices.begin();
		while (_path_finder->_it_v!= _path_finder->_vertices.end()) {
			_path_finder->_it_e= _path_finder->_it_v->second._edges.begin();
			while (_path_finder->_it_e!= _path_finder->_it_v->second._edges.end()) {
				/*if (bbox != NULL && std::find(edges.begin(), edges.end(), std::make_pair(_path_finder->_it_v->first, _path_finder->_it_e->first)) == edges.end()) {
					continue;
				}*/

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


void Map::update_terrain_grid_with_aabb(AABB_2D * aabb) {
	for (auto & ut : _unit_types) {
		UnitType * unit_type = ut.second;
		AABB_2D * aabb_buffered = aabb->buffered(unit_type->buffer_size());

		std::vector<uint_pair> edges = _path_finder->edges_intersecting_aabb(aabb_buffered);
		for (auto & e : edges) {
			GraphEdge & edge = _path_finder->_vertices[e.first]._edges[e.second];
			EdgeData * data = (EdgeData *)(edge._data);
			data->_type[unit_type] = TERRAIN_OBSTACLE;
		}
	}
}


void Map::sync2elevation() {
	update_alti_grid();
	update_elevation_grid();
	update_terrain_grid_with_elevation();
}


void Map::clear_units_position_grid() {
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


std::vector<uint_pair> Map::waiting_unit_positions_edges(Unit * unit, UnitType * unit_type) {
	AABB_2D * aabb = new AABB_2D(pt_2d(unit->_bbox->_aabb->_vmin - 0.5 * unit_type->buffer_size()), pt_2d(unit->_bbox->_aabb->size() + unit_type->buffer_size()));
	std::vector<uint_pair> edges = _path_finder->edges_intersecting_aabb(aabb);
	delete aabb;

	return edges;
}


std::vector<uint_pair> Map::moving_unit_positions_edges(Unit * unit, UnitType * unit_type, bool all) {
	AABB_2D * aabb_unit = unit->_bbox->_aabb->aabb2d();

	std::vector<uint_pair> edges;
	bool intersection_happened = false;

	AABB_2D * aabb_start = new AABB_2D(unit->_path->_start - 0.5 * pt_2d(unit_type->buffer_size()), pt_2d(unit->_bbox->_aabb->size() + unit_type->buffer_size()));
	if (!all && !intersection_happened) {
		if (aabb2d_intersects_aabb2d(aabb_unit, aabb_start)) {
			intersection_happened = true;
		}
	}
	if (all || !intersection_happened) {
		std::vector<uint_pair> start_edges = _path_finder->edges_intersecting_aabb(aabb_start);
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
			std::vector<uint_pair> path_edges = _path_finder->edges_intersecting_bbox(buffered_bbox);
			edges.insert(edges.end(), path_edges.begin(), path_edges.end());
		}
		delete buffered_bbox;
	}

	AABB_2D * aabb_goal = new AABB_2D(unit->_path->_goal - pt_2d(unit->_type->buffer_size()- 0.5 * unit_type->buffer_size()), pt_2d(unit->_type->buffer_size() + unit_type->buffer_size()));
	if (!all && !intersection_happened) {
		if (aabb2d_intersects_aabb2d(aabb_unit, aabb_goal)) {
			intersection_happened = true;
		}
	}
	if (all || !intersection_happened) {
		std::vector<uint_pair> goal_edges = _path_finder->edges_intersecting_aabb(aabb_goal);
		edges.insert(edges.end(), goal_edges.begin(), goal_edges.end());
	}
	delete aabb_goal;

	delete aabb_unit;
	
	return edges;
}


void Map::add_waiting_unit_to_position_grid(Unit * unit) {
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


void Map::remove_waiting_unit_from_position_grid(Unit * unit) {
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


void Map::add_moving_unit_to_position_grid(Unit * unit) {
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


void Map::remove_moving_unit_from_position_grid(Unit * unit, bool all) {
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


void Map::pause_all_units(bool pause) {
	for (auto & team : _teams) {
		for (auto & unit : team->_units) {
			unit->_paused = pause;
		}
	}
}


void Map::clear() {
	clear_units_position_grid();
	
	for (auto & team : _teams) {
		team->clear();
	}

	_elements->clear();

	_elevation->set_alti_all(-0.01);
	
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
	bool path_find_thr_active = false;

	for (auto & team : _teams) {
		for (auto & unit : team->_units) {
			std::mutex mtx;
			mtx.lock();
			UNIT_STATUS status = unit->_status;
			mtx.unlock();

			if (status == COMPUTING_PATH) {
				continue;
			}

			if (status == COMPUTING_PATH_DONE) {
				path_find_thr_active = false;
				_path_find_thr.join();
				update_alti_path(unit);
				unit->set_status(MOVING);
				remove_waiting_unit_from_position_grid(unit);
				add_moving_unit_to_position_grid(unit);
			}
			else if (status == COMPUTING_PATH_FAILED) {
				path_find_thr_active = false;
				_path_find_thr.join();
				unit->_instructions.push({unit->_path->_goal, t + std::chrono::milliseconds(1000)});
				unit->set_status(WAITING);
			}
			else if (status == CHECKPOINT_CHECKED) {
				remove_moving_unit_from_position_grid(unit, false);
				unit->_path->_idx_path++;
				unit->set_status(MOVING);
			}
			else if (status == LAST_CHECKPOINT_CHECKED) {
				remove_moving_unit_from_position_grid(unit, true);
				add_waiting_unit_to_position_grid(unit);
				unit->set_status(WAITING);
			}
			else if (status == MOVING) {
				unit->anim(_elevation);
				//_unit_groups[unit->_type]->update_unit(unit);
				
				/*for (auto & unit2 : _units) {
					if (unit2 == unit) {
						continue;
					}

					if (aabb_intersects_aabb(unit->_bbox->_aabb, unit2->_bbox->_aabb)) {
						std::cerr << "intersection unit " << unit->_id << " et " << unit2->_id << "\n";
					}
				}*/
			}

			if (!path_find_thr_active && !unit->_instructions.empty()) {
				Instruction i = unit->_instructions.front();
				if (i._t <= t) {
					unit->_instructions.pop();

					pt_2d pt = i._destination;

					unit->set_status(COMPUTING_PATH);
					_path_find_thr= std::thread(&Map::path_find, this, unit, pt);
					path_find_thr_active = true;
				}
			}
		}
	}
}


void Map::selected_units_goto(pt_2d pt, time_point t) {
	uint compt = 0;
	for (auto & team : _teams) {
		for (auto & unit : team->_units) {
			if (unit->_selected) {
				//unit->_instructions.push({pt, t + std::chrono::milliseconds(500 * compt)});
				unit->_instructions.push({pt, t});
				compt++;
			}
		}
	}
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

