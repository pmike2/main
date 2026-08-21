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


Map::Map(fs unit_types_dir, fs ammo_types_dir, fs elements_dir, fs explosion_dir, fs barrier_types_dir, 
	pt_2d origin, pt_2d size, pt_2d path_resolution, pt_2d elevation_resolution, pt_2d fow_resolution, time_point t) :
	_unit_types_dir(unit_types_dir), _ammo_types_dir(ammo_types_dir), _elements_dir(elements_dir), _barrier_types_dir(barrier_types_dir),
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
		std::cout << "init UnitTypes / AmmoTypes / BarrierTypes / Explosion\n";
	}
	
	for (auto & json_path : std::filesystem::directory_iterator(_unit_types_dir)) {
		if (json_path.path().extension() == ".json") {
			UnitType * unit_type = new UnitType(json_path);
			_unit_types[unit_type->_name] = unit_type;
			_path_finder->_gmo_types.push_back((GridMovingObjectType *)(unit_type));
		}
	}
	
	for (auto & json_path : std::filesystem::directory_iterator(_ammo_types_dir)) {
		if (json_path.path().extension() == ".json") {
			AmmoType * ammo_type = new AmmoType(json_path);
			_ammo_types[ammo_type->_name] = ammo_type;
		}
	}

	for (auto & json_path : std::filesystem::directory_iterator(_barrier_types_dir)) {
		if (json_path.path().extension() == ".json") {
			BarrierType * barrier_type = new BarrierType(json_path);
			_barrier_types[barrier_type->_name] = barrier_type;
		}
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
	_elements = new Elements(_elements_dir / "tree_species", elements_dir / "stone_species", _elevation);

	// ------------------------------------------------
	if (verbose) {
		std::cout << "init Teams\n";
	}
	_teams.push_back(new Team("Team1", glm::vec3(1.0f, 0.0f, 0.0f), _elevation, _path_finder, _fow_resolution));
	_teams.push_back(new Team("Team2", glm::vec3(0.0f, 0.0f, 1.0f), _elevation, _path_finder, _fow_resolution));
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
			if (team->add_unit_check(_unit_types["infantery"], position, false, false)) {
				team->add_unit(_unit_types["infantery"], position, t);
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


void Map::add_barrier(std::string type, pt_2d pos, number orientation) {
	Barrier * barrier = new Barrier(_barrier_types[type], pos, orientation, _elevation);
	_barriers.push_back(barrier);
	std::vector<uint> id_nodes = _path_finder->vertices_in_bbox(barrier->_bbox->bbox2d()->buffered(1.0));
	//id_nodes = _path_finder->buffered_ids(id_nodes, 1); std::cout << id_nodes.size() << "\n";
	//id_nodes = _path_finder->prune(id_nodes, 6); std::cout << id_nodes.size() << "\n";
	//id_nodes = _path_finder->prune(id_nodes); std::cout << id_nodes.size() << "\n";
	_path_finder->set_vertex(id_nodes, "barrier");
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


void Map::pause_all_units(bool pause) {
	for (auto & team : _teams) {
		for (auto & unit : team->_units) {
			unit->_paused = pause;
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


void Map::ia(time_point t, bool fow_active) {
	bool verbose = false;

	for (auto & team : _teams) {
		if (!team->_ia) {
			continue;
		}

		// construction
		for (auto & unit_type : _unit_types) {
			for (uint compt = 0; compt < IA_MAX_CONSTRUCTION_TRY; compt++) {
				pt_2d pos = rand_pt_2d(_aabb->_pos, _aabb->_pos + _aabb->_size);
				if (team->add_unit_check(unit_type.second, pos, fow_active, true)) {
					if (verbose) {
						std::cout << "IA construction " << unit_type.first << "\n";
					}
					team->add_unit(unit_type.second, pos, t);
					break;
				}
			}
		}

		for (auto & unit : team->_units) {
			if (unit->_unit_status == UNIT_INACTIVE || unit->_unit_status == UNIT_UNDER_CONSTRUCTION) {
				continue;
			}

			if (unit->_gmo_status == GMO_MOVING || unit->_gmo_status == GMO_WAITING) {
				if (unit->_hit_status != NO_HIT) {
					_path_finder->stop_gmo(unit);
					unit->_unit_status = UNIT_WATCHING;
					if (verbose) {
						std::cout << "IA unit " << unit->_id << " attacked => MOVING -> WATCHING\n";
					}
				}

				Unit * ennemy_unit = NULL;
				for (auto & ennemy_team : _teams) {
					ennemy_unit = team->search_target(unit, ennemy_team, fow_active);
					if (ennemy_unit != NULL) {
						break;
					}
				}
				
				if (ennemy_unit != NULL) {
					_path_finder->stop_gmo(unit);
					team->unit_attack(unit, ennemy_unit, t, fow_active);
					if (verbose) {
						std::cout << "IA unit " << unit->_id << " target found => MOVING -> ATTACKING\n";
					}
				}
			}
			
			else if (unit->_gmo_status == GMO_IDLE) {
				/*else if (unit->_gmo_status == ) {
					if (unit->_life > 0.95 * unit->_type->_life_init) {
						//unit->set_status(WATCHING, t);
						unit->_unit_status = WATCHING;
						if (verbose) {
							std::cout << "IA unit " << unit->_id << " life OK => WAITING -> WATCHING\n";
						}
					}
				}*/
				if (unit->_unit_status == UNIT_WATCHING) {
					Unit * ennemy_unit = NULL;
					for (auto & ennemy_team : _teams) {
						ennemy_unit = team->search_target(unit, ennemy_team, fow_active);
						if (ennemy_unit != NULL) {
							break;
						}
					}
					
					if (ennemy_unit != NULL) {
						team->unit_attack(unit, ennemy_unit, t, fow_active);
						if (verbose) {
							std::cout << "IA unit " << unit->_id << " target found => WATCHING -> ATTACKING\n";
						}
					}
					else {
						if (!_path_finder->is_gmo_expecting_path(unit)) {
							for (uint compt = 0; compt < IA_MAX_MOVING_TRY; compt++) {
								// recherche sur un disque troué
								pt_2d destination = rand_pt_2d(unit->_position, unit->_type->_vision_distance, unit->_type->_vision_distance * 0.75);
								if (team->move_unit_check(unit, destination, fow_active)) {
									_path_finder->goto_gmo(unit, destination, true);
									if (verbose) {
										std::cout << "IA unit " << unit->_id << " no target found => WATCHING -> MOVING\n";
									}
									break;
								}
							}
						}
					}
				}
				else if (unit->_unit_status == UNIT_ATTACKING) {
					if (!team->attack_unit_check(unit, unit->_target, fow_active)) {
						unit->_unit_status = UNIT_WATCHING;
						pt_2d destination = pt_2d(unit->_target->_position);
						if (team->move_unit_check(unit, destination, fow_active) && !_path_finder->is_gmo_expecting_path(unit)) {
							_path_finder->goto_gmo(unit, destination, true);
							if (verbose) {
								std::cout << "IA unit " << unit->_id << " target unreachable => ATTACKING -> MOVING\n";
							}
						}
						else {
							if (verbose) {
								std::cout << "IA unit " << unit->_id << " target lost => ATTACKING -> WATCHING\n";
							}
						}
					}
				}
			}
		}
	}
}


void Map::anim(time_point t, bool fow_active) {
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
	ia(t, fow_active);

	// anim par unité -------------------------------------------------
	if (verbose) {
		std::cout << "Map anim : path_finder\n";
	}
	for (auto & team : _teams) {
		team->anim_units(t);
	}

	// ammo / explosion -----------------------------------------------
	if (verbose) {
		std::cout << "Map anim : ammo\n";
	}
	
	for (auto & team : _teams) {
		for (auto & unit : team->_units) {
			if (unit->_unit_status == UNIT_SHOOTING) {
				_ammos.push_back(new Ammo(unit->_type->_ammo_type, unit->_position, unit->_target->_position));
				unit->_unit_status = UNIT_ATTACKING;
			}
		}
	}

	for (auto & ammo : _ammos) {
		ammo->anim();
		if (ammo->_target_hit) {
			_explosion_system->new_explosion(ammo->_target, t, ammo->_type->_explosion_config, ammo->_type->_explosion_radius);
			for (auto & team : _teams) {
				for (auto & unit : team->_units) {
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


void Map::remove_units_in_aabb(AABB_2D * aabb) {
	for (auto & team : _teams) {
		std::vector<Unit *> units = team->get_units_in_aabb(aabb);
		for (auto & unit : units) {
			unit->_unit_status = UNIT_DESTROYED;
		}
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


void Map::save_teams(fs teams_json_path) {
	json teams_js;
	teams_js["teams"] = json::array();
	for (auto & team : _teams) {
		teams_js["teams"].push_back(team->get_json());
	}

	std::ofstream teams_ofs(teams_json_path);
	teams_ofs << std::setw(4) << teams_js << "\n";
}


void Map::save_fixed(fs map_path) {
	fs general_json_path = map_path / "general.json";
	fs elements_json_path = map_path / "elements.json";
	fs elevation_path = map_path / "elevation.raw";

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


void Map::save(fs map_path) {
	save_fixed(map_path);
	fs teams_json_path = map_path / "teams.json";
	save_teams(teams_json_path);
}


void Map::load(fs map_path, time_point t) {
	fs general_json_path = map_path / "general.json";
	fs elements_json_path = map_path / "elements.json";
	fs teams_json_path = map_path / "teams.json";
	fs elevation_path = map_path / "elevation.raw";

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
		Team * team = new Team(team_name, team_color, _elevation, _path_finder, _fow_resolution);
		for (auto & unit_js : team_js["units"]) {
			Unit * unit = team->add_unit(_unit_types[unit_js["type"]], pt_2d(unit_js["position"][0], unit_js["position"][1]), t);
			
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

