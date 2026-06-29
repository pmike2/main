#include <queue>
#include <fstream>
#include <tuple>
#include <algorithm>
#include <filesystem>

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


Map::Map(std::string unit_types_dir, std::string ammo_types_dir, std::string elements_dir, std::string explosion_dir, pt_2d origin, pt_2d size, pt_2d path_resolution, pt_2d elevation_resolution, pt_2d fow_resolution, time_point t) :
	_unit_types_dir(unit_types_dir), _ammo_types_dir(ammo_types_dir), _elements_dir(elements_dir), 
	_path_resolution(path_resolution), _elevation_resolution(elevation_resolution), _fow_resolution(fow_resolution)
{
	bool verbose = false;

	_aabb = new AABB_2D(origin, size);

	// ------------------------------------------------
	if (verbose) {
		std::cout << "init Path Finder\n";
	}
	uint n_ligs = uint(size.y / _path_resolution.y) + 1;
	uint n_cols = uint(size.x / _path_resolution.x) + 1;
	_path_finder = new PathFinder(origin, size, n_ligs, n_cols, t);

	// ------------------------------------------------
	if (verbose) {
		std::cout << "init Elevation\n";
	}
	uint n_ligs_elevation = uint(size.y / _elevation_resolution.y) + 1;
	uint n_cols_elevation = uint(size.x / _elevation_resolution.x) + 1;
	_elevation = new Elevation(origin, size, n_ligs_elevation, n_cols_elevation);

	// ------------------------------------------------
	if (verbose) {
		std::cout << "init UnitTypes / AmmoTypes / Explosion\n";
	}
	std::vector<std::string> unit_type_json_paths = list_files(_unit_types_dir, "json");
	for (auto & json_path : unit_type_json_paths) {
		UnitType * unit_type = new UnitType(json_path);
		_unit_types[unit_type->_name] = unit_type;
		_path_finder->_gmo_types.push_back((GridMovingObjectType *)(unit_type));
	}
	
	std::vector<std::string> ammo_type_json_paths = list_files(_ammo_types_dir, "json");
	for (auto & json_path : ammo_type_json_paths) {
		AmmoType * ammo_type = new AmmoType(json_path);
		_ammo_types[ammo_type->_name] = ammo_type;
	}

	for (auto & unit_type : _unit_types) {
		unit_type.second->_ammo_type = _ammo_types[unit_type.second->_ammo_type_str];
	}

	_explosion_system = new ExplosionSystem(explosion_dir);
	for (auto & ammo_type : _ammo_types) {
		ammo_type.second->_explosion_config = _explosion_system->_configs[ammo_type.second->_explosion_config_str];
	}

	// ------------------------------------------------
	if (verbose) {
		std::cout << "init Elements\n";
	}
	_elements = new Elements(_elements_dir + "/tree_species", elements_dir + "/stone_species", _elevation);

	// ------------------------------------------------
	if (verbose) {
		std::cout << "init Teams\n";
	}
	_teams.push_back(new Team("Team1", glm::vec3(1.0f, 0.0f, 0.0f), _elevation, _fow_resolution));
	_teams.push_back(new Team("Team2", glm::vec3(0.0f, 0.0f, 1.0f), _elevation, _fow_resolution));
}


Map::~Map() {
	clear_units();
	clear_elements();

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

	delete _aabb;

	delete _explosion_system;
}


bool Map::fow_check(Team * team, pt_2d pos) {
	if (!point_in_aabb2d(pos, _aabb)) {
		return false;
	}

	std::vector<uint> vertices = team->_fow->vertices_in_cell_containing_pt(pos);
	for (auto & v : vertices) {
		GraphVertex vertex = team->_fow->get_vertex(v);
		FowVertexData * data = (FowVertexData *)(vertex._data);
		if (data->_status == UNDISCOVERED) {
			return false;
		}
	}
	return true;
}


bool Map::construction_check(Team * team, std::string type) {
	if (team->get_unit_under_construction(_unit_types[type]) != NULL) {
		return false;
	}
	return true;
}


