#include <glm/gtc/type_ptr.hpp>

#include "test_a_star.h"


// -------------------------------------------------
UnitType::UnitType() {

}


UnitType::UnitType(pt_type size, number velocity) : _size(size), _velocity(velocity) {

}


UnitType::~UnitType() {

}


Unit::Unit() {

}


Unit::Unit(UnitType * type, pt_type pos) :_type(type), _selected(false), _idx_path(0), _mode(WAITING) {
	_aabb = new AABB_2D(pos - 0.5 * _type->_size, _type->_size);
}


Unit::~Unit() {
	
}


void Unit::clear_path() {
	_path.clear();
	_idx_path = 0;
}


// -------------------------------------------------
TestAStar::TestAStar() {

}


TestAStar::TestAStar(std::map<std::string, GLuint> progs) : _mode(FREE) {
	GLuint buffers[2];
	glGenBuffers(2, buffers);

	_contexts["normal"]= new DrawContext(progs["repere"], buffers[0],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});
	
	unsigned int n_ligs= 50;
	unsigned int n_cols= 50;
	glm::vec2 origin(-50.0f, -50.0f);
	glm::vec2 size(100.0f, 100.0f);
	_path_finder = new PathFinder(n_ligs, n_cols, origin, size);

	_unit_types["small"] = new UnitType(pt_type(0.5, 0.5), 0.08);
	_unit_types["big"] = new UnitType(pt_type(2.0, 2.0), 0.04);
}


TestAStar::~TestAStar() {
	delete _path_finder;
}


