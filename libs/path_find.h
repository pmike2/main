#ifndef PATH_FIND
#define PATH_FIND

#include <utility>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <queue>
#include <chrono>

#include <glm/glm.hpp>

#include "typedefs.h"
#include "geom_2d.h"
#include "bbox_2d.h"
#include "bbox.h"
#include "graph.h"


const number MAX_UNIT_MOVING_WEIGHT = 100.0;
const number UNIT_DIST_PATH_EPS = 0.05;


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
	pt_type_3d _size;
	number _max_velocity;
	std::map<OBSTACLE_TYPE, number> _weights;
	std::vector<UnitElevationCoeff> _elevation_coeffs;
};


struct Path {
	Path();
	~Path();
	void clear();
	bool empty();
	friend std::ostream & operator << (std::ostream & os, Path & p);


	std::vector<pt_type_3d> _pts;
	std::vector<uint> _nodes;
	std::vector<number> _weights;
	uint _idx_path;
};


struct Unit {
	Unit();
	Unit(UnitType * type, pt_type_3d pos);
	~Unit();
	void anim();
	void goto_next_checkpoint();
	void stop();
	friend std::ostream & operator << (std::ostream & os, Unit & unit);
	
	
	uint _id;
	UnitType * _type;
	bool _selected;
	AABB * _aabb;
	UNIT_MODE _mode;
	Path * _path;
	pt_type_3d _velocity;
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
	pt_type id2pt(uint id);
	std::pair<uint, uint> pt2col_lig(pt_type pt);
	uint pt2id(pt_type pt);
	number get_alti(int col, int lig);
	number get_alti(pt_type pt);
	number get_alti_over_polygon(Polygon2D * polygon);
	std::vector<uint> get_ids_over_aabb(AABB_2D * aabb);
	void set_alti(int col, int lig, number alti);
	void set_alti_over_polygon(Polygon2D * polygon, number alti);
	void set_alti_all(number alti);
	void randomize();
	void alti2pbm(std::string pbm_path);


	pt_type _origin;
	pt_type _size;
	pt_type _resolution;
	uint _n_ligs;
	uint _n_cols;
	number * _altis;
};


bool frontier_cmp(std::pair<uint, number> x, std::pair<uint, number> y);


struct PathFinder {
	PathFinder();
	~PathFinder();
	number cost(uint i, uint j, std::vector<GraphGrid *> grid);
	number heuristic(uint i, uint j, std::vector<GraphGrid *> grid);
	number line_of_sight_max_weight(pt_type_3d pt1, pt_type_3d pt2, std::vector<GraphGrid *> grid);
	bool path_find_nodes(uint start, uint goal, std::vector<GraphGrid *> grid, std::vector<uint> & path);
	bool path_find(pt_type_3d start, pt_type_3d goal, std::vector<GraphGrid *> grid, Path * path);
	void draw_svg(GraphGrid * grid, Path * path, std::string svg_path);


	bool _use_line_of_sight;
};


struct Instruction {
	Unit * _unit;
	pt_type _destination;
};


struct Map {
	Map();
	Map(std::string unit_types_dir, pt_type origin, pt_type size, pt_type path_resolution, pt_type terrain_resolution, time_point t);
	~Map();
	void add_unit(std::string type_name, pt_type pos);
	Obstacle * add_obstacle(OBSTACLE_TYPE type, const std::vector<pt_type> & pts);
	void update_alti_grid(GraphGrid * grid);
	void update_static_grids();
	void update_unit_grid(Unit * unit);
	void add_unit_to_position_grids(Unit * unit);
	void remove_unit_from_position_grids(Unit * unit);
	void clear();
	void read_shapefile(std::string shp_path, pt_type origin, pt_type size, bool reverse_y=false);
	void anim(time_point t);
	void selected_units_goto(pt_type pt);
	void randomize();
	void save(std::string json_path);
	void load(std::string json_path);
	friend std::ostream & operator << (std::ostream & os, Map & map);


	static uint _next_unit_id;
	pt_type _origin;
	pt_type _size;
	std::map<std::string, UnitType *> _unit_types;
	std::vector<Unit *> _units;
	std::vector<Obstacle *> _obstacles;
	PathFinder * _path_finder;
	std::map<UnitType * , GraphGrid *> _static_grids;
	std::map<UnitType * , GraphGrid *> _units_position_grids;
	std::map<Unit * , GraphGrid *> _unit_grids;
	std::map<UnitType * , std::vector<Obstacle *> > _buffered_obstacles;
	Terrain * _terrain;
	bool _paused;
	std::queue<Instruction> _instructions;
	time_point _last_instruction_t;
};


#endif
