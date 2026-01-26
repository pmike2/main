#include <iostream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

#include "strategy.h"



StrategyConfig::StrategyConfig() {
	_show_info = false;
	_mode = PLAY;
	_play_mode = SELECT_UNIT;
	_unit_type = INFANTERY;
	_visible_grid_unit_type = INFANTERY;
	_visible_grid_type = TERRAIN;
	_element_type = ELEMENT_TREE;
	_elevation_mode = ELEVATION_ZERO;
	_elevation_radius = 1.0;
	_elevation_factor = 1.0;
	_elevation_exponent = 2.0;
	_n_elements = 1;
	_elements_dispersion = 0.0;
}


StrategyConfig::~StrategyConfig() {
	
}


// -------------------------------------------------
Strategy::Strategy() {

}


Strategy::Strategy(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t) :
	_gl_draw_manager(gl_draw_manager), _view_system(view_system), _angle(0.0)
{
	_config = new StrategyConfig();

	_font = new Font(_gl_draw_manager, "../../fonts/Silom.ttf", 48, _view_system->_screengl);
	_font->_z= 100.0f; // pour que l'affichage des infos se fassent par dessus le reste

	_ihm = new GLIHM(_gl_draw_manager, _view_system->_screengl, "../data/ihm.json");

	_map = new Map("../data/unit_types", "../data/elements", MAP_ORIGIN, MAP_SIZE, PATH_RESOLUTION, ELEVATION_RESOLUTION, t);

	//_map->randomize();
	_map->clear();

	update_all();

	set_ihm();
}


Strategy::~Strategy() {
	delete _map;
	delete _font;
	delete _config;
	delete _ihm;
}


void Strategy::set_ihm() {
	_ihm->get_element("general", "show_info")->set_callback([this](){_config->_show_info = true;}, [this](){_config->_show_info = false;});
	_ihm->get_element("general", "units_paused")->set_callback(
		[this](){_config->_units_paused = true; _map->pause_all_units(true);},
		[this](){_config->_units_paused = false; _map->pause_all_units(false);}
	);

	_ihm->get_element("mode", "view")->set_callback([this](){_config->_mode = VIEW;});
	_ihm->get_element("mode", "play")->set_callback([this](){_config->_mode = PLAY;});
	_ihm->get_element("mode", "add_unit")->set_callback([this](){_config->_mode = ADD_UNIT;});
	_ihm->get_element("mode", "add_element")->set_callback([this](){_config->_mode = ADD_ELEMENT;});
	_ihm->get_element("mode", "edit_elevation")->set_callback([this](){_config->_mode = EDIT_ELEVATION;});

	_ihm->get_element("play_mode", "select_unit")->set_callback([this](){_config->_play_mode = SELECT_UNIT;});
	_ihm->get_element("play_mode", "move_unit")->set_callback([this](){_config->_play_mode = MOVE_UNIT;});
	
	for (auto & unit_type : std::vector<UNIT_TYPE>{INFANTERY, TANK, HELICOPTER, BOAT}) {
		_ihm->get_element("units", unit_type2str(unit_type))->set_callback([this, unit_type](){_config->_unit_type = unit_type;});
	}
	
	for (auto & element_type : std::vector<ELEMENT_TYPE>{ELEMENT_TREE, ELEMENT_STONE, ELEMENT_RIVER, ELEMENT_LAKE}) {
		_ihm->get_element("elements", element_type2str(element_type))->set_callback([this, element_type](){_config->_element_type = element_type;});
	}
	
	_ihm->get_element("elements_params", "n_elements")->set_callback([this](){
		GLIHMElement * element = _ihm->get_element("elements_params", "n_elements");
		GLIHMSlider * slider = (GLIHMSlider *)(element);
		_config->_n_elements = slider->_value;
	});

	_ihm->get_element("elements_params", "elements_dispersion")->set_callback([this](){
		GLIHMElement * element = _ihm->get_element("elements_params", "elements_dispersion");
		GLIHMSlider * slider = (GLIHMSlider *)(element);
		_config->_elements_dispersion = slider->_value;
	});

	for (auto & elevation_mode : std::vector<ELEVATION_MODE>{ELEVATION_ZERO, ELEVATION_PLUS, ELEVATION_MINUS}) {
		_ihm->get_element("elevation", elevation_mode2str(elevation_mode))->set_callback([this, elevation_mode](){_config->_elevation_mode = elevation_mode;});
	}

	_ihm->get_element("edit_elevation_params", "elevation_radius")->set_callback([this](){
		GLIHMElement * element = _ihm->get_element("edit_elevation_params", "elevation_radius");
		GLIHMSlider * slider = (GLIHMSlider *)(element);
		_config->_elevation_radius = slider->_value;
	});
	
	_ihm->get_element("edit_elevation_params", "elevation_factor")->set_callback([this](){
		GLIHMElement * element = _ihm->get_element("edit_elevation_params", "elevation_factor");
		GLIHMSlider * slider = (GLIHMSlider *)(element);
		_config->_elevation_factor = slider->_value;
	});

	_ihm->get_element("edit_elevation_params", "elevation_exponent")->set_callback([this](){
		GLIHMElement * element = _ihm->get_element("edit_elevation_params", "elevation_exponent");
		GLIHMSlider * slider = (GLIHMSlider *)(element);
		_config->_elevation_exponent = slider->_value;
	});

	_ihm->get_element("global_edit", "randomize")->set_callback([this](){
		_map->randomize();
		update_all();
	});
	
	_ihm->get_element("global_edit", "clear")->set_callback([this](){
		_map->clear();
		update_all();
	});
	
	_ihm->get_element("global_edit", "load")->set_callback([this](){
		_map->load("../data/map.json");
	});
	
	_ihm->get_element("global_edit", "save")->set_callback([this](){
		_map->save("../data/map.json");
	});
	
	for (auto & visu_type : std::vector<std::string>{"elevation", "grid", "bbox", "sea"}) {
		_ihm->get_element("visu", visu_type)->set_callback(
			[this, visu_type](){_gl_draw_manager->set_active(visu_type);}, 
			[this, visu_type](){_gl_draw_manager->set_inactive(visu_type);}
		);
	}
	
	_ihm->get_element("visu", "elements")->set_callback(
		[this](){
			_gl_draw_manager->set_active("tree_stone");
			_gl_draw_manager->set_active("river");
			_gl_draw_manager->set_active("lake");
		},
		[this](){
			_gl_draw_manager->set_inactive("tree_stone");
			_gl_draw_manager->set_inactive("river");
			_gl_draw_manager->set_inactive("lake");
		}
	);
	
	_ihm->get_element("visu", "repere")->set_callback(
		[this](){
			_gl_draw_manager->set_active("repere");
			_gl_draw_manager->set_inactive("ground");
			_gl_draw_manager->set_active("box");
		},
		[this](){
			_gl_draw_manager->set_inactive("repere");
			_gl_draw_manager->set_inactive("ground");
			_gl_draw_manager->set_inactive("box");
		}
	);
	
	_ihm->get_element("visu", "units")->set_callback(
		[this](){
			for (auto & unit_type : _map->_unit_types) {
				_gl_draw_manager->set_active(unit_type2str(unit_type.first));
			}
		},
		[this](){
			for (auto & unit_type : _map->_unit_types) {
				_gl_draw_manager->set_inactive(unit_type2str(unit_type.first));
			}
		}
	);
	
	for (auto & grid_type : std::vector<VISIBLE_GRID_TYPE>{ELEVATION, TERRAIN, UNITS_POSITION}) {
		_ihm->get_element("grid_type", visible_grid2str(grid_type))->set_callback([this, grid_type](){
			_config->_visible_grid_type = grid_type;
			update_grid();
		});
	}
	
	for (auto & unit_type_name : std::vector<UNIT_TYPE>{INFANTERY, TANK, HELICOPTER, BOAT}) {
		_ihm->get_element("grid_unit_type", unit_type2str(unit_type_name))->set_callback([this, unit_type_name](){
			_config->_visible_grid_unit_type = unit_type_name;
			update_grid();
		});
	}

	_ihm->all_callbacks(); // synchro de l'état de l'ihm avec l'état de strategy

	//std::cout << *_ihm << "\n";
}


