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
#include "elements.h"
#include "path_finder.h"
#include "lake.h"
#include "river.h"
#include "unit.h"
#include "unit_type.h"
#include "elevation.h"


/*struct Obstacle {
	Obstacle();
	Obstacle(OBSTACLE_TYPE type, const std::vector<pt_2d> & pts);
	Obstacle(OBSTACLE_TYPE type, Polygon2D * polygon);
	~Obstacle();


	OBSTACLE_TYPE _type;
	Polygon2D * _polygon;
};*/


struct Map {
	Map();
	Map(std::string unit_types_dir, std::string elements_dir, pt_2d origin, pt_2d size, pt_2d path_resolution, pt_2d elevation_resolution, time_point t);
	~Map();
	void add_unit(std::string type_name, pt_2d pos, time_point t);
	River * add_river(pt_2d src);
	Lake * add_lake(pt_2d src);

	void update_alti_grid();
	void update_alti_path(Unit * unit);
	void update_elevation_grid();
	
	void update_terrain_grid_with_elevation();
	void update_terrain_grid_with_element(Element * element);
	
	void sync2elevation();
	
	void clear_units_position_grid();
	std::vector<uint_pair> waiting_unit_positions_edges(Unit * unit, UnitType * unit_type);
	std::vector<uint_pair> moving_unit_positions_edges(Unit * unit, UnitType * unit_type, bool all);
	void add_waiting_unit_to_position_grid(Unit * unit);
	void remove_waiting_unit_from_position_grid(Unit * unit);
	void add_moving_unit_to_position_grid(Unit * unit);
	void remove_moving_unit_from_position_grid(Unit * unit, bool all);

	void path_find(Unit * unit, pt_2d goal);
	void clear();
	//void read_shapefile(std::string shp_path, pt_2d origin, pt_2d size, bool reverse_y=false);
	void anim(time_point t);
	void selected_units_goto(pt_2d pt, time_point t);
	void randomize();
	void save(std::string json_path);
	void load(std::string json_path, time_point t);
	friend std::ostream & operator << (std::ostream & os, Map & map);


	static uint _next_unit_id;
	pt_2d _origin;
	pt_2d _size;
	std::map<std::string, UnitType *> _unit_types;
	std::vector<Unit *> _units;
	PathFinder * _path_finder;
	Elevation * _elevation;
	Elements * _elements;
	std::vector<River * > _rivers;
	std::vector<Lake *> _lakes;

	bool _paused;
	std::thread _path_find_thr;
	bool _path_find_thr_active;
};


#endif
