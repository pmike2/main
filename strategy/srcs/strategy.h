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


struct StrategyConfig {
	StrategyConfig();
	~StrategyConfig();


	bool _edit;
	bool _show_info;
	bool _units_paused;
	bool _fow_active;
	PLAY_MODE _play_mode;
	EDIT_MODE _edit_mode;
	UNIT_ACTION_MODE _unit_action_mode;
	VISIBLE_GRID_TYPE _visible_grid_type;
	UNIT_TYPE _visible_grid_unit_type;
	UNIT_TYPE _add_unit_type;
	ELEMENT_TYPE _element_type;
	ELEVATION_MODE _elevation_mode;
	number _elevation_radius;
	number _elevation_factor;
	number _elevation_exponent;
	number _trees_dispersion;
	number _stones_dispersion;
	number _erase_radius;
	uint _n_trees;
	uint _n_stones;
	uint _selected_team_idx;
};


class Strategy {
public:
	Strategy();
	Strategy(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t);
	~Strategy();

	Team * get_selected_team();
	void zoom2first_unit_of_selected_team();
	
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
	void update_angles();
	
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
	bool _cursor_in_world;
	bool _cursor_hover_ihm;
	Unit * _cursor_hover_unit;

	bool _fow_ok, _add_unit_ok, _add_unit_fow_ok, _move_unit_ok, _attack_unit_ok;

	number _angle_lake, _angle_river, _angle_sea;
};

#endif
