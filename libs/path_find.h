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
#include <thread>

#include <glm/glm.hpp>

#include "typedefs.h"
#include "geom_2d.h"
#include "bbox_2d.h"
#include "bbox.h"
#include "graph.h"
#include "elements.h"


const number MAX_UNIT_MOVING_WEIGHT = 100.0;
const number UNIT_DIST_PATH_EPS = 0.05;


enum UNIT_STATUS {WAITING, MOVING, COMPUTING_PATH, COMPUTING_PATH_DONE};
std::string mode2str(UNIT_STATUS mode);

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
	pt_3d _size;
	number _max_velocity;
	std::map<OBSTACLE_TYPE, number> _weights;
	std::vector<UnitElevationCoeff> _elevation_coeffs;
};


struct Path {
	Path();
	~Path();
	void clear();
	bool empty();
	pt_3d destination();
	friend std::ostream & operator << (std::ostream & os, Path & p);


	std::vector<pt_3d> _pts;
	std::vector<uint> _nodes;
	std::vector<number> _weights;
	std::vector<BBox_2D *> _bboxs;
	uint _idx_path;
};


struct Instruction {
	pt_2d _destination;
	time_point _t;
};


struct Unit {
	Unit();
	Unit(UnitType * type, pt_3d pos, time_point t);
	~Unit();
	void anim(time_point t);
	void goto_next_checkpoint(time_point t);
	void stop();
	friend std::ostream & operator << (std::ostream & os, Unit & unit);
	
	
	uint _id;
	UnitType * _type;
	bool _selected;
	AABB * _aabb;
	UNIT_STATUS _status;
	Path * _path;
	pt_3d _velocity;
	std::queue<Instruction> _instructions;
	time_point _last_anim_t;
	std::thread _thr;
};


struct Obstacle {
	Obstacle();
	Obstacle(OBSTACLE_TYPE type, const std::vector<pt_2d> & pts);
	Obstacle(OBSTACLE_TYPE type, Polygon2D * polygon);
	~Obstacle();


	OBSTACLE_TYPE _type;
	Polygon2D * _polygon;
};


struct Terrain {
	Terrain();
	Terrain(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols);
	~Terrain();
	std::pair<uint, uint> id2col_lig(uint id);
	uint col_lig2id(uint col, uint lig);
	pt_2d col_lig2pt(uint col, uint lig);
	pt_2d id2pt(uint id);
	std::pair<uint, uint> pt2col_lig(pt_2d pt);
	uint pt2id(pt_2d pt);
	number get_alti(int col, int lig);
	number get_alti(pt_2d pt);
	number get_alti_over_polygon(Polygon2D * polygon);
	std::vector<uint> get_ids_over_aabb(AABB_2D * aabb);
	void set_alti(int col, int lig, number alti);
	void set_alti_over_polygon(Polygon2D * polygon, number alti);
	void set_alti_all(number alti);
	void set_negative_alti_2zero();
	void randomize();
	void alti2pbm(std::string pbm_path);


	pt_2d _origin;
	pt_2d _size;
	pt_2d _resolution;
	uint _n_ligs;
	uint _n_cols;
	number * _altis;
};


bool frontier_cmp(std::pair<uint, number> x, std::pair<uint, number> y);


struct PathFinder {
	PathFinder();
	~PathFinder();
	number units_position_weight(GraphEdge edge, Unit * unit);
	number cost(uint i, uint j, GraphGrid * static_grid, GraphGrid * units_position_grid, Unit * unit);
	number heuristic(uint i, uint j, GraphGrid * grid);
	number line_of_sight_max_weight(pt_2d pt1, pt_2d pt2, GraphGrid * static_grid, GraphGrid * units_position_grid, Unit * unit);
	bool path_find_nodes(uint start, uint goal, GraphGrid * static_grid, GraphGrid * units_position_grid, Unit * unit);
	bool path_find(pt_2d start, pt_2d goal, GraphGrid * static_grid, GraphGrid * units_position_grid, Unit * unit);
	void draw_svg(GraphGrid * grid, Path * path, std::string svg_path);


	bool _use_line_of_sight;
};


struct UnitsPositionEdgeData {
	std::vector<uint> _ids;
};


struct Map {
	Map();
	Map(std::string unit_types_dir, std::string elements_dir, pt_2d origin, pt_2d size, pt_2d path_resolution, pt_2d terrain_resolution, time_point t);
	~Map();
	void add_unit(std::string type_name, pt_2d pos, time_point t);
	Obstacle * add_obstacle(OBSTACLE_TYPE type, const std::vector<pt_2d> & pts);
	void add_static_element(std::string element_name, pt_3d pos, pt_3d size);
	void update_alti_grid(GraphGrid * grid);
	void update_alti_path(Unit * unit);
	void update_static_grids();
	std::vector<std::pair<uint, uint> > unit_positions_edges(Unit * unit, UnitType * unit_type);
	void add_unit_to_position_grids(Unit * unit);
	void remove_unit_from_position_grids(Unit * unit);
	void path_find(Unit * unit, pt_2d goal);
	void clear();
	void read_shapefile(std::string shp_path, pt_2d origin, pt_2d size, bool reverse_y=false);
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
	std::vector<Obstacle *> _obstacles;
	PathFinder * _path_finder;
	std::map<UnitType * , GraphGrid *> _static_grids;
	std::map<UnitType * , GraphGrid *> _units_position_grids;
	std::map<UnitType * , std::vector<Obstacle *> > _buffered_obstacles;
	Terrain * _terrain;
	Elements * _elements;
	bool _paused;
};


#endif
