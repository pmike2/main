#ifndef STRATEGY
#define STRATEGY

#include <string>
#include <vector>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "gl_utils.h"
#include "input_state.h"
#include "font.h"
#include "typedefs.h"
#include "repere.h"
#include "gl_ihm.h"

#include "map.h"


enum STRATEGY_MODE {PLAY, ADD_UNIT, ADD_ELEMENT, EDIT_ELEVATION};

const uint NEW_PT_IN_POLYGON_MS = 300;

const std::map<std::string, glm::vec4> UNIT_COLORS = {
	{"infantery", glm::vec4(1.0f, 0.7f, 0.2f, 1.0f)},
	{"tank", glm::vec4(0.7f, 0.7f, 0.6f, 1.0f)},
	{"boat", glm::vec4(0.6f, 0.4f, 0.9f, 1.0f)}
};
const glm::vec4 SELECTED_UNIT_COLOR(1.0f, 1.0f, 0.0f, 1.0f);

const number Z_OFFSET_EDGE = 0.05;
const number Z_OFFSET_UNIT = 0.05;
const number Z_OFFSET_PATH = 0.5;

const glm::vec3 light_position(0.0f, 0.0f, 50.0f);
const glm::vec3 light_color(1.0f);


class Strategy {
public:
	Strategy();
	Strategy(std::map<std::string, GLuint> progs, ViewSystem * view_system, time_point t);
	~Strategy();
	void set_ihm();
	void draw_linear(std::string context_name);
	void draw_dash(std::string context_name);
	void draw_surface(std::string context_name);
	void draw_lake();
	void draw_river();
	void draw_sea();
	void draw_unit(UnitType * unit_type);
	void draw();
	void anim(time_point t, InputState * input_state);
	//GraphGrid * get_visible_grid();
	glm::vec4 get_edge_color();
	void update_grid();
	//void update_obstacle();
	void update_unit_linear();
	void update_path();
	void update_debug();
	void update_elevation();
	void update_elements();
	void update_river();
	void update_lake();
	void update_sea();
	void update_unit_obj(UnitType * unit_type);
	void update_unit_matrices(UnitType * unit_type);
	void update_all();
	void update_text(InputState * input_state);
	bool mouse_button_down(InputState * input_state, time_point t);
	bool mouse_button_up(InputState * input_state, time_point t);
	bool mouse_motion(InputState * input_state, time_point t);
	bool key_down(InputState * input_state, SDL_Keycode key, time_point t);
	bool key_up(InputState * input_state, SDL_Keycode key, time_point t);

	
	std::map<std::string, DrawContext *> _contexts;
	STRATEGY_MODE _mode;
	Font * _font;
	ViewSystem * _view_system;
	Map * _map;

	// TODO : remplacer ces strings par des enums
	std::string _visible_grid_type;
	std::string _visible_grid_unit_type;
	std::string _current_unit_type_name;
	std::string _current_element_name;
	std::string _current_elevation_mode;

	number _angle;

	GLIHM * _ihm;
};

#endif
