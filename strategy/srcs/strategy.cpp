#include <iostream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

#include "strategy.h"


// -------------------------------------------------
Strategy::Strategy() {

}


Strategy::Strategy(std::map<std::string, GLuint> progs, ViewSystem * view_system, time_point t) :
	_mode(PLAY), _view_system(view_system), _current_unit_type_name("infantery"), _visible_grid_unit_type("infantery"), 
	_visible_grid_type("terrain"), _current_element_name("tree"), _current_elevation_mode("elevation_plus"), _angle(0.0)
{
	_contexts["debug"]= new DrawContext(progs["repere"], 
		std::vector<std::string>{"position_in:3", "color_in:4"},
		std::vector<std::string>{"world2clip_matrix"},
		GL_STATIC_DRAW, false);

	_contexts["grid"]= new DrawContext(progs["repere"], 
		std::vector<std::string>{"position_in:3", "color_in:4"},
		std::vector<std::string>{"world2clip_matrix"},
		GL_STATIC_DRAW, false);

	_contexts["unit_linear"]= new DrawContext(progs["repere"], 
		std::vector<std::string>{"position_in:3", "color_in:4"},
		std::vector<std::string>{"world2clip_matrix"},
		GL_DYNAMIC_DRAW, true);

	_contexts["path"]= new DrawContext(progs["dash"], 
		std::vector<std::string>{"position_in:3", "color_in:4"},
		std::vector<std::string>{"world2clip_matrix", "gap_size", "dash_size", "viewport_size", "thickness"},
		GL_STATIC_DRAW, true);
	
	_contexts["elevation"]= new DrawContext(progs["elevation_smooth"], 
		std::vector<std::string>{"position_in:3", "color_in:4", "normal_in:3"},
		std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position"},
		GL_DYNAMIC_DRAW, true);

	_contexts["tree_stone"]= new DrawContext(progs["elevation_smooth"], 
		std::vector<std::string>{"position_in:3", "color_in:4", "normal_in:3"},
		std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position"},
		GL_STATIC_DRAW, true);

	_contexts["river"]= new DrawContext(progs["river"], 
		std::vector<std::string>{"position_in:3", "color_in:4", "normal_in:3", "direction_in:2"},
		std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position", "angle"},
		GL_STATIC_DRAW, true);

	_contexts["lake"]= new DrawContext(progs["lake"], 
		std::vector<std::string>{"position_in:3", "color_in:4"},
		std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position", "angle"},
		GL_STATIC_DRAW, true);

	_contexts["sea"]= new DrawContext(progs["lake"], 
		std::vector<std::string>{"position_in:3", "color_in:4"},
		std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position", "angle"},
		GL_STATIC_DRAW, true);

	_font = new Font(progs, "../../fonts/Silom.ttf", 48, _view_system->_screengl);
	_font->_z= 100.0f; // pour que l'affichage des infos se fassent par dessus le reste

	_view_system->_rect_select->_z = -1.0; // pour que l'affichage du rectangle de sélection se fassent par dessus le reste

	_ihm = new GLIHM(progs, _view_system->_screengl, "../data/ihm.json");

	_map = new Map("../data/unit_types", "../data/elements", pt_2d(-100.0, -100.0), pt_2d(200.0, 200.0), pt_2d(2.0), pt_2d(0.5), t);
	//_map = new Map("../data/unit_types", pt_2d(0.0, 0.0), pt_2d(10.0, 10.0), pt_2d(2.0), pt_2d(2.0), t);
	//std::cout << *_map << "\n";

	for (auto & unit_type : _map->_unit_types) {
		_contexts[unit_type.first]= new DrawContext(progs["unit"], 
			std::vector<std::string>{"position_in:3", "normal_in:3", "ambient_in:3", "diffuse_in:3", "specular_in:3", "shininess_in:1", "opacity_in:1", "model2world_matrix:16:new_instanced_buffer"},
			std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position"},
			GL_DYNAMIC_DRAW, true);
	}

	_map->randomize();

	/*for (uint i=0; i<200; ++i) {
		pt_2d pos = rand_pt_2d(-20.0, 20.0, -20.0, 20.0);
		_map->add_unit("helicopter", pos, t);
	}*/

	update_all();

	set_ihm();
}


