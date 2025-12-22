#include <iostream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

#include "test_a_star.h"


// -------------------------------------------------
TestAStar::TestAStar() {

}


TestAStar::TestAStar(std::map<std::string, GLuint> progs, ViewSystem * view_system, time_point t) :
	_mode(FREE), _view_system(view_system)
{
	GLuint buffers[7];
	glGenBuffers(6, buffers);

	_contexts["grid"]= new DrawContext(progs["repere"], buffers[0],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});
	
	//_contexts["grid"]->_active = false;

	_contexts["obstacle"]= new DrawContext(progs["repere"], buffers[1],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});

	_contexts["unit"]= new DrawContext(progs["repere"], buffers[2],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});

	_contexts["path"]= new DrawContext(progs["repere"], buffers[3],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});

	_contexts["terrain"]= new DrawContext(progs["light"], buffers[4],
		std::vector<std::string>{"position_in", "color_in", "normal_in"},
		std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position"});

	_contexts["elements"]= new DrawContext(progs["light"], buffers[5],
		std::vector<std::string>{"position_in", "color_in", "normal_in"},
		std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position"});

	_contexts["debug"]= new DrawContext(progs["repere"], buffers[6],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});

	_font = new Font(progs, "../../fonts/Silom.ttf", 48, _view_system->_screengl);
	_font->_z= 100.0f; // pour que l'affichage des infos se fassent par dessus le reste

	_view_system->_rect_select->_z = -1.0; // pour que l'affichage du rectangle de sélection se fassent par dessus le reste

	_map = new Map("../data/unit_types", "../data/elements", pt_2d(-50.0, -50.0), pt_2d(100.0, 100.0), pt_2d(1.0), pt_2d(0.25), t);
	//_map = new Map("../data/unit_types", pt_2d(0.0, 0.0), pt_2d(10.0, 10.0), pt_2d(2.0), pt_2d(2.0), t);
	//std::cout << *_map << "\n";
	//_map->randomize();

	_visible_grid = _map->_static_grids[_map->_unit_types["boat"]];

	update_all();
}


TestAStar::~TestAStar() {
	delete _map;
	for (auto context : _contexts) {
		delete context.second;
	}
	_contexts.clear();
	delete _font;
}