bool Map::add_unit_check(Team * team, std::string type, pt_2d pos, bool fow_active, bool construction_active) {
	if (team->_units.size() >= N_MAX_UNITS_PER_TEAM) {
		return false;
	}

	if (!point_in_aabb2d(pos, _aabb)) {
		return false;
	}

	if (fow_active && !fow_check(team, pos)) {
		return false;
	}

	if (construction_active && !construction_check(team, type)) {
		return false;
	}

	pt_2d unit_size = _unit_types[type]->get_max_square_size();
	AABB_2D * aabb = new AABB_2D(pos - 0.5 * unit_size, unit_size);
	// petit buffer
	aabb->buffer(1.5);
	std::vector<uint> vertices = _path_finder->vertices_in_aabb(aabb);
	delete aabb;
	for (auto & v : vertices) {
		if (_path_finder->is_vertex_obstacle(_unit_types[type]->_name, v)) {
			return false;
		}
	}

	return true;
}


bool Map::move_unit_check(Unit * unit, pt_2d pos, bool fow_active) {
	if (fow_active && !fow_check(unit->_team, pos)) {
		return false;
	}

	pt_2d unit_size = unit->_type->get_max_square_size();
	AABB_2D * aabb = new AABB_2D(pos - 0.5 * unit_size, unit_size);
	std::vector<uint> vertices = _path_finder->vertices_in_aabb(aabb);
	delete aabb;
	for (auto & v : vertices) {
		if (_path_finder->is_vertex_obstacle(unit->_type->_name, v, unit)) {
			return false;
		}
	}
	return true;
}


bool Map::attack_unit_check(Unit * attacking_unit, Unit * attacked_unit, bool fow_active) {
	if (fow_active && !fow_check(attacking_unit->_team, pt_2d(attacked_unit->_position))) {
		return false;
	}
	return true;
}


Unit * Map::add_unit(Team * team, std::string type, pt_2d pos, time_point t) {
	Unit * unit = team->add_unit(_unit_types[type], pos, t);
	_path_finder->init_gmo(unit);

	return unit;
}


void Map::add_first_units2teams(time_point t) {
	for (auto & team : _teams) {
		if (!team->empty()) {
			continue;
		}

		uint compt = 0;
		while (true) {
			compt++;
			if (compt > 100) {
				std::cerr << "Map::add_first_units2teams : impossible de trouver un endroit\n";
				break;
			}

			// buffer pour ne pas être sur les bords
			pt_2d position = rand_pt_2d(_aabb->_pos + 0.1 * _aabb->_size, _aabb->_pos + 0.9 * _aabb->_size);
			if (add_unit_check(team, "infantery", position, false, false)) {
				add_unit(team, "infantery", position, t);
				break;
			}
		}
	}
}


void Map::add_river(pt_2d pos) {
	River * river = _elements->add_river(pos);
	if (river == NULL) {
		return;
	}

	_path_finder->set_vertex(graph_id_convert(_elevation, _path_finder, river->_id_nodes), "river");
}


void Map::add_lake(pt_2d pos) {
	Lake * lake = _elements->add_lake(pos);
	if (lake == NULL) {
		return;
	}

	_path_finder->set_vertex(graph_id_convert(_elevation, _path_finder, lake->_id_nodes), "lake");
}


void Map::add_tree(std::string species_name, pt_2d pos) {
	Tree * tree = _elements->add_tree(species_name, pos);
	if (tree == NULL) {
		return;
	}

	_path_finder->set_vertex(graph_id_convert(_elevation, _path_finder, tree->_id_nodes), "tree");
}


void Map::add_trees(std::string species_name, pt_2d pos, uint n_trees, number dispersion) {
	for (uint j=0; j<n_trees; ++j) {
		pt_2d pos_tree = rand_gaussian(pos, pt_2d(dispersion));
		add_tree(species_name, pos_tree);
	}
}


void Map::add_stone(std::string species_name, pt_2d pos) {
	Stone * stone = _elements->add_stone(species_name, pos);
	if (stone == NULL) {
		return;
	}

	_path_finder->set_vertex(graph_id_convert(_elevation, _path_finder, stone->_id_nodes), "stone");
}


