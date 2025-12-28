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


enum UNIT_STATUS {WAITING, MOVING, COMPUTING_PATH, COMPUTING_PATH_DONE, COMPUTING_PATH_FAILED};
std::string unit_status2str(UNIT_STATUS mode);

enum TERRAIN_TYPE {GROUND, OBSTACLE, WATER, UNKNOWN};
TERRAIN_TYPE str2terrain_type(std::string s);
std::string terrain_type2str(TERRAIN_TYPE t);


struct UnitElevationCoeff {
	number _elevation_min;
	number _elevation_max;
	number _coeff;
};


struct UnitType {
	UnitType();
	UnitType(std::string json_path);
	~UnitType();
	number elevation_coeff(number delta_elevation);
	friend std::ostream & operator << (std::ostream & os, UnitType & ut);
	
	
	std::string _name;
	pt_3d _size;
	number _max_velocity;
	std::map<TERRAIN_TYPE, number> _terrain_weights;
	std::vector<UnitElevationCoeff> _delta_elevation_coeffs;
};


struct Path {
	Path();
	~Path();
	void clear();
	bool empty();
	friend std::ostream & operator << (std::ostream & os, Path & p);


	pt_2d _start;
	pt_2d _goal;
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
	bool checkpoint_checked();
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
};


/*struct Obstacle {
	Obstacle();
	Obstacle(OBSTACLE_TYPE type, const std::vector<pt_2d> & pts);
	Obstacle(OBSTACLE_TYPE type, Polygon2D * polygon);
	~Obstacle();


	OBSTACLE_TYPE _type;
	Polygon2D * _polygon;
};*/


struct Elevation {
	Elevation();
	Elevation(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols);
	~Elevation();
	std::pair<uint, uint> id2col_lig(uint id);
	uint col_lig2id(uint col, uint lig);
	pt_2d col_lig2pt(uint col, uint lig);
	pt_2d id2pt_2d(uint id);
	pt_3d id2pt_3d(uint id);
	std::pair<uint, uint> pt2col_lig(pt_2d pt);
	uint pt2id(pt_2d pt);
	number get_alti(uint id);
	number get_alti(int col, int lig);
	number get_alti(pt_2d pt);
	number get_alti_over_polygon(Polygon2D * polygon);
	std::vector<uint> get_ids_over_aabb(AABB_2D * aabb);
	std::vector<uint> get_neighbors(uint id);
	pt_3d get_normal(uint id);
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


struct River {
	River();
	River(Elevation * elevation, pt_2d src);
	~River();
	void update_data();
	

	Elevation * _elevation;
	std::vector<uint> _id_nodes;
	std::vector<std::tuple<pt_3d, pt_3d, pt_3d, pt_3d> > _triangles;
	float * _data;
	uint _n_pts;
};


struct Lake {
	Lake();
	Lake(Elevation * elevation, pt_2d src);
	~Lake();
	void update_data();

	Elevation * _elevation;
	std::vector<uint> _id_nodes;
	std::vector<std::tuple<pt_3d, pt_3d, pt_3d, pt_3d> > _triangles;
	float * _data;
	uint _n_pts;
};


struct PathFinder {
	PathFinder();
	PathFinder(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols);
	~PathFinder();
	number elevation_weight(uint i, uint j);
	number units_position_weight(Unit * unit, uint i, uint j);
	number terrain_weight(Unit * unit, uint i, uint j);
	number cost(Unit * unit, uint i, uint j);
	number heuristic(uint i, uint j);
	number line_of_sight_max_weight(Unit * unit, pt_2d pt1, pt_2d pt2);
	bool path_find_nodes(Unit * unit, uint start, uint goal);
	bool path_find(Unit * unit, pt_2d goal);
	//void draw_svg(GraphGrid * grid, Path * path, std::string svg_path);


	GraphGrid * _elevation_grid;
	GraphGrid * _units_position_grid;
	GraphGrid * _terrain_grid;
	bool _use_line_of_sight;
	bool _verbose;
};


struct ElevationEdgeData {
	number _delta_elevation = 0.0;
};


struct UnitsPositionEdgeData {
	std::vector<uint> _ids;
};


struct TerrainEdgeData {
	TERRAIN_TYPE _type = UNKNOWN;
};


struct Map {
	Map();
	Map(std::string unit_types_dir, std::string elements_dir, pt_2d origin, pt_2d size, pt_2d path_resolution, pt_2d elevation_resolution, time_point t);
	~Map();
	void add_unit(std::string type_name, pt_2d pos, time_point t);
	void add_static_element(std::string element_name, pt_3d pos, pt_3d size);
	void add_river(pt_2d src);
	void add_lake(pt_2d src);
	
	void update_alti_grid(GraphGrid * grid);
	void update_alti_path(Unit * unit);
	void update_elevation_grids();
	
	void update_terrain_grids_with_elevation(BBox_2D * bbox = NULL);
	
	void sync2elevation();
	
	void clear_units_position_grids();
	std::vector<std::pair<uint, uint> > waiting_unit_positions_edges(Unit * unit, UnitType * unit_type);
	std::vector<std::pair<uint, uint> > moving_unit_positions_edges(Unit * unit, UnitType * unit_type, bool all);
	void add_waiting_unit_to_position_grids(Unit * unit);
	void remove_waiting_unit_from_position_grids(Unit * unit);
	void add_moving_unit_to_position_grids(Unit * unit);
	void remove_moving_unit_from_position_grids(Unit * unit, bool all);

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
	std::map<UnitType *, PathFinder *> _path_finders;
	Elevation * _elevation;
	Elements * _elements;
	std::vector<River * > _rivers;
	std::vector<Lake *> _lakes;

	bool _paused;
	std::thread _path_find_thr;
	bool _path_find_thr_active;
};


#endif
