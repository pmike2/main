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
	Map(std::string unit_types_dir, std::string ammo_types_dir, std::string elements_dir, pt_2d origin, pt_2d size, pt_2d path_resolution, pt_2d elevation_resolution, time_point t);
	~Map();
	bool add_unit_check(UNIT_TYPE type, pt_2d pos);
	Unit * add_unit(Team * team, UNIT_TYPE type, pt_2d pos);
	void add_river(pt_2d pos);
	void add_lake(pt_2d pos);
	void add_trees(std::string species_name, pt_2d pos, uint n_trees, number dispersion);
	void add_stones(std::string species_name, pt_2d pos, uint n_stones, number dispersion);

	Unit * get_unit(uint unit_id);
	std::vector<Unit *> get_units_in_aabb(AABB_2D * aabb);
	Team * get_team(std::string team_name);
	
	void remove_units_in_aabb(AABB_2D * aabb);
	void remove_elements_in_aabb(AABB_2D * aabb);

	void update_alti_grid();
	//void update_alti_path(Unit * unit);
	void update_elevation_grid();
	
	void update_terrain_grid_with_elevation();
	//void update_terrain_grid_with_aabb(AABB_2D * aabb);
	//void clear_terrain_grid_with_aabb(AABB_2D * aabb);

	void add_element_to_terrain_grid(Element * element);
	void remove_element_from_terrain_grid(Element * element);
	
	void sync2elevation();
	
	void clear_units_position_grid();
	std::vector<uint_pair> waiting_unit_positions_edges(Unit * unit, UnitType * unit_type);
	void fill_unit_path_edges(Unit * unit);
	void add_unit_to_position_grid(Unit * unit);
	void remove_unit_from_position_grid(Unit * unit);
	void advance_unit_in_position_grid(Unit * unit);

	//void path_find(Unit * unit, pt_3d goal);
	void path_find();
	void pause_all_units(bool pause);

	void clear();
	//void read_shapefile(std::string shp_path, pt_2d origin, pt_2d size, bool reverse_y=false);
	void anim(time_point t);
	void collisions(time_point t);
	
	void randomize();
	void save(std::string json_path);
	void load(std::string json_path);
	friend std::ostream & operator << (std::ostream & os, Map & map);


	static uint _next_unit_id;
	std::map<UNIT_TYPE, UnitType *> _unit_types;
	std::map<std::string, AmmoType * > _ammo_types;
	PathFinder * _path_finder;
	Elevation * _elevation;
	Elements * _elements;
	std::vector<Team *> _teams;
	std::vector<Ammo *> _ammos;

	std::thread _path_find_thr;
	bool _path_find_thr_running;
	SafeQueue<PathFinderInput *> _path_queue_thr_input;
	SafeQueue<UnitPath *> _path_queue_thr_output;
	bool _path_finder_computing;
};


#endif
