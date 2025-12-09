#ifndef TEST_GEOM
#define TEST_GEOM

#include <string>
#include <vector>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "path_find.h"
#include "gl_utils.h"
#include "input_state.h"
#include "font.h"
#include "typedefs.h"
#include "repere.h"


enum EDIT_MODE {ADDING_SOLID_OBSTACLE, ADDING_WATER_OBSTACLE, FREE, EDIT_ALTI};

const uint NEW_PT_IN_POLYGON_MS = 300;
const number CROSS_SIZE = 0.2;

const glm::vec4 GRID_COLOR(0.8f, 0.8f, 0.7f, 1.0f);
const std::map<OBSTACLE_TYPE, glm::vec4> OBSTACLE_COLORS = {
	{SOLID, glm::vec4(1.0f, 0.5f, 0.1f, 1.0f)},
	{WATER, glm::vec4(0.1f, 0.5f, 0.9f, 1.0f)}
};
const std::map<EDIT_MODE, glm::vec4> EDITED_OBSTACLE_COLORS = {
	{ADDING_SOLID_OBSTACLE, glm::vec4(1.0f, 0.5f, 0.1f, 1.0f)},
	{ADDING_WATER_OBSTACLE, glm::vec4(0.1f, 0.5f, 0.9f, 1.0f)}
};
const std::map<std::string, glm::vec4> UNIT_COLORS = {
	{"infantery", glm::vec4(1.0f, 0.7f, 0.2f, 1.0f)},
	{"tank", glm::vec4(0.5f, 0.5f, 0.6f, 1.0f)},
	{"boat", glm::vec4(0.3f, 0.4f, 0.7f, 1.0f)}
};
const glm::vec4 SELECTED_UNIT_COLOR(1.0f, 1.0f, 0.0f, 1.0f);
const glm::vec4 PATH_COLOR(0.7f, 0.8f, 0.3f, 1.0f);

const number ALTI_UNIT = 0.0;
const number ALTI_PATH = -0.1;
const number ALTI_OBSTACLE = -0.2;
const number ALTI_CROSS = -0.3;
const number ALTI_EDGE = -0.4;
const number ALTI_TERRAIN = -0.5;



class TestAStar {
public:
	TestAStar();
	TestAStar(std::map<std::string, GLuint> progs, ViewSystem * view_system);
	~TestAStar();
	void draw_linear(std::string context_name);
	void draw_terrain();
	void draw();
	void anim(time_point t, InputState * input_state);
	void update_grid();
	void update_obstacle();
	void update_unit();
	void update_path();
	void update_terrain();
	void update_all();
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool mouse_motion(InputState * input_state, time_point t);
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);

	
	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	EDIT_MODE _mode;
	std::vector<pt_type> _obstacle_pts;
	time_point _last_added_pt_t;
	Font * _font;
	ViewSystem * _view_system;
	Map * _map;
};

#endif
