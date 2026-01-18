#include <iostream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

#include "strategy.h"


// -------------------------------------------------
Strategy::Strategy() {

}


Strategy::Strategy(std::map<std::string, GLuint> progs, ViewSystem * view_system, time_point t) :
	_mode(FREE), _view_system(view_system), _progs(progs), _angle(0.0)
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

	_contexts["elements"]= new DrawContext(progs["elevation_smooth"], 
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

	_ihm = new GLIHM("../data/ihm.json");

	_map = new Map("../data/unit_types", "../data/elements", pt_2d(-100.0, -100.0), pt_2d(200.0, 200.0), pt_2d(2.0), pt_2d(0.5), t);
	//_map = new Map("../data/unit_types", pt_2d(0.0, 0.0), pt_2d(10.0, 10.0), pt_2d(2.0), pt_2d(2.0), t);
	//std::cout << *_map << "\n";

	for (auto & unit_type : _map->_unit_types) {
		
		_contexts[unit_type.first]= new DrawContext(progs["unit"], 
			std::vector<std::string>{"position_in:3", "normal_in:3", "ambient_in:3", "diffuse_in:3", "specular_in:3", "shininess_in:1", "opacity_in:1", "model2world_matrix:16:new_instanced_buffer"},
			std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position"},
			GL_DYNAMIC_DRAW, true);

		//std::cout << *_contexts[unit_type.first] << "\n";
	}

	_map->randomize();

	/*for (uint i=0; i<200; ++i) {
		pt_2d pos = rand_pt_2d(-20.0, 20.0, -20.0, 20.0);
		_map->add_unit("helicopter", pos, t);
	}*/

	_visible_grid_unit_type = "infantery";
	_visible_grid_type = "terrain";

	update_all();
}


Strategy::~Strategy() {
	delete _map;
	for (auto context : _contexts) {
		delete context.second;
	}
	_contexts.clear();
	delete _font;
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
	glDrawArraysInstanced(GL_TRIANGLES, 0, context->_n_pts, _map->_unit_groups[unit_type]->_units.size());
	context->deactivate();
}


