#include <fstream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "json.hpp"

#include "track_editor.h"


using json = nlohmann::json;


// TilesPool -------------------------------------------
TilesPool::TilesPool() {
	std::vector<std::string> jsons= list_files("../data/tiles", "json");
	for (auto json_path : jsons) {
		std::string key= basename(json_path);
		_tiles[key]= new TrackTile(json_path);
		_tile_names.push_back(key);
	}
	_width= 1;
	_height= _tiles.size();
	_cell_size= 1.0;
}


TilesPool::~TilesPool() {

}


unsigned int TilesPool::coord2idx(unsigned int col_idx, unsigned int row_idx) {
	return col_idx+ row_idx* _width;
}


std::pair<unsigned int, unsigned int> TilesPool::idx2coord(unsigned int idx) {
	return std::make_pair(idx % _width, idx / _width);
}


TrackTile * TilesPool::get_tile_by_idx(unsigned int col_idx, unsigned int row_idx) {
	unsigned int idx_tile= coord2idx(col_idx, row_idx);
	if (idx_tile> _tile_names.size()- 1) {
		std::cerr << "TilesPool::get_tile_by_idx : " << idx_tile << " >= " << _tile_names.size()- 1 << "\n";
		return NULL;
	}
	return _tiles[_tile_names[idx_tile]];
}


// TrackEditor -----------------------------------------
TrackEditor::TrackEditor() {

}