void TestAStar::draw_linear(std::string context_name) {
	DrawContext * context= _contexts[context_name];
	if (!context->_active) {
		return;
	}

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void TestAStar::draw_surface(std::string context_name) {
	DrawContext * context= _contexts[context_name];
	if (!context->_active) {
		return;
	}

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);

	glm::vec3 light_position(0.0f, 0.0f, 50.0f);
	glm::vec3 light_color(1.0f);

	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	glUniform3fv(context->_locs_uniform["light_position"], 1, glm::value_ptr(light_position));
	glUniform3fv(context->_locs_uniform["light_color"], 1, glm::value_ptr(light_color));
	glUniform3fv(context->_locs_uniform["view_position"], 1, glm::value_ptr(glm::vec3(_view_system->_eye)));
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["normal_in"], 3, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(7* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void TestAStar::draw() {
	for (auto context_name : std::vector<std::string>{"grid", "obstacle", "unit", "path", "debug"}) {
		draw_linear(context_name);
	}
	for (auto context_name : std::vector<std::string>{"terrain", "elements"}) {
		draw_surface(context_name);
	}
	_font->draw_3d(_view_system->_world2clip);
	_font->draw();
}


void TestAStar::anim(time_point t, InputState * input_state) {
	//pt_2d pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);
	pt_3d pt = _view_system->screen2world_depthbuffer(input_state->_x, input_state->_y);

	if (_view_system->_new_single_selection) {
		_view_system->_new_single_selection= false;
		for (auto unit : _map->_units) {
			unit->_selected = false;
			if (_view_system->single_selection_intersects_aabb(unit->_aabb, false)) {
				unit->_selected = true;
			}
		}
	}
	else if (_view_system->_new_rect_selection) {
		_view_system->_new_rect_selection= false;
		for (auto unit : _map->_units) {
			unit->_selected = false;
			BBox * bbox = new BBox(unit->_aabb);
			if (_view_system->rect_selection_intersects_bbox(bbox, false)) {
				unit->_selected = true;
			}
			delete bbox;
		}
	}

	_map->anim(t);

	update_unit();
	update_path();
	//update_grid();
	update_text(input_state);

	if (_mode == EDIT_ALTI && input_state->_left_mouse) {
		const number alti_aabb_size = 10.0;
		const number alti_inc_max = 1.0;
		AABB_2D * aabb = new AABB_2D(pt_2d(pt) - pt_2d(alti_aabb_size * 0.5), pt_2d(alti_aabb_size));
		_map->_elements->remove_in_aabb(aabb);
		std::vector<uint> ids = _map->_terrain->get_ids_over_aabb(aabb);
		delete aabb;
		for (auto id : ids) {
			pt_2d pt2 = _map->_terrain->id2pt(id);
			number dist = glm::distance(pt_2d(pt), pt2) / (alti_aabb_size * 0.5);
			if (dist > 1.0) {
				continue;
			}
			number alti_inc = alti_inc_max * (1.0 - dist * dist);

			if (input_state->_keys[SDLK_LSHIFT]) {
				_map->_terrain->_altis[id] -= alti_inc;
			}
			else {
				_map->_terrain->_altis[id] += alti_inc;
			}
		}
		_map->_terrain->set_negative_alti_2zero();

		for (auto grid : _map->_static_grids) {
			_map->update_alti_grid(grid.second);
		}
		/*for (auto grid : _map->_unit_grids) {
			_map->update_alti_grid(grid.second);
		}*/
		_map->update_static_grids();
		update_terrain();
		update_grid();
	}
}


glm::vec4 TestAStar::get_edge_color() {
	GraphEdge edge = _visible_grid->_it_e->second;
	number edge_weight = edge._weight;
	glm::vec4 edge_color;
	if (_visible_grid == _map->_static_grids[_map->_unit_types["infantery"]]) {
		if (edge_weight < 0.0) {
			edge_color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		}
		else if (edge_weight > 100.0) {
			edge_color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		}
		else {
			edge_color = glm::vec4(float(edge_weight) / 100.0f, 1.0f - float(edge_weight) / 100.0f, 0.5f, 1.0f);
		}
	}
	else if (_visible_grid == _map->_static_grids[_map->_unit_types["boat"]]) {
		if (edge_weight > 0.1 ) {
			edge_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else {
			edge_color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		}
	}
	else if (_visible_grid == _map->_units_position_grids[_map->_unit_types["infantery"]]
	|| _visible_grid == _map->_units_position_grids[_map->_unit_types["boat"]]) {
		
		/*edge_weight = fmod(edge_weight, 7.0);
		if (edge_weight < 1.0) {
			edge_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else if (edge_weight < 2.0) {
			edge_color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		}
		else if (edge_weight < 3.0) {
			edge_color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		}
		else if (edge_weight < 4.0) {
			edge_color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		}
		else if (edge_weight < 5.0) {
			edge_color = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		}
		else if (edge_weight < 6.0) {
			edge_color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
		}
		else {
			edge_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}*/

		UnitsPositionEdgeData * data = (UnitsPositionEdgeData *)(edge._data);
		if (data->_ids.size() == 0) {
			edge_color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		}
		else if (data->_ids.size() == 1) {
			edge_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else {
			edge_color = glm::vec4(0.5f, 0.0f, 1.0f, 1.0f);
		}
	}

	return edge_color;
}


void TestAStar::update_grid() {
	DrawContext * context= _contexts["grid"];
	
	context->_n_attrs_per_pts= 7;
	context->_n_pts = 0;

	// croix sommets
	/*grid->_it_v= grid->_vertices.begin();
	while (grid->_it_v!= grid->_vertices.end()) {
		context->_n_pts += 4;
		grid->_it_v++;
	}*/

	// edges
	_visible_grid->_it_v= _visible_grid->_vertices.begin();
	while (_visible_grid->_it_v!= _visible_grid->_vertices.end()) {
		_visible_grid->_it_e= _visible_grid->_it_v->second._edges.begin();
		while (_visible_grid->_it_e!= _visible_grid->_it_v->second._edges.end()) {
			context->_n_pts += 2;
			_visible_grid->_it_e++;
		}
		_visible_grid->_it_v++;
	}

	float * data = new float[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr = data;

	// croix sommets
	/*grid->_it_v= grid->_vertices.begin();
	while (grid->_it_v!= grid->_vertices.end()) {
		pt_3d pos = grid->_it_v->second._pos;
		number positions[8] = {
			pos.x - CROSS_SIZE, pos.y - CROSS_SIZE, 
			pos.x + CROSS_SIZE, pos.y + CROSS_SIZE, 
			pos.x - CROSS_SIZE, pos.y + CROSS_SIZE, 
			pos.x + CROSS_SIZE, pos.y - CROSS_SIZE
		};
		for (uint i=0; i<4; ++i) {
			ptr[0] = float(positions[2 * i]);
			ptr[1] = float(positions[2 * i + 1]);
			//ptr[2] = float(ALTI_CROSS);
			ptr[2] = float(pos.z + Z_OFFSET_CROSS);
			ptr[3] = GRID_COLOR.r;
			ptr[4] = GRID_COLOR.g;
			ptr[5] = GRID_COLOR.b;
			ptr[6] = GRID_COLOR.a;
			ptr += 7;
		}
		grid->_it_v++;
	}*/

	// edges
	_visible_grid->_it_v= _visible_grid->_vertices.begin();
	while (_visible_grid->_it_v!= _visible_grid->_vertices.end()) {
		_visible_grid->_it_e= _visible_grid->_it_v->second._edges.begin();
		while (_visible_grid->_it_e!= _visible_grid->_it_v->second._edges.end()) {
			glm::vec4 edge_color = get_edge_color();

			pt_3d & p1 = _visible_grid->_it_v->second._pos;
			pt_3d & p2 = _visible_grid->_vertices[_visible_grid->_it_e->first]._pos;
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
			
			_visible_grid->_it_e++;
		}
		_visible_grid->_it_v++;
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] data;
}

void TestAStar::update_obstacle() {
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void TestAStar::update_unit() {
	DrawContext * context= _contexts["unit"];
	
	context->_n_attrs_per_pts= 7;
	context->_n_pts = 0;

	for (auto unit : _map->_units) {
		context->_n_pts += 48; // dessin AABB
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr = data;

	// units
	for (auto unit : _map->_units) {
		std::vector<pt_3d> segs = unit->_aabb->segments();

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

		/*number positions[16] = {
			unit->_aabb->_pos.x, unit->_aabb->_pos.y,
			unit->_aabb->_pos.x + unit->_aabb->_size.x, unit->_aabb->_pos.y,
			
			unit->_aabb->_pos.x + unit->_aabb->_size.x, unit->_aabb->_pos.y,
			unit->_aabb->_pos.x + unit->_aabb->_size.x, unit->_aabb->_pos.y + unit->_aabb->_size.y,
			
			unit->_aabb->_pos.x + unit->_aabb->_size.x, unit->_aabb->_pos.y + unit->_aabb->_size.y,
			unit->_aabb->_pos.x, unit->_aabb->_pos.y + unit->_aabb->_size.y,
			
			unit->_aabb->_pos.x, unit->_aabb->_pos.y + unit->_aabb->_size.y,
			unit->_aabb->_pos.x, unit->_aabb->_pos.y
		};*/
		for (uint i=0; i < segs.size(); ++i) {
			//ptr[0] = float(positions[2 * i]);
			//ptr[1] = float(positions[2 * i + 1]);
			//ptr[2] = float(unit->_z);
			//ptr[2] = float(ALTI_UNIT);
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
				/*ptr[3] = UNIT_COLORS.at(unit->_type->_name).r;
				ptr[4] = UNIT_COLORS.at(unit->_type->_name).g;
				ptr[5] = UNIT_COLORS.at(unit->_type->_name).b;
				ptr[6] = UNIT_COLORS.at(unit->_type->_name).a;*/
				ptr[3] = unit_color.r;
				ptr[4] = unit_color.g;
				ptr[5] = unit_color.b;
				ptr[6] = unit_color.a;
			}
			ptr += 7;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void TestAStar::update_path() {
	DrawContext * context= _contexts["path"];
	
	context->_n_attrs_per_pts= 7;
	context->_n_pts = 0;
	
	for (auto unit : _map->_units) {
		context->_n_pts += unit->_path->_pts.size() * 2;
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr = data;

	for (auto unit : _map->_units) {
		if (unit->_path->_pts.empty()) {
			continue;
		}
		for (uint i=0; i<unit->_path->_pts.size() - 1; ++i) {
			ptr[0] = float(unit->_path->_pts[i].x);
			ptr[1] = float(unit->_path->_pts[i].y);
			ptr[2] = float(unit->_path->_pts[i].z + Z_OFFSET_PATH);
			//ptr[2] = float(ALTI_PATH);
			ptr[3] = PATH_COLOR.r;
			ptr[4] = PATH_COLOR.g;
			ptr[5] = PATH_COLOR.b;
			ptr[6] = PATH_COLOR.a;

			ptr[7] = float(unit->_path->_pts[i + 1].x);
			ptr[8] = float(unit->_path->_pts[i + 1].y);
			ptr[9] = float(unit->_path->_pts[i + 1].z + Z_OFFSET_PATH);
			//ptr[9] = float(ALTI_PATH);
			ptr[10] = PATH_COLOR.r;
			ptr[11] = PATH_COLOR.g;
			ptr[12] = PATH_COLOR.b;
			ptr[13] = PATH_COLOR.a;

			ptr += 14;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void TestAStar::update_debug() {
	DrawContext * context= _contexts["debug"];
	
	context->_n_attrs_per_pts= 7;
	
	uint w = 100;
	uint h = 100;
	number step = 0.1;
	context->_n_pts = w * h * 2;

	glm::vec4 DEBUG_COLOR(1.0, 0.8, 0.7, 1.0);

	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr = data;

	for (uint col=0; col<w; ++col) {
		for (uint lig=0; lig<h; ++lig) {
			pt_2d pt(number(col) * step, number(lig) * step);
			ptr[0] = float(pt.x);
			ptr[1] = float(pt.y);
			ptr[2] = float(_map->_terrain->get_alti(pt));
			ptr[3] = DEBUG_COLOR.r;
			ptr[4] = DEBUG_COLOR.g;
			ptr[5] = DEBUG_COLOR.b;
			ptr[6] = DEBUG_COLOR.a;

			ptr[7] = float(pt.x + 0.02);
			ptr[8] = float(pt.y + 0.02);
			ptr[9] = float(_map->_terrain->get_alti(pt));
			ptr[10] = DEBUG_COLOR.r;
			ptr[11] = DEBUG_COLOR.g;
			ptr[12] = DEBUG_COLOR.b;
			ptr[13] = DEBUG_COLOR.a;

			ptr += 14;
		}
	}

	/*for (uint i=0; i<context->_n_pts* context->_n_attrs_per_pts; ++i) {
		std::cout << data[i] << " ; ";
	}
	std::cout << "\n";*/

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void TestAStar::update_terrain() {
	DrawContext * context= _contexts["terrain"];

	context->_n_pts = 6 * (_map->_terrain->_n_ligs - 1) * (_map->_terrain->_n_cols - 1);
	context->_n_attrs_per_pts = 10;

	uint idx_tris[6] = {0, 1, 2, 0, 2, 3};
	//float data[context->_n_pts* context->_n_attrs_per_pts]; // génère segfault car trop grand
	float * data = new float[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr = data;
	for (uint col = 0; col<_map->_terrain->_n_cols - 1; ++col) {
		for (uint lig = 0; lig<_map->_terrain->_n_ligs - 1; ++lig) {
			pt_3d pts[4] = {
				pt_3d(_map->_terrain->col_lig2pt(col, lig), _map->_terrain->get_alti(col, lig)),
				pt_3d(_map->_terrain->col_lig2pt(col + 1, lig), _map->_terrain->get_alti(col + 1, lig)),
				pt_3d(_map->_terrain->col_lig2pt(col + 1, lig + 1), _map->_terrain->get_alti(col + 1, lig + 1)),
				pt_3d(_map->_terrain->col_lig2pt(col, lig + 1), _map->_terrain->get_alti(col, lig + 1))
			};

			std::vector<pt_3d> normals;
			normals.push_back(glm::normalize(glm::cross(pts[1] - pts[0], pts[2] - pts[0])));
			normals.push_back(glm::normalize(glm::cross(pts[2] - pts[0], pts[3] - pts[0])));
			
			for (uint i=0; i<6; ++i) {
				glm::vec4 color;
				number alti = pts[idx_tris[i]].z;
				uint idx_normal = i / 3;

				if (alti < 0.01) {
					color = glm::vec4(0.0f, 0.2f, 0.8f, 1.0f);
				}
				else if (alti < 0.3) {
					color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
				}
				else if (alti > 7.0) {
					color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				}
				else  {
					color = glm::vec4(0.2f, 1.0f - float(alti) / 10.0f, 0.2f, 1.0f);
				}

				ptr[0] = float(pts[idx_tris[i]].x);
				ptr[1] = float(pts[idx_tris[i]].y);
				//ptr[2] = float(ALTI_TERRAIN);
				ptr[2] = float(alti);
				ptr[3] = color.r;
				ptr[4] = color.g;
				ptr[5] = color.b;
				ptr[6] = color.a;
				ptr[7] = float(normals[idx_normal].x);
				ptr[8] = float(normals[idx_normal].y);
				ptr[9] = float(normals[idx_normal].z);

				ptr += 10;
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	delete[] data;
}


void TestAStar::update_elements() {

	uint n_pts = 0;
	for (auto element : _map->_elements->_elements) {
		n_pts += element->_n_pts;
	}

	DrawContext * context= _contexts["elements"];
	context->_n_pts= n_pts;
	context->_n_attrs_per_pts= 10;

	float * data = new float[context->_n_pts* context->_n_attrs_per_pts];

	uint compt = 0;
	for (auto element : _map->_elements->_elements) {
		for (uint i=0; i<element->_n_pts * context->_n_attrs_per_pts; ++i) {
			data[compt++] = element->_data[i];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	delete[] data;
}


void TestAStar::update_all() {
	update_grid();
	update_obstacle();
	update_unit();
	update_path();
	update_terrain();
	update_elements();
}


void TestAStar::update_text(InputState * input_state) {
	pt_3d pt = _view_system->screen2world_depthbuffer(input_state->_x, input_state->_y);

	std::vector<Text3D> texts_3d;
	for (auto unit : _map->_units) {
		texts_3d.push_back(Text3D(unit->_type->_name + " " + std::to_string(unit->_id), glm::vec3(unit->_aabb->_vmin)+ glm::vec3(0.1, 0.1, 0.5), 0.01, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
	}
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


bool TestAStar::mouse_button_down(InputState * input_state, time_point t) {
	//pt_2d pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);
	pt_3d pt_3d = _view_system->screen2world_depthbuffer(input_state->_x, input_state->_y);
	pt_2d pt(pt_3d.x, pt_3d.y);

	//std::vector<number> v = _map->_static_grids[_map->_unit_types["boat"]]->weights_in_cell_containing_pt(pt);
	/*std::vector<number> v = _map->_units_position_grids[_map->_unit_types["boat"]]->weights_in_cell_containing_pt(pt);
	for (auto x : v) {
		std::cout << x << " ; ";
	}
	std::cout << "\n";*/

	/*if (_map->_units.size() > 0) {
		std::vector<std::pair<uint, uint>> edges = _map->_units_position_grids[_map->_unit_types["boat"]]->edges_in_cell_containing_pt(pt);
		for (auto & edge : edges) {
			GraphEdge e = _map->_units_position_grids[_map->_unit_types["boat"]]->_vertices[edge.first]._edges[edge.second];
			UnitsPositionEdgeData * data = (UnitsPositionEdgeData *)(e._data);
			//number w = _map->_path_finder->units_position_weight(edge, _map->_units[0]);
			//std::cout << w << " ; ";
			for (auto & x : data->_ids) {
				std::cout << x << ",";
			}
			std::cout << " ; ";
		}
		std::cout << "\n";
	}*/

	if (_mode == ADDING_SOLID_OBSTACLE) {
		_obstacle_pts.clear();
		return true;
	}
	else if (_mode == FREE) {
		if (input_state->_left_mouse) {
			if (input_state->_keys[SDLK_i]) {
				_map->add_unit("infantery", pt, t);
				//update_grid();
				return true;
			}
			else if (input_state->_keys[SDLK_t]) {
				_map->add_unit("tank", pt, t);
				//update_grid();
				return true;
			}
			else if (input_state->_keys[SDLK_b]) {
				_map->add_unit("boat", pt, t);
				//update_grid();
				return true;
			}
			else if (input_state->_keys[SDLK_p]) {
				_map->selected_units_goto(pt, t);
				return true;
			}
		}
	}
	return false;
}


bool TestAStar::mouse_button_up(InputState * input_state, time_point t) {
	if (_mode == ADDING_SOLID_OBSTACLE) {
		_map->add_obstacle(SOLID, _obstacle_pts);
		_map->update_static_grids();
		_obstacle_pts.clear();
		update_grid();
		update_obstacle();
		return true;
	}
	else if (_mode == ADDING_WATER_OBSTACLE) {
		Obstacle * obstacle = _map->add_obstacle(WATER, _obstacle_pts);
		_map->_terrain->set_alti_over_polygon(obstacle->_polygon, 0.0);
		_map->update_static_grids();
		_obstacle_pts.clear();
		update_grid();
		update_obstacle();
		update_terrain();
		return true;
	}
	return false;
}


bool TestAStar::mouse_motion(InputState * input_state, time_point t) {
	if ((_mode == ADDING_SOLID_OBSTACLE || _mode == ADDING_WATER_OBSTACLE) && input_state->_left_mouse) {
		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_added_pt_t).count();
		if (dt> NEW_PT_IN_POLYGON_MS) {
			pt_2d pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);
			_obstacle_pts.push_back(pt);
		}
		return true;
	}
	else if (_mode == EDIT_ALTI) {
		// le code est dans anim
		return true;
	}
	return false;
}


bool TestAStar::key_down(InputState * input_state, SDL_Keycode key, time_point t) {
	if (key == SDLK_a) {
		/*std::cout << _map->_terrain->get_alti(pt_2d(0.0, 0.0)) << " ; ";
		std::cout << _map->_terrain->get_alti(pt_2d(-0.1, -0.1)) << " ; ";
		std::cout << _map->_terrain->get_alti(pt_2d(0.1, 0.1)) << "\n";*/
		update_debug();
	}
	if (key == SDLK_o) {
		_mode = ADDING_SOLID_OBSTACLE;
		return true;
	}
	else if (key == SDLK_w) {
		_mode = ADDING_WATER_OBSTACLE;
		return true;
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
			if (_visible_grid == _map->_static_grids[_map->_unit_types["boat"]]) {
				_visible_grid = _map->_units_position_grids[_map->_unit_types["boat"]];
			}
			else {
				_visible_grid = _map->_static_grids[_map->_unit_types["boat"]];
			}
			update_grid();
		}
		else {
			_contexts["grid"]->_active = !_contexts["grid"]->_active;
		}
		return true;
	}
	else if (key == SDLK_h) {
		_contexts["terrain"]->_active = !_contexts["terrain"]->_active;
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
		update_grid();
		return true;
	}
	return false;
}


bool TestAStar::key_up(InputState * input_state, SDL_Keycode key, time_point t) {
	_mode = FREE;
	return true;
}