void Strategy::draw() {
	for (auto context_name : std::vector<std::string>{"grid", "unit_linear"}) {
		draw_linear(context_name);
	}

	for (auto context_name : std::vector<std::string>{"elevation", "elements"}) {
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
		for (auto unit : _map->_units) {
			unit->_selected = false;
			if (_view_system->single_selection_intersects_aabb(unit->_bbox->_aabb, false)) {
				unit->_selected = true;
			}
		}
	}
	else if (_view_system->_new_rect_selection) {
		_view_system->_new_rect_selection= false;
		for (auto unit : _map->_units) {
			unit->_selected = false;
			BBox * bbox = new BBox(unit->_bbox->_aabb);
			if (_view_system->rect_selection_intersects_bbox(bbox, false)) {
				unit->_selected = true;
			}
			delete bbox;
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

	/*_wave += 0.01;
	if (_wave > 1.0) {
		_wave -= 1.0;
	}*/
	//auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_anim_t).count();
	//_last_anim_t =t;
	_angle += 0.01;
	if (_angle > 2.0 * M_PI) {
		_angle -= 2.0 * M_PI;
	}
	//_wave = cos(_angle);

	update_lake();

	if (_mode == EDIT_ALTI && input_state->_left_mouse) {
		const number alti_aabb_size = 10.0;
		const number alti_inc_max = 1.0;
		AABB_2D * aabb = new AABB_2D(pt_2d(pt) - pt_2d(alti_aabb_size * 0.5), pt_2d(alti_aabb_size));
		_map->_elements->remove_in_aabb(aabb);
		std::vector<uint> ids = _map->_elevation->vertices_in_aabb(aabb);

		for (auto id : ids) {
			pt_2d pt2 = _map->_elevation->id2pt_2d(id);
			number dist = glm::distance(pt_2d(pt), pt2) / (alti_aabb_size * 0.5);
			if (dist > 1.0) {
				continue;
			}
			number alti_inc = alti_inc_max * (1.0 - dist * dist);

			if (input_state->_keys[SDLK_LSHIFT]) {
				_map->_elevation->set_alti(id, _map->_elevation->get_alti(id) - alti_inc);
			}
			else {
				_map->_elevation->set_alti(id, _map->_elevation->get_alti(id) + alti_inc);
			}
		}

		//_map->_elevation->set_negative_alti_2zero();
		_map->_elevation->update_normals(aabb);
		_map->_elevation->update_data(aabb);
		_map->sync2elevation();

		update_elevation();
		update_grid();
		update_elements();

		delete aabb;
	}
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

	for (auto unit : _map->_units) {
		context->_n_pts += 48; // dessin AABB
	}

	float data[context->data_size()];
	float * ptr = data;

	// units
	uint x = 0;
	for (auto unit : _map->_units) {
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

	/*for (uint i=0; i<context->data_size(); ++i) {
		std::cout << data[i] << " ; ";
	}*/

	context->set_data(data);
}


void Strategy::update_path() {
	DrawContext * context= _contexts["path"];
	
	context->_n_pts = 0;
	
	for (auto unit : _map->_units) {
		context->_n_pts += unit->_path->_pts.size() * 2;
	}

	float data[context->data_size()];
	float * ptr = data;

	for (auto unit : _map->_units) {
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
			//ptr[2] = float(ALTI_PATH);
			ptr[3] = path_color.r;
			ptr[4] = path_color.g;
			ptr[5] = path_color.b;
			ptr[6] = path_color.a;

			ptr[7] = float(unit->_path->_pts[i + 1].x);
			ptr[8] = float(unit->_path->_pts[i + 1].y);
			ptr[9] = float(unit->_path->_pts[i + 1].z + Z_OFFSET_PATH);
			//ptr[9] = float(ALTI_PATH);
			ptr[10] = path_color.r;
			ptr[11] = path_color.g;
			ptr[12] = path_color.b;
			ptr[13] = path_color.a;

			ptr += 14;
		}
	}

	context->set_data(data);
}


void Strategy::update_debug() {
	DrawContext * context= _contexts["debug"];
	
	context->_n_pts = 0;

	/*if (_map->_units.empty()) {
		context->_active = false;
		return;
	}

	std::vector<BBox_2D *> bboxs = _map->_units[0]->_path->_bboxs;
	if (bboxs.empty()) {
		context->_active = false;
		return;
	}

	for (auto & bbox : bboxs) {
		context->_n_pts += 8;
	}*/

	/*if (_map->_rivers.empty()) {
		context->_active = false;
		return;
	}
	context->_n_pts = 2 * _map->_rivers[_map->_rivers.size() - 1]->_id_nodes.size();*/
	
	if (_map->_lakes.empty()) {
		context->_active = false;
		return;
	}
	context->_n_pts = 2 * _map->_lakes[_map->_lakes.size() - 1]->_id_nodes.size();

	context->_active = true;

	glm::vec4 DEBUG_COLOR(1.0, 0.0, 0.0, 1.0);
	glm::vec4 DEBUG_COLOR2(0.0, 1.0, 0.0, 1.0);

	float * data = new float[context->data_size()];
	float * ptr = data;

	/*for (auto & bbox : bboxs) {
		for (uint i=0; i<4; ++i) {
			uint j = i + 1;
			if (j == 4) {
				j = 0;
			}

			ptr[0] = float(bbox->_pts[i].x);
			ptr[1] = float(bbox->_pts[i].y);
			ptr[2] = float(_map->_elevation->get_alti(bbox->_pts[i]) + 0.5);
			ptr[3] = DEBUG_COLOR.r;
			ptr[4] = DEBUG_COLOR.g;
			ptr[5] = DEBUG_COLOR.b;
			ptr[6] = DEBUG_COLOR.a;

			ptr[7] = float(bbox->_pts[j].x);
			ptr[8] = float(bbox->_pts[j].y);
			ptr[9] = float(_map->_elevation->get_alti(bbox->_pts[j]) + 0.5);
			ptr[10] = DEBUG_COLOR.r;
			ptr[11] = DEBUG_COLOR.g;
			ptr[12] = DEBUG_COLOR.b;
			ptr[13] = DEBUG_COLOR.a;

			ptr += 14;
		}
	}*/

	/*River * river = _map->_rivers[_map->_rivers.size() - 1];
	for (uint i=0; i<river->_id_nodes.size(); ++i) {
		pt_3d pt1 = _map->_elevation->id2pt_3d(river->_id_nodes[i]) + pt_3d(0.0, 0.0, 0.2);
		pt_3d pt2 = pt1 + pt_3d(0.1, 0.1, 0.0);*/
	
	Lake * lake = _map->_lakes[_map->_lakes.size() - 1];
	for (uint i=0; i<lake->_id_nodes.size(); ++i) {
		pt_3d pt1 = _map->_elevation->id2pt_3d(lake->_id_nodes[i]) + pt_3d(0.0, 0.0, 0.2);
		pt_3d pt2 = pt1 + pt_3d(0.1, 0.1, 0.0);

		ptr[0] = float(pt1.x);
		ptr[1] = float(pt1.y);
		ptr[2] = float(pt1.z);
		ptr[3] = DEBUG_COLOR.r;
		ptr[4] = DEBUG_COLOR.g;
		ptr[5] = DEBUG_COLOR.b;
		ptr[6] = DEBUG_COLOR.a;

		ptr[7] = float(pt2.x);
		ptr[8] = float(pt2.y);
		ptr[9] = float(pt2.z);
		ptr[10] = DEBUG_COLOR.r;
		ptr[11] = DEBUG_COLOR.g;
		ptr[12] = DEBUG_COLOR.b;
		ptr[13] = DEBUG_COLOR.a;

		ptr += 14;
	}

	/*for (int i=0; i<context->_n_pts* context->_n_attrs_per_pts; ++i) {
		if (i % 7 == 0) {
			std::cout << "\n\n";
		}
		std::cout << data[i] << " ; ";
	}
	std::cout << "\n";*/

	context->set_data(data);
	delete[] data;
}


void Strategy::update_elevation() {
	DrawContext * context= _contexts["elevation"];

	context->_n_pts = _map->_elevation->_n_pts;

	context->set_data(_map->_elevation->_data);
}


void Strategy::update_elements() {
	DrawContext * context= _contexts["elements"];

	context->_n_pts = 0;
	for (auto & element : _map->_elements->_elements) {
		context->_n_pts += element->_n_pts;
	}

	float * data = new float[context->data_size()];

	uint compt = 0;
	uint n_attrs_per_pts = context->_buffers[0]._n_attrs_per_pts;
	for (auto & element : _map->_elements->_elements) {
		for (uint i=0; i<element->_n_pts * n_attrs_per_pts; ++i) {
			data[compt++] = element->_data[i];
		}
	}

	context->set_data(data);
	delete[] data;
}


void Strategy::update_river() {
	DrawContext * context= _contexts["river"];

	if (_map->_rivers.empty()) {
		//context->_active = false;
		return;
	}

	context->_n_pts = 0;
	for (auto & river : _map->_rivers) {
		context->_n_pts += river->_n_pts;
	}

	float * data = new float[context->data_size()];

	uint compt = 0;
	uint n_attrs_per_pts = context->_buffers[0]._n_attrs_per_pts;
	for (auto & river : _map->_rivers) {
		for (uint i=0; i<river->_n_pts * n_attrs_per_pts; ++i) {
			data[compt++] = river->_data[i];
		}
	}

	context->set_data(data);
	delete[] data;
}


void Strategy::update_lake() {
	DrawContext * context= _contexts["lake"];

	if (_map->_lakes.empty()) {
		//context->_active = false;
		return;
	}

	context->_n_pts = 0;
	for (auto & lake : _map->_lakes) {
		context->_n_pts += lake->_n_pts;
	}

	float * data = new float[context->data_size()];

	uint compt = 0;
	uint n_attrs_per_pts = context->_buffers[0]._n_attrs_per_pts;
	for (auto & lake : _map->_lakes) {
		for (uint i=0; i<lake->_n_pts * n_attrs_per_pts; ++i) {
			data[compt++] = lake->_data[i];
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

	context->_n_instances = _map->_unit_groups[unit_type]->_units.size();
	context->set_data(_map->_unit_groups[unit_type]->_matrices, 1);
}


void Strategy::update_all() {
	_ihm->update();

	update_grid();
	update_unit_linear();
	update_path();
	update_elevation();
	update_elements();
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
	for (auto unit : _map->_units) {
		texts_3d.push_back(Text3D(unit->_type->_name + " " + std::to_string(unit->_id), glm::vec3(unit->_bbox->_aabb->_vmin)+ glm::vec3(0.1, 0.1, 0.5), 0.01, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
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
		return;
	}

	pt_3d pt_3d = _view_system->screen2world_depthbuffer(input_state->_x, input_state->_y);
	pt_2d pt(pt_3d.x, pt_3d.y);

	if (_mode == FREE) {
		if (input_state->_left_mouse) {
			if (input_state->_keys[SDLK_i]) {
				_map->add_unit("infantery", pt, t);
				update_unit_matrices(_map->_unit_types["infantery"]);
				//update_grid();
				return true;
			}
			else if (input_state->_keys[SDLK_t]) {
				_map->add_unit("tank", pt, t);
				update_unit_matrices(_map->_unit_types["tank"]);
				//update_grid();
				return true;
			}
			else if (input_state->_keys[SDLK_b]) {
				_map->add_unit("boat", pt, t);
				update_unit_matrices(_map->_unit_types["boat"]);
				//update_grid();
				return true;
			}
			else if (input_state->_keys[SDLK_h]) {
				_map->add_unit("helicopter", pt, t);
				update_unit_matrices(_map->_unit_types["helicopter"]);
				//update_grid();
				return true;
			}
			else if (input_state->_keys[SDLK_p]) {
				_map->selected_units_goto(pt, t);
				return true;
			}
			else if (input_state->_keys[SDLK_w]) {
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
		}
	}
	return false;
}


bool Strategy::mouse_button_up(InputState * input_state, time_point t) {
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
	/*if ((_mode == ADDING_SOLID_OBSTACLE || _mode == ADDING_WATER_OBSTACLE) && input_state->_left_mouse) {
		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_added_pt_t).count();
		if (dt> NEW_PT_IN_POLYGON_MS) {
			pt_2d pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);
			_obstacle_pts.push_back(pt);
		}
		return true;
	}
	else*/ if (_mode == EDIT_ALTI) {
		// le code est dans anim
		return true;
	}
	return false;
}


bool Strategy::key_down(InputState * input_state, SDL_Keycode key, time_point t) {
	if (_ihm->key_down(input_state, key, t)) {
		return;
	}

	if (key == SDLK_a) {
		_contexts["debug"]->_active = !_contexts["debug"]->_active;
		_contexts["unit"]->_active = !_contexts["unit"]->_active;
		_contexts["path"]->_active = !_contexts["path"]->_active;
	}
	else if (key == SDLK_z) {
		_mode = EDIT_ALTI;
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
	else if (key == SDLK_v) {
		for (auto & context_name : std::vector<std::string> {"elevation", "lake", "river", "elements"}) {
			_contexts[context_name]->_active = !_contexts[context_name]->_active;
		}
		return true;
	}
	else if (key == SDLK_r) {
		_map->randomize();
		update_all();
		return true;
	}
	else if (key == SDLK_m) {
		_map->_path_finder->_use_line_of_sight = !_map->_path_finder->_use_line_of_sight;
		return true;
	}
	else if (key == SDLK_l) {
		_map->load("../data/map.json", t);
		update_all();
		return true;
	}
	else if (key == SDLK_s) {
		_map->save("../data/map.json");
		return true;
	}
	else if (key == SDLK_q) {
		_map->_paused = !_map->_paused;
		return true;
	}
	else if (key == SDLK_SPACE) {
		/*for (auto & context_name : std::vector<std::string> {"elevation", "elements", "river", "lake"}) {
			if (_contexts[context_name]->_prog == _progs["elevation_flat"]) {
				_contexts[context_name]->_prog = _progs["elevation_smooth"];
			}
			else {
				_contexts[context_name]->_prog = _progs["elevation_flat"];
			}
		}*/

		/*AABB_2D * aabb = new AABB_2D(pt_2d(0.0, 0.0), pt_2d(10.0, 10.0));
		std::vector<uint> ids = _map->_elevation->vertices_in_aabb(aabb);
		for (auto id : ids) {
			_map->_elevation->set_alti(id, _map->_elevation->get_alti(id) + 0.5);
			//_map->_elevation->set_alti(id, 0.0);
		}
		_map->_elevation->update_normals(aabb);
		_map->_elevation->update_data(aabb);
		update_elevation();
		delete aabb;*/

		/*ObjData * data = _map->_unit_types["tank"]->_obj_data;
		for (int i=0; i<data->_n_pts * data->_n_attrs_per_pts; ++i) {
			std::cout << data->_data[i] << " ; ";
		}
		std::cout << "\n";*/

		/*float * data = _map->_unit_groups[_map->_unit_types["tank"]]->_matrices;
		for (int i=0; i<N_MAX_UNITS_PER_GROUP * 16; ++i) {
			std::cout << data[i] << " ; ";
		}
		std::cout << "\n";*/


		//_contexts["unit_linear"]->show_data();

		/*std::vector<pt_3d> segs = _map->_units[0]->_bbox->_aabb->segments();
		for (auto & x : segs) {
			std::cout << glm_to_string(x) << "\n";
		}*/

		std::cout << *_map->_units[0]->_bbox << "\n";

		return true;
	}
	return false;
}


bool Strategy::key_up(InputState * input_state, SDL_Keycode key, time_point t) {
	_mode = FREE;
	return true;
}

