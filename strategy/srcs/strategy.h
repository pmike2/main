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
#include "gl_draw.h"

#include "const.h"
#include "map.h"


enum STRATEGY_MODE {VIEW, PLAY, ADD_UNIT, ADD_ELEMENT, EDIT_ELEVATION, ERASE};
enum PLAY_MODE {SELECT_UNIT, MOVE_UNIT, ATTACK_UNIT};

const glm::vec4 SELECTED_UNIT_COLOR(1.0f, 1.0f, 0.0f, 1.0f);
const glm::vec4 EDIT_MAP_COLOR(0.0f, 1.0f, 1.0f, 1.0f);

const number Z_OFFSET_EDGE = 0.05;
const number Z_OFFSET_UNIT = 0.05;
const number Z_OFFSET_PATH = 0.5;
const number Z_OFFSET_EDIT_MAP = 0.5;
const number Z_OFFSET_UNIT_PATH_BBOX = 0.5;
const number Z_OFFSET_SELECTION = 0.2;
const number Z_OFFSET_ATTACK_UNIT = 0.2;
const number Z_OFFSET_MOVE_UNIT = 0.2;

const glm::vec3 light_position(0.0f, 0.0f, 50.0f);
const glm::vec3 light_color(1.0f);

const uint EDIT_MAP_N_VERTICES_PER_CIRCLE = 32;
const uint SELECTION_N_VERTICES_PER_CIRCLE = 8;
const uint ATTACK_UNIT_N_VERTICES_PER_CIRCLE = 8;

const number DEFAULT_ATTACK_UNIT_CIRCLE_RADIUS = 1.0;
const number MOVE_UNIT_SEGMENT_SIZE = 1.5;

const pt_2d MAP_ORIGIN(-100.0, -100.0);
const pt_2d MAP_SIZE(200.0, 200.0);
const pt_2d PATH_RESOLUTION(2.0);
const pt_2d ELEVATION_RESOLUTION(1.0);
const pt_2d FOW_RESOLUTION(2.0);

const number LAKE_WAVE_AMPLITUDE = 0.2;
const number LAKE_WAVE_FREQ = 2.0;
const number SEA_WAVE_AMPLITUDE = 0.2;
const number SEA_WAVE_FREQ = 0.2;


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
	number _erase_radius;
	bool _units_paused;
	uint _selected_team_idx;
	bool _fow_active;
};


class Strategy {
public:
	Strategy();
	Strategy(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t);
	~Strategy();

	Team * get_selected_team();
	
	void set_ihm();
	
	void draw_select();
	void draw_linear(std::string context_name);
	void draw_dash(std::string context_name, number dash_size, number gap_size, number thickness);
	void draw_tree_stone();
	void draw_elevation();
	void draw_lake();
	void draw_river();
	void draw_sea();
	void draw_unit(UnitType * unit_type);
	void draw_unit_life();
	void draw_ammo(AmmoType * ammo_type);
	//void draw_fow();
	void draw();
	
	void anim(time_point t);
	
	glm::vec4 get_grid_edge_color();
	glm::vec4 get_path_color(number weight);
	
	void update_select();
	void update_grid();
	void update_bbox();
	void update_path();
	void update_edit_map();
	void update_cursor();
	void update_debug();
	void update_elevation();
	void update_tree_stone();
	void update_river();
	void update_lake();
	void update_sea();
	void update_unit_obj(UnitType * unit_type);
	void update_unit_matrices(UnitType * unit_type);
	void update_unit_life();
	void update_ammo_obj(AmmoType * ammo_type);
	void update_ammo_matrices(AmmoType * ammo_type);
	void update_selection();
	void update_fow_texture();
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
	bool _cursor_hover_ihm;
	number _angle;
	Unit * _cursor_hover_unit;

	//uint _texture_fow;
};

#endif