void Strategy::draw_select() {
	DrawContext * context= _gl_draw_manager->get_context("select");
	if (!context->_active) {
		return;
	}

	context->activate();
	// pour que l'affichage du rectangle de sélection se fassent par dessus le reste
	glUniform1f(context->_locs_uniform["z"], -1.0);
	glUniform1f(context->_locs_uniform["thickness"], 7.0);
	glUniform2f(context->_locs_uniform["viewport_size"], _view_system->_screengl->_screen_width, _view_system->_screengl->_screen_height);
	glDrawArrays(GL_LINES, 0, context->_n_pts);
	context->deactivate();
}


void Strategy::draw_linear(std::string context_name) {
	DrawContext * context= _gl_draw_manager->get_context(context_name);
	if (!context->_active) {
		return;
	}

	context->activate();
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	glDrawArrays(GL_LINES, 0, context->_n_pts);
	context->deactivate();
}


void Strategy::draw_dash(std::string context_name) {
	DrawContext * context= _gl_draw_manager->get_context(context_name);
	if (!context->_active) {
		return;
	}

	context->activate();
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	glUniform1f(context->_locs_uniform["dash_size"], 4.0);
	glUniform1f(context->_locs_uniform["gap_size"], 2.0);
	glUniform1f(context->_locs_uniform["thickness"], 2.0);
	glUniform2f(context->_locs_uniform["viewport_size"], _view_system->_screengl->_screen_width, _view_system->_screengl->_screen_height);
	glDrawArrays(GL_LINES, 0, context->_n_pts);
	context->deactivate();

}


void Strategy::draw_surface(std::string context_name) {
	DrawContext * context= _gl_draw_manager->get_context(context_name);
	if (!context->_active) {
		return;
	}

	context->activate();
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	glUniform3fv(context->_locs_uniform["light_position"], 1, glm::value_ptr(light_position));
	glUniform3fv(context->_locs_uniform["light_color"], 1, glm::value_ptr(light_color));
	glUniform3fv(context->_locs_uniform["view_position"], 1, glm::value_ptr(glm::vec3(_view_system->_eye)));
	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);
	context->deactivate();
}


void Strategy::draw_lake() {
	DrawContext * context= _gl_draw_manager->get_context("lake");
	if (!context->_active) {
		return;
	}

	context->activate();
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	glUniform3fv(context->_locs_uniform["light_position"], 1, glm::value_ptr(light_position));
	glUniform3fv(context->_locs_uniform["light_color"], 1, glm::value_ptr(light_color));
	glUniform3fv(context->_locs_uniform["view_position"], 1, glm::value_ptr(glm::vec3(_view_system->_eye)));
	glUniform1f(context->_locs_uniform["angle"], float(_angle));
	glUniform1f(context->_locs_uniform["amplitude"], 0.2f);
	glUniform1f(context->_locs_uniform["freq"], 2.0f);
	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);
	context->deactivate();
}


void Strategy::draw_river() {
	DrawContext * context= _gl_draw_manager->get_context("river");
	if (!context->_active) {
		return;
	}

	context->activate();
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	glUniform3fv(context->_locs_uniform["light_position"], 1, glm::value_ptr(light_position));
	glUniform3fv(context->_locs_uniform["light_color"], 1, glm::value_ptr(light_color));
	glUniform3fv(context->_locs_uniform["view_position"], 1, glm::value_ptr(glm::vec3(_view_system->_eye)));
	glUniform1f(context->_locs_uniform["angle"], float(_angle));
	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);
	context->deactivate();
}


