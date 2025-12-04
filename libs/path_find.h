#ifndef PATH_FIND
#define PATH_FIND

#include <utility>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>

#include <OpenGL/gl3.h>

#include <glm/glm.hpp>

#include "typedefs.h"
#include "geom_2d.h"
#include "bbox_2d.h"
#include "graph.h"


enum UNIT_MODE {WAITING, MOVING};
enum OBSTACLE_TYPE {UNKNOWN, GROUND, SOLID, WATER};


OBSTACLE_TYPE str2type(std::string s);


struct UnitElevationCoeff {
	number _elevation_min;
	number _elevation_max;
	number _coeff;
};


struct UnitType {
	UnitType();
	UnitType(std::string json_path);
	~UnitType();
	
	
	std::string _name;
	pt_type _size;
	number _velocity;
	std::map<OBSTACLE_TYPE, number> _weights;
	std::vector<UnitElevationCoeff> _elevation_coeffs;
};


struct Unit {
	Unit();
	Unit(UnitType * type, pt_type pos, GraphGrid * grid);
	~Unit();
	void clear_path();
	
	
	UnitType * _type;
	bool _selected;
	AABB_2D * _aabb;
	std::vector<pt_type> _path;
	uint _idx_path;
	UNIT_MODE _mode;
	GraphGrid * _grid;
};


struct Obstacle {
	Obstacle();
	Obstacle(OBSTACLE_TYPE type, const std::vector<pt_type> & pts);
	~Obstacle();


	OBSTACLE_TYPE _type;
	Polygon2D * _polygon;
};


struct Terrain {
	Terrain();
	Terrain(pt_type origin, pt_type size, uint n_ligs, uint n_cols);
	~Terrain();
	number get_alti(pt_type pt);
	void randomize();


	pt_type _origin;
	pt_type _size;
	uint _n_ligs;
	uint _n_cols;
	number * _altis;
};


bool frontier_cmp(std::pair<uint, number> x, std::pair<uint, number> y);


struct PathFinder {
	PathFinder();
	~PathFinder();
	number cost(uint i, uint j, GraphGrid * grid);
	number heuristic(uint i, uint j, GraphGrid * grid);
	bool line_of_sight(pt_type pt1, pt_type pt2, GraphGrid * grid);
	bool path_find_nodes(uint start, uint goal, GraphGrid * grid, std::vector<uint> & path);
	bool path_find(pt_type start, pt_type goal, GraphGrid * grid, std::vector<pt_type> & path);
};


struct Map {
	Map();
	Map(std::string unit_types_dir, pt_type origin, pt_type size, pt_type path_resolution, pt_type terrain_resolution);
	~Map();
	void add_unit(std::string type_name, pt_type pos);
	void add_obstacle(OBSTACLE_TYPE type, const std::vector<pt_type> & pts);
	void update_grids();
	void clear();
	void read_shapefile(std::string shp_path, pt_type origin, pt_type size, bool reverse_y=false);
	void anim();
	void selected_units_goto(pt_type pt);
	//void rand(uint n_polys, uint n_pts_per_poly, number poly_radius);
	//void draw_svg(const std::vector<uint> & path, std::string svg_path);


	pt_type _origin;
	pt_type _size;
	std::map<std::string, UnitType *> _unit_types;
	std::vector<Unit *> _units;
	std::vector<Obstacle *> _obstacles;
	PathFinder * _path_finder;
	std::map<UnitType * , GraphGrid *> _grids;
	Terrain * _terrain;
};


#endif
