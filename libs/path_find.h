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
std::string mode2str(UNIT_MODE mode);

enum OBSTACLE_TYPE {UNKNOWN, GROUND, SOLID, WATER};
OBSTACLE_TYPE str2type(std::string s);
std::string type2str(OBSTACLE_TYPE t);




struct UnitElevationCoeff {
	number _elevation_min;
	number _elevation_max;
	number _coeff;
};


struct UnitType {
	UnitType();
	UnitType(std::string json_path);
	~UnitType();
	number elevation_coeff(number elevation);
	friend std::ostream & operator << (std::ostream & os, UnitType & ut);
	
	
	std::string _name;
	pt_type _size;
	number _velocity;
	std::map<OBSTACLE_TYPE, number> _weights;
	std::vector<UnitElevationCoeff> _elevation_coeffs;
};


struct Path {
	Path();
	~Path();
	void clear();


	std::vector<pt_type> _pts;
	std::vector<uint> _nodes;
	uint _idx_path;
};


struct Unit {
	Unit();
	Unit(UnitType * type, pt_type pos, GraphGrid * grid);
	~Unit();
	void clear_path();
	void anim();
	friend std::ostream & operator << (std::ostream & os, Unit & unit);
	
	
	UnitType * _type;
	bool _selected;
	AABB_2D * _aabb;
	UNIT_MODE _mode;
	GraphGrid * _grid;
	Path * _path;
};


struct Obstacle {
	Obstacle();
	Obstacle(OBSTACLE_TYPE type, const std::vector<pt_type> & pts);
	Obstacle(OBSTACLE_TYPE type, Polygon2D * polygon);
	~Obstacle();


	OBSTACLE_TYPE _type;
	Polygon2D * _polygon;
};


struct Terrain {
	Terrain();
	Terrain(pt_type origin, pt_type size, uint n_ligs, uint n_cols);
	~Terrain();
	std::pair<uint, uint> id2col_lig(uint id);
	uint col_lig2id(uint col, uint lig);
	pt_type col_lig2pt(uint col, uint lig);
	number get_alti(int col, int lig);
	number get_alti(pt_type pt);
	number get_alti_over_polygon(Polygon2D * polygon);
	void set_alti(int col, int lig, number alti);
	void set_alti_over_polygon(Polygon2D * polygon, number alti);
	void set_alti_all(number alti);
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
	number line_of_sight_max_weight(pt_type pt1, pt_type pt2, GraphGrid * grid);
	bool path_find_nodes(uint start, uint goal, GraphGrid * grid, std::vector<uint> & path);
	bool path_find(pt_type start, pt_type goal, GraphGrid * grid, Path * path);
};


struct Map {
	Map();
	Map(std::string unit_types_dir, pt_type origin, pt_type size, pt_type path_resolution, pt_type terrain_resolution);
	~Map();
	void add_unit(std::string type_name, pt_type pos);
	Obstacle * add_obstacle(OBSTACLE_TYPE type, const std::vector<pt_type> & pts);
	void update_grids();
	void clear();
	void read_shapefile(std::string shp_path, pt_type origin, pt_type size, bool reverse_y=false);
	void anim();
	void selected_units_goto(pt_type pt);
	//void rand(uint n_polys, uint n_pts_per_poly, number poly_radius);
	//void draw_svg(const std::vector<uint> & path, std::string svg_path);
	friend std::ostream & operator << (std::ostream & os, Map & map);


	pt_type _origin;
	pt_type _size;
	std::map<std::string, UnitType *> _unit_types;
	std::vector<Unit *> _units;
	std::vector<Obstacle *> _obstacles;
	PathFinder * _path_finder;
	std::map<UnitType * , GraphGrid *> _grids;
	std::map<UnitType * , std::vector<Obstacle *> > _buffered_obstacles;
	Terrain * _terrain;
};


#endif
