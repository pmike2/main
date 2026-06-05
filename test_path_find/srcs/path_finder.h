#ifndef PATH_FINDER_H
#define PATH_FINDER_H

#include <string>
#include <vector>
#include <set>
#include <algorithm>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "gl_utils.h"
#include "input_state.h"
#include "font.h"
#include "gl_draw.h"
#include "graph.h"
#include "typedefs.h"
#include "thread.h"
#include "bbox_2d.h"
#include "repere.h"


const pt_2d GRID_ORIGIN(-50.0, -50.0);
const pt_2d GRID_SIZE(100.0, 100.0);
const pt_2d GRID_RESOLUTION(1.0);

const number GRID_CENTER_SIZE_RATIO = 0.1;

const float Z_EDGE = 0.0;
const float Z_CENTER = 0.01;
const float Z_GMO = 0.02;
const float Z_GMO_PATH = 0.03;
const float Z_FONT = 0.04;

const glm::vec4 SELECT_COLOR(1.0f, 1.0f, 0.0f, 0.5f);

const number HARD_EDGE = 100.0;
const number SOFT_EDGE = 10.0;
const number OCCUPIED_EDGE = 1000.0;

const uint N_VERTICES_CHECK = 4;

const uint WAIT_N_MS = 3000;


enum GMO_STATUS {GMO_IDLE, GMO_MOVING, GMO_WAITING};
enum VERTEX_STATUS {VERTEX_NO_OBSTACLE, VERTEX_OBSTACLE};


struct GridMovingObject {
	GridMovingObject();
	GridMovingObject(pt_2d pos, pt_2d size, number speed);
	~GridMovingObject();


	AABB_2D * _aabb;
	number _speed;
	std::vector<uint> _path;
	uint _idx_path;
	bool _selected;
	uint _id;
	//std::vector<uint_pair> _edges;
	std::vector<uint> _vertices;
	GMO_STATUS _status;
	time_point _t_wait;
};


struct VertexData {
	uint _id_gmo;
	VERTEX_STATUS _obstacle;
};


struct EdgeData {
	number _cost;
};


struct PathFinder : public GraphGrid {
	PathFinder();
	PathFinder(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols);
	~PathFinder();

	VertexData * get_vertex_data(uint id_vertex);
	EdgeData * get_edge_data(uint from , uint to);
	number cost(uint id_gmo, uint from, uint to);
	number heuristic(uint i, uint j);
	void path_find(uint id_gmo, uint start, uint goal, std::vector<uint> & path);
};


struct PathFinderTest {
	PathFinderTest();
	PathFinderTest(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t);
	~PathFinderTest();
	
	void add_gmo(pt_2d pt, number size, number speed);
	GridMovingObject * get_gmo(uint id);
	void delete_selected_gmos();
	void goto_gmo(GridMovingObject * gmo, uint id_vertex);
	void goto_gmo(GridMovingObject * gmo, pt_2d target);
	void goto_selected_gmos(pt_2d target);
	void stop_gmo(GridMovingObject * gmo);
	void clear_edges();
	void randomize_edges();
	void update_gmo_grid(GridMovingObject * gmo);
	void set_vertex(pt_2d center, number size, VERTEX_STATUS status);
	void set_edges(pt_2d center, number size, number value);
	void anim(time_point t);
	
	void draw_font();
	void draw_grid_centers();
	void draw_grid_edges();
	void draw_gmos();
	void draw_gmos_path();
	void draw_select();
	void draw();
	
	void update_font();
	void update_select();
	void update_grid_centers();
	void update_grid_edges();
	void update_gmos();
	void update_gmos_path();
	void update();
	
	bool mouse_button_down(InputState * input_state, time_point t);
	bool mouse_button_up(InputState * input_state, time_point t);
	bool mouse_motion(InputState * input_state, time_point t);
	bool key_down(InputState * input_state, SDL_Keycode key, time_point t);
	bool key_up(InputState * input_state, SDL_Keycode key, time_point t);


	std::vector<GridMovingObject *> _gmos;
	PathFinder * _pfi;
	GLDrawManager * _gl_draw_manager;
	Font * _font;
	ViewSystem * _view_system;

	static uint _next_gmo_id;
	uint _debug = 0;
};


#endif