TrackEditor::TrackEditor(GLuint prog_simple, GLuint prog_font, ScreenGL * screengl) :
	_row_idx_select(0), _col_idx_select(0), _row_idx_pool(0), _col_idx_pool(0)
{
	_camera2clip= glm::ortho(-screengl->_gl_width* 0.5f, screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f, screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
	_world2camera= glm::translate(glm::mat4(1.0f), glm::vec3(-screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f, 0.0f));
	_font= new Font(prog_font, "../../fonts/Silom.ttf", 48, screengl);

	unsigned int n_buffers= 4;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	_contexts["grid"]= new DrawContext(prog_simple, _buffers[0],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["selection"]= new DrawContext(prog_simple, _buffers[1],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["obstacle"]= new DrawContext(prog_simple, _buffers[2],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["pool"]= new DrawContext(prog_simple, _buffers[3],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_pool= new TilesPool();
	_track= new Track();
	reinit();
	update_obstacle();
	update_grid();
	update_selection();
	update_pool();
}


TrackEditor::~TrackEditor() {
	delete _track;
}


void TrackEditor::reinit() {
	_track->set_size(12, 10, 1.0);
	_track->set_all(_pool->_tiles["empty"]);
}


void TrackEditor::load_json(std::string json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	_track->set_size(js["width"], js["height"], js["cell_size"]);
	unsigned int compt= 0;
	for (auto tilename : js["tiles"]) {
		_track->set_tile(_pool->_tiles[tilename], compt);
		compt++;
	}
}


void TrackEditor::save_json(std::string json_path) {
	std::ofstream ofs(json_path);
	json js;

	js["width"]= _track->_width;
	js["height"]= _track->_height;
	js["cell_size"]= _track->_cell_size;
	js["tile"]= json::array();
	for (auto tile : _track->_tiles) {
		js["tile"].push_back(basename(tile->_json_path));
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


void TrackEditor::draw_obstacle() {
	DrawContext * context= _contexts["obstacle"];
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


void TrackEditor::draw_pool() {
	DrawContext * context= _contexts["pool"];

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


void TrackEditor::draw() {
	draw_obstacle();
	draw_pool();
	draw_selection();
	draw_grid();
}


void TrackEditor::update_grid() {
	DrawContext * context= _contexts["grid"];
	context->_n_pts= 2* (_track->_width+ 1)+ 2* (_track->_height+ 1);
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;
	for (int i=0; i<_track->_width+ 1; ++i) {
		data[compt++]= float(i)* _track->_cell_size+ float(GRID_OFFSET.x);
		data[compt++]= float(GRID_OFFSET.y);
		compt+= 4;
		data[compt++]= float(i)* _track->_cell_size+ float(GRID_OFFSET.x);
		data[compt++]= _track->_height+ float(GRID_OFFSET.y);
		compt+= 4;
	}
	for (int i=0; i<_track->_height+ 1; ++i) {
		data[compt++]= float(GRID_OFFSET.x);
		data[compt++]= float(i)* _track->_cell_size+ float(GRID_OFFSET.y);
		compt+= 4;
		data[compt++]= _track->_width+ float(GRID_OFFSET.x);
		data[compt++]= float(i)* _track->_cell_size+ float(GRID_OFFSET.y);
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
	context->_n_pts= 12;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	number positions[24]= {
		_track->_cell_size* number(_col_idx_select)+ GRID_OFFSET.x, _track->_cell_size* number(_row_idx_select)+ GRID_OFFSET.y,
		_track->_cell_size* number(_col_idx_select+ 1)+ GRID_OFFSET.x, _track->_cell_size* number(_row_idx_select)+ GRID_OFFSET.y,
		_track->_cell_size* number(_col_idx_select+ 1)+ GRID_OFFSET.x, _track->_cell_size* number(_row_idx_select+ 1)+ GRID_OFFSET.y,

		_track->_cell_size* number(_col_idx_select)+ GRID_OFFSET.x, _track->_cell_size* number(_row_idx_select)+ GRID_OFFSET.y,
		_track->_cell_size* number(_col_idx_select+ 1)+ GRID_OFFSET.x, _track->_cell_size* number(_row_idx_select+ 1)+ GRID_OFFSET.y,
		_track->_cell_size* number(_col_idx_select)+ GRID_OFFSET.x, _track->_cell_size* number(_row_idx_select+ 1)+ GRID_OFFSET.y,

		_pool->_cell_size* number(_col_idx_pool)+ POOL_OFFSET.x, _pool->_cell_size* number(_row_idx_pool)+ POOL_OFFSET.y,
		_pool->_cell_size* number(_col_idx_pool+ 1)+ POOL_OFFSET.x, _pool->_cell_size* number(_row_idx_pool)+ POOL_OFFSET.y,
		_pool->_cell_size* number(_col_idx_pool+ 1)+ POOL_OFFSET.x, _pool->_cell_size* number(_row_idx_pool+ 1)+ POOL_OFFSET.y,

		_pool->_cell_size* number(_col_idx_pool)+ POOL_OFFSET.x, _pool->_cell_size* number(_row_idx_pool)+ POOL_OFFSET.y,
		_pool->_cell_size* number(_col_idx_pool+ 1)+ POOL_OFFSET.x, _pool->_cell_size* number(_row_idx_pool+ 1)+ POOL_OFFSET.y,
		_pool->_cell_size* number(_col_idx_pool)+ POOL_OFFSET.x, _pool->_cell_size* number(_row_idx_pool+ 1)+ POOL_OFFSET.y,
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


void TrackEditor::update_obstacle() {
	DrawContext * context= _contexts["obstacle"];
	context->_n_pts= 0;
	context->_n_attrs_per_pts= 6;

	for (auto tile : _track->_tiles) {
		for (auto polygon : tile->_obstacles) {
			context->_n_pts+= polygon->_pts.size();
		}
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;
	for (auto tile : _track->_tiles) {
		for (auto polygon : tile->_obstacles) {
			for (auto pt : polygon->_pts) {
				data[compt++]= float(pt.x)+ float(GRID_OFFSET.x);
				data[compt++]= float(pt.y)+ float(GRID_OFFSET.y);
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


void TrackEditor::update_pool() {
	DrawContext * context= _contexts["pool"];
	context->_n_pts= 0;
	context->_n_attrs_per_pts= 6;

	for (auto tile : _pool->_tiles) {
		for (auto polygon : tile.second->_obstacles) {
			context->_n_pts+= polygon->_pts.size();
		}
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;
	unsigned int idx_tile= 0;
	for (auto tile : _pool->_tiles) {
		for (auto polygon : tile.second->_obstacles) {
			for (auto pt : polygon->_pts) {
				data[compt++]= float(pt.x)+ float(POOL_OFFSET.x);
				data[compt++]= float(pt.y)+ float(POOL_OFFSET.y)+ float(idx_tile)* float(_track->_cell_size);
				compt+= 4;
			}
		}
		idx_tile++;
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


void TrackEditor::randomize() {

}


bool TrackEditor::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_i) {
		reinit();
		update_obstacle();
		update_grid();
		update_selection();
		return true;
	}
	else if (key== SDLK_l) {
		load_json("../data/tracks/track1.json");
		_row_idx_select= _col_idx_select= 0;
		update_obstacle();
		update_grid();
		update_selection();
		return true;
	}
	else if (key== SDLK_s) {
		save_json("../data/tracks/track_editor.json");
		return true;
	}
	else if (key== SDLK_DOWN) {
		if (input_state->get_key(SDLK_LSHIFT)) {
			_row_idx_pool--;
			if (_row_idx_pool< 0) {
				_row_idx_pool= 0;
			}
		}
		else {
			_row_idx_select--;
			if (_row_idx_select< 0) {
				_row_idx_select= 0;
			}
			if (input_state->get_key(SDLK_w)) {
				_track->set_tile(_pool->get_tile_by_idx(_col_idx_pool, _row_idx_pool), _col_idx_select, _row_idx_select);
				update_obstacle();
			}
		}
		update_selection();
		return true;
	}
	else if (key== SDLK_UP) {
		if (input_state->get_key(SDLK_LSHIFT)) {
			_row_idx_pool++;
			if (_row_idx_pool> _pool->_height- 1) {
				_row_idx_pool= _pool->_height- 1;
			}
		}
		else {
			_row_idx_select++;
			if (_row_idx_select> _track->_height- 1) {
				_row_idx_select= _track->_height- 1;
			}
			if (input_state->get_key(SDLK_w)) {
				_track->set_tile(_pool->get_tile_by_idx(_col_idx_pool, _row_idx_pool), _col_idx_select, _row_idx_select);
				update_obstacle();
			}
		}
		update_selection();
		return true;
	}
	else if (key== SDLK_LEFT) {
		if (input_state->get_key(SDLK_LSHIFT)) {
			_col_idx_pool--;
			if (_col_idx_pool< 0) {
				_col_idx_pool= 0;
			}
		}
		else {
			_col_idx_select--;
			if (_col_idx_select< 0) {
				_col_idx_select= 0;
			}
			if (input_state->get_key(SDLK_w)) {
				_track->set_tile(_pool->get_tile_by_idx(_col_idx_pool, _row_idx_pool), _col_idx_select, _row_idx_select);
				update_obstacle();
			}
		}
		update_selection();
		return true;
	}
	else if (key== SDLK_RIGHT) {
		if (input_state->get_key(SDLK_LSHIFT)) {
			_col_idx_pool++;
			if (_col_idx_pool> _pool->_width- 1) {
				_col_idx_pool= _pool->_width- 1;
			}
		}
		else {
			_col_idx_select++;
			if (_col_idx_select> _track->_width- 1) {
				_col_idx_select= _track->_width- 1;
			}
			if (input_state->get_key(SDLK_w)) {
				_track->set_tile(_pool->get_tile_by_idx(_col_idx_pool, _row_idx_pool), _col_idx_select, _row_idx_select);
				update_obstacle();
			}
		}
		update_selection();
		return true;
	}
	else if (key== SDLK_w) {
		_track->set_tile(_pool->get_tile_by_idx(_col_idx_pool, _row_idx_pool), _col_idx_select, _row_idx_select);
		update_obstacle();
		//std::cout << *_track << "\n";
		return true;
	}
	return false;
}


bool TrackEditor::key_up(InputState * input_state, SDL_Keycode key) {
	return false;
}
