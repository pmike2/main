#include "path_finder_test.h"


// GMOTest -----------------------------------------------------------------------------------------------
GMOTest::GMOTest() {

}


GMOTest::GMOTest(GridMovingObjectType * type, pt_2d pos, pt_2d size, number speed) :
	GridMovingObject(type, pos, size), _speed(speed), _selected(false)
{

}


GMOTest::~GMOTest() {
	
}


// PathFinderTest ----------------------------------------------------------------------------------------
PathFinderTest::PathFinderTest() {

}


PathFinderTest::PathFinderTest(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t) :
	_gl_draw_manager(gl_draw_manager), _view_system(view_system) 
{
	uint n_ligs = uint(GRID_SIZE.y / GRID_RESOLUTION.y) + 1;
	uint n_cols = uint(GRID_SIZE.x / GRID_RESOLUTION.x) + 1;

	_pf = new PathFinder(GRID_ORIGIN, GRID_SIZE, n_ligs, n_cols, t);

	_font = new Font(_gl_draw_manager, "../../fonts/Silom.ttf", 48, _view_system->_screengl);
	_font->_z = 100.0f; // pour que l'affichage des infos se fassent par dessus le reste

	//randomize_edges();
	update();
}


PathFinderTest::~PathFinderTest() {
	delete _pf;
}


void PathFinderTest::add_gmo(std::string type_name, pt_2d pos, pt_2d size, number speed) {
	GridMovingObjectType * type = get_gmo_type(type_name);
	uint id = pt2closest_id(pos);
	pt_2d center = id2pt_2d(id);
	GMOTest * gmo = new GMOTest(type, center - 0.5 * pt_2d(size), pt_2d(size), speed);
	_pf->init_gmo(gmo);
	_gmos.push_back(gmo);
}


void PathFinderTest::delete_selected_gmos() {
	_gmos.erase(std::remove_if(_gmos.begin(), _gmos.end(), [](GridMovingObject * gmo) { return gmo->_selected; }), _gmos.end());
}


void PathFinderTest::goto_selected_gmos(pt_2d target) {
	uint goal = pt2closest_id(target);

	for (auto & gmo : _gmos) {
		if (gmo->_selected) {
			goto_gmo(gmo, target);
		}
	}
}


void PathFinderTest::anim(time_point t) {
	_pf->anim_gmo(_gmos, t);

	update_select();
	update_gmos();
	update_gmos_path();
	update_font();
}


void PathFinderTest::draw_font() {
	_font->draw_3d(_view_system->_world2clip);
}


