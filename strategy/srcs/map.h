#ifndef MAP_H
#define MAP_H

#include <utility>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <queue>
#include <chrono>
#include <thread>

#include <glm/glm.hpp>

#include "typedefs.h"
#include "geom_2d.h"
#include "bbox_2d.h"
#include "bbox.h"
#include "graph.h"

#include "const.h"
#include "path_finder.h"
#include "unit.h"
#include "unit_type.h"
#include "elevation.h"
#include "ammo.h"
#include "ammo_type.h"
#include "elements/elements.h"
#include "elements/lake.h"
#include "elements/river.h"
#include "elements/tree.h"
#include "elements/stone.h"


struct Map {
	Map();
	Map(std::string unit_types_dir, std::string ammo_types_dir, std::string elements_dir, pt_2d origin, pt_2d size, pt_2d path_resolution, pt_2d elevation_resolution, pt_2d fow_resolution);
	~Map();
	
	bool fow_check(Team * team, pt_2d pos);
	bool add_unit_check(Team * team, UNIT_TYPE type, pt_2d pos);
	bool add_unit_fow_check(Team * team, pt_2d pos);
	bool move_unit_check(Unit * unit, pt_2d pos);
	bool attack_unit_check(Unit * attacking_unit, Unit * attacked_unit);

	Unit * add_unit(Team * team, UNIT_TYPE type, pt_2d pos, time_point t);
	void add_first_units2teams(time_point t);
	void add_river(pt_2d pos);
	void add_lake(pt_2d pos);
	void add_trees(std::string species_name, pt_2d pos, uint n_trees, number dispersion);
	void add_stones(std::string species_name, pt_2d pos, uint n_stones, number dispersion);

	Unit * get_unit(uint unit_id);
	std::vector<Unit *> get_units_in_aabb(AABB_2D * aabb);
	Team * get_team(std::string team_name);
	uint get_team_idx(std::string team_name);
	
	void remove_units_in_aabb(AABB_2D * aabb);
	void remove_elements_in_aabb(AABB_2D * aabb);

	void clear_units();
	void clear_elements();

	void update_alti_grid();
	void update_elevation_grid();
	void update_terrain_grid_with_elevation();
	void sync2elevation();

	void add_element_to_terrain_grid(Element * element);
	void remove_element_from_terrain_grid(Element * element);
	
	std::vector<uint_pair> waiting_unit_positions_edges(Unit * unit, UnitType * unit_type);
	void fill_unit_path_edges(Unit * unit);
	void add_unit_to_position_grid(Unit * unit);
	void remove_unit_from_position_grid(Unit * unit);
	void advance_unit_in_position_grid(Unit * unit);

	void path_find();
	void pause_all_units(bool pause);

	void ia(time_point t);
	void anim(time_point t);
	void collisions(time_point t);
	
	void clear();
	void randomize(ElevationRandConfig * rand_config);
	void save_teams(std::string teams_json_path);
	void save_fixed(std::string dir_map);
	void save(std::string dir_map);
	void load(std::string dir_map, time_point t);
	friend std::ostream & operator << (std::ostream & os, Map & map);


	std::string _unit_types_dir, _ammo_types_dir, _elements_dir;
	pt_2d _path_resolution, _elevation_resolution, _fow_resolution;
	AABB_2D * _aabb;
	
	static uint _next_unit_id;
	std::map<UNIT_TYPE, UnitType *> _unit_types;
	std::map<std::string, AmmoType * > _ammo_types;
	Elevation * _elevation;
	Elements * _elements;
	std::vector<Team *> _teams;
	std::vector<Ammo *> _ammos;

	PathFinder * _path_finder;
	std::thread _path_find_thr;
	std::mutex _path_find_mtx;
	bool _path_find_thr_running;
	SafeQueue<PathFinderInput *> _path_queue_thr_input;
	SafeQueue<UnitPath *> _path_queue_thr_output;
	bool _path_finder_computing;
};


#endif