void Map::add_stones(std::string species_name, pt_2d pos, uint n_stones, number dispersion) {
	for (uint j=0; j<n_stones; ++j) {
		pt_2d pos_stone = rand_gaussian(pos, pt_2d(dispersion));
		add_stone(species_name, pos_stone);
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


void Map::selected_units_goto(Team * team, pt_3d pt) {
	for (auto & unit : team->_units) {
		if (unit->_selected) {
			_path_finder->goto_gmo(unit, pt_2d(pt), false);
		}
	}
}


void Map::remove_units_in_aabb(AABB_2D * aabb) {
	std::vector<Unit *> units = get_units_in_aabb(aabb);
	for (auto & unit : units) {
		unit->_unit_status = UNIT_DESTROYED;
	}
}


void Map::remove_elements_in_aabb(AABB_2D * aabb) {
	std::vector<Element *> elements = _elements->get_elements_in_aabb(aabb);
	for (auto & element : elements) {
		element->_delete = true;
		for (auto & v : graph_id_convert(_elevation, _path_finder, element->_id_nodes)) {
			pt_3d pt = _path_finder->id2pt_3d(v);
			if (pt.z < 0.01) {
				_path_finder->set_vertex(v, "sea");
			}
			else {
				_path_finder->set_vertex(v, "land");
			}
		}
	}
	_elements->clear2delete();
}


void Map::clear_units() {
	_path_finder->clear();
	for (auto & team : _teams) {
		team->clear();
	}
}


void Map::pause_all_units(bool pause) {
	for (auto & team : _teams) {
		for (auto & unit : team->_units) {
			unit->_paused = pause;
		}
	}
}


void Map::clear_elements() {
	_elements->clear();
	
	_path_finder->_it_v= _path_finder->_vertices.begin();
	while (_path_finder->_it_v!= _path_finder->_vertices.end()) {
		PathFinderVertexData * vertex_data = _path_finder->get_vertex_data(_path_finder->_it_v->first);
		pt_3d & pt = _path_finder->_it_v->second._pos;
		if (vertex_data->_type == "tree" || vertex_data->_type == "stone" || vertex_data->_type == "river" || vertex_data->_type == "lake") {
			if (pt.z < 0.01) {
				vertex_data->_type = "sea";
			}
			else {
				vertex_data->_type = "land";
			}
		}
		_path_finder->_it_v++;
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
	_path_finder->_it_v= _path_finder->_vertices.begin();
	while (_path_finder->_it_v!= _path_finder->_vertices.end()) {
		_path_finder->_it_e= _path_finder->_it_v->second._edges.begin();
		while (_path_finder->_it_e!= _path_finder->_it_v->second._edges.end()) {
			pt_3d & pt_begin= _path_finder->_it_v->second._pos;
			pt_3d & pt_end= _path_finder->_vertices[_path_finder->_it_e->first]._pos;
			number delta_elevation = pt_end.z - pt_begin.z;
			PathFinderEdgeData * edge_data = _path_finder->get_edge_data(_path_finder->_it_v->first, _path_finder->_it_e->first);
			
			if (delta_elevation < -2.0) {
				edge_data->_type = "hard_down";
			}
			else if (delta_elevation < -1.0) {
				edge_data->_type = "down";
			}
			else if (delta_elevation < 0.1) {
				edge_data->_type = "flat";
			}
			else if (delta_elevation < 2.0) {
				edge_data->_type = "up";
			}
			else {
				edge_data->_type = "hard_up";
			}

			_path_finder->_it_e++;
		}
		_path_finder->_it_v++;
	}
}


void Map::update_terrain_grid_with_elevation() {
	_path_finder->_it_v= _path_finder->_vertices.begin();
	while (_path_finder->_it_v!= _path_finder->_vertices.end()) {
		PathFinderVertexData * vertex_data = _path_finder->get_vertex_data(_path_finder->_it_v->first);
		pt_3d & pt = _path_finder->_it_v->second._pos;
		if (pt.z < 0.01) {
			vertex_data->_type = "sea";
		}
		else if (vertex_data->_type == "sea") {
		//else {
			vertex_data->_type = "land";
		}
		
		/*_path_finder->_it_e= _path_finder->_it_v->second._edges.begin();
		while (_path_finder->_it_e!= _path_finder->_it_v->second._edges.end()) {
			PathFinderEdgeData * edge_data = _path_finder->get_edge_data(_path_finder->_it_v->first, _path_finder->_it_e->first);
			pt_3d & pt_to = _path_finder->_vertices[_path_finder->_it_e->first]._pos;
			number delta_alti = pt_to.z - pt_from.z;
			
			TERRAIN_TYPE terrain_type = _path_finder->get_terrain_type(_path_finder->_it_v->first, _path_finder->_it_e->first, unit_type);

			if (pt_begin.z < 0.01 && pt_end.z < 0.01) {
				_path_finder->set_terrain_type(_path_finder->_it_v->first, _path_finder->_it_e->first, unit_type, TERRAIN_SEA);
			}
			else if ((pt_begin.z < 0.01 && pt_end.z > 0.01) || (pt_begin.z > 0.01 && pt_end.z < 0.01)) {
				_path_finder->set_terrain_type(_path_finder->_it_v->first, _path_finder->_it_e->first, unit_type, TERRAIN_SEA_COAST);
			}
			else if (terrain_type == TERRAIN_UNKNOWN || terrain_type == TERRAIN_SEA || terrain_type == TERRAIN_SEA_COAST) {
				_path_finder->set_terrain_type(_path_finder->_it_v->first, _path_finder->_it_e->first, unit_type, TERRAIN_GROUND);
			}

			_path_finder->_it_e++;
		}*/
		_path_finder->_it_v++;
	}
}


void Map::sync2elevation() {
	update_alti_grid();
	update_elevation_grid();
	update_terrain_grid_with_elevation();
}


void Map::anim_unit(Unit * unit, time_point t) {
	bool verbose = false;

	if (unit->_unit_status == UNIT_INACTIVE) {
		return;
	}
	
	if (unit->_unit_status == UNIT_DESTROYED) {
		unit->_unit_status = UNIT_INACTIVE;
		unit->_gmo_status = GMO_IDLE;
		unit->_selected = false;

		for (auto & id_tile : unit->_visible_tiles) {
			GraphVertex vertex = unit->_team->_fow->get_vertex(id_tile);
			FowVertexData * data = (FowVertexData *)(vertex._data);
			data->_changed = true;
			data->_n_units--;
		}

		_path_finder->remove_gmo(unit);
		return;
	}

	if (unit->_gmo_status == GMO_MOVING) {

	}
	else {
		if (unit->_unit_status == UNIT_ATTACKING) {
			if (unit->_target->_unit_status == UNIT_DESTROYED || unit->_target->_hit_status == FINAL_HIT) {
				unit->_unit_status = UNIT_WATCHING;
			}
		}
		else if (unit->_unit_status == UNIT_SHOOTING) {
			_ammos.push_back(new Ammo(unit->_type->_ammo_type, unit->_position, unit->_target->_position));
			unit->_unit_status = UNIT_ATTACKING;
		}
	}
	unit->anim(t);
}


void Map::ia(time_point t) {
	bool ia_verbose = false;

	for (auto & team : _teams) {
		if (!team->_ia) {
			continue;
		}

		// construction
		for (auto & unit_type : std::vector<std::string>{"infantery", "tank", "helicopter", "boat"}) {
			for (uint compt = 0; compt < IA_MAX_CONSTRUCTION_TRY; compt++) {
				pt_2d pos = rand_pt_2d(_aabb->_pos, _aabb->_pos + _aabb->_size);
				if (add_unit_check(team, unit_type, pos, true, true)) {
					if (ia_verbose) {
						std::cout << "IA construction " << unit_type << "\n";
					}
					add_unit(team, unit_type, pos, t);
					break;
				}
			}
		}

		for (auto & unit : team->_units) {
			if (unit->_unit_status == UNIT_INACTIVE || unit->_unit_status == UNIT_UNDER_CONSTRUCTION) {
				continue;
			}

			if (unit->_gmo_status == GMO_MOVING) {
				if (unit->_hit_status != NO_HIT) {
					_path_finder->stop_gmo(unit);
					unit->_unit_status = UNIT_WATCHING;
					if (ia_verbose) {
						std::cout << "IA unit " << unit->_id << " attacked => MOVING -> WATCHING\n";
					}
				}

				Unit * ennemy_unit = NULL;
				for (auto & ennemy_team : _teams) {
					ennemy_unit = team->search_target(unit, ennemy_team);
					if (ennemy_unit != NULL) {
						break;
					}
				}
				
				if (ennemy_unit != NULL) {
					_path_finder->stop_gmo(unit);
					team->unit_attack(unit, ennemy_unit, t);
					if (ia_verbose) {
						std::cout << "IA unit " << unit->_id << " target found => MOVING -> ATTACKING\n";
					}
				}
			}
			else {
				/*else if (unit->_gmo_status == ) {
					if (unit->_life > 0.95 * unit->_type->_life_init) {
						//unit->set_status(WATCHING, t);
						unit->_unit_status = WATCHING;
						if (ia_verbose) {
							std::cout << "IA unit " << unit->_id << " life OK => WAITING -> WATCHING\n";
						}
					}
				}*/
				if (unit->_unit_status == UNIT_WATCHING) {
					Unit * ennemy_unit = NULL;
					for (auto & ennemy_team : _teams) {
						ennemy_unit = team->search_target(unit, ennemy_team);
						if (ennemy_unit != NULL) {
							break;
						}
					}
					
					if (ennemy_unit != NULL) {
						team->unit_attack(unit, ennemy_unit, t);
						if (ia_verbose) {
							std::cout << "IA unit " << unit->_id << " target found => WATCHING -> ATTACKING\n";
						}
					}
					else {
						for (uint compt = 0; compt < IA_MAX_MOVING_TRY; compt++) {
							// recherche sur un disque troué
							pt_2d destination = rand_pt_2d(unit->_position, unit->_type->_vision_distance, unit->_type->_vision_distance * 0.75);
							if (move_unit_check(unit, destination, true)) {
								_path_finder->goto_gmo(unit, destination, true);
								if (ia_verbose) {
									std::cout << "IA unit " << unit->_id << " no target found => WATCHING -> MOVING\n";
								}
								break;
							}
						}
					}
				}
				else if (unit->_unit_status == UNIT_ATTACKING) {
					if (!team->is_target_reachable(unit, unit->_target)) {
						pt_2d destination = pt_2d(unit->_target->_position);
						if (move_unit_check(unit, destination, true)) {
							_path_finder->goto_gmo(unit, destination, true);
							if (ia_verbose) {
								std::cout << "IA unit " << unit->_id << " target unreachable => ATTACKING -> MOVING\n";
							}
						}
						else {
							unit->_unit_status = UNIT_WATCHING;
							if (ia_verbose) {
								std::cout << "IA unit " << unit->_id << " target lost => ATTACKING -> WATCHING\n";
							}
						}
					}
				}
			}
		}
	}
}


void Map::anim(time_point t) {
	bool verbose = false;

	// path find -------------------------------------------------
	if (verbose) {
		std::cout << "Map anim : path_finder\n";
	}
	std::vector<GridMovingObject *> base_gmos;
	for (auto & team : _teams) {
		for (auto & unit : team->_units) {
			base_gmos.push_back((GridMovingObject *)(unit));
		}
	}
	_path_finder->anim_gmos(base_gmos, t);

	// IA ------------------------------------------------------------
	if (verbose) {
		std::cout << "Map anim : IA\n";
	}
	ia(t);

	// anim par unité -------------------------------------------------
	if (verbose) {
		std::cout << "Map anim : path_finder\n";
	}
	for (auto & team : _teams) {
		for (auto & unit : team->_units) {
			anim_unit(unit, t);
		}
	}

	// ammo / explosion -----------------------------------------------
	if (verbose) {
		std::cout << "Map anim : ammo\n";
	}
	for (auto & ammo : _ammos) {
		ammo->anim();
		if (ammo->_target_hit) {
			_explosion_system->new_explosion(ammo->_target, t, ammo->_type->_explosion_config, ammo->_type->_explosion_radius);
			for (auto & team : _teams) {
				for (auto & unit : team->_units) {
					//if (pt_in_bbox2d(pt_2d(ammo->_target), unit->_bbox->bbox2d())) {
					if (glm::length(ammo->_target - unit->_bbox->_aabb->center()) < ammo->_type->_explosion_radius) {
						unit->hit(ammo, t);
					}
				}
			}
		}
	}

	_ammos.erase(std::remove_if(_ammos.begin(), _ammos.end(), [](Ammo * w) {
		return w->_target_hit;
	}), _ammos.end());

	_explosion_system->anim(t);

	// maj FOW ---------------------------------------------------------
	if (verbose) {
		std::cout << "Map anim : maj FOW\n";
	}
	for (auto & team : _teams) {
		team->update_fow();
	}
}


/*void Map::collisions(time_point t) {
	bool verbose = false;

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
}*/


void Map::clear() {
	clear_units();
	clear_elements();
	_path_finder->set_vertex("land");
	_elevation->set_alti_all(1.0);
	sync2elevation();
}



void Map::randomize(ElevationRandConfig * rand_config) {
	clear_units();
	clear_elements();
	_path_finder->set_vertex("land");
	_elevation->randomize(rand_config);
	sync2elevation();

	for (uint i=0; i<50; ++i) {
		pt_2d pt = rand_gaussian(_aabb->_pos + 0.5 * _aabb->_size, 0.3 * _aabb->_size);
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
		pt_2d pt = rand_gaussian(_aabb->_pos + 0.5 * _aabb->_size, 0.3 * _aabb->_size);
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


void Map::save_teams(std::string teams_json_path) {
	json teams_js;
	teams_js["teams"] = json::array();
	for (auto & team : _teams) {
		teams_js["teams"].push_back(team->get_json());
	}

	std::ofstream teams_ofs(teams_json_path);
	teams_ofs << std::setw(4) << teams_js << "\n";
}


void Map::save_fixed(std::string dir_map) {
	std::filesystem::path map_path = dir_map;
	std::filesystem::path general_json_path = map_path / "general.json";
	std::filesystem::path elements_json_path = map_path / "elements.json";
	std::filesystem::path elevation_path = map_path / "elevation.raw";

	if (!std::filesystem::is_directory(map_path)) {
		std::filesystem::create_directory(map_path);
	}

	// général ---------------------
	json general_js;
	
	general_js["unit_types_dir"] = _unit_types_dir;
	general_js["ammo_types_dir"] = _ammo_types_dir;
	general_js["elements_dir"] = _elements_dir;
	general_js["path_resolution"] = json::array();
	general_js["path_resolution"].push_back(_path_resolution.x);
	general_js["path_resolution"].push_back(_path_resolution.y);
	general_js["elevation_resolution"] = json::array();
	general_js["elevation_resolution"].push_back(_elevation_resolution.x);
	general_js["elevation_resolution"].push_back(_elevation_resolution.y);
	general_js["fow_resolution"] = json::array();
	general_js["fow_resolution"].push_back(_fow_resolution.x);
	general_js["fow_resolution"].push_back(_fow_resolution.y);
	general_js["origin"] = json::array();
	general_js["origin"].push_back(_aabb->_pos.x);
	general_js["origin"].push_back(_aabb->_pos.y);

	std::ofstream general_ofs(general_json_path.string());
	general_ofs << std::setw(4) << general_js << "\n";

	// elements -----------------------
	json elements_js;

	elements_js["elements"] = json::array();
	for (auto & element : _elements->_elements) {
		elements_js["elements"].push_back(element->get_json());
	}

	std::ofstream elements_ofs(elements_json_path.string());
	elements_ofs << std::setw(4) << elements_js << "\n";

	// elevation --------------------------
	_elevation->write(elevation_path.string());
}


void Map::save(std::string dir_map) {
	save_fixed(dir_map);
	std::filesystem::path map_path = dir_map;
	std::filesystem::path teams_json_path = map_path / "teams.json";
	save_teams(teams_json_path.string());
}


void Map::load(std::string dir_map, time_point t) {
	std::filesystem::path map_path = dir_map;
	std::filesystem::path general_json_path = map_path / "general.json";
	std::filesystem::path elements_json_path = map_path / "elements.json";
	std::filesystem::path teams_json_path = map_path / "teams.json";
	std::filesystem::path elevation_path = map_path / "elevation.raw";

	clear_units();
	clear_elements();
	_path_finder->set_vertex("land");

	_elevation->read(elevation_path.string());
	sync2elevation();
	

	std::ifstream elements_ifs(elements_json_path);
	json elements_js = json::parse(elements_ifs);
	elements_ifs.close();
	for (auto & element_js : elements_js["elements"]) {
		std::string type = element_js["type"];
		pt_2d position = pt_2d(element_js["position"][0], element_js["position"][1]);
		if (type == "stone") {
			std::string species_name = element_js["species"];
			add_stone(species_name, position);
		}
		else if (type == "tree") {
			std::string species_name = element_js["species"];
			add_tree(species_name, position);
		}
		else if (type == "lake") {
			add_lake(position);
		}
		else if (type == "river") {
			add_river(position);
		}
	}

	_teams.clear();
	std::ifstream teams_ifs(teams_json_path);
	json teams_js = json::parse(teams_ifs);
	teams_ifs.close();
	for (auto & team_js : teams_js["teams"]) {
		std::string team_name = team_js["name"];
		glm::vec3 team_color = glm::vec3(team_js["color"][0], team_js["color"][1], team_js["color"][2]);
		Team * team = new Team(team_name, team_color, _elevation, _fow_resolution);
		for (auto & unit_js : team_js["units"]) {
			Unit * unit = add_unit(team, unit_js["type"], pt_2d(unit_js["position"][0], unit_js["position"][1]), t);
			
			// on les met dans un état d'attente par défaut
			unit->_life = unit_js["life"];
			unit->_unit_status = UNIT_WATCHING;
			unit->_gmo_status = GMO_IDLE;
		}
		_teams.push_back(team);
	}
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

