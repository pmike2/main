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
#include "path_find.h"

#include "const.h"
#include "unit.h"
#include "unit_type.h"
#include "elevation.h"
#include "ammo.h"
#include "ammo_type.h"
#include "explosion.h"
#include "barrier_type.h"
#include "barrier.h"
#include "elements/elements.h"
#include "elements/lake.h"
#include "elements/river.h"
#include "elements/tree.h"
#include "elements/stone.h"


struct Map {
	Map();
	Map(
		std::string unit_types_dir, std::string ammo_types_dir, std::string elements_dir, std::string explosion_dir, std::string barrier_types_dir,
		pt_2d origin, pt_2d size, pt_2d path_resolution, pt_2d elevation_resolution, pt_2d fow_resolution, time_point t
	);
	~Map();
	
	bool fow_check(Team * team, pt_2d pos);
	bool construction_check(Team * team, std::string type);
	bool add_unit_check(Team * team, std::string type, pt_2d pos, bool fow_active, bool construction_active);
	bool move_unit_check(Unit * unit, pt_2d pos, bool fow_active);
	bool attack_unit_check(Unit * attacking_unit, Unit * attacked_unit, bool fow_active);
	bool add_barrier_check(Team * team, std::string type, pt_2d pos, number orientation, bool fow_active);

	Unit * add_unit(Team * team, std::string type, pt_2d pos, time_point t);
	void add_first_units2teams(time_point t);
	void add_river(pt_2d pos);
	void add_lake(pt_2d pos);
	void add_tree(std::string species_name, pt_2d pos);
	void add_trees(std::string species_name, pt_2d pos, uint n_trees, number dispersion);
	void add_stone(std::string species_name, pt_2d pos);
	void add_stones(std::string species_name, pt_2d pos, uint n_stones, number dispersion);
	void add_barrier(std::string type, pt_2d pos, number orientation);

	Unit * get_unit(uint unit_id);
	std::vector<Unit *> get_units_in_aabb(AABB_2D * aabb);
	Team * get_team(std::string team_name);
	uint get_team_idx(std::string team_name);
	void selected_units_goto(Team * team, pt_3d pt);
	
	void remove_units_in_aabb(AABB_2D * aabb);
	void remove_elements_in_aabb(AABB_2D * aabb);
	void clear_units();

	void pause_all_units(bool pause);

	void clear_elements();

	void update_alti_grid();
	void update_elevation_grid();
	void update_terrain_grid_with_elevation();
	void sync2elevation();

	void anim_unit(Unit * unit, time_point t);
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


	std::string _unit_types_dir, _ammo_types_dir, _elements_dir, _barrier_types_dir;
	pt_2d _path_resolution, _elevation_resolution, _fow_resolution;
	AABB_2D * _aabb;
	PathFinder * _path_finder;

	static uint _next_unit_id;
	std::map<std::string, UnitType *> _unit_types;
	std::map<std::string, AmmoType * > _ammo_types;
	std::map<std::string, BarrierType * > _barrier_types;
	Elevation * _elevation;
	Elements * _elements;
	std::vector<Team *> _teams;
	std::vector<Ammo *> _ammos;
	std::vector<Barrier *> _barriers;
	ExplosionSystem * _explosion_system;
};


#endif
