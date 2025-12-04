#include <glm/gtc/type_ptr.hpp>

#include "test_a_star.h"


// -------------------------------------------------
TestAStar::TestAStar() {

}


TestAStar::TestAStar(std::map<std::string, GLuint> progs, ViewSystem * view_system) : _mode(FREE), _view_system(view_system) {
	GLuint buffers[2];
	glGenBuffers(2, buffers);

	_contexts["linear"]= new DrawContext(progs["repere"], buffers[0],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});

	_contexts["terrain"]= new DrawContext(progs["repere"], buffers[1],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});
	
	_font = new Font(progs, "../../fonts/Silom.ttf", 48, _view_system->_screengl);

	_map = new Map("../data/unit_types", pt_type(-50.0, -50.0), pt_type(100.0, 100.0), pt_type(1.0, 1.0), pt_type(2.0, 2.0));

	update_linear();
	update_terrain();
}


TestAStar::~TestAStar() {
	clear();
	delete _map;
	for (auto context : _contexts) {
		delete context.second;
	}
	_contexts.clear();
	delete _font;
}


void TestAStar::clear() {
	_map->clear();
	_map->update_grids();
}


void TestAStar::draw_linear() {
	DrawContext * context= _contexts["linear"];

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


void TestAStar::draw_terrain() {
	DrawContext * context= _contexts["terrain"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void TestAStar::draw() {
	draw_linear();
	draw_terrain();
	_font->draw_3d(_view_system->_world2clip);
}


void TestAStar::anim(time_point t) {
	if (_view_system->_new_single_selection) {
		_view_system->_new_single_selection= false;
		for (auto unit : _map->_units) {
			AABB * aabb = new AABB(unit->_aabb);
			if (_view_system->single_selection_intersects_aabb(aabb, false)) {
				unit->_selected = true;
			}
			delete aabb;
		}
	}
	else if (_view_system->_new_rect_selection) {
		_view_system->_new_rect_selection= false;
		for (auto unit : _map->_units) {
			unit->_selected = false;
			AABB * aabb = new AABB(unit->_aabb);
			BBox * bbox = new BBox(aabb);
			if (_view_system->rect_selection_intersects_bbox(bbox, false)) {
				unit->_selected = true;
			}
			delete aabb;
			delete bbox;
		}
	}

	_map->anim();

	update_linear();

	std::vector<Text3D> texts;
	for (auto unit : _map->_units) {
		texts.push_back(Text3D(unit->_type->_name, glm::vec3(float(unit->_aabb->_pos.x), float(unit->_aabb->_pos.y), 0.0), 0.06, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
	}
	_font->set_text(texts);
}


void TestAStar::update_linear() {
	DrawContext * context= _contexts["linear"];
	
	context->_n_pts = 0;

	// les croix de la grille
	GraphGrid * grid = _map->_grids[_map->_unit_types["infantery"]];
	
	grid->_it_v= grid->_vertices.begin();
	while (grid->_it_v!= grid->_vertices.end()) {
		context->_n_pts += 4;
		grid->_it_v++;
	}

	for (auto obstacle : _map->_obstacles) {
		context->_n_pts += obstacle->_polygon->_pts.size() * 2;
	}
	
	if (_obstacle_pts.size() > 1) {
		context->_n_pts += 2 * (_obstacle_pts.size() - 1);
	}

	for (auto unit : _map->_units) {
		context->_n_pts += 8; // dessin AABB
		context->_n_pts += unit->_path.size() * 2;
	}

	context->_n_attrs_per_pts= 7;

	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr = data;

	grid->_it_v= grid->_vertices.begin();
	while (grid->_it_v!= grid->_vertices.end()) {
		pt_type_3d pos = grid->_it_v->second._pos;
		number positions[8] = {
			pos.x - CROSS_SIZE, pos.y - CROSS_SIZE, 
			pos.x + CROSS_SIZE, pos.y + CROSS_SIZE, 
			pos.x - CROSS_SIZE, pos.y + CROSS_SIZE, 
			pos.x + CROSS_SIZE, pos.y - CROSS_SIZE
		};
		for (uint i=0; i<4; ++i) {
			ptr[0] = float(positions[2 * i]);
			ptr[1] = float(positions[2 * i + 1]);
			ptr[2] = 0.0f;
			ptr[3] = GRID_COLOR.r;
			ptr[4] = GRID_COLOR.g;
			ptr[5] = GRID_COLOR.b;
			ptr[6] = GRID_COLOR.a;
			ptr += 7;
		}
		grid->_it_v++;
	}

	for (auto obstacle : _map->_obstacles) {
		for (uint i=0; i<obstacle->_polygon->_pts.size(); ++i) {
			uint j = i + 1;
			if (j == obstacle->_polygon->_pts.size()) {
				j = 0;
			}
			ptr[0] = float(obstacle->_polygon->_pts[i].x);
			ptr[1] = float(obstacle->_polygon->_pts[i].y);
			ptr[2] = 0.0f;
			ptr[3] = OBSTACLE_COLORS.at(obstacle->_type).r;
			ptr[4] = OBSTACLE_COLORS.at(obstacle->_type).g;
			ptr[5] = OBSTACLE_COLORS.at(obstacle->_type).b;
			ptr[6] = OBSTACLE_COLORS.at(obstacle->_type).a;

			ptr[7] = float(obstacle->_polygon->_pts[j].x);
			ptr[8] = float(obstacle->_polygon->_pts[j].y);
			ptr[9] = 0.0f;
			ptr[10] = OBSTACLE_COLORS.at(obstacle->_type).r;
			ptr[11] = OBSTACLE_COLORS.at(obstacle->_type).g;
			ptr[12] = OBSTACLE_COLORS.at(obstacle->_type).b;
			ptr[13] = OBSTACLE_COLORS.at(obstacle->_type).a;

			ptr += 14;
		}
	}

	if (_obstacle_pts.size() > 1) {
		for (uint i=0; i<_obstacle_pts.size() - 1; ++i) {
			ptr[0] = float(_obstacle_pts[i].x);
			ptr[1] = float(_obstacle_pts[i].y);
			ptr[2] = 0.0f;
			ptr[3] = EDITED_OBSTACLE_COLORS.at(_mode).r;
			ptr[4] = EDITED_OBSTACLE_COLORS.at(_mode).g;
			ptr[5] = EDITED_OBSTACLE_COLORS.at(_mode).b;
			ptr[6] = EDITED_OBSTACLE_COLORS.at(_mode).a;

			ptr[7] = float(_obstacle_pts[i + 1].x);
			ptr[8] = float(_obstacle_pts[i + 1].y);
			ptr[9] = 0.0f;
			ptr[10] = EDITED_OBSTACLE_COLORS.at(_mode).r;
			ptr[11] = EDITED_OBSTACLE_COLORS.at(_mode).g;
			ptr[12] = EDITED_OBSTACLE_COLORS.at(_mode).b;
			ptr[13] = EDITED_OBSTACLE_COLORS.at(_mode).a;

			ptr += 14;
		}
	}

	for (auto unit : _map->_units) {
		number positions[16] = {
			unit->_aabb->_pos.x, unit->_aabb->_pos.y,
			unit->_aabb->_pos.x + unit->_aabb->_size.x, unit->_aabb->_pos.y,
			
			unit->_aabb->_pos.x + unit->_aabb->_size.x, unit->_aabb->_pos.y,
			unit->_aabb->_pos.x + unit->_aabb->_size.x, unit->_aabb->_pos.y + unit->_aabb->_size.y,
			
			unit->_aabb->_pos.x + unit->_aabb->_size.x, unit->_aabb->_pos.y + unit->_aabb->_size.y,
			unit->_aabb->_pos.x, unit->_aabb->_pos.y + unit->_aabb->_size.y,
			
			unit->_aabb->_pos.x, unit->_aabb->_pos.y + unit->_aabb->_size.y,
			unit->_aabb->_pos.x, unit->_aabb->_pos.y
		};
		for (uint i=0; i<8; ++i) {
			ptr[0] = float(positions[2 * i]);
			ptr[1] = float(positions[2 * i + 1]);
			ptr[2] = 0.0f;
			if (unit->_selected) {
				ptr[3] = SELECTED_UNIT_COLOR.r;
				ptr[4] = SELECTED_UNIT_COLOR.g;
				ptr[5] = SELECTED_UNIT_COLOR.b;
				ptr[6] = SELECTED_UNIT_COLOR.a;
			}
			else {
				ptr[3] = UNIT_COLORS.at(unit->_type->_name).r;
				ptr[4] = UNIT_COLORS.at(unit->_type->_name).g;
				ptr[5] = UNIT_COLORS.at(unit->_type->_name).b;
				ptr[6] = UNIT_COLORS.at(unit->_type->_name).a;
			}
			ptr += 7;
		}
	}

	for (auto unit : _map->_units) {
		if (unit->_path.empty()) {
			continue;
		}
		for (uint i=0; i<unit->_path.size() - 1; ++i) {
			ptr[0] = float(unit->_path[i].x);
			ptr[1] = float(unit->_path[i].y);
			ptr[2] = 0.0f;
			ptr[3] = PATH_COLOR.r;
			ptr[4] = PATH_COLOR.g;
			ptr[5] = PATH_COLOR.b;
			ptr[6] = PATH_COLOR.a;

			ptr[7] = float(unit->_path[i + 1].x);
			ptr[8] = float(unit->_path[i + 1].y);
			ptr[9] = 0.0f;
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


void TestAStar::update_terrain() {
	DrawContext * context= _contexts["terrain"];
	
	context->_n_pts = 0;
	context->_n_attrs_per_pts= 7;

	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr = data;

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool TestAStar::mouse_button_down(InputState * input_state) {
	pt_type pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);
	if (_mode == ADDING_SOLID_OBSTACLE) {
		_obstacle_pts.clear();
		return true;
	}
	else if (_mode == FREE) {
		if (input_state->_left_mouse) {
			if (input_state->_keys[SDLK_i]) {
				_map->add_unit("infantery", pt);
				return true;
			}
			else if (input_state->_keys[SDLK_t]) {
				_map->add_unit("tank", pt);
				return true;
			}
			else if (input_state->_keys[SDLK_b]) {
				_map->add_unit("boat", pt);
				return true;
			}
			else if (input_state->_keys[SDLK_g]) {
				_map->selected_units_goto(pt);
				return true;
			}
		}
	}
	return false;
}


bool TestAStar::mouse_button_up(InputState * input_state) {
	if (_mode == ADDING_SOLID_OBSTACLE) {
		_map->add_obstacle(SOLID, _obstacle_pts);
		_map->update_grids();
		_obstacle_pts.clear();
		return true;
	}
	else if (_mode == ADDING_WATER_OBSTACLE) {
		_map->add_obstacle(WATER, _obstacle_pts);
		_map->update_grids();
		_obstacle_pts.clear();
		return true;
	}
	return false;
}


bool TestAStar::mouse_motion(InputState * input_state, time_point t) {
	if ((_mode == ADDING_SOLID_OBSTACLE || _mode == ADDING_WATER_OBSTACLE) && input_state->_left_mouse) {
		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_added_pt_t).count();
		if (dt> NEW_PT_IN_POLYGON_MS) {
			pt_type pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);
			_obstacle_pts.push_back(pt);
		}
		return true;
	}
	return false;
}


bool TestAStar::key_down(InputState * input_state, SDL_Keycode key) {
	if (key == SDLK_o) {
		_mode = ADDING_SOLID_OBSTACLE;
		return true;
	}
	else if (key == SDLK_w) {
		_mode = ADDING_WATER_OBSTACLE;
		return true;
	}
	else if (key == SDLK_c) {
		clear();
		return true;
	}
	return false;
}


bool TestAStar::key_up(InputState * input_state, SDL_Keycode key) {
	if (key == SDLK_o || key == SDLK_w) {
		_mode = FREE;
		return true;
	}
	return false;
}