void Strategy::draw_sea() {
	DrawContext * context= _gl_draw_manager->get_context("sea");
	if (!context->_active) {
		return;
	}

	context->activate();
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	glUniform3fv(context->_locs_uniform["light_position"], 1, glm::value_ptr(light_position));
	glUniform3fv(context->_locs_uniform["light_color"], 1, glm::value_ptr(light_color));
	glUniform3fv(context->_locs_uniform["view_position"], 1, glm::value_ptr(glm::vec3(_view_system->_eye)));
	glUniform1f(context->_locs_uniform["angle"], float(_angle));
	glUniform1f(context->_locs_uniform["amplitude"], 0.2f);
	glUniform1f(context->_locs_uniform["freq"], 0.2f);
	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);
	context->deactivate();
}


void Strategy::draw_unit(UnitType * unit_type) {
	DrawContext * context= _gl_draw_manager->get_context(unit_type2str(unit_type->_type));
	if (!context->_active) {
		return;
	}

	context->activate();
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	glUniform3fv(context->_locs_uniform["light_position"], 1, glm::value_ptr(light_position));
	glUniform3fv(context->_locs_uniform["light_color"], 1, glm::value_ptr(light_color));
	glUniform3fv(context->_locs_uniform["view_position"], 1, glm::value_ptr(glm::vec3(_view_system->_eye)));
	glDrawArraysInstanced(GL_TRIANGLES, 0, context->_n_pts, context->_n_instances);
	context->deactivate();
}


void Strategy::draw() {
	for (auto context_name : std::vector<std::string>{"grid", "bbox"}) {
		draw_linear(context_name);
	}

	for (auto context_name : std::vector<std::string>{"elevation", "tree_stone"}) {
		draw_surface(context_name);
	}
	
	for (auto context_name : std::vector<std::string>{"path", "edit_map"}) {
		draw_dash(context_name);
	}

	for (auto & unit_type : _map->_unit_types) {
		draw_unit(unit_type.second);
	}

	draw_lake();
	draw_river();
	draw_sea();

	draw_select();
	
	if (_config->_show_info) {
		_font->draw_3d(_view_system->_world2clip);
		_font->draw();
	}

	_ihm->draw();
}


void Strategy::anim(time_point t) {
	_ihm->anim();

	_map->anim(t);

	update_select();
	update_bbox();
	update_path();
	update_edit_map();
	update_text();
	for (auto & unit_type : _map->_unit_types) {
		update_unit_matrices(unit_type.second);
	}

	_angle += 0.01;
	if (_angle > 2.0 * M_PI) {
		_angle -= 2.0 * M_PI;
	}
	update_lake();
}


