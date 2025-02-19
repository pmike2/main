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


GridEditor::GridEditor(GLuint prog_simple, ScreenGL * screengl) : _screengl(screengl), _row_idx_select(0), _col_idx_select(0) {
	_camera2clip= glm::ortho(float(-screengl->_gl_width)* 0.5f, float(screengl->_gl_width)* 0.5f, -float(screengl->_gl_height)* 0.5f, float(screengl->_gl_height)* 0.5f, Z_NEAR, Z_FAR);

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
	//glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
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
	//glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
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
	//glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
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
		data[compt++]= float(i)* CELL_SIZE+ float(_grid->_origin.x);
		data[compt++]= float(_grid->_origin.y);
		compt+= 4;
		data[compt++]= float(i)* CELL_SIZE+ float(_grid->_origin.x);
		data[compt++]= _grid->_height+ float(_grid->_origin.y);
		compt+= 4;
	}
	for (int i=0; i<_grid->_height+ 1; ++i) {
		data[compt++]= float(_grid->_origin.x);
		data[compt++]= float(i)* CELL_SIZE+ float(_grid->_origin.y);
		compt+= 4;
		data[compt++]= _grid->_width+ float(_grid->_origin.x);
		data[compt++]= float(i)* CELL_SIZE+ float(_grid->_origin.y);
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
		CELL_SIZE* number(_col_idx_select)+ _grid->_origin.x, CELL_SIZE* number(_row_idx_select)+ _grid->_origin.y,
		CELL_SIZE* number(_col_idx_select+ 1)+ _grid->_origin.x, CELL_SIZE* number(_row_idx_select)+ _grid->_origin.y,
		CELL_SIZE* number(_col_idx_select+ 1)+ _grid->_origin.x, CELL_SIZE* number(_row_idx_select+ 1)+ _grid->_origin.y,

		CELL_SIZE* number(_col_idx_select)+ _grid->_origin.x, CELL_SIZE* number(_row_idx_select)+ _grid->_origin.y,
		CELL_SIZE* number(_col_idx_select+ 1)+ _grid->_origin.x, CELL_SIZE* number(_row_idx_select+ 1)+ _grid->_origin.y,
		CELL_SIZE* number(_col_idx_select)+ _grid->_origin.x, CELL_SIZE* number(_row_idx_select+ 1)+ _grid->_origin.y,
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
				data[compt++]= float(pt.x)+ float(_grid->_origin.x);
				data[compt++]= float(pt.y)+ float(_grid->_origin.y);
				compt+= 4;
			}
		}
	}

	for (unsigned int idx_pt=0; idx_pt<context->_n_pts; ++idx_pt) {
		for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
			data[idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= OBSTACLE_COLOR[idx_color];
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


bool GridEditor::mouse_button_down(InputState * input_state) {
	number x, y;
	_screengl->screen2gl(input_state->_x, input_state->_y, x, y);
	std::pair<int, int> coord= _grid->number2coord(x, y);
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
	_row_idx_select(0), _col_idx_select(0), _screengl(screengl)
{
	_camera2clip= glm::ortho(float(-screengl->_gl_width)* 0.5f, float(screengl->_gl_width)* 0.5f, -float(screengl->_gl_height)* 0.5f, float(screengl->_gl_height)* 0.5f, Z_NEAR, Z_FAR);

	unsigned int n_buffers= 4;
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

	_contexts["floating_objects"]= new DrawContext(prog_simple, _buffers[3],
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

	ofs << std::setw(4) << js << "\n";
}


void TrackEditor::draw_grid() {
	DrawContext * context= _contexts["grid"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	//glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
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
	//glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
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
	//glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
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


void TrackEditor::draw_floating_objects() {
	DrawContext * context= _contexts["floating_objects"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	//glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
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


void TrackEditor::draw() {
	draw_floating_objects();
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
		data[compt++]= float(i)* CELL_SIZE+ float(_track->_grid->_origin.x);
		data[compt++]= float(_track->_grid->_origin.y);
		compt+= 4;
		data[compt++]= float(i)* CELL_SIZE+ float(_track->_grid->_origin.x);
		data[compt++]= _track->_grid->_height+ float(_track->_grid->_origin.y);
		compt+= 4;
	}
	for (int i=0; i<_track->_grid->_height+ 1; ++i) {
		data[compt++]= float(_track->_grid->_origin.x);
		data[compt++]= float(i)* CELL_SIZE+ float(_track->_grid->_origin.y);
		compt+= 4;
		data[compt++]= _track->_grid->_width+ float(_track->_grid->_origin.x);
		data[compt++]= float(i)* CELL_SIZE+ float(_track->_grid->_origin.y);
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
		CELL_SIZE* number(_col_idx_select)+ _track->_grid->_origin.x, CELL_SIZE* number(_row_idx_select)+ _track->_grid->_origin.y,
		CELL_SIZE* number(_col_idx_select+ 1)+ _track->_grid->_origin.x, CELL_SIZE* number(_row_idx_select)+ _track->_grid->_origin.y,
		CELL_SIZE* number(_col_idx_select+ 1)+ _track->_grid->_origin.x, CELL_SIZE* number(_row_idx_select+ 1)+ _track->_grid->_origin.y,

		CELL_SIZE* number(_col_idx_select)+ _track->_grid->_origin.x, CELL_SIZE* number(_row_idx_select)+ _track->_grid->_origin.y,
		CELL_SIZE* number(_col_idx_select+ 1)+ _track->_grid->_origin.x, CELL_SIZE* number(_row_idx_select+ 1)+ _track->_grid->_origin.y,
		CELL_SIZE* number(_col_idx_select)+ _track->_grid->_origin.x, CELL_SIZE* number(_row_idx_select+ 1)+ _track->_grid->_origin.y,
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
				data[compt++]= float(pt.x)+ float(_track->_grid->_origin.x);
				data[compt++]= float(pt.y)+ float(_track->_grid->_origin.y);
				compt+= 4;
			}
		}
	}
	for (unsigned int idx_pt=0; idx_pt<context->_n_pts; ++idx_pt) {
		for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
			data[idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= OBSTACLE_COLOR[idx_color];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void TrackEditor::update_floating_objects() {
	DrawContext * context= _contexts["floating_objects"];
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
				data[compt++]= float(pt.x)+ float(_track->_grid->_origin.x);
				data[compt++]= float(pt.y)+ float(_track->_grid->_origin.y);
				compt+= 4;
			}
		}
	}
	for (unsigned int idx_pt=0; idx_pt<context->_n_pts; ++idx_pt) {
		for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
			data[idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= OBSTACLE_COLOR[idx_color];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void TrackEditor::update() {
	update_grid();
	update_selection();
	update_tiles();
	update_floating_objects();
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


bool TrackEditor::mouse_button_down(InputState * input_state) {
	number x, y;
	_screengl->screen2gl(input_state->_x, input_state->_y, x, y);
	std::pair<int, int> coord= _track->_grid->number2coord(x, y);
	if (coord.first>= 0) {
		_col_idx_select= coord.first;
		_row_idx_select= coord.second;
		update_selection();
		return true;
	}
	return false;
}


// Editor -----------------------------------------------------
Editor::Editor() {

}


Editor::Editor(GLuint prog_simple, GLuint prog_font, ScreenGL * screengl) {
	_track_editor= new TrackEditor(prog_simple, screengl);
	_tile_grid_editor= new GridEditor(prog_simple, screengl);
	_floating_grid_editor= new GridEditor(prog_simple, screengl);

	_track_editor->_track->_grid->_origin= TRACK_ORIGIN;
	_tile_grid_editor->_grid->_origin= TILES_ORIGIN;
	_floating_grid_editor->_grid->_origin= FLOATING_OBJECTS_ORIGIN;

	_tile_grid_editor->_grid->_width= 3;
	_tile_grid_editor->_grid->_type= VERTICAL_GRID;
	_floating_grid_editor->_grid->_height= 4;
	_floating_grid_editor->_grid->_type= HORIZONTAL_GRID;

	for (auto model : _track_editor->_track->_models) {
		if (model.second->_type== OBSTACLE_SETTING) {
			std::cout << "loading OBSTACLE_SETTING : " << model.first << "\n";
			_tile_grid_editor->_grid->push_tile(model.second);
		}
		else if (model.second->_type== OBSTACLE_FLOATING) {
			std::cout << "loading OBSTACLE_FLOATING : " << model.first << "\n";
			_floating_grid_editor->_grid->push_tile(model.second);
		}
	}

	std::cout << "track_editor" << _track_editor->_track->_grid->_width << " ; " << _track_editor->_track->_grid->_height << "\n";
	std::cout << "tile_grid_editor=" << _tile_grid_editor->_grid->_width << " ; " << _tile_grid_editor->_grid->_height << "\n";
	std::cout << "floating_grid_editor=" << _floating_grid_editor->_grid->_width << " ; " << _floating_grid_editor->_grid->_height << "\n";

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

