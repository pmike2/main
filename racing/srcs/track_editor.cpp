#include <fstream>
#include <sstream>
#include <iomanip>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "json.hpp"

#include "track_editor.h"


using json = nlohmann::json;


// GridEditor ------------------------------------------
GridEditor::GridEditor() {

}


GridEditor::GridEditor(GLuint prog_simple, ScreenGL * screengl, glm::vec4 tile_color) :
	_screengl(screengl), _row_idx_select(0), _col_idx_select(0), _tile_color(tile_color), _translation(pt_type(0.0)), _scale(1.0)
{
	_camera2clip= glm::ortho(float(-screengl->_gl_width)* 0.5f, float(screengl->_gl_width)* 0.5f, -float(screengl->_gl_height)* 0.5f, float(screengl->_gl_height)* 0.5f, Z_NEAR, Z_FAR);
	_world2camera= glm::mat4(1.0f);

	unsigned int n_buffers= 3;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	_contexts["grid"]= new DrawContext(prog_simple, _buffers[0],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["selection"]= new DrawContext(prog_simple, _buffers[1],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["tiles"]= new DrawContext(prog_simple, _buffers[2],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_grid= new StaticObjectGrid();
	update();
}


GridEditor::~GridEditor() {

}


void GridEditor::draw_grid() {
	DrawContext * context= _contexts["grid"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	glUniform1f(context->_locs_uniform["z"], -10.0f);
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void GridEditor::draw_selection() {
	DrawContext * context= _contexts["selection"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	glUniform1f(context->_locs_uniform["z"], -20.0f);
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void GridEditor::draw_tiles() {
	DrawContext * context= _contexts["tiles"];
	if (context->_n_pts== 0) {
		return;
	}

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	glUniform1f(context->_locs_uniform["z"], -30.0f);
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void GridEditor::draw() {
	draw_tiles();
	draw_selection();
	draw_grid();
}


void GridEditor::update_grid() {
	DrawContext * context= _contexts["grid"];
	context->_n_pts= 2* (_grid->_width+ 1)+ 2* (_grid->_height+ 1);
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;

	for (int i=0; i<_grid->_width+ 1; ++i) {
		//data[compt++]= float(i)* CELL_SIZE+ float(_grid->_origin.x);
		//data[compt++]= float(_grid->_origin.y);
		data[compt++]= float(i)* float(CELL_SIZE);
		data[compt++]= 0.0f;
		compt+= 4;
		//data[compt++]= float(i)* CELL_SIZE+ float(_grid->_origin.x);
		//data[compt++]= _grid->_height+ float(_grid->_origin.y);
		data[compt++]= float(i)* float(CELL_SIZE);
		data[compt++]= float(_grid->_height)* float(CELL_SIZE);
		compt+= 4;
	}
	for (int i=0; i<_grid->_height+ 1; ++i) {
		//data[compt++]= float(_grid->_origin.x);
		//data[compt++]= float(i)* CELL_SIZE+ float(_grid->_origin.y);
		data[compt++]= 0.0;
		data[compt++]= float(i)* float(CELL_SIZE);
		compt+= 4;
		//data[compt++]= _grid->_width+ float(_grid->_origin.x);
		//data[compt++]= float(i)* CELL_SIZE+ float(_grid->_origin.y);
		data[compt++]= float(_grid->_width)* float(CELL_SIZE);
		data[compt++]= float(i)* float(CELL_SIZE);
		compt+= 4;
	}

	for (unsigned int idx_pt=0; idx_pt<context->_n_pts; ++idx_pt) {
		for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
			data[idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= GRID_COLOR[idx_color];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void GridEditor::update_selection() {

	DrawContext * context= _contexts["selection"];
	context->_n_pts= 6;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	number positions[24]= {
		/*CELL_SIZE* number(_col_idx_select)+ _grid->_origin.x, CELL_SIZE* number(_row_idx_select)+ _grid->_origin.y,
		CELL_SIZE* number(_col_idx_select+ 1)+ _grid->_origin.x, CELL_SIZE* number(_row_idx_select)+ _grid->_origin.y,
		CELL_SIZE* number(_col_idx_select+ 1)+ _grid->_origin.x, CELL_SIZE* number(_row_idx_select+ 1)+ _grid->_origin.y,

		CELL_SIZE* number(_col_idx_select)+ _grid->_origin.x, CELL_SIZE* number(_row_idx_select)+ _grid->_origin.y,
		CELL_SIZE* number(_col_idx_select+ 1)+ _grid->_origin.x, CELL_SIZE* number(_row_idx_select+ 1)+ _grid->_origin.y,
		CELL_SIZE* number(_col_idx_select)+ _grid->_origin.x, CELL_SIZE* number(_row_idx_select+ 1)+ _grid->_origin.y,*/

		CELL_SIZE* number(_col_idx_select), CELL_SIZE* number(_row_idx_select),
		CELL_SIZE* number(_col_idx_select+ 1), CELL_SIZE* number(_row_idx_select),
		CELL_SIZE* number(_col_idx_select+ 1), CELL_SIZE* number(_row_idx_select+ 1),

		CELL_SIZE* number(_col_idx_select), CELL_SIZE* number(_row_idx_select),
		CELL_SIZE* number(_col_idx_select+ 1), CELL_SIZE* number(_row_idx_select+ 1),
		CELL_SIZE* number(_col_idx_select), CELL_SIZE* number(_row_idx_select+ 1)
	};

	for (unsigned int idx_pt=0; idx_pt<context->_n_pts; ++idx_pt) {
		data[idx_pt* context->_n_attrs_per_pts+ 0]= positions[2* idx_pt+ 0];
		data[idx_pt* context->_n_attrs_per_pts+ 1]= positions[2* idx_pt+ 1];
		for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
			data[idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= SELECTION_COLOR[idx_color];
		}
	}
	
	/*for (int i=0; i<context->_n_pts* context->_n_attrs_per_pts; ++i) {
		std::cout << data[i] << " ; ";
	}std::cout << "\n";*/


	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void GridEditor::update_tiles() {
	DrawContext * context= _contexts["tiles"];
	context->_n_pts= 0;
	context->_n_attrs_per_pts= 6;

	for (auto obj : _grid->_objects) {
		context->_n_pts+= 3* obj->_footprint->_triangles_idx.size();
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;
	for (auto obj : _grid->_objects) {
		for (auto tri : obj->_footprint->_triangles_idx) {
			for (int i=0; i<3; ++i) {
				pt_type pt= obj->_footprint->_pts[tri[i]];
				data[compt++]= float(pt.x);
				data[compt++]= float(pt.y);
				compt+= 4;
			}
		}
	}

	for (unsigned int idx_pt=0; idx_pt<context->_n_pts; ++idx_pt) {
		for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
			data[idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= _tile_color[idx_color];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void GridEditor::update() {
	update_grid();
	update_selection();
	update_tiles();

	_world2camera= glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(float(_translation.x), float(_translation.y), 0.0f)), glm::vec3(_scale, _scale, _scale));
}


bool GridEditor::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_DOWN) {
		_row_idx_select--;
		if (_row_idx_select< 0) {
			_row_idx_select= 0;
		}
		update_selection();
		return true;
	}
	else if (key== SDLK_UP) {
		_row_idx_select++;
		if (_row_idx_select> _grid->_height- 1) {
			_row_idx_select= _grid->_height- 1;
		}
		update_selection();
		return true;
	}
	else if (key== SDLK_LEFT) {
		_col_idx_select--;
		if (_col_idx_select< 0) {
			_col_idx_select= 0;
		}
		update_selection();
		return true;
	}
	else if (key== SDLK_RIGHT) {
		_col_idx_select++;
		if (_col_idx_select> _grid->_width- 1) {
			_col_idx_select= _grid->_width- 1;
		}
		update_selection();
		return true;
	}
	return false;
}


bool GridEditor::key_up(InputState * input_state, SDL_Keycode key) {
	return false;
}


pt_type GridEditor::screen2pt(int x, int y) {
	pt_type pos= _screengl->screen2gl(x, y);
	glm::vec4 v= glm::inverse(_world2camera)* glm::vec4(float(pos.x), float(pos.y), 0.0f, 1.0f);
	return pt_type(v.x, v.y);
}


bool GridEditor::mouse_button_down(InputState * input_state) {
	pt_type pos= screen2pt(input_state->_x, input_state->_y);
	std::pair<int, int> coord= _grid->number2coord(pos);
	if (coord.first>= 0) {
		_col_idx_select= coord.first;
		_row_idx_select= coord.second;
		update_selection();
		return true;
	}
	
	return false;
}



// TrackEditor -----------------------------------------
TrackEditor::TrackEditor() {

}


TrackEditor::TrackEditor(GLuint prog_simple, ScreenGL * screengl) :
	_row_idx_select(0), _col_idx_select(0), _screengl(screengl), _translation(pt_type(0.0)), _scale(1.0)
{
	_camera2clip= glm::ortho(float(-screengl->_gl_width)* 0.5f, float(screengl->_gl_width)* 0.5f, -float(screengl->_gl_height)* 0.5f, float(screengl->_gl_height)* 0.5f, Z_NEAR, Z_FAR);
	_world2camera= glm::mat4(1.0f);

	unsigned int n_buffers= 5;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	_contexts["grid"]= new DrawContext(prog_simple, _buffers[0],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["selection"]= new DrawContext(prog_simple, _buffers[1],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["tiles"]= new DrawContext(prog_simple, _buffers[2],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["floating_objects_footprint"]= new DrawContext(prog_simple, _buffers[3],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["floating_objects_bbox"]= new DrawContext(prog_simple, _buffers[4],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_track= new Track();

	update();
}


TrackEditor::~TrackEditor() {
	delete _track;
}


void TrackEditor::reinit() {
	_track->set_all("empty", 12, 10);
}


void TrackEditor::load_json(std::string json_path) {
	_track->load_json(json_path);
}


void TrackEditor::save_json(std::string json_path) {
	std::ofstream ofs(json_path);
	json js;

	js["width"]= _track->_grid->_width;
	js["height"]= _track->_grid->_height;
	js["tiles"]= json::array();
	for (auto tile : _track->_grid->_objects) {
		js["tiles"].push_back(basename(tile->_model->_json_path));
	}
	js["floating_objects"]= json::array();
	for (auto obj : _track->_floating_objects) {
		json js_obj;
		js_obj["name"]= basename(obj->_model->_json_path);
		js_obj["position"]= json::array();
		js_obj["position"].push_back(obj->_com.x);
		js_obj["position"].push_back(obj->_com.y);
		js_obj["alpha"]= obj->_alpha;
		js_obj["scale"]= json::array();
		js_obj["scale"].push_back(obj->_scale.x);
		js_obj["scale"].push_back(obj->_scale.y);
		js["floating_objects"].push_back(js_obj);
	}

	ofs << std::setw(4) << js << "\n";
}


void TrackEditor::draw_grid() {
	DrawContext * context= _contexts["grid"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	glUniform1f(context->_locs_uniform["z"], -10.0f);
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void TrackEditor::draw_selection() {
	DrawContext * context= _contexts["selection"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	glUniform1f(context->_locs_uniform["z"], -20.0f);
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void TrackEditor::draw_tiles() {
	DrawContext * context= _contexts["tiles"];
	if (context->_n_pts== 0) {
		return;
	}

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	glUniform1f(context->_locs_uniform["z"], -30.0f);
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void TrackEditor::draw_floating_objects_footprint() {
	DrawContext * context= _contexts["floating_objects_footprint"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	glUniform1f(context->_locs_uniform["z"], -30.0f);
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void TrackEditor::draw_floating_objects_bbox() {
	DrawContext * context= _contexts["floating_objects_bbox"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	glUniform1f(context->_locs_uniform["z"], -20.0f);
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void TrackEditor::draw() {
	draw_floating_objects_footprint();
	draw_floating_objects_bbox();
	draw_tiles();
	draw_selection();
	draw_grid();
}


void TrackEditor::update_grid() {
	DrawContext * context= _contexts["grid"];
	context->_n_pts= 2* (_track->_grid->_width+ 1)+ 2* (_track->_grid->_height+ 1);
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;

	for (int i=0; i<_track->_grid->_width+ 1; ++i) {
		//data[compt++]= float(i)* CELL_SIZE+ float(_grid->_origin.x);
		//data[compt++]= float(_grid->_origin.y);
		data[compt++]= float(i)* float(CELL_SIZE);
		data[compt++]= 0.0f;
		compt+= 4;
		//data[compt++]= float(i)* CELL_SIZE+ float(_grid->_origin.x);
		//data[compt++]= _grid->_height+ float(_grid->_origin.y);
		data[compt++]= float(i)* float(CELL_SIZE);
		data[compt++]= float(_track->_grid->_height)* float(CELL_SIZE);
		compt+= 4;
	}
	for (int i=0; i<_track->_grid->_height+ 1; ++i) {
		//data[compt++]= float(_grid->_origin.x);
		//data[compt++]= float(i)* CELL_SIZE+ float(_grid->_origin.y);
		data[compt++]= 0.0;
		data[compt++]= float(i)* float(CELL_SIZE);
		compt+= 4;
		//data[compt++]= _grid->_width+ float(_grid->_origin.x);
		//data[compt++]= float(i)* CELL_SIZE+ float(_grid->_origin.y);
		data[compt++]= float(_track->_grid->_width)* float(CELL_SIZE);
		data[compt++]= float(i)* float(CELL_SIZE);
		compt+= 4;
	}

	for (unsigned int idx_pt=0; idx_pt<context->_n_pts; ++idx_pt) {
		for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
			data[idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= GRID_COLOR[idx_color];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void TrackEditor::update_selection() {

	DrawContext * context= _contexts["selection"];
	context->_n_pts= 6;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	number positions[24]= {
		/*CELL_SIZE* number(_col_idx_select)+ _track->_grid->_origin.x, CELL_SIZE* number(_row_idx_select)+ _track->_grid->_origin.y,
		CELL_SIZE* number(_col_idx_select+ 1)+ _track->_grid->_origin.x, CELL_SIZE* number(_row_idx_select)+ _track->_grid->_origin.y,
		CELL_SIZE* number(_col_idx_select+ 1)+ _track->_grid->_origin.x, CELL_SIZE* number(_row_idx_select+ 1)+ _track->_grid->_origin.y,

		CELL_SIZE* number(_col_idx_select)+ _track->_grid->_origin.x, CELL_SIZE* number(_row_idx_select)+ _track->_grid->_origin.y,
		CELL_SIZE* number(_col_idx_select+ 1)+ _track->_grid->_origin.x, CELL_SIZE* number(_row_idx_select+ 1)+ _track->_grid->_origin.y,
		CELL_SIZE* number(_col_idx_select)+ _track->_grid->_origin.x, CELL_SIZE* number(_row_idx_select+ 1)+ _track->_grid->_origin.y,*/

		CELL_SIZE* number(_col_idx_select), CELL_SIZE* number(_row_idx_select),
		CELL_SIZE* number(_col_idx_select+ 1), CELL_SIZE* number(_row_idx_select),
		CELL_SIZE* number(_col_idx_select+ 1), CELL_SIZE* number(_row_idx_select+ 1),

		CELL_SIZE* number(_col_idx_select), CELL_SIZE* number(_row_idx_select),
		CELL_SIZE* number(_col_idx_select+ 1), CELL_SIZE* number(_row_idx_select+ 1),
		CELL_SIZE* number(_col_idx_select), CELL_SIZE* number(_row_idx_select+ 1),
	};

	for (unsigned int idx_pt=0; idx_pt<context->_n_pts; ++idx_pt) {
		data[idx_pt* context->_n_attrs_per_pts+ 0]= positions[2* idx_pt+ 0];
		data[idx_pt* context->_n_attrs_per_pts+ 1]= positions[2* idx_pt+ 1];
		for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
			data[idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= SELECTION_COLOR[idx_color];
		}
	}
	
	/*for (int i=0; i<context->_n_pts* context->_n_attrs_per_pts; ++i) {
		std::cout << data[i] << " ; ";
	}std::cout << "\n";*/


	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void TrackEditor::update_tiles() {
	DrawContext * context= _contexts["tiles"];
	context->_n_pts= 0;
	context->_n_attrs_per_pts= 6;

	for (auto obj : _track->_grid->_objects) {
		context->_n_pts+= 3* obj->_footprint->_triangles_idx.size();
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;
	for (auto obj : _track->_grid->_objects) {
		for (auto tri : obj->_footprint->_triangles_idx) {
			for (int i=0; i<3; ++i) {
				pt_type pt= obj->_footprint->_pts[tri[i]];
				data[compt++]= float(pt.x);
				data[compt++]= float(pt.y);
				compt+= 4;
			}
		}
	}
	for (unsigned int idx_pt=0; idx_pt<context->_n_pts; ++idx_pt) {
		for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
			data[idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= OBSTACLE_SETTING_COLOR[idx_color];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void TrackEditor::update_floating_objects_footprint() {
	DrawContext * context= _contexts["floating_objects_footprint"];
	context->_n_pts= 0;
	context->_n_attrs_per_pts= 6;

	for (auto obj : _track->_floating_objects) {
		context->_n_pts+= 3* obj->_footprint->_triangles_idx.size();
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;
	for (auto obj : _track->_floating_objects) {
		for (auto tri : obj->_footprint->_triangles_idx) {
			for (int i=0; i<3; ++i) {
				pt_type pt= obj->_footprint->_pts[tri[i]];
				data[compt++]= float(pt.x);
				data[compt++]= float(pt.y);
				compt+= 4;
			}
		}
	}
	for (unsigned int idx_pt=0; idx_pt<context->_n_pts; ++idx_pt) {
		for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
			data[idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= OBSTACLE_FLOATING_FOOTPRINT_COLOR[idx_color];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void TrackEditor::update_floating_objects_bbox() {
	DrawContext * context= _contexts["floating_objects_bbox"];
	context->_n_pts= 8* _track->_floating_objects.size();
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	float * ptr= data;
	for (auto obj : _track->_floating_objects) {
		std::vector<pt_type> pts(obj->_bbox->_pts, obj->_bbox->_pts+ 4);
		ptr= draw_polygon(ptr, pts, OBSTACLE_FLOATING_BBOX_COLOR);
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void TrackEditor::update() {
	update_grid();
	update_selection();
	update_tiles();
	update_floating_objects_footprint();
	update_floating_objects_bbox();

	_world2camera= glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(float(_translation.x), float(_translation.y), 0.0f)), glm::vec3(_scale, _scale, _scale));
}


bool TrackEditor::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_DOWN) {
		_row_idx_select--;
		if (_row_idx_select< 0) {
			_row_idx_select= 0;
		}
		update_selection();
		return true;
	}
	else if (key== SDLK_UP) {
		_row_idx_select++;
		if (_row_idx_select> _track->_grid->_height- 1) {
			_row_idx_select= _track->_grid->_height- 1;
		}
		update_selection();
		return true;
	}
	else if (key== SDLK_LEFT) {
		_col_idx_select--;
		if (_col_idx_select< 0) {
			_col_idx_select= 0;
		}
		update_selection();
		return true;
	}
	else if (key== SDLK_RIGHT) {
		_col_idx_select++;
		if (_col_idx_select> _track->_grid->_width- 1) {
			_col_idx_select= _track->_grid->_width- 1;
		}
		update_selection();
		return true;
	}
	return false;
}


bool TrackEditor::key_up(InputState * input_state, SDL_Keycode key) {
	return false;
}


pt_type TrackEditor::screen2pt(int x, int y) {
	pt_type pos= _screengl->screen2gl(x, y);
	glm::vec4 v= glm::inverse(_world2camera)* glm::vec4(float(pos.x), float(pos.y), 0.0f, 1.0f);
	return pt_type(v.x, v.y);
}


bool TrackEditor::mouse_button_down(InputState * input_state) {

	if (input_state->get_key(SDLK_d)) {
		pt_type pos= screen2pt(input_state->_x, input_state->_y);
		//pos-= _track->_grid->_origin;
		StaticObject * obj= _track->get_floating_object(pos);
		if (obj!= NULL) {
			_track->delete_floating_object(obj);
			update();
			return true;
		}
	}
	else if (input_state->get_key(SDLK_m) || input_state->get_key(SDLK_r) || input_state->get_key(SDLK_s)) {
		pt_type pos= screen2pt(input_state->_x, input_state->_y);
		//pos-= _track->_grid->_origin;
		StaticObject * obj= _track->get_floating_object(pos);
		if (obj!= NULL) {
			_selected_floating_object= obj;
			return true;
		}
	}
	else {
		pt_type pos= screen2pt(input_state->_x, input_state->_y);
		std::pair<int, int> coord= _track->_grid->number2coord(pos);
		if (coord.first>= 0) {
			_col_idx_select= coord.first;
			_row_idx_select= coord.second;
			update_selection();
			return true;
		}
	}
	return false;
}


bool TrackEditor::mouse_button_up(InputState * input_state) {
	_selected_floating_object= NULL;
	return true;
}


bool TrackEditor::mouse_motion(InputState * input_state) {
	if (input_state->get_key(SDLK_m) && _selected_floating_object!= NULL) {
		pt_type pos= screen2pt(input_state->_x, input_state->_y);
		//pos-= _track->_grid->_origin;
		_selected_floating_object->reinit(pos, _selected_floating_object->_alpha, _selected_floating_object->_scale);
		update();
		return true;
	}
	else if (input_state->get_key(SDLK_r) && _selected_floating_object!= NULL) {
		number alpha= _selected_floating_object->_alpha- 0.1* number(input_state->_yrel);
		_selected_floating_object->reinit(_selected_floating_object->_com, alpha, _selected_floating_object->_scale);
		update();
		return true;
	}
	else if (input_state->get_key(SDLK_s) && _selected_floating_object!= NULL) {
		pt_type scale= _selected_floating_object->_scale+ pt_type(0.05* number(input_state->_xrel), -0.05* number(input_state->_yrel));
		_selected_floating_object->reinit(_selected_floating_object->_com, _selected_floating_object->_alpha, scale);
		update();
		return true;
	}
	else if (input_state->get_key(SDLK_SPACE)) {
		_translation.x+= 0.02* number(input_state->_xrel);
		_translation.y-= 0.02* number(input_state->_yrel);
		update();
	}
	return false;
}


bool TrackEditor::mouse_wheel(InputState * input_state) {
	_scale+= 0.1* number(input_state->_y_wheel);
	update();
	return true;
}


// Editor -----------------------------------------------------
Editor::Editor() {

}


Editor::Editor(GLuint prog_simple, GLuint prog_font, ScreenGL * screengl) : _screengl(screengl) {
	_track_editor= new TrackEditor(prog_simple, screengl);
	_tile_grid_editor= new GridEditor(prog_simple, screengl, OBSTACLE_SETTING_COLOR);
	_floating_grid_editor= new GridEditor(prog_simple, screengl, OBSTACLE_FLOATING_FOOTPRINT_COLOR);

	/*_track_editor->_track->_grid->_origin= TRACK_ORIGIN;
	_tile_grid_editor->_grid->_origin= TILES_ORIGIN;
	_floating_grid_editor->_grid->_origin= FLOATING_OBJECTS_ORIGIN;*/
	_track_editor->_translation= TRACK_ORIGIN;
	_tile_grid_editor->_translation= TILES_ORIGIN;
	_floating_grid_editor->_translation= FLOATING_OBJECTS_ORIGIN;

	_tile_grid_editor->_grid->_width= 4;
	_tile_grid_editor->_grid->_type= VERTICAL_GRID;
	_floating_grid_editor->_grid->_height= 4;
	_floating_grid_editor->_grid->_type= HORIZONTAL_GRID;

	for (auto model : _track_editor->_track->_models) {
		if (model.second->_type== OBSTACLE_SETTING) {
			//std::cout << "loading OBSTACLE_SETTING : " << model.first << "\n";
			_tile_grid_editor->_grid->push_tile(model.second);
		}
		else if (model.second->_type== OBSTACLE_FLOATING || model.second->_type== HERO_CAR || model.second->_type== ENNEMY_CAR) {
			//std::cout << "loading OBSTACLE_FLOATING : " << model.first << "\n";
			_floating_grid_editor->_grid->push_tile(model.second);
		}
	}

	/*std::cout << "track_editor" << _track_editor->_track->_grid->_width << " ; " << _track_editor->_track->_grid->_height << "\n";
	std::cout << "tile_grid_editor=" << _tile_grid_editor->_grid->_width << " ; " << _tile_grid_editor->_grid->_height << "\n";
	std::cout << "floating_grid_editor=" << _floating_grid_editor->_grid->_width << " ; " << _floating_grid_editor->_grid->_height << "\n";*/

	_track_editor->update();
	_tile_grid_editor->update();
	_floating_grid_editor->update();
}


Editor::~Editor() {
	
}


void Editor::draw() {
	_track_editor->draw();
	_tile_grid_editor->draw();
	_floating_grid_editor->draw();
}


void Editor::sync_track_with_tile() {
	StaticObject * current_tile= _tile_grid_editor->_grid->get_tile(_tile_grid_editor->_col_idx_select, _tile_grid_editor->_row_idx_select);
	StaticObjectModel * model= current_tile->_model;
	_track_editor->_track->_grid->set_tile(model, _track_editor->_col_idx_select, _track_editor->_row_idx_select);
	_track_editor->update();
}


void Editor::add_floating_object(pt_type pos) {
	StaticObject * current_floating_object= _floating_grid_editor->_grid->get_tile(_floating_grid_editor->_col_idx_select, _floating_grid_editor->_row_idx_select);
	StaticObjectModel * model= current_floating_object->_model;
	//std::cout << pos.x << " ; " << pos.y << "\n";
	_track_editor->_track->_floating_objects.push_back(new StaticObject(model, pt_type(pos.x, pos.y), 0.0, pt_type(1.0)));
	_track_editor->update();
}


bool Editor::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_i) {
		_track_editor->reinit();
		_track_editor->update();
		return true;
	}
	else if (key== SDLK_l) {
		_track_editor->load_json("../data/tracks/track1.json");
		_track_editor->update();
		return true;
	}
	else if (key== SDLK_s) {
		_track_editor->save_json("../data/tracks/track_editor.json");
		return true;
	}
	else if (key== SDLK_DOWN || key== SDLK_UP || key== SDLK_LEFT || key== SDLK_RIGHT) {
		if (input_state->get_key(SDLK_LSHIFT)) {
			_tile_grid_editor->key_down(input_state, key);
		}
		else {
			_track_editor->key_down(input_state, key);
			if (input_state->get_key(SDLK_w)) {
				sync_track_with_tile();
			}
		}
		return true;
	}
	else if (key== SDLK_w) {
		sync_track_with_tile();
		return true;
	}
	return false;
}


bool Editor::key_up(InputState * input_state, SDL_Keycode key) {
	return false;
}


bool Editor::mouse_button_down(InputState * input_state) {
	if (input_state->get_key(SDLK_f)) {
		pt_type pos= _track_editor->screen2pt(input_state->_x, input_state->_y);
		//pos-= _track_editor->_track->_grid->_origin;
		if (pos.x>=0 && pos.x<number(_track_editor->_track->_grid->_width)* CELL_SIZE && pos.y>=0 && pos.y<number(_track_editor->_track->_grid->_height)* CELL_SIZE) {
			add_floating_object(pos);
			return true;
		}
	}

	if (_track_editor->mouse_button_down(input_state)) {
		return true;
	}
	else if (_tile_grid_editor->mouse_button_down(input_state)) {
		return true;
	}
	else if (_floating_grid_editor->mouse_button_down(input_state)) {
		return true;
	}
	return false;
}


bool Editor::mouse_button_up(InputState * input_state) {
	if (_track_editor->mouse_button_up(input_state)) {
		return true;
	}
	return false;
}


bool Editor::mouse_motion(InputState * input_state) {
	if (_track_editor->mouse_motion(input_state)) {
		return true;
	}
	return false;
}


bool Editor::mouse_wheel(InputState * input_state) {
	if (_track_editor->mouse_wheel(input_state)) {
		return true;
	}
	return false;
}