Strategy::~Strategy() {
	delete _map;
	for (auto context : _contexts) {
		delete context.second;
	}
	_contexts.clear();
	delete _font;
}


void Strategy::set_ihm() {
	_ihm->get_element("mode", "play")->set_callback([this](){_mode = PLAY;});
	_ihm->get_element("mode", "add_unit")->set_callback([this](){_mode = ADD_UNIT;});
	_ihm->get_element("mode", "add_element")->set_callback([this](){_mode = ADD_ELEMENT;});
	_ihm->get_element("mode", "edit_elevation")->set_callback([this](){_mode = EDIT_ELEVATION;});
	
	for (auto & unit_type_name : std::vector<std::string>{"infantery", "tank", "helicopter", "boat"}) {
		_ihm->get_element("units", unit_type_name)->set_callback([this, unit_type_name](){_current_unit_type_name = unit_type_name;});
	}
	
	for (auto & element_name : std::vector<std::string>{"tree", "stone", "river", "lake"}) {
		_ihm->get_element("elements", element_name)->set_callback([this, element_name](){_current_element_name = element_name;});
	}
	
	for (auto & elevation_mode : std::vector<std::string>{"elevation_plus", "elevation_minus"}) {
		_ihm->get_element("elevation", elevation_mode)->set_callback([this, elevation_mode](){_current_elevation_mode = elevation_mode;});
	}

	_ihm->get_element("edit_elevation_params", "elevation_radius")->set_callback([this](){
		GLIHMElement * element = _ihm->get_element("edit_elevation_params", "elevation_radius");
		GLIHMSlider * slider = (GLIHMSlider *)(element);
		_current_elevation_radius = slider->_value;
	});
	
	_ihm->get_element("edit_elevation_params", "elevation_factor")->set_callback([this](){
		GLIHMElement * element = _ihm->get_element("edit_elevation_params", "elevation_factor");
		GLIHMSlider * slider = (GLIHMSlider *)(element);
		_current_elevation_factor = slider->_value;
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
	
	for (auto & visu_type : std::vector<std::string>{"elevation", "grid", "unit_linear", "sea"}) {
		_ihm->get_element("visu", visu_type)->set_callback(
			[this, visu_type](){_contexts[visu_type]->_active = true;}, 
			[this, visu_type](){_contexts[visu_type]->_active = false;}
		);
	}
	
	_ihm->get_element("visu", "elements")->set_callback(
		[this](){
			_contexts["tree_stone"]->_active = true;
			_contexts["river"]->_active = true;
			_contexts["lake"]->_active = true;
		},
		[this](){
			_contexts["tree_stone"]->_active = false;
			_contexts["river"]->_active = false;
			_contexts["lake"]->_active = false;
		}
	);
	
	_ihm->get_element("visu", "repere")->set_callback(
		[this](){
			_view_system->_repere->_contexts["repere"]->_active= true;
			_view_system->_repere->_contexts["ground"]->_active= false;
			_view_system->_repere->_contexts["box"]->_active= true;
		},
		[this](){
			_view_system->_repere->_contexts["repere"]->_active= false;
			_view_system->_repere->_contexts["ground"]->_active= false;
			_view_system->_repere->_contexts["box"]->_active= false;
		}
	);
	
	_ihm->get_element("visu", "units")->set_callback(
		[this](){
			for (auto & unit_type : _map->_unit_types) {
				_contexts[unit_type.first]->_active = true;
			}
		},
		[this](){
			for (auto & unit_type : _map->_unit_types) {
				_contexts[unit_type.first]->_active = false;
			}
		}
	);
	
	for (auto & grid_type : std::vector<std::string>{"elevation", "terrain", "units_position"}) {
		_ihm->get_element("grid_type", grid_type)->set_callback([this, grid_type](){
			_visible_grid_type = grid_type;
			update_grid();
		});
	}
	
	for (auto & unit_type_name : std::vector<std::string>{"infantery", "tank", "helicopter", "boat"}) {
		_ihm->get_element("grid_unit_type", unit_type_name)->set_callback([this, unit_type_name](){
			_visible_grid_unit_type = unit_type_name;
			update_grid();
		});
	}

	_ihm->all_callbacks(); // synchro de l'état de l'ihm avec l'état de strategy

	//std::cout << *_ihm << "\n";
}


void Strategy::draw_linear(std::string context_name) {
	DrawContext * context= _contexts[context_name];
	if (!context->_active) {
		return;
	}

	context->activate();
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	glDrawArrays(GL_LINES, 0, context->_n_pts);
	context->deactivate();
}


void Strategy::draw_dash(std::string context_name) {
	DrawContext * context= _contexts[context_name];
	if (!context->_active) {
		return;
	}

	context->activate();
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	glUniform1f(context->_locs_uniform["dash_size"], 8.0);
	glUniform1f(context->_locs_uniform["gap_size"], 8.0);
	glUniform1f(context->_locs_uniform["thickness"], 4.0);
	glUniform2f(context->_locs_uniform["viewport_size"], _view_system->_screengl->_screen_width, _view_system->_screengl->_screen_height);
	glDrawArrays(GL_LINES, 0, context->_n_pts);
	context->deactivate();

}


void Strategy::draw_surface(std::string context_name) {
	DrawContext * context= _contexts[context_name];
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
	DrawContext * context= _contexts["lake"];
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


void Strategy::draw_river() {
	DrawContext * context= _contexts["river"];
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
	DrawContext * context= _contexts["sea"];
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


void Strategy::draw_unit(UnitType * unit_type) {
	DrawContext * context= _contexts[unit_type->_name];
	if (!context->_active) {
		return;
	}

	context->activate();
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	glUniform3fv(context->_locs_uniform["light_position"], 1, glm::value_ptr(light_position));
	glUniform3fv(context->_locs_uniform["light_color"], 1, glm::value_ptr(light_color));
	glUniform3fv(context->_locs_uniform["view_position"], 1, glm::value_ptr(glm::vec3(_view_system->_eye)));
	//glDrawArraysInstanced(GL_TRIANGLES, 0, context->_n_pts, _map->_unit_groups[unit_type]->_units.size());
	glDrawArraysInstanced(GL_TRIANGLES, 0, context->_n_pts, context->_n_instances);
	context->deactivate();
}


void Strategy::draw() {
	for (auto context_name : std::vector<std::string>{"grid", "unit_linear"}) {
		draw_linear(context_name);
	}

	for (auto context_name : std::vector<std::string>{"elevation", "tree_stone"}) {
		draw_surface(context_name);
	}
	
	draw_dash("path");
	draw_lake();
	draw_river();
	draw_sea();
	for (auto & unit_type : _map->_unit_types) {
		draw_unit(unit_type.second);
	}
	
	_font->draw_3d(_view_system->_world2clip);
	_font->draw();

	_ihm->draw();
}


void Strategy::anim(time_point t, InputState * input_state) {
	_ihm->anim();

	//pt_2d pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);
	pt_3d pt = _view_system->screen2world_depthbuffer(input_state->_x, input_state->_y);

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

	_map->anim(t);

	update_unit_linear();
	update_path();
	//update_grid();
	update_text(input_state);
	for (auto & unit_type : _map->_unit_types) {
		update_unit_matrices(unit_type.second);
	}

	_angle += 0.01;
	if (_angle > 2.0 * M_PI) {
		_angle -= 2.0 * M_PI;
	}
	update_lake();
}


glm::vec4 Strategy::get_edge_color() {
	GraphEdge edge = _map->_path_finder->_it_e->second;
	EdgeData * data = (EdgeData *)(edge._data);
	UnitType * unit_type = _map->_unit_types[_visible_grid_unit_type];
	glm::vec4 edge_color;

	if (_visible_grid_type == "elevation") {
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
	else if (_visible_grid_type == "terrain") {
		if (data->_type[unit_type] == SEA ) {
			edge_color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		}
		else if (data->_type[unit_type] == GROUND ) {
			edge_color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
		}
		else if (data->_type[unit_type] == OBSTACLE ) {
			edge_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else if (data->_type[unit_type] == LAKE ) {
			edge_color = glm::vec4(0.5f, 0.0f, 1.0f, 1.0f);
		}
		else if (data->_type[unit_type] == RIVER ) {
			edge_color = glm::vec4(0.0f, 0.5f, 1.0f, 1.0f);
		}
		else if (data->_type[unit_type] == SEA_COAST ) {
			edge_color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		}
		else if (data->_type[unit_type] == LAKE_COAST ) {
			edge_color = glm::vec4(1.0f, 1.0f, 0.5f, 1.0f);
		}
		else {
			edge_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
	else if (_visible_grid_type == "units_position") {
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


void Strategy::update_grid() {
	DrawContext * context= _contexts["grid"];
	
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
			glm::vec4 edge_color = get_edge_color();

			pt_3d & p1 = _map->_path_finder->_it_v->second._pos;
			pt_3d & p2 = _map->_path_finder->_vertices[_map->_path_finder->_it_e->first]._pos;
			pt_3d p1b = p1 + (p2 - p1) * 0.1;
			pt_3d p_middle = (p1 + p2) * 0.5 - (p2 - p1) * 0.1;
			
			ptr[0] = float(p1b.x);
			ptr[1] = float(p1b.y);
			//ptr[2] = float(ALTI_EDGE);
			ptr[2] = float(p1b.z + Z_OFFSET_EDGE);
			ptr[3] = edge_color.r;
			ptr[4] = edge_color.g;
			ptr[5] = edge_color.b;
			ptr[6] = edge_color.a;

			ptr[7] = float(p_middle.x);
			ptr[8] = float(p_middle.y);
			//ptr[9] = float(ALTI_EDGE);
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


/*void Strategy::update_obstacle() {
	DrawContext * context= _contexts["obstacle"];
	
	context->_n_attrs_per_pts= 7;
	context->_n_pts = 0;

	// obstacles existants
	for (auto obstacle : _map->_obstacles) {
		context->_n_pts += obstacle->_polygon->_pts.size() * 2;
	}
	
	// obstacle édité
	if (_obstacle_pts.size() > 1) {
		context->_n_pts += 2 * (_obstacle_pts.size() - 1);
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr = data;

	// obstacles
	for (auto obstacle : _map->_obstacles) {
		for (uint i=0; i<obstacle->_polygon->_pts.size(); ++i) {
			uint j = i + 1;
			if (j == obstacle->_polygon->_pts.size()) {
				j = 0;
			}
			ptr[0] = float(obstacle->_polygon->_pts[i].x);
			ptr[1] = float(obstacle->_polygon->_pts[i].y);
			ptr[2] = float(ALTI_OBSTACLE);
			ptr[3] = OBSTACLE_COLORS.at(obstacle->_type).r;
			ptr[4] = OBSTACLE_COLORS.at(obstacle->_type).g;
			ptr[5] = OBSTACLE_COLORS.at(obstacle->_type).b;
			ptr[6] = OBSTACLE_COLORS.at(obstacle->_type).a;

			ptr[7] = float(obstacle->_polygon->_pts[j].x);
			ptr[8] = float(obstacle->_polygon->_pts[j].y);
			ptr[9] = float(ALTI_OBSTACLE);
			ptr[10] = OBSTACLE_COLORS.at(obstacle->_type).r;
			ptr[11] = OBSTACLE_COLORS.at(obstacle->_type).g;
			ptr[12] = OBSTACLE_COLORS.at(obstacle->_type).b;
			ptr[13] = OBSTACLE_COLORS.at(obstacle->_type).a;

			ptr += 14;
		}
	}

	// obstacle édité
	if (_obstacle_pts.size() > 1) {
		for (uint i=0; i<_obstacle_pts.size() - 1; ++i) {
			ptr[0] = float(_obstacle_pts[i].x);
			ptr[1] = float(_obstacle_pts[i].y);
			ptr[2] = float(ALTI_OBSTACLE);
			ptr[3] = EDITED_OBSTACLE_COLORS.at(_mode).r;
			ptr[4] = EDITED_OBSTACLE_COLORS.at(_mode).g;
			ptr[5] = EDITED_OBSTACLE_COLORS.at(_mode).b;
			ptr[6] = EDITED_OBSTACLE_COLORS.at(_mode).a;

			ptr[7] = float(_obstacle_pts[i + 1].x);
			ptr[8] = float(_obstacle_pts[i + 1].y);
			ptr[9] = float(ALTI_OBSTACLE);
			ptr[10] = EDITED_OBSTACLE_COLORS.at(_mode).r;
			ptr[11] = EDITED_OBSTACLE_COLORS.at(_mode).g;
			ptr[12] = EDITED_OBSTACLE_COLORS.at(_mode).b;
			ptr[13] = EDITED_OBSTACLE_COLORS.at(_mode).a;

			ptr += 14;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, context->_usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}*/


void Strategy::update_unit_linear() {
	DrawContext * context= _contexts["unit_linear"];
	
	context->_n_pts = 0;

	for (auto & team : _map->_teams) {
		for (auto & unit : team->_units) {
			context->_n_pts += 48; // dessin AABB
		}
	}

	for (auto & element : _map->_elements->_elements) {
		context->_n_pts += 48; // dessin AABB
	}

	float data[context->data_size()];
	float * ptr = data;

	// units
	uint x = 0;
	for (auto & team : _map->_teams) {
		for (auto & unit : team->_units) {
			std::vector<pt_3d> segs = unit->_bbox->_aabb->segments();

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

			for (uint i=0; i < segs.size(); ++i) {
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
				x++;
			}
		}
	}

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
			x++;
		}
	}

	context->set_data(data);
}


void Strategy::update_path() {
	DrawContext * context= _contexts["path"];
	
	context->_n_pts = 0;
	
	for (auto & team : _map->_teams) {
		for (auto & unit : team->_units) {
			context->_n_pts += unit->_path->_pts.size() * 2;
		}
	}

	float data[context->data_size()];
	float * ptr = data;

	for (auto & team : _map->_teams) {
		for (auto & unit : team->_units) {
			if (unit->_path->_pts.empty()) {
				continue;
			}
			for (uint i=0; i<unit->_path->_pts.size() - 1; ++i) {
				glm::vec4 path_color;
				if (unit->_path->_weights[i + 1] >= MAX_UNIT_MOVING_WEIGHT) {
					path_color = glm::vec4(1.0, 0.0, 0.0, 1.0);
				}
				else {
					path_color = glm::vec4(0.0, 1.0, 0.0, 1.0);
				}

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
		}
	}

	context->set_data(data);
}


void Strategy::update_debug() {
	DrawContext * context= _contexts["debug"];
	
	context->_n_pts = 0;

	//context->set_data(data);
	//delete[] data;
}


void Strategy::update_elevation() {
	DrawContext * context= _contexts["elevation"];

	context->_n_pts = _map->_elevation->_n_pts;

	context->set_data(_map->_elevation->_data);
}


void Strategy::update_tree_stone() {
	DrawContext * context= _contexts["tree_stone"];

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
	DrawContext * context= _contexts["river"];

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
	DrawContext * context= _contexts["lake"];

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
	DrawContext * context= _contexts["sea"];

	context->_n_pts = 6;

	float * data = new float[context->data_size()];

	const glm::vec4 SEA_COLOR(0.6, 0.8, 0.9, 0.6);
	std::vector<pt_3d> pts = {
		pt_3d(_map->_origin.x, _map->_origin.y, 0.0),
		pt_3d(_map->_origin.x + _map->_size.x, _map->_origin.y, 0.0),
		pt_3d(_map->_origin.x + _map->_size.x, _map->_origin.y + _map->_size.y, 0.0),
		pt_3d(_map->_origin.x, _map->_origin.y + _map->_size.y, 0.0)
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
	DrawContext * context= _contexts[unit_type->_name];

	context->_n_pts = unit_type->_obj_data->_n_pts;
	context->set_data(unit_type->_obj_data->_data, 0);
}


void Strategy::update_unit_matrices(UnitType * unit_type) {
	DrawContext * context= _contexts[unit_type->_name];

	//context->_n_instances = _map->_unit_groups[unit_type]->_units.size();
	//context->set_data(_map->_unit_groups[unit_type]->_matrices, 1);

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
	update_grid();
	update_unit_linear();
	update_path();
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


void Strategy::update_text(InputState * input_state) {
	pt_3d pt = _view_system->screen2world_depthbuffer(input_state->_x, input_state->_y);

	std::vector<Text3D> texts_3d;
	for (auto & team : _map->_teams) {
		for (auto & unit : team->_units) {
			texts_3d.push_back(Text3D(unit->_type->_name + " " + std::to_string(unit->_id), glm::vec3(unit->_bbox->_aabb->_vmin)+ glm::vec3(0.1, 0.1, 0.5), 0.01, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
		}
	}

	/*if (!_map->_rivers.empty()) {
		for (uint i=0; i<_map->_rivers[0]->_id_nodes.size(); ++i) {
			texts_3d.push_back(Text3D(std::to_string(i), _map->_elevation->id2pt_3d(_map->_rivers[0]->_id_nodes[i]), 0.005, glm::vec4(0.9f, 0.8f, 0.8f, 1.0f)));
		}
	}*/
	_font->set_text(texts_3d);

	std::vector<Text> texts_2d;
	
	std::string s_pos = glm_to_string(pt);
	texts_2d.push_back(Text(s_pos, glm::vec2(-4.8, 3.8), 0.003, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));

	/*for (auto unit : _map->_units) {
		if (unit->_selected) {
			std::ostringstream stream;
			stream << *unit;
			std::string s_unit = stream.str();
			texts_2d.push_back(Text(s_unit, glm::vec2(-4.8, 3.2), 0.002, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f), 8.0f));
			break;
		}
	}*/

	//number w = _map->_units_position_grids[_map->_unit_types["infantery"]]->average_weight_in_cell_containing_pt(pt_2d(pt.x, pt.y));
	//texts_2d.push_back(Text(std::to_string(w), glm::vec2(-4.8, 3.2), 0.002, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f), 8.0f));

	_font->set_text(texts_2d);
}


bool Strategy::mouse_button_down(InputState * input_state, time_point t) {
	if (_ihm->mouse_button_down(input_state, t)) {
		return true;
	}

	pt_3d pt_3d = _view_system->screen2world_depthbuffer(input_state->_x, input_state->_y);
	pt_2d pt(pt_3d.x, pt_3d.y);

	if (_mode == ADD_UNIT && input_state->_left_mouse) {
		_map->add_unit(_map->_teams[0], _current_unit_type_name, pt);
		update_unit_matrices(_map->_unit_types[_current_unit_type_name]);
		return true;
	}
	else if (_mode == ADD_ELEMENT && input_state->_left_mouse) {
		if (_current_element_name == "tree") {
			_map->add_trees("oak", pt, 20, 3.0);
			update_tree_stone();
		}
		else if (_current_element_name == "stone") {
			_map->add_stones("dark_stone", pt, 20, 3.0);
			update_tree_stone();
		}
		else if (_current_element_name == "river") {
			_map->add_river(pt);
			update_river();
		}
		else if (_current_element_name == "lake") {
			_map->add_lake(pt);
			update_lake();
		}
		return true;
	}
	else if (_mode == EDIT_ELEVATION && input_state->_left_mouse) {
		AABB_2D * aabb = new AABB_2D(pt_2d(pt) - pt_2d(_current_elevation_radius * 0.5), pt_2d(_current_elevation_radius));
		
		_map->_elements->remove_elements_in_aabb(aabb);
		for (auto & team : _map->_teams) {
			team->remove_units_in_aabb(aabb);
		}
		
		std::vector<uint> ids = _map->_elevation->vertices_in_aabb(aabb);

		for (auto id : ids) {
			pt_2d pt2 = _map->_elevation->id2pt_2d(id);
			number dist = glm::distance(pt_2d(pt), pt2) / (_current_elevation_radius * 0.5);
			if (dist > 1.0) {
				continue;
			}
			number alti_inc = _current_elevation_factor * (1.0 - dist * dist);

			if (_current_elevation_mode == "elevation_minus") {
				_map->_elevation->set_alti(id, _map->_elevation->get_alti(id) - alti_inc);
			}
			else if (_current_elevation_mode == "elevation_plus") {
				_map->_elevation->set_alti(id, _map->_elevation->get_alti(id) + alti_inc);
			}
		}

		//_map->_elevation->set_negative_alti_2zero();
		_map->_elevation->update_normals(aabb);
		_map->_elevation->update_data(aabb);
		_map->sync2elevation();

		update_elevation();
		update_grid();
		update_tree_stone();

		delete aabb;
		return true;
	}

	/*else if (_mode == PLAY && input_state->_left_mouse) {
		_map->selected_units_goto(pt, t);
		return true;
	}*/
	
	/*else if (input_state->_keys[SDLK_w]) {
			if (input_state->_keys[SDLK_LSHIFT]) {
				_map->add_river(pt);
				update_debug();
				update_river();
				update_elevation();
				update_grid();
			}
			else {
				_map->add_lake(pt);
				update_debug();
				update_lake();
				update_elevation();
				update_grid();
			}
			return true;
		}
	}*/
	return false;
}


bool Strategy::mouse_button_up(InputState * input_state, time_point t) {
	pt_3d pt_3d = _view_system->screen2world_depthbuffer(input_state->_x, input_state->_y);
	pt_2d pt(pt_3d.x, pt_3d.y);

	if (_mode == PLAY && input_state->_left_mouse) {
		_map->selected_units_goto(pt, t);
		return true;
	}

	/*if (_mode == ADDING_SOLID_OBSTACLE) {
		_map->add_obstacle(SOLID, _obstacle_pts);
		_map->update_static_grids();
		_obstacle_pts.clear();
		update_grid();
		update_obstacle();
		return true;
	}
	else if (_mode == ADDING_WATER_OBSTACLE) {
		Obstacle * obstacle = _map->add_obstacle(WATER, _obstacle_pts);
		_map->_elevation->set_alti_over_polygon(obstacle->_polygon, 0.0);
		_map->update_static_grids();
		_obstacle_pts.clear();
		update_grid();
		update_obstacle();
		update_elevation();
		return true;
	}*/
	return false;
}


bool Strategy::mouse_motion(InputState * input_state, time_point t) {
	return false;
}


bool Strategy::key_down(InputState * input_state, SDL_Keycode key, time_point t) {
	if (_ihm->key_down(input_state, key, t)) {
		return true;
	}

	/*if (key == SDLK_a) {
		_contexts["debug"]->_active = !_contexts["debug"]->_active;
		_contexts["unit"]->_active = !_contexts["unit"]->_active;
		_contexts["path"]->_active = !_contexts["path"]->_active;
	}
	else if (key == SDLK_z) {
		_mode = EDIT_ELEVATION;
		return true;
	}
	else if (key == SDLK_c) {
		_map->clear();
		update_all();
		return true;
	}
	else if (key == SDLK_g) {
		if (input_state->_keys[SDLK_LSHIFT]) {
			if (_visible_grid_type == "elevation") {
				_visible_grid_type = "units_position";
			}
			else if (_visible_grid_type == "units_position") {
				_visible_grid_type = "terrain";
			}
			else if (_visible_grid_type == "terrain") {
				_visible_grid_type = "elevation";
			}
			std::cout << "visible_grid_type = " << _visible_grid_type << " ; visible_grid_unit_type = " << _visible_grid_unit_type << "\n";
			update_grid();
		}
		else if (input_state->_keys[SDLK_RSHIFT]) {
			if (_visible_grid_unit_type == "infantery") {
				_visible_grid_unit_type = "tank";
			}
			else if (_visible_grid_unit_type == "tank") {
				_visible_grid_unit_type = "boat";
			}
			else if (_visible_grid_unit_type == "boat") {
				_visible_grid_unit_type = "infantery";
			}
			std::cout << "visible_grid_type = " << _visible_grid_type << " ; visible_grid_unit_type = " << _visible_grid_unit_type << "\n";
			update_grid();
		}
		else {
			_contexts["grid"]->_active = !_contexts["grid"]->_active;
		}
		return true;
	}
	else if (key == SDLK_m) {
		_map->_path_finder->_use_line_of_sight = !_map->_path_finder->_use_line_of_sight;
		return true;
	}
	else if (key == SDLK_q) {
		_map->_paused = !_map->_paused;
		return true;
	}
	else if (key == SDLK_SPACE) {
		return true;
	}*/
	return false;
}


bool Strategy::key_up(InputState * input_state, SDL_Keycode key, time_point t) {
	return true;
}

