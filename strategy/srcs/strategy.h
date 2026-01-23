#ifndef STRATEGY_H
#define STRATEGY_H

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

#include "const.h"
#include "map.h"


enum STRATEGY_MODE {VIEW, PLAY, ADD_UNIT, ADD_ELEMENT, EDIT_ELEVATION};
enum PLAY_MODE {SELECT_UNIT, MOVE_UNIT};

const glm::vec4 SELECTED_UNIT_COLOR(1.0f, 1.0f, 0.0f, 1.0f);
const glm::vec4 EDIT_MAP_COLOR(0.0f, 1.0f, 1.0f, 0.5f);

const number Z_OFFSET_EDGE = 0.05;
const number Z_OFFSET_UNIT = 0.05;
const number Z_OFFSET_PATH = 0.5;
const number Z_OFFSET_EDIT_MAP = 0.5;

const glm::vec3 light_position(0.0f, 0.0f, 50.0f);
const glm::vec3 light_color(1.0f);

const uint EDIT_MAP_N_VERTICES_PER_CIRCLE = 32;


struct StrategyConfig {
	StrategyConfig();
	~StrategyConfig();


	bool _show_info;
	STRATEGY_MODE _mode;
	PLAY_MODE _play_mode;
	VISIBLE_GRID_TYPE _visible_grid_type;
	UNIT_TYPE _visible_grid_unit_type;
	UNIT_TYPE _unit_type;
	ELEMENT_TYPE _element_type;
	ELEVATION_MODE _elevation_mode;
	number _elevation_radius;
	number _elevation_factor;
	number _elevation_exponent;
	uint _n_elements;
	number _elements_dispersion;
	bool _units_paused;
};


class Strategy {
public:
	Strategy();
	Strategy(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t);
	~Strategy();
	
	void set_ihm();
	
	void draw_select();
	void draw_linear(std::string context_name);
	void draw_dash(std::string context_name);
	void draw_surface(std::string context_name);
	void draw_lake();
	void draw_river();
	void draw_sea();
	void draw_unit(UnitType * unit_type);
	void draw();
	
	void anim(time_point t);
	
	glm::vec4 get_grid_edge_color();
	glm::vec4 get_path_color(number weight);
	
	void update_select();
	void update_grid();
	void update_unit_linear();
	void update_path();
	void update_edit_map();
	void update_debug();
	void update_elevation();
	void update_tree_stone();
	void update_river();
	void update_lake();
	void update_sea();
	void update_unit_obj(UnitType * unit_type);
	void update_unit_matrices(UnitType * unit_type);
	void update_all();
	void update_text();
	
	bool mouse_button_down(InputState * input_state, time_point t);
	bool mouse_button_up(InputState * input_state, time_point t);
	bool mouse_motion(InputState * input_state, time_point t);
	bool key_down(InputState * input_state, SDL_Keycode key, time_point t);
	bool key_up(InputState * input_state, SDL_Keycode key, time_point t);

	
	GLDrawManager * _gl_draw_manager;
	Font * _font;
	ViewSystem * _view_system;
	GLIHM * _ihm;
	Map * _map;
	StrategyConfig * _config;

	pt_3d _cursor_world_position;

	number _angle;
};

#endif