glm::vec4 Strategy::get_grid_edge_color() {
	GraphEdge edge = _map->_path_finder->_it_e->second;
	EdgeData * data = (EdgeData *)(edge._data);
	UnitType * unit_type = _map->_unit_types[_config->_visible_grid_unit_type];
	glm::vec4 edge_color;

	if (_config->_visible_grid_type == ELEVATION) {
		if (data->_delta_elevation[unit_type] < 0.0) {
			edge_color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		}
		else if (data->_delta_elevation[unit_type] > 100.0) {
			edge_color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		}
		else {
			edge_color = glm::vec4(float(data->_delta_elevation[unit_type]) / 100.0f, 1.0f - float(data->_delta_elevation[unit_type]) / 100.0f, 0.5f, 1.0f);
		}
	}
	else if (_config->_visible_grid_type == TERRAIN) {
		if (data->_type[unit_type] == TERRAIN_SEA ) {
			edge_color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		}
		else if (data->_type[unit_type] == TERRAIN_GROUND ) {
			edge_color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
		}
		else if (data->_type[unit_type] == TERRAIN_OBSTACLE ) {
			edge_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else if (data->_type[unit_type] == TERRAIN_LAKE ) {
			edge_color = glm::vec4(0.5f, 0.0f, 1.0f, 1.0f);
		}
		else if (data->_type[unit_type] == TERRAIN_RIVER ) {
			edge_color = glm::vec4(0.0f, 0.5f, 1.0f, 1.0f);
		}
		else if (data->_type[unit_type] == TERRAIN_SEA_COAST ) {
			edge_color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		}
		else if (data->_type[unit_type] == TERRAIN_LAKE_COAST ) {
			edge_color = glm::vec4(1.0f, 1.0f, 0.5f, 1.0f);
		}
		else {
			edge_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
	else if (_config->_visible_grid_type == UNITS_POSITION) {
		if (data->_ids[unit_type].size() == 0) {
			edge_color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		}
		else if (data->_ids[unit_type].size() == 1) {
			edge_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else {
			edge_color = glm::vec4(0.5f, 0.0f, 1.0f, 1.0f);
		}
	}

	return edge_color;
}


glm::vec4 Strategy::get_path_color(number weight) {
	if (weight >= MAX_UNIT_MOVING_WEIGHT) {
		return glm::vec4(1.0, 0.0, 1.0, 1.0);
	}
	else if (weight > 100.0) {
		return glm::vec4(1.0, 0.0, 0.0, 1.0);
	}
	else if (weight > 50.0) {
		return glm::vec4(1.0, 1.0, 0.0, 1.0);
	}
	else {
		return glm::vec4(0.0, 1.0 - float(weight) / 50.0, 0.5, 1.0);
	}
}


void Strategy::update_select() {
	DrawContext * context= _gl_draw_manager->get_context("select");

	if (!_view_system->_rect_select->_is_active) {
		context->_active = false;
		return;
	}

	context->_active = true;
	context->_n_pts = 8;

	glm::vec4 SELECT_COLOR(1.0f, 1.0f, 0.0f, 0.5f);
	float * data = new float[context->data_size()];
	pt_2d pt_min = _view_system->_rect_select->_gl_origin;
	pt_2d pt_max = _view_system->_rect_select->_gl_moving;
	pt_2d pts[8] = {
		pt_2d(pt_min.x, pt_min.y), pt_2d(pt_max.x, pt_min.y),
		pt_2d(pt_max.x, pt_min.y), pt_2d(pt_max.x, pt_max.y),
		pt_2d(pt_max.x, pt_max.y), pt_2d(pt_min.x, pt_max.y),
		pt_2d(pt_min.x, pt_max.y), pt_2d(pt_min.x, pt_min.y)
	};
	float * ptr = data;
	for (uint i=0; i<8; ++i) {
		ptr[0] = float(pts[i].x);
		ptr[1] = float(pts[i].y);
		ptr[2] = float(SELECT_COLOR.r);
		ptr[3] = float(SELECT_COLOR.g);
		ptr[4] = float(SELECT_COLOR.b);
		ptr[5] = float(SELECT_COLOR.a);
		ptr += 6;
	}

	context->set_data(data);
	delete[] data;
}


void Strategy::update_grid() {
	DrawContext * context= _gl_draw_manager->get_context("grid");
	
	context->_n_pts = 0;

	_map->_path_finder->_it_v= _map->_path_finder->_vertices.begin();
	while (_map->_path_finder->_it_v!= _map->_path_finder->_vertices.end()) {
		_map->_path_finder->_it_e= _map->_path_finder->_it_v->second._edges.begin();
		while (_map->_path_finder->_it_e!= _map->_path_finder->_it_v->second._edges.end()) {
			context->_n_pts += 2;
			_map->_path_finder->_it_e++;
		}
		_map->_path_finder->_it_v++;
	}

	float * data = new float[context->data_size()];
	float * ptr = data;

	_map->_path_finder->_it_v= _map->_path_finder->_vertices.begin();
	while (_map->_path_finder->_it_v!= _map->_path_finder->_vertices.end()) {
		_map->_path_finder->_it_e= _map->_path_finder->_it_v->second._edges.begin();
		while (_map->_path_finder->_it_e!= _map->_path_finder->_it_v->second._edges.end()) {
			glm::vec4 edge_color = get_grid_edge_color();

			pt_3d & p1 = _map->_path_finder->_it_v->second._pos;
			pt_3d & p2 = _map->_path_finder->_vertices[_map->_path_finder->_it_e->first]._pos;
			pt_3d p1b = p1 + (p2 - p1) * 0.1;
			pt_3d p_middle = (p1 + p2) * 0.5 - (p2 - p1) * 0.1;
			
			ptr[0] = float(p1b.x);
			ptr[1] = float(p1b.y);
			ptr[2] = float(p1b.z + Z_OFFSET_EDGE);
			ptr[3] = edge_color.r;
			ptr[4] = edge_color.g;
			ptr[5] = edge_color.b;
			ptr[6] = edge_color.a;

			ptr[7] = float(p_middle.x);
			ptr[8] = float(p_middle.y);
			ptr[9] = float(p_middle.z + Z_OFFSET_EDGE);
			ptr[10] = edge_color.r;
			ptr[11] = edge_color.g;
			ptr[12] = edge_color.b;
			ptr[13] = edge_color.a;

			ptr += 14;
			
			_map->_path_finder->_it_e++;
		}
		_map->_path_finder->_it_v++;
	}

	context->set_data(data);

	delete[] data;
}


void Strategy::update_bbox() {
	DrawContext * context= _gl_draw_manager->get_context("bbox");

	context->_n_pts = 0;

	// BBox units + path
	for (auto & team : _map->_teams) {
		for (auto & unit : team->_units) {
			context->_n_pts += 48;
			if (!unit->_path->empty()) {
				context->_n_pts += 8 * unit->_path->_intervals.size();
				context->_n_pts += 8 * unit->_path->_intervals_los.size();
			}
		}
	}

	// BBox elements
	for (auto & element : _map->_elements->_elements) {
		context->_n_pts += 48;
	}

	float data[context->data_size()];
	float * ptr = data;

	// units
	for (auto & team : _map->_teams) {
		for (auto & unit : team->_units) {
			std::vector<pt_3d> segs = unit->_bbox->segments();

			glm::vec4 unit_color;
			if (unit->_status == MOVING) {
				unit_color = glm::vec4(0.5, 1.0, 0.5, 1.0);
			}
			else if (unit->_status == WAITING) {
				if (unit->_instructions.empty()) {
					unit_color = glm::vec4(1.0, 0.5, 0.5, 1.0);
				}
				else {
					unit_color = glm::vec4(0.5, 0.5, 1.0, 1.0);
				}
			}

			for (uint i=0; i<segs.size(); ++i) {
				ptr[0] = float(segs[i].x);
				ptr[1] = float(segs[i].y);
				ptr[2] = float(segs[i].z + Z_OFFSET_UNIT);
				if (unit->_selected) {
					ptr[3] = SELECTED_UNIT_COLOR.r;
					ptr[4] = SELECTED_UNIT_COLOR.g;
					ptr[5] = SELECTED_UNIT_COLOR.b;
					ptr[6] = SELECTED_UNIT_COLOR.a;
				}
				else {
					ptr[3] = unit_color.r;
					ptr[4] = unit_color.g;
					ptr[5] = unit_color.b;
					ptr[6] = unit_color.a;
				}
				ptr += 7;
			}
		}
	}

	// units path
	for (auto & team : _map->_teams) {
		for (auto & unit : team->_units) {
			if (unit->_path->empty()) {
				continue;
			}

			glm::vec4 unit_path_color(1.0, 0.8, 0.7, 1.0);
			for (auto & interval : unit->_path->_intervals) {
				std::vector<pt_2d> segs = interval->_bbox->segments();
				for (uint i=0; i<segs.size(); ++i) {
					number alti = _map->_elevation->get_alti(interval->_bbox->_center);
					ptr[0] = float(segs[i].x);
					ptr[1] = float(segs[i].y);
					ptr[2] = float(alti + Z_OFFSET_UNIT_PATH_BBOX);
					ptr[3] = unit_path_color.r;
					ptr[4] = unit_path_color.g;
					ptr[5] = unit_path_color.b;
					ptr[6] = unit_path_color.a;
					ptr += 7;
				}
			}

			glm::vec4 unit_path_color_los(0.8, 0.8, 1.0, 1.0);
			for (auto & interval : unit->_path->_intervals_los) {
				std::vector<pt_2d> segs = interval->_bbox->segments();
				for (uint i=0; i<segs.size(); ++i) {
					number alti = _map->_elevation->get_alti(interval->_bbox->_center);
					ptr[0] = float(segs[i].x);
					ptr[1] = float(segs[i].y);
					ptr[2] = float(alti + Z_OFFSET_UNIT_PATH_BBOX);
					ptr[3] = unit_path_color.r;
					ptr[4] = unit_path_color.g;
					ptr[5] = unit_path_color.b;
					ptr[6] = unit_path_color.a;
					ptr += 7;
				}
			}
		}
	}

	// elements
	for (auto & element : _map->_elements->_elements) {
		std::vector<pt_3d> segs = element->_bbox->_aabb->segments();

		glm::vec4 element_color(0.5, 1.0, 0.5, 1.0);

		for (uint i=0; i < segs.size(); ++i) {
			ptr[0] = float(segs[i].x);
			ptr[1] = float(segs[i].y);
			ptr[2] = float(segs[i].z + Z_OFFSET_UNIT);
			ptr[3] = element_color.r;
			ptr[4] = element_color.g;
			ptr[5] = element_color.b;
			ptr[6] = element_color.a;
			ptr += 7;
		}
	}

	context->set_data(data);
}


void Strategy::update_path() {
	DrawContext * context= _gl_draw_manager->get_context("path");
	
	context->_n_pts = 0;
	
	for (auto & team : _map->_teams) {
		for (auto & unit : team->_units) {
			if (!unit->_path->empty()) {
				context->_n_pts += (unit->_path->_pts.size() - 1) * 2;
				context->_n_pts += (unit->_path->_pts_los.size() - 1) * 2;
				context->_n_pts += 4; // croix de départ
				context->_n_pts += 4; // croix d'arrivée
			}
		}
	}

	float * data = new float[context->data_size()];
	float * ptr = data;

	for (auto & team : _map->_teams) {
		for (auto & unit : team->_units) {
			if (unit->_path->_pts.empty()) {
				continue;
			}

			// sans LOS
			for (uint i=0; i<unit->_path->_pts.size() - 1; ++i) {
				glm::vec4 path_color = get_path_color(unit->_path->_intervals[i]->_weight);
				
				ptr[0] = float(unit->_path->_pts[i].x);
				ptr[1] = float(unit->_path->_pts[i].y);
				ptr[2] = float(unit->_path->_pts[i].z + Z_OFFSET_PATH);
				ptr[3] = path_color.r;
				ptr[4] = path_color.g;
				ptr[5] = path_color.b;
				ptr[6] = path_color.a;

				ptr[7] = float(unit->_path->_pts[i + 1].x);
				ptr[8] = float(unit->_path->_pts[i + 1].y);
				ptr[9] = float(unit->_path->_pts[i + 1].z + Z_OFFSET_PATH);
				ptr[10] = path_color.r;
				ptr[11] = path_color.g;
				ptr[12] = path_color.b;
				ptr[13] = path_color.a;

				ptr += 14;
			}

			// avec LOS 
			const number Z_OFFSET_LOS = 1.0;
			for (uint i=0; i<unit->_path->_pts_los.size() - 1; ++i) {
				glm::vec4 path_color = get_path_color(unit->_path->_intervals_los[i]->_weight);
				
				ptr[0] = float(unit->_path->_pts_los[i].x);
				ptr[1] = float(unit->_path->_pts_los[i].y);
				ptr[2] = float(unit->_path->_pts_los[i].z + Z_OFFSET_PATH + Z_OFFSET_LOS);
				ptr[3] = path_color.r;
				ptr[4] = path_color.g;
				ptr[5] = path_color.b;
				ptr[6] = path_color.a;

				ptr[7] = float(unit->_path->_pts_los[i + 1].x);
				ptr[8] = float(unit->_path->_pts_los[i + 1].y);
				ptr[9] = float(unit->_path->_pts_los[i + 1].z + Z_OFFSET_PATH + Z_OFFSET_LOS);
				ptr[10] = path_color.r;
				ptr[11] = path_color.g;
				ptr[12] = path_color.b;
				ptr[13] = path_color.a;

				ptr += 14;
			}

			// croix de départ ---------------------------------------------------------------
			const number PATH_START_CROSS_SIZE = 1.0;
			const glm::vec4 PATH_START_CROSS_COLOR(1.0, 0.0, 1.0, 1.0);
			pt_3d start_cross_pts[4] = {
				unit->_path->_start - pt_3d(PATH_START_CROSS_SIZE, 0.0, 0.0),
				unit->_path->_start + pt_3d(PATH_START_CROSS_SIZE, 0.0, 0.0),
				unit->_path->_start - pt_3d(0.0, PATH_START_CROSS_SIZE, 0.0),
				unit->_path->_start + pt_3d(0.0, PATH_START_CROSS_SIZE, 0.0)
			};
			for (uint i=0; i<4; ++i) {
				ptr[0] = float(start_cross_pts[i].x);
				ptr[1] = float(start_cross_pts[i].y);
				ptr[2] = float(start_cross_pts[i].z + Z_OFFSET_PATH);
				ptr[3] = PATH_START_CROSS_COLOR.r;
				ptr[4] = PATH_START_CROSS_COLOR.g;
				ptr[5] = PATH_START_CROSS_COLOR.b;
				ptr[6] = PATH_START_CROSS_COLOR.a;

				ptr += 7;
			}

			// croix d'arrivée ---------------------------------------------------------------
			const number PATH_GOAL_CROSS_SIZE = 1.0;
			const glm::vec4 PATH_GOAL_CROSS_COLOR(1.0, 0.0, 1.0, 1.0);
			pt_3d goal_cross_pts[4] = {
				unit->_path->_goal - pt_3d(PATH_GOAL_CROSS_SIZE, 0.0, 0.0),
				unit->_path->_goal + pt_3d(PATH_GOAL_CROSS_SIZE, 0.0, 0.0),
				unit->_path->_goal - pt_3d(0.0, PATH_GOAL_CROSS_SIZE, 0.0),
				unit->_path->_goal + pt_3d(0.0, PATH_GOAL_CROSS_SIZE, 0.0)
			};
			for (uint i=0; i<4; ++i) {
				ptr[0] = float(goal_cross_pts[i].x);
				ptr[1] = float(goal_cross_pts[i].y);
				ptr[2] = float(goal_cross_pts[i].z + Z_OFFSET_PATH);
				ptr[3] = PATH_GOAL_CROSS_COLOR.r;
				ptr[4] = PATH_GOAL_CROSS_COLOR.g;
				ptr[5] = PATH_GOAL_CROSS_COLOR.b;
				ptr[6] = PATH_GOAL_CROSS_COLOR.a;

				ptr += 7;
			}
		}
	}

	context->set_data(data);
	delete[] data;
}


void Strategy::update_edit_map() {
	DrawContext * context= _gl_draw_manager->get_context("edit_map");
	
	if (_config->_mode != EDIT_ELEVATION && _config->_mode != ADD_ELEMENT) {
		context->_n_pts = 0;
		context->_active = false;
		return;
	}

	AABB_2D * aabb = new AABB_2D(pt_2d(_cursor_world_position) - pt_2d(_config->_elevation_radius), 2.0 * pt_2d(_config->_elevation_radius));
	if (!_map->_elevation->in_boundaries(aabb)) {
		context->_n_pts = 0;
		context->_active = false;
		delete aabb;
		return;
	}
	delete aabb;

	context->_n_pts = EDIT_MAP_N_VERTICES_PER_CIRCLE * 2;
	context->_active = true;

	std::vector<pt_2d> circle_pts;
	if (_config->_mode == EDIT_ELEVATION) {
		circle_pts = circle_vertices(pt_2d(_cursor_world_position), _config->_elevation_radius, EDIT_MAP_N_VERTICES_PER_CIRCLE);
	}
	else if (_config->_mode == ADD_ELEMENT) {
		circle_pts = circle_vertices(pt_2d(_cursor_world_position), _config->_elements_dispersion * 2.0, EDIT_MAP_N_VERTICES_PER_CIRCLE);
	}

	float data[context->data_size()];
	float * ptr = data;
	
	for (uint i=0; i<EDIT_MAP_N_VERTICES_PER_CIRCLE; ++i) {
		uint j = i + 1;
		if (j == EDIT_MAP_N_VERTICES_PER_CIRCLE) {
			j = 0;
		}

		ptr[0] = float(circle_pts[i].x);
		ptr[1] = float(circle_pts[i].y);
		ptr[2] = float(_map->_elevation->get_alti(circle_pts[i]) + Z_OFFSET_EDIT_MAP);
		ptr[3] = EDIT_MAP_COLOR.r;
		ptr[4] = EDIT_MAP_COLOR.g;
		ptr[5] = EDIT_MAP_COLOR.b;
		ptr[6] = EDIT_MAP_COLOR.a;

		ptr[7] = float(circle_pts[j].x);
		ptr[8] = float(circle_pts[j].y);
		ptr[9] = float(_map->_elevation->get_alti(circle_pts[j]) + Z_OFFSET_EDIT_MAP);
		ptr[10] = EDIT_MAP_COLOR.r;
		ptr[11] = EDIT_MAP_COLOR.g;
		ptr[12] = EDIT_MAP_COLOR.b;
		ptr[13] = EDIT_MAP_COLOR.a;

		ptr += 14;
	}

	context->set_data(data);
}


void Strategy::update_debug() {
	DrawContext * context= _gl_draw_manager->get_context("debug");
	
	context->_n_pts = 0;

	//context->set_data(data);
	//delete[] data;
}


void Strategy::update_elevation() {
	DrawContext * context= _gl_draw_manager->get_context("elevation");

	context->_n_pts = _map->_elevation->_n_pts;

	context->set_data(_map->_elevation->_data);
}


void Strategy::update_tree_stone() {
	DrawContext * context= _gl_draw_manager->get_context("tree_stone");

	context->_n_pts = 0;
	for (auto & element : _map->_elements->_elements) {
		if (element->_type == ELEMENT_TREE || element->_type == ELEMENT_STONE) {
			context->_n_pts += element->_n_pts;
		}
	}

	float * data = new float[context->data_size()];

	uint compt = 0;
	uint n_attrs_per_pts = context->_buffers[0]._n_attrs_per_pts;
	for (auto & element : _map->_elements->_elements) {
		if (element->_type == ELEMENT_TREE || element->_type == ELEMENT_STONE) {
			for (uint i=0; i<element->_n_pts * n_attrs_per_pts; ++i) {
				data[compt++] = element->_data[i];
			}
		}
	}

	context->set_data(data);
	delete[] data;
}


void Strategy::update_river() {
	DrawContext * context= _gl_draw_manager->get_context("river");

	context->_n_pts = 0;
	for (auto & element : _map->_elements->_elements) {
		if (element->_type == ELEMENT_RIVER) {
			context->_n_pts += element->_n_pts;
		}
	}

	float * data = new float[context->data_size()];

	uint compt = 0;
	uint n_attrs_per_pts = context->_buffers[0]._n_attrs_per_pts;
	for (auto & element : _map->_elements->_elements) {
		if (element->_type == ELEMENT_RIVER) {
			for (uint i=0; i<element->_n_pts * n_attrs_per_pts; ++i) {
				data[compt++] = element->_data[i];
			}
		}
	}

	context->set_data(data);
	delete[] data;
}


void Strategy::update_lake() {
	DrawContext * context= _gl_draw_manager->get_context("lake");

	context->_n_pts = 0;
	for (auto & element : _map->_elements->_elements) {
		if (element->_type == ELEMENT_LAKE) {
			context->_n_pts += element->_n_pts;
		}
	}

	float * data = new float[context->data_size()];

	uint compt = 0;
	uint n_attrs_per_pts = context->_buffers[0]._n_attrs_per_pts;
	for (auto & element : _map->_elements->_elements) {
		if (element->_type == ELEMENT_LAKE) {
			for (uint i=0; i<element->_n_pts * n_attrs_per_pts; ++i) {
				data[compt++] = element->_data[i];
			}
		}
	}

	context->set_data(data);
	delete[] data;
}


void Strategy::update_sea() {
	DrawContext * context= _gl_draw_manager->get_context("sea");

	context->_n_pts = 6;

	float * data = new float[context->data_size()];

	const glm::vec4 SEA_COLOR(0.6, 0.8, 0.9, 0.6);
	std::vector<pt_3d> pts = {
		pt_3d(_map->_elevation->_origin.x, _map->_elevation->_origin.y, 0.0),
		pt_3d(_map->_elevation->_origin.x + _map->_elevation->_size.x, _map->_elevation->_origin.y, 0.0),
		pt_3d(_map->_elevation->_origin.x + _map->_elevation->_size.x, _map->_elevation->_origin.y + _map->_elevation->_size.y, 0.0),
		pt_3d(_map->_elevation->_origin.x, _map->_elevation->_origin.y + _map->_elevation->_size.y, 0.0)
	};
	std::vector<uint> idxs = {0, 1, 2, 0, 2, 3};
	float * ptr = data;
	for (uint i=0; i<6; ++i) {
		ptr[0] = float(pts[idxs[i]].x);
		ptr[1] = float(pts[idxs[i]].y);
		ptr[2] = float(pts[idxs[i]].z);
		ptr[3] = SEA_COLOR.r;
		ptr[4] = SEA_COLOR.g;
		ptr[5] = SEA_COLOR.b;
		ptr[6] = SEA_COLOR.a;
		ptr += 7;
	}

	context->set_data(data);
	delete[] data;
}


void Strategy::update_unit_obj(UnitType * unit_type) {
	DrawContext * context= _gl_draw_manager->get_context(unit_type2str(unit_type->_type));

	context->_n_pts = unit_type->_obj_data->_n_pts;
	context->set_data(unit_type->_obj_data->_data, 0);
}


void Strategy::update_unit_matrices(UnitType * unit_type) {
	DrawContext * context= _gl_draw_manager->get_context(unit_type2str(unit_type->_type));

	context->_n_instances = 0;
	for (auto & team : _map->_teams) {
		for (auto & unit : team->_units) {
			if (unit->_type == unit_type) {
				context->_n_instances++;
			}
		}
	}
	float * data = new float[context->_n_instances * 16];
	float * ptr = data;
	for (auto & team : _map->_teams) {
		for (auto & unit : team->_units) {
			if (unit->_type == unit_type) {
				const float * unit_data = glm::value_ptr(glm::mat4(unit->_model2world));
				std::memcpy(ptr, unit_data, 16 * sizeof(float));
				ptr += 16;
			}
		}
	}

	context->set_data(data, 1);
	delete[] data;
}


void Strategy::update_all() {
	update_select();
	update_grid();
	update_bbox();
	update_path();
	update_edit_map();
	update_elevation();
	update_tree_stone();
	update_river();
	update_lake();
	update_sea();
	for (auto & unit_type : _map->_unit_types) {
		update_unit_obj(unit_type.second);
		update_unit_matrices(unit_type.second);
	}
}


void Strategy::update_text() {
	std::vector<Text3D> texts_3d;
	for (auto & team : _map->_teams) {
		for (auto & unit : team->_units) {
			texts_3d.push_back(Text3D(unit_type2str(unit->_type->_type) + " " + std::to_string(unit->_id), glm::vec3(unit->_bbox->_aabb->_vmin)+ glm::vec3(0.1, 0.1, 0.5), 0.01, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
		}
	}

	_font->set_text(texts_3d);

	std::vector<Text> texts_2d;
	
	std::string s_pos = glm_to_string(_cursor_world_position);
	texts_2d.push_back(Text(s_pos, glm::vec2(-4.8, 3.8), 0.003, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
	
	if (_map->_path_finder->in_boundaries(pt_2d(_cursor_world_position))) {
		GraphEdge edge = _map->_path_finder->get_edge(_map->_path_finder->pt2closest_edge(pt_2d(_cursor_world_position)));
		GraphEdge opposite_edge = _map->_path_finder->opposite_edge(edge);
		EdgeData * data = (EdgeData *)(edge._data);
		EdgeData * opposite_data = (EdgeData *)(opposite_edge._data);

		std::string str_terrain = terrain_type2str(data->_type[_map->_unit_types[_config->_visible_grid_unit_type]]);
		str_terrain += " ; " + std::to_string(data->_delta_elevation[_map->_unit_types[_config->_visible_grid_unit_type]]);
		str_terrain += " ; " + std::to_string(opposite_data->_delta_elevation[_map->_unit_types[_config->_visible_grid_unit_type]]);
		
		texts_2d.push_back(Text(str_terrain, glm::vec2(-1.5, 3.8), 0.003, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
	}

	_font->set_text(texts_2d);
}


bool Strategy::mouse_button_down(InputState * input_state, time_point t) {
	if (_ihm->mouse_button_down(input_state, t)) {
		return true;
	}

	if (!input_state->_left_mouse) {
		return false;
	}

	if (_config->_mode == VIEW) {
		return false;
	}

	if (_config->_mode == PLAY && _config->_play_mode == MOVE_UNIT) {
		_map->selected_units_goto(_cursor_world_position, t);
		return true;
	}
	else if (_config->_mode == ADD_UNIT) {
		Unit * unit = _map->add_unit(_map->_teams[0], _config->_unit_type, pt_2d(_cursor_world_position));
		if (_config->_units_paused) {
			unit->_paused = true;
		}
		else {
			unit->_paused = false;
		}
		update_unit_matrices(_map->_unit_types[_config->_unit_type]);
		return true;
	}
	else if (_config->_mode == ADD_ELEMENT) {
		if (_config->_element_type == ELEMENT_TREE) {
			_map->add_trees("oak", pt_2d(_cursor_world_position), _config->_n_elements, _config->_elements_dispersion);
			update_tree_stone();
			update_grid();
		}
		else if (_config->_element_type == ELEMENT_STONE) {
			_map->add_stones("dark_stone", pt_2d(_cursor_world_position), _config->_n_elements, _config->_elements_dispersion);
			update_tree_stone();
			update_grid();
		}
		else if (_config->_element_type == ELEMENT_RIVER) {
			_map->add_river(pt_2d(_cursor_world_position));
			update_river();
			update_grid();
		}
		else if (_config->_element_type == ELEMENT_LAKE) {
			_map->add_lake(pt_2d(_cursor_world_position));
			update_lake();
			update_grid();
		}
		return true;
	}
	else if (_config->_mode == EDIT_ELEVATION) {
		AABB_2D * aabb = new AABB_2D(pt_2d(_cursor_world_position) - pt_2d(_config->_elevation_radius), 2.0 * pt_2d(_config->_elevation_radius));
		
		_map->_elements->remove_elements_in_aabb(aabb);
		for (auto & team : _map->_teams) {
			team->remove_units_in_aabb(aabb);
		}
		
		std::vector<uint> ids = _map->_elevation->vertices_in_aabb(aabb);

		for (auto id : ids) {
			pt_2d pt2 = _map->_elevation->id2pt_2d(id);
			number dist = glm::distance(pt_2d(_cursor_world_position), pt2) / (_config->_elevation_radius);
			if (dist > 1.0) {
				continue;
			}
			number alti_inc = _config->_elevation_factor * (1.0 - pow(dist, _config->_elevation_exponent));
			number new_alti;

			if (_config->_elevation_mode == ELEVATION_MINUS) {
				new_alti = _map->_elevation->get_alti(id) - alti_inc;
			}
			else if (_config->_elevation_mode == ELEVATION_PLUS) {
				new_alti = _map->_elevation->get_alti(id) + alti_inc;
			}
			else if (_config->_elevation_mode == ELEVATION_ZERO) {
				new_alti = -0.01;
			}

			if (abs(new_alti) < 0.01) {
				new_alti = -0.01;
			}

			_map->_elevation->set_alti(id, new_alti);
		}

		//_map->_elevation->set_negative_alti_2zero();

		// buffer pour mettre à jour un peu autour du AABB modifié (voir note TODO dans graph.h / aabb2col_lig_min_max)
		aabb->buffer(_map->_elevation->_resolution.x + 0.01);
		_map->_elevation->update_normals(aabb);
		_map->_elevation->update_data(aabb);
		_map->sync2elevation();

		update_elevation();
		update_grid();
		update_tree_stone();

		delete aabb;
		return true;
	}

	return false;
}


bool Strategy::mouse_button_up(InputState * input_state, time_point t) {
	if (_config->_mode == PLAY && _config->_play_mode == SELECT_UNIT) {
		if (_view_system->_new_single_selection) {
			_view_system->_new_single_selection= false;
			for (auto & team : _map->_teams) {
				for (auto & unit : team->_units) {
					unit->_selected = false;
					if (_view_system->single_selection_intersects_aabb(unit->_bbox->_aabb, false)) {
						unit->_selected = true;
					}
				}
			}
		}
		else if (_view_system->_new_rect_selection) {
			_view_system->_new_rect_selection= false;
			for (auto & team : _map->_teams) {
				for (auto & unit : team->_units) {
					unit->_selected = false;
					BBox * bbox = new BBox(unit->_bbox->_aabb);
					if (_view_system->rect_selection_intersects_bbox(bbox, false)) {
						unit->_selected = true;
					}
					delete bbox;
				}
			}
		}
		
		return true;
	}
	return false;
}


bool Strategy::mouse_motion(InputState * input_state, time_point t) {
	_cursor_world_position = _view_system->screen2world_depthbuffer(input_state->_x, input_state->_y);
	return false;
}


bool Strategy::key_down(InputState * input_state, SDL_Keycode key, time_point t) {
	if (_ihm->key_down(input_state, key, t)) {
		return true;
	}
	return false;
}


bool Strategy::key_up(InputState * input_state, SDL_Keycode key, time_point t) {
	return true;
}