void PathFinderTest::draw_grid_centers() {
	GLDrawContext * context = _gl_draw_manager->get_context("grid_centers");
	context->activate();
	context->set_uniform("world2clip_matrix", glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	context->set_uniform("z", Z_CENTER);
	context->draw();
	context->deactivate();
}


void PathFinderTest::draw_grid_edges() {
	GLDrawContext * context = _gl_draw_manager->get_context("grid_edges");
	context->activate();
	context->set_uniform("world2clip_matrix", glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	context->set_uniform("z", Z_EDGE);
	context->draw();
	context->deactivate();
}


void PathFinderTest::draw_gmos() {
	GLDrawContext * context = _gl_draw_manager->get_context("gmos");
	context->activate();
	context->set_uniform("world2clip_matrix", glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	context->set_uniform("z", Z_GMO);
	context->draw();
	context->deactivate();
}


void PathFinderTest::draw_gmos_path() {
	GLDrawContext * context = _gl_draw_manager->get_context("gmos_path");
	context->activate();
	context->set_uniform("world2clip_matrix", glm::value_ptr(glm::mat4(_view_system->_world2clip)));
	context->set_uniform("z", Z_GMO_PATH);
	context->set_uniform("thickness", 6.0f);
	context->set_uniform("viewport_size", glm::value_ptr(glm::vec2(float(_view_system->_screengl->_screen_width), float(_view_system->_screengl->_screen_height))));
	context->draw();
	context->deactivate();
}


void PathFinderTest::draw_select() {
	GLDrawContext * context= _gl_draw_manager->get_context("select");
	context->activate();
	// pour que l'affichage du rectangle de sélection se fassent par dessus le reste
	context->set_uniform("z", -1.0f);
	context->set_uniform("thickness", 7.0f);
	context->set_uniform("viewport_size", glm::value_ptr(glm::vec2(float(_view_system->_screengl->_screen_width), float(_view_system->_screengl->_screen_height))));
	context->draw();
	context->deactivate();
}


void PathFinderTest::draw() {
	glViewport(0, 0, _view_system->_screengl->_screen_width, _view_system->_screengl->_screen_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_view_system->draw();
	draw_grid_centers();
	draw_grid_edges();
	draw_gmos();
	draw_gmos_path();
	draw_select();
	draw_font();
}


void PathFinderTest::update_font() {
	std::vector<Text3D> texts_3d;

	for (auto & gmo : _gmos) {
		texts_3d.push_back(Text3D(std::to_string(gmo->_id), glm::vec3(gmo->_aabb->_pos.x, gmo->_aabb->_pos.y, Z_FONT), 0.01, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
	}

	_font->set_text(texts_3d);
}


void PathFinderTest::update_select() {
	GLDrawContext * context= _gl_draw_manager->get_context("select");

	if (!_view_system->_rect_select->_is_active) {
		context->_active = false;
		return;
	}

	context->_active = true;
	context->_n_pts = 8;

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


void PathFinderTest::update_grid_centers() {
	GLDrawContext * context= _gl_draw_manager->get_context("grid_centers");

	context->_active = true;
	context->_n_pts = _pf->_n_ligs * _pf->_n_cols * 6;

	float * data = new float[context->data_size()];
	float * ptr = data;

	for (uint lig=0; lig<_pf->_n_ligs; lig++) {
		for (uint col=0; col<_pf->_n_cols; col++) {
			pt_2d center = _pf->col_lig2pt_2d(col, lig);

			pt_2d pt1 = center + pt_2d(-1.0 * _pf->_resolution.x * GRID_CENTER_SIZE_RATIO, -1.0 * _pf->_resolution.y * GRID_CENTER_SIZE_RATIO);
			pt_2d pt2 = center + pt_2d(1.0 * _pf->_resolution.x * GRID_CENTER_SIZE_RATIO, -1.0 * _pf->_resolution.y * GRID_CENTER_SIZE_RATIO);
			pt_2d pt3 = center + pt_2d(1.0 * _pf->_resolution.x * GRID_CENTER_SIZE_RATIO, 1.0 * _pf->_resolution.y * GRID_CENTER_SIZE_RATIO);
			pt_2d pt4 = center + pt_2d(-1.0 * _pf->_resolution.x * GRID_CENTER_SIZE_RATIO, 1.0 * _pf->_resolution.y * GRID_CENTER_SIZE_RATIO);

			pt_2d pts[6] = {pt1, pt2, pt3, pt1, pt3, pt4};
			
			PathFinderVertexData * vertex_data = _pf->get_vertex_data(_pf->col_lig2id(col, lig));
			glm::vec4 vertex_color;
			if (vertex_data->_type == GRID_VERTEX_LAND) {
				vertex_color = glm::vec4(0.3, 0.9, 0.3, 1.0);
			}
			else if (vertex_data->_type == GRID_VERTEX_WATER) {
				vertex_color = glm::vec4(0.3, 0.3, 0.9, 1.0);
			}
			else if (vertex_data->_type == GRID_VERTEX_LAND_OBSTACLE) {
				vertex_color = glm::vec4(0.7, 0.7, 0.7, 1.0);
			}

			for (uint i=0; i<6; ++i) {
				ptr[0] = float(pts[i].x);
				ptr[1] = float(pts[i].y);
				ptr[2] = vertex_color.r;
				ptr[3] = vertex_color.g;
				ptr[4] = vertex_color.b;
				ptr[5] = vertex_color.a;
				ptr += 6;
			}
		}
	}

	context->set_data(data);
	delete[] data;
}


void PathFinderTest::update_grid_edges() {
	GLDrawContext * context= _gl_draw_manager->get_context("grid_edges");

	context->_active = true;

	_pf->_it_v= _pf->_vertices.begin();
	while (_pf->_it_v!= _pf->_vertices.end()) {
		_pf->_it_e= _pf->_it_v->second._edges.begin();
		while (_pf->_it_e!= _pf->_it_v->second._edges.end()) {
			context->_n_pts += 2;
			_pf->_it_e++;
		}
		_pf->_it_v++;
	}

	float * data = new float[context->data_size()];
	float * ptr = data;

	_pf->_it_v= _pf->_vertices.begin();
	while (_pf->_it_v!= _pf->_vertices.end()) {
		_pf->_it_e= _pf->_it_v->second._edges.begin();
		while (_pf->_it_e!= _pf->_it_v->second._edges.end()) {
			glm::vec4 edge_color;
			PathFinderEdgeData * edge_data = _pf->get_edge_data(_pf->_it_v->first, _pf->_it_e->first);
			if (edge_data->_type == GRID_EDGE_FLAT) {
				edge_color = glm::vec4(0.0, 1.0, 0.0, 1.0);
			}
			else {
				edge_color = glm::vec4(1.0, 0.0, 0.0, 1.0);
			}

			pt_3d & p1 = _pf->_it_v->second._pos;
			pt_3d & p2 = _pf->_vertices[_pf->_it_e->first]._pos;
			pt_3d p1b = p1 + (p2 - p1) * 0.1;
			pt_3d p_middle = (p1 + p2) * 0.5 - (p2 - p1) * 0.1;
			
			ptr[0] = float(p1b.x);
			ptr[1] = float(p1b.y);
			ptr[2] = edge_color.r;
			ptr[3] = edge_color.g;
			ptr[4] = edge_color.b;
			ptr[5] = edge_color.a;

			ptr[6] = float(p_middle.x);
			ptr[7] = float(p_middle.y);
			ptr[8] = edge_color.r;
			ptr[9] = edge_color.g;
			ptr[10] = edge_color.b;
			ptr[11] = edge_color.a;

			ptr += 12;
			
			_pf->_it_e++;
		}
		_pf->_it_v++;
	}

	context->set_data(data);
	delete[] data;
}


void PathFinderTest::update_gmos() {
	GLDrawContext * context= _gl_draw_manager->get_context("gmos");

	if (_gmos.size() == 0) {
		context->_active = false;
		return;
	}

	context->_active = true;
	context->_n_pts = _gmos.size() * 6;

	float * data = new float[context->data_size()];
	float * ptr = data;

	for (auto & gmo : _gmos) {
		pt_2d pts[6] = {
			gmo->_aabb->_pos, gmo->_aabb->_pos + pt_2d(gmo->_aabb->_size.x, 0.0), gmo->_aabb->_pos + gmo->_aabb->_size,
			gmo->_aabb->_pos, gmo->_aabb->_pos + gmo->_aabb->_size, gmo->_aabb->_pos + pt_2d(0.0, gmo->_aabb->_size.y)
		};
		
		glm::vec4 color;
		float alpha = 0.3;
		if (gmo->_selected) {
			color = glm::vec4(1.0, 1.0, 0.0, alpha);
		}
		else if (gmo->_status == GMO_IDLE) {
			color = glm::vec4(0.0, 0.2, 1.0, alpha);
		}
		else if (gmo->_status == GMO_MOVING) {
			color = glm::vec4(0.0, 0.8, 1.0, alpha);
		}
		else if (gmo->_status == GMO_WAITING) {
			color = glm::vec4(0.8, 0.2, 0.3, alpha);
		}
		
		for (uint i=0; i<6; ++i) {
			ptr[0] = float(pts[i].x);
			ptr[1] = float(pts[i].y);
			ptr[2] = color.r;
			ptr[3] = color.g;
			ptr[4] = color.b;
			ptr[5] = color.a;
			ptr += 6;
		}
	}

	context->set_data(data);
	delete[] data;
}


void PathFinderTest::update_gmos_path() {
	GLDrawContext * context= _gl_draw_manager->get_context("gmos_path");

	if (_gmos.size() == 0) {
		context->_active = false;
		return;
	}

	context->_n_pts = 0;
	for (auto & gmo : _gmos) {
		if (gmo->_path.empty()) {
			continue;
		}
		
		context->_n_pts += 2 * (gmo->_path.size() - 1);
	}
	
	if (context->_n_pts == 0) {
		context->_active = false;
		return;
	}

	context->_active = true;

	float * data = new float[context->data_size()];
	float * ptr = data;

	for (auto & gmo : _gmos) {
		if (gmo->_path.empty()) {
			continue;
		}

		for (uint i=0; i<gmo->_path.size() - 1; ++i) {
			pt_2d pt1 = _pf->id2pt_2d(gmo->_path[i]);
			pt_2d pt2 = _pf->id2pt_2d(gmo->_path[i + 1]);
			glm::vec4 color(0.7, 0.8, 0.9, 0.5);

			ptr[0] = float(pt1.x);
			ptr[1] = float(pt1.y);
			ptr[2] = color.r;
			ptr[3] = color.g;
			ptr[4] = color.b;
			ptr[5] = color.a;

			ptr[6] = float(pt2.x);
			ptr[7] = float(pt2.y);
			ptr[8] = color.r;
			ptr[9] = color.g;
			ptr[10] = color.b;
			ptr[11] = color.a;

			ptr += 12;
		}
	}

	context->set_data(data);
	delete[] data;
}


void PathFinderTest::update() {
	update_grid_centers();
	update_grid_edges();
	update_gmos();
	update_gmos_path();
	update_select();
	update_font();
}


bool PathFinderTest::mouse_button_down(InputState * input_state, time_point t) {
	if (input_state->_keys[SDLK_a]) {
		pt_2d pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);
		number size = 1.5;
		number speed = 0.04;
		add_gmo("infantery", pt, size, speed);
		update_gmos();
		return true;
	}
	else if (input_state->_keys[SDLK_z]) {
		pt_2d pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);
		number size = 5.0;
		number speed = 0.02;
		add_gmo("boat", pt, size, speed);
		update_gmos();
		return true;
	}
	else if (input_state->_keys[SDLK_g]) {
		pt_2d pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);
		goto_selected_gmos(pt);
	}

	if (_view_system->mouse_button_down(input_state, t)) {
		return true;
	}

	return false;
}


bool PathFinderTest::mouse_button_up(InputState * input_state, time_point t) {
	if (_view_system->mouse_button_up(input_state, t)) {
		//return true;
	}

	if (_view_system->_new_single_selection) {
		_view_system->_new_single_selection= false;
		for (auto & gmo : _gmos) {
			gmo->_selected = false;
			if (_view_system->single_selection_intersects_aabb_2d(gmo->_aabb)) {
				gmo->_selected = true;
			}
		}
	}
	else if (_view_system->_new_rect_selection) {
		_view_system->_new_rect_selection= false;
		for (auto & gmo : _gmos) {
			gmo->_selected = false;
			if (_view_system->intersects_aabb_2d(gmo->_aabb, true)) {
				gmo->_selected = true;
			}
		}
	}

	return false;
}
	

bool PathFinderTest::mouse_motion(InputState * input_state, time_point t) {
	pt_2d pt = _view_system->screen2world(input_state->_x, input_state->_y, 0.0);

	if (input_state->_keys[SDLK_w]) {
		_pf->set_edges(pt, 3.0, GRID_EDGE_FLAT);
		return true;
	}
	if (input_state->_keys[SDLK_x]) {
		_pf->set_edges(pt, 3.0, GRID_EDGE_HARD_UP);
		return true;
	}
	if (input_state->_keys[SDLK_b]) {
		_pf->set_vertex(pt, 3.0, GRID_VERTEX_LAND);
		return true;
	}
	if (input_state->_keys[SDLK_n]) {
		_pf->set_vertex(pt, 3.0, GRID_VERTEX_WATER);
		return true;
	}

	if (_view_system->mouse_motion(input_state, t)) {
		return true;
	}

	return false;
}


bool PathFinderTest::key_down(InputState * input_state, SDL_Keycode key, time_point t) {
	if (key == SDLK_d) {
		delete_selected_gmos();
		return true;
	}
	if (key == SDLK_c) {
		_pf->clear_edges();
		return true;
	}
	if (key == SDLK_r) {
		_pf->randomize_edges();
		return true;
	}
	if (key == SDLK_SPACE) {
		update_grid_edges();
		update_grid_centers();
		/*for (auto & gmo : _gmos) {
			std::cout << gmo->_id << " vertices = ";
			for (auto & v : gmo->_vertices) {
				std::cout << v << " ; ";
			}
			std::cout << "\n";
		}*/
		return true;
	}

	if (_view_system->key_down(input_state, key, t)) {
		return true;
	}

	return false;
}


bool PathFinderTest::key_up(InputState * input_state, SDL_Keycode key, time_point t) {
	if (_view_system->key_up(input_state, key, t)) {
		return true;
	}

	return false;
}
