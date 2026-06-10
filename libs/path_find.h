#ifndef PATH_FIND_H
#define PATH_FIND_H

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <map>

#include <glm/glm.hpp>

#include "graph.h"
#include "typedefs.h"
#include "bbox_2d.h"


const number VERTEX_OBSTACLE_THRESH = 1000.0;

const uint N_VERTICES_CHECK = 4;

const uint WAIT_N_MS = 3000;
const uint PATH_FIND_N_MS = 100;


enum GMO_STATUS {GMO_IDLE, GMO_MOVING, GMO_WAITING};
enum GRID_VERTEX_TYPE {GRID_VERTEX_LAND, GRID_VERTEX_WATER, GRID_VERTEX_LAND_OBSTACLE};
enum GRID_EDGE_TYPE {GRID_EDGE_FLAT, GRID_EDGE_SOFT_DOWN, GRID_EDGE_SOFT_UP, GRID_EDGE_HARD_DOWN, GRID_EDGE_HARD_UP};


struct GridMovingObjectType {
	GridMovingObjectType();
	GridMovingObjectType(std::string name, std::map<GRID_VERTEX_TYPE, number> vertex_cost, std::map<GRID_EDGE_TYPE, number> edge_cost);
	~GridMovingObjectType();


	std::string _name;
	std::map<GRID_VERTEX_TYPE, number> _vertex_cost;
	std::map<GRID_EDGE_TYPE, number> _edge_cost;
};


struct GridMovingObject {
	GridMovingObject();
	GridMovingObject(GridMovingObjectType * type, pt_2d pos, pt_2d size);
	~GridMovingObject();


	GridMovingObjectType * _type;
	AABB_2D * _aabb;
	std::vector<uint> _path;
	uint _idx_path;
	uint _id;
	std::vector<uint> _vertices;
	GMO_STATUS _status;
	time_point _t_wait;
	uint _n_grid_size;
};


struct PathFinderVertexData {
	GridMovingObject * _gmo;
	GRID_VERTEX_TYPE _type;
};


struct PathFinderEdgeData {
	GRID_EDGE_TYPE _type;
};


struct PathFinderInput {
	PathFinderInput();
	PathFinderInput(GridMovingObject * gmo, uint goal);
	~PathFinderInput();

	GridMovingObject * _gmo;
	uint _goal;
};


struct PathFinder : public GraphGrid {
	PathFinder();
	PathFinder(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols, time_point t);
	~PathFinder();
	GridMovingObjectType * get_gmo_type(std::string name);
	PathFinderVertexData * get_vertex_data(uint id_vertex);
	PathFinderEdgeData * get_edge_data(uint from , uint to);
	number cost(GridMovingObject * gmo, uint from, uint to);
	number heuristic(uint i, uint j);
	void path_find(PathFinderInput * input);
	//GridMovingObject * add_gmo(std::string gmo_type_name, pt_2d pt, number size);
	//GridMovingObject * get_gmo(uint id);
	void init_gmo(GridMovingObject * gmo);
	void update_gmo_grid(GridMovingObject * gmo);
	void goto_gmo(GridMovingObject * gmo, uint id_vertex);
	void goto_gmo(GridMovingObject * gmo, pt_2d target);
	void stop_gmo(GridMovingObject * gmo);
	void clear_edges();
	void randomize_edges();
	void set_vertex(pt_2d center, number size, GRID_VERTEX_TYPE type);
	void set_edges(pt_2d center, number size, GRID_EDGE_TYPE type);
	void parse_input_queue(time_point t);
	void anim_gmos(std::vector<GridMovingObject *> & gmos, time_point t);


	std::vector<GridMovingObjectType *> _gmo_types;
	//std::vector<GridMovingObject *> _gmos;
	std::queue<PathFinderInput *> _inputs;
	time_point _t_last_path_find;

	static uint _next_gmo_id;
};


#endif