void TestAStar::draw(const glm::mat4 & world2clip) {
	DrawContext * context= _contexts["normal"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
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


void TestAStar::anim(time_point t, ViewSystem * view_system) {
	if (view_system->_new_single_selection) {
		view_system->_new_single_selection= false;
		for (auto unit : _units) {
			AABB * aabb = new AABB(unit->_aabb);
			if (view_system->single_selection_intersects_aabb(aabb)) {
				std::cout << "ok\n";
			}
			delete aabb;
		}
	}
	else if (view_system->_new_rect_selection) {
		view_system->_new_rect_selection= false;
		for (auto unit : _units) {
			unit->_selected = false;
			AABB * aabb = new AABB(unit->_aabb);
			BBox * bbox = new BBox(aabb);
			if (view_system->rect_selection_intersects_bbox(bbox, false)) {
				unit->_selected = true;
			}
			delete aabb;
			delete bbox;
		}
	}

	for (auto unit : _units) {
		if (unit->_mode == MOVING) {
			/*if (glm::distance(unit->_aabb->_pos, unit->_path[unit->_path.size() - 1]) < 0.1) {
				unit->_mode = WAITING;
				unit->clear_path();
				continue;
			}*/

			if (glm::distance(unit->_aabb->_pos, unit->_path[unit->_idx_path]) < 0.1) {
				unit->_idx_path++;
				if (unit->_idx_path == unit->_path.size()) {
					unit->_mode = WAITING;
					unit->clear_path();
					continue;
				}
			}

			unit->_aabb->_pos += unit->_type->_velocity * glm::normalize(unit->_path[unit->_idx_path] - unit->_aabb->_pos);
		}
	}

	update();
}


void TestAStar::update() {
	DrawContext * context= _contexts["normal"];
	context->_n_pts = 0;

	_path_finder->_grid->_it_v= _path_finder->_grid->_vertices.begin();
	while (_path_finder->_grid->_it_v!= _path_finder->_grid->_vertices.end()) {
		context->_n_pts += 4;
		_path_finder->_grid->_it_v++;
	}

	for (auto poly : _path_finder->_polygons) {
		context->_n_pts += poly->_pts.size() * 2;
	}
	for (auto unit : _units) {
		context->_n_pts += 8; // dessin AABB
		context->_n_pts += unit->_path.size() * 2;
	}

	context->_n_attrs_per_pts= 7;

	glm::vec4 grid_color(0.8f, 0.8f, 0.7f, 1.0f);
	glm::vec4 obstacle_color(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 unit_color(0.0f, 1.0f, 0.0f, 1.0f);
	glm::vec4 selected_unit_color(1.0f, 1.0f, 0.0f, 1.0f);
	glm::vec4 path_color(0.7f, 0.8f, 0.3f, 1.0f);

	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr = data;

	number cross_size = 0.2;
	_path_finder->_grid->_it_v= _path_finder->_grid->_vertices.begin();
	while (_path_finder->_grid->_it_v!= _path_finder->_grid->_vertices.end()) {
		pt_type_3d pos = _path_finder->_grid->_it_v->second._pos;
		number positions[8] = {
			pos.x - cross_size, pos.y - cross_size, 
			pos.x + cross_size, pos.y + cross_size, 
			pos.x - cross_size, pos.y + cross_size, 
			pos.x + cross_size, pos.y - cross_size
		};
		for (uint i=0; i<4; ++i) {
			ptr[0] = float(positions[2 * i]);
			ptr[1] = float(positions[2 * i + 1]);
			ptr[2] = 0.0f;
			ptr[3] = grid_color.r;
			ptr[4] = grid_color.g;
			ptr[5] = grid_color.b;
			ptr[6] = grid_color.a;
			ptr += 7;
		}
		_path_finder->_grid->_it_v++;
	}

	for (auto poly : _path_finder->_polygons) {
		for (uint i=0; i<poly->_pts.size(); ++i) {
			uint j = i + 1;
			if (j == poly->_pts.size()) {
				j = 0;
			}
			ptr[0] = float(poly->_pts[i].x);
			ptr[1] = float(poly->_pts[i].y);
			ptr[2] = 0.0f;
			ptr[3] = obstacle_color.r;
			ptr[4] = obstacle_color.g;
			ptr[5] = obstacle_color.b;
			ptr[6] = obstacle_color.a;

			ptr[7] = float(poly->_pts[j].x);
			ptr[8] = float(poly->_pts[j].y);
			ptr[9] = 0.0f;
			ptr[10] = obstacle_color.r;
			ptr[11] = obstacle_color.g;
			ptr[12] = obstacle_color.b;
			ptr[13] = obstacle_color.a;

			ptr += 14;
		}
	}

	for (auto unit : _units) {
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
				ptr[3] = selected_unit_color.r;
				ptr[4] = selected_unit_color.g;
				ptr[5] = selected_unit_color.b;
				ptr[6] = selected_unit_color.a;
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

	for (auto unit : _units) {
		if (unit->_path.empty()) {
			continue;
		}
		for (uint i=0; i<unit->_path.size() - 1; ++i) {
			ptr[0] = float(unit->_path[i].x);
			ptr[1] = float(unit->_path[i].y);
			ptr[2] = 0.0f;
			ptr[3] = path_color.r;
			ptr[4] = path_color.g;
			ptr[5] = path_color.b;
			ptr[6] = path_color.a;

			ptr[7] = float(unit->_path[i + 1].x);
			ptr[8] = float(unit->_path[i + 1].y);
			ptr[9] = 0.0f;
			ptr[10] = path_color.r;
			ptr[11] = path_color.g;
			ptr[12] = path_color.b;
			ptr[13] = path_color.a;

			ptr += 14;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool TestAStar::mouse_button_down(InputState * input_state, ViewSystem * view_system) {
	pt_type pt = view_system->screen2world(input_state->_x, input_state->_y, 0.0);
	if (input_state->_left_mouse) {
		if (input_state->_keys[SDLK_a]) {
			_units.push_back(new Unit(_unit_types["small"], pt));
			return true;
		}
		else if (input_state->_keys[SDLK_b]) {
			_units.push_back(new Unit(_unit_types["big"], pt));
			return true;
		}
		else if (input_state->_keys[SDLK_g]) {
			for (auto unit : _units) {
				if (unit->_selected) {
					unit->clear_path();
					_path_finder->path_find(unit->_aabb->_pos, pt, unit->_path);
					unit->_mode = MOVING;
				}
			}
		}
	}
	return false;
}


bool TestAStar::mouse_motion(InputState * input_state, ViewSystem * view_system, time_point t) {
	if (_mode == ADDING_OBSTACLE) {
		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_added_pt_t).count();
		if (dt> NEW_PT_IN_POLYGON_MS) {
			pt_type pt = view_system->screen2world(input_state->_x, input_state->_y, 0.0);
			_polygon_pts.push_back(pt);
		}
		return true;
	}
	return false;
}


bool TestAStar::key_down(InputState * input_state, SDL_Keycode key) {
	if (key == SDLK_o) {
		if (_mode == FREE) {
			_mode = ADDING_OBSTACLE;
			//_edited_polygon->clear();
			_polygon_pts.clear();
			return true;
		}
	}
	return false;
}


bool TestAStar::key_up(InputState * input_state, SDL_Keycode key) {
	if (key == SDLK_o) {
		_mode = FREE;
		Polygon2D * polygon = new Polygon2D(_polygon_pts, true);
		polygon->update_all();
		_path_finder->_polygons.push_back(polygon);
		_path_finder->update_grid();
		return true;
	}
	return false;
}

