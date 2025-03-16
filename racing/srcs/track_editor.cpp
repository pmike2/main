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


GridEditor::GridEditor(GLuint prog_simple, GLuint prog_texture, GLuint prog_font, ScreenGL * screengl, glm::vec4 tile_color, number cell_size, GridType type) :
	_draw_bbox(false), _draw_texture(true),
	_screengl(screengl), _row_idx_select(0), _col_idx_select(0), _tile_color(tile_color), _translation(pt_type(0.0)), _scale(1.0)
{
	_camera2clip= glm::ortho(float(-screengl->_gl_width)* 0.5f, float(screengl->_gl_width)* 0.5f, -float(screengl->_gl_height)* 0.5f, float(screengl->_gl_height)* 0.5f, Z_NEAR, Z_FAR);
	_world2camera= glm::mat4(1.0f);
	_font= new Font(prog_font, "../../fonts/Silom.ttf", 48, screengl);

	unsigned int n_buffers= 4;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	unsigned int n_textures= 1;
	_textures= new GLuint(n_textures);
	glGenTextures(n_textures, _textures);

	_contexts["grid"]= new DrawContext(prog_simple, _buffers[0],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["selection"]= new DrawContext(prog_simple, _buffers[1],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["tiles"]= new DrawContext(prog_simple, _buffers[2],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["texture"]= new DrawContext(prog_texture, _buffers[3],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "texture_array"});

	_grid= new StaticObjectGrid(cell_size, type);

	//fill_texture_array();
	//update();
}


GridEditor::~GridEditor() {

}


void GridEditor::fill_texture_array() {
	const unsigned int TEXTURE_SIZE= 1024;
	unsigned int n_tex= _grid->_objects.size();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textures[0]);
	
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, TEXTURE_SIZE, TEXTURE_SIZE, n_tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	unsigned int compt= 0;
	for (auto obj : _grid->_objects) {
		//_model_tex_idxs[obj->_model->_name]= compt;
		obj->_model->_texture_idx= float(compt);
		std::string png_abs= dirname(obj->_model->_json_path)+ "/textures/"+ obj->_model->_name+ ".png";
		//std::cout << "png=" << png_abs << " ; compt=" << compt << "\n";
		if (!file_exists(png_abs)) {
			std::cout << "png=" << png_abs << " n'existe pas\n";
			continue;
		}
		SDL_Surface * surface= IMG_Load(png_abs.c_str());
		if (!surface) {
			std::cout << "IMG_Load error :" << IMG_GetError() << "\n";
			return;
		}

		// sais pas pourquoi mais GL_BGRA fonctionne mieux que GL_RGBA
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
						0,                             // mipmap number
						0, 0, compt,   // xoffset, yoffset, zoffset
						TEXTURE_SIZE, TEXTURE_SIZE, 1, // width, height, depth
						GL_BGRA,                       // format
						GL_UNSIGNED_BYTE,              // type
						surface->pixels);              // pointer to data

		SDL_FreeSurface(surface);

		compt++;
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);

	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
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


void GridEditor::draw_texture() {
	DrawContext * context= _contexts["texture"];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textures[0]);
	glActiveTexture(0);

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);

	glUniform1i(context->_locs_uniform["texture_array"], 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	//glUniform1f(context->_locs_uniform["z"], -25.0f);
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["tex_coord_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["current_layer_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(5* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void GridEditor::draw() {
	if (_draw_bbox) {
		draw_tiles();
	}
	draw_selection();
	draw_grid();
	if (_draw_texture) {
		draw_texture();
	}
}


void GridEditor::show_info() {
	const float font_scale= 0.002f;
	const glm::vec4 text_color(1.0, 1.0, 1.0, 0.8);

	std::vector<Text> texts;

	for (auto obj : _grid->_objects) {
		glm::vec4 v= _world2camera* glm::vec4(obj->_com.x, obj->_com.y, 0.0, 1.0);
		texts.push_back(Text(basename(obj->_model->_json_path), pt_type(v.x, v.y), font_scale, glm::vec4(1.0, 1.0, 1.0, 1.0)));
	}

	_font->set_text(texts);
	_font->draw();
}


void GridEditor::update_grid() {
	DrawContext * context= _contexts["grid"];
	context->_n_pts= 2* (_grid->_width+ 1)+ 2* (_grid->_height+ 1);
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;

	for (int i=0; i<_grid->_width+ 1; ++i) {
		data[compt++]= float(i)* float(_grid->_cell_size);
		data[compt++]= 0.0f;
		compt+= 4;
		data[compt++]= float(i)* float(_grid->_cell_size);
		data[compt++]= float(_grid->_height)* float(_grid->_cell_size);
		compt+= 4;
	}
	for (int i=0; i<_grid->_height+ 1; ++i) {
		data[compt++]= 0.0;
		data[compt++]= float(i)* float(_grid->_cell_size);
		compt+= 4;
		data[compt++]= float(_grid->_width)* float(_grid->_cell_size);
		data[compt++]= float(i)* float(_grid->_cell_size);
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
		_grid->_cell_size* number(_col_idx_select), _grid->_cell_size* number(_row_idx_select),
		_grid->_cell_size* number(_col_idx_select+ 1), _grid->_cell_size* number(_row_idx_select),
		_grid->_cell_size* number(_col_idx_select+ 1), _grid->_cell_size* number(_row_idx_select+ 1),

		_grid->_cell_size* number(_col_idx_select), _grid->_cell_size* number(_row_idx_select),
		_grid->_cell_size* number(_col_idx_select+ 1), _grid->_cell_size* number(_row_idx_select+ 1),
		_grid->_cell_size* number(_col_idx_select), _grid->_cell_size* number(_row_idx_select+ 1)
	};

	for (unsigned int idx_pt=0; idx_pt<context->_n_pts; ++idx_pt) {
		data[idx_pt* context->_n_attrs_per_pts+ 0]= positions[2* idx_pt+ 0];
		data[idx_pt* context->_n_attrs_per_pts+ 1]= positions[2* idx_pt+ 1];
		for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
			data[idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= SELECTION_COLOR[idx_color];
		}
	}


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


void GridEditor::update_texture() {
	const unsigned int n_pts_per_obj= 6;

	DrawContext * context= _contexts["texture"];
	context->_n_pts= n_pts_per_obj* _grid->_objects.size();
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	for (unsigned int idx_obj=0; idx_obj<_grid->_objects.size(); ++idx_obj) {
		StaticObject * obj;
		obj= _grid->_objects[idx_obj];
		
		// à cause du système de reference opengl il faut inverser les 0 et les 1 des y des textures
		number positions[n_pts_per_obj* 5]= {
			obj->_bbox->_pts[0].x, obj->_bbox->_pts[0].y, obj->_z, 0.0, 1.0,
			obj->_bbox->_pts[1].x, obj->_bbox->_pts[1].y, obj->_z, 1.0, 1.0,
			obj->_bbox->_pts[2].x, obj->_bbox->_pts[2].y, obj->_z, 1.0, 0.0,

			obj->_bbox->_pts[0].x, obj->_bbox->_pts[0].y, obj->_z, 0.0, 1.0,
			obj->_bbox->_pts[2].x, obj->_bbox->_pts[2].y, obj->_z, 1.0, 0.0,
			obj->_bbox->_pts[3].x, obj->_bbox->_pts[3].y, obj->_z, 0.0, 0.0
		};
		
		for (unsigned int idx_pt=0; idx_pt<n_pts_per_obj; ++idx_pt) {
			for (unsigned int i=0; i<5; ++i) {
				data[idx_obj* n_pts_per_obj* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ i]= float(positions[5* idx_pt+ i]);
			}
			//data[idx_obj* n_pts_per_obj* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 5]= float(_model_tex_idxs[basename(obj->_model->_json_path)]);
			data[idx_obj* n_pts_per_obj* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 5]= obj->_model->_texture_idx;
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
	update_texture();

	_world2camera= glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(float(_translation.x), float(_translation.y), 0.0f)), glm::vec3(_scale, _scale, _scale));
}


bool GridEditor::key_down(InputState * input_state, SDL_Keycode key) {
	// touches directionnelles == chgmt tuile sélectionnée
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

	else if (key== SDLK_b) {
		_draw_bbox= !_draw_bbox;
	}
	else if (key== SDLK_t) {
		_draw_texture= !_draw_texture;
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


TrackEditor::TrackEditor(GLuint prog_simple, GLuint prog_texture, GLuint prog_font, ScreenGL * screengl, number cell_size) :
	_draw_bbox(false), _draw_texture(true),
	_row_idx_select(0), _col_idx_select(0), _screengl(screengl), _translation(pt_type(0.0)), _scale(1.0), _last_checkpoint(NULL),
	_selected_floating_object(NULL)
{
	_camera2clip= glm::ortho(float(-screengl->_gl_width)* 0.5f, float(screengl->_gl_width)* 0.5f, -float(screengl->_gl_height)* 0.5f, float(screengl->_gl_height)* 0.5f, Z_NEAR, Z_FAR);
	_world2camera= glm::mat4(1.0f);
	_font= new Font(prog_font, "../../fonts/Silom.ttf", 48, screengl);

	unsigned int n_buffers= 6;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	unsigned int n_textures= 1;
	_textures= new GLuint(n_textures);
	glGenTextures(n_textures, _textures);

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

	_contexts["texture"]= new DrawContext(prog_texture, _buffers[5],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "texture_array"});

	_track= new Track();
	_track->set_all("full_obst", TRACK_DEFAULT_SIZE, TRACK_DEFAULT_SIZE);
	_track->_n_laps= DEFAULT_N_LAPS;

	fill_texture_array();

	update();
}


TrackEditor::~TrackEditor() {
	delete _track;
}


void TrackEditor::reinit() {
	_track->set_all("full_obst", TRACK_DEFAULT_SIZE, TRACK_DEFAULT_SIZE);
}


void TrackEditor::load_json(std::string json_path) {
	_track->load_json(json_path);
}


void TrackEditor::save_json(std::string json_path) {
	std::ofstream ofs(json_path);
	json js;

	js["width"]= _track->_grid->_width;
	js["height"]= _track->_grid->_height;
	js["cell_size"]= _track->_grid->_cell_size;

	js["n_laps"]= _track->_n_laps;

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

		if (obj->_model->_type== CHECKPOINT || obj->_model->_type== START) {
			CheckPoint * checkpoint= (CheckPoint *)(obj);
			js_obj["idx_checkpoint"]= _track->get_checkpoint_index(checkpoint);
		}
		
		js["floating_objects"].push_back(js_obj);
	}

	ofs << std::setw(4) << js << "\n";
}


void TrackEditor::fill_texture_array() {
	const unsigned int TEXTURE_SIZE= 1024;
	unsigned int n_tex= 0;
	for (auto model : _track->_models) {
		n_tex++;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textures[0]);
	
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, TEXTURE_SIZE, TEXTURE_SIZE, n_tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	unsigned int compt= 0;
	for (auto model : _track->_models) {
		//_model_tex_idxs[model.first]= compt;
		model.second->_texture_idx= float(compt);
		std::string png_abs= dirname(model.second->_json_path)+ "/textures/"+ model.first+ ".png";
		//std::cout << "png=" << png_abs << " ; compt=" << compt << "\n";
		if (!file_exists(png_abs)) {
			std::cout << "png=" << png_abs << " n'existe pas\n";
			continue;
		}
		SDL_Surface * surface= IMG_Load(png_abs.c_str());
		if (!surface) {
			std::cout << "IMG_Load error :" << IMG_GetError() << "\n";
			return;
		}

		// sais pas pourquoi mais GL_BGRA fonctionne mieux que GL_RGBA
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
						0,                             // mipmap number
						0, 0, compt,   // xoffset, yoffset, zoffset
						TEXTURE_SIZE, TEXTURE_SIZE, 1, // width, height, depth
						GL_BGRA,                       // format
						GL_UNSIGNED_BYTE,              // type
						surface->pixels);              // pointer to data

		SDL_FreeSurface(surface);

		compt++;
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);

	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
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


void TrackEditor::draw_texture() {
	DrawContext * context= _contexts["texture"];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textures[0]);
	glActiveTexture(0);

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);

	glUniform1i(context->_locs_uniform["texture_array"], 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	//glUniform1f(context->_locs_uniform["z"], -25.0f);
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["tex_coord_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["current_layer_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(5* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void TrackEditor::draw() {
	if (_draw_bbox) {
		draw_floating_objects_footprint();
		draw_floating_objects_bbox();
		draw_tiles();
	}
	draw_selection();
	draw_grid();
	if (_draw_texture) {
		draw_texture();
	}
}


void TrackEditor::show_info() {
	const float font_scale= 0.004f;
	const glm::vec4 text_color(1.0, 1.0, 1.0, 0.8);

	std::vector<Text> texts;

	for (auto obj : _track->_floating_objects) {
		glm::vec4 v= _world2camera* glm::vec4(obj->_com.x, obj->_com.y, 0.0, 1.0);
		std::string s= basename(obj->_model->_json_path);
		if (obj->_model->_type== CHECKPOINT) {
			CheckPoint * checkpoint= (CheckPoint *)(obj);
			s+= std::to_string(_track->get_checkpoint_index(checkpoint));
		}
		texts.push_back(Text(s, pt_type(v.x, v.y), font_scale, glm::vec4(1.0, 1.0, 1.0, 1.0)));
	}

	_font->set_text(texts);
	_font->draw();
}


void TrackEditor::update_grid() {
	DrawContext * context= _contexts["grid"];
	context->_n_pts= 2* (_track->_grid->_width+ 1)+ 2* (_track->_grid->_height+ 1);
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;

	for (int i=0; i<_track->_grid->_width+ 1; ++i) {
		data[compt++]= float(i)* float(_track->_grid->_cell_size);
		data[compt++]= 0.0f;
		compt+= 4;
		data[compt++]= float(i)* float(_track->_grid->_cell_size);
		data[compt++]= float(_track->_grid->_height)* float(_track->_grid->_cell_size);
		compt+= 4;
	}
	for (int i=0; i<_track->_grid->_height+ 1; ++i) {
		data[compt++]= 0.0;
		data[compt++]= float(i)* float(_track->_grid->_cell_size);
		compt+= 4;
		data[compt++]= float(_track->_grid->_width)* float(_track->_grid->_cell_size);
		data[compt++]= float(i)* float(_track->_grid->_cell_size);
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
		_track->_grid->_cell_size* number(_col_idx_select), _track->_grid->_cell_size* number(_row_idx_select),
		_track->_grid->_cell_size* number(_col_idx_select+ 1), _track->_grid->_cell_size* number(_row_idx_select),
		_track->_grid->_cell_size* number(_col_idx_select+ 1), _track->_grid->_cell_size* number(_row_idx_select+ 1),

		_track->_grid->_cell_size* number(_col_idx_select), _track->_grid->_cell_size* number(_row_idx_select),
		_track->_grid->_cell_size* number(_col_idx_select+ 1), _track->_grid->_cell_size* number(_row_idx_select+ 1),
		_track->_grid->_cell_size* number(_col_idx_select), _track->_grid->_cell_size* number(_row_idx_select+ 1),
	};

	for (unsigned int idx_pt=0; idx_pt<context->_n_pts; ++idx_pt) {
		data[idx_pt* context->_n_attrs_per_pts+ 0]= positions[2* idx_pt+ 0];
		data[idx_pt* context->_n_attrs_per_pts+ 1]= positions[2* idx_pt+ 1];
		for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
			data[idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= SELECTION_COLOR[idx_color];
		}
	}

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


void TrackEditor::update_texture() {
	const unsigned int n_pts_per_obj= 6;

	DrawContext * context= _contexts["texture"];
	context->_n_pts= n_pts_per_obj* (_track->_floating_objects.size()+ _track->_grid->_objects.size());
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	for (unsigned int idx_obj=0; idx_obj<_track->_floating_objects.size()+ _track->_grid->_objects.size(); ++idx_obj) {
		StaticObject * obj;
		if (idx_obj< _track->_floating_objects.size()) {
			obj= _track->_floating_objects[idx_obj];
		}
		else {
			obj= _track->_grid->_objects[idx_obj- _track->_floating_objects.size()];
		}
		
		// à cause du système de reference opengl il faut inverser les 0 et les 1 des y des textures
		number positions[n_pts_per_obj* 5]= {
			obj->_bbox->_pts[0].x, obj->_bbox->_pts[0].y, obj->_z, 0.0, 1.0,
			obj->_bbox->_pts[1].x, obj->_bbox->_pts[1].y, obj->_z, 1.0, 1.0,
			obj->_bbox->_pts[2].x, obj->_bbox->_pts[2].y, obj->_z, 1.0, 0.0,

			obj->_bbox->_pts[0].x, obj->_bbox->_pts[0].y, obj->_z, 0.0, 1.0,
			obj->_bbox->_pts[2].x, obj->_bbox->_pts[2].y, obj->_z, 1.0, 0.0,
			obj->_bbox->_pts[3].x, obj->_bbox->_pts[3].y, obj->_z, 0.0, 0.0
		};
		
		for (unsigned int idx_pt=0; idx_pt<n_pts_per_obj; ++idx_pt) {
			for (unsigned int i=0; i<5; ++i) {
				data[idx_obj* n_pts_per_obj* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ i]= float(positions[5* idx_pt+ i]);
			}
			//data[idx_obj* n_pts_per_obj* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 5]= float(_model_tex_idxs[basename(obj->_model->_json_path)]);
			data[idx_obj* n_pts_per_obj* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 5]= obj->_model->_texture_idx;
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
	update_floating_objects_footprint();
	update_floating_objects_bbox();
	update_texture();

	_world2camera= glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(float(_translation.x), float(_translation.y), 0.0f)), glm::vec3(_scale, _scale, _scale));
}


bool TrackEditor::key_down(InputState * input_state, SDL_Keycode key) {
	// RSHIFT + touches directionnelles = redimensionnement track
	if (input_state->get_key(SDLK_RSHIFT)) {
		if (key== SDLK_DOWN) {
			_track->drop_row();
			update();
			return true;
		}
		else if (key== SDLK_UP) {
			_track->add_row("empty");
			update();
			return true;
		}
		else if (key== SDLK_LEFT) {
			_track->drop_col();
			update();
			return true;
		}
		else if (key== SDLK_RIGHT) {
			_track->add_col("empty");
			update();
			return true;
		}
	}

	// touches directionnelles = sélection tuile
	else {
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
	}
	
	// i = remise à 0
	if (key== SDLK_i) {
		reinit();
		update();
		return true;
	}
	// l = chgmt json
	else if (key== SDLK_l) {
		load_json("../data/tracks/track1.json");
		update();
		return true;
	}
	// s = sauvegarde json
	else if (key== SDLK_s) {
		save_json("../data/tracks/track1.json");
		return true;
	}
	else if (key== SDLK_b) {
		_draw_bbox= !_draw_bbox;
	}
	else if (key== SDLK_t) {
		_draw_texture= !_draw_texture;
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
	// click + d = delete objet flottant
	if (input_state->get_key(SDLK_d)) {
		pt_type pos= screen2pt(input_state->_x, input_state->_y);
		StaticObject * obj= _track->get_floating_object(pos);
		if (obj!= NULL) {
			_track->delete_floating_object(obj);
			update();
			return true;
		}
	}
	// m,r,g -> sélection (voir mouse_motion())
	else if (input_state->get_key(SDLK_m) || input_state->get_key(SDLK_r) || input_state->get_key(SDLK_g)) {
		pt_type pos= screen2pt(input_state->_x, input_state->_y);
		StaticObject * obj= _track->get_floating_object(pos);
		if (obj!= NULL) {
			_selected_floating_object= obj;
			return true;
		}
	}
	// sinon sélection tuile
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
	// m = move objet
	if (input_state->get_key(SDLK_m) && _selected_floating_object!= NULL) {
		pt_type pos= screen2pt(input_state->_x, input_state->_y);
		_selected_floating_object->reinit(pos, _selected_floating_object->_alpha, _selected_floating_object->_scale);
		update();
		return true;
	}
	// r = rotate objet
	else if (input_state->get_key(SDLK_r) && _selected_floating_object!= NULL) {
		number alpha= _selected_floating_object->_alpha- 0.1* number(input_state->_yrel);
		_selected_floating_object->reinit(_selected_floating_object->_com, alpha, _selected_floating_object->_scale);
		update();
		return true;
	}
	// g = grossissement objet
	else if (input_state->get_key(SDLK_g) && _selected_floating_object!= NULL) {
		pt_type scale= _selected_floating_object->_scale+ pt_type(0.05* number(input_state->_xrel), -0.05* number(input_state->_yrel));
		_selected_floating_object->reinit(_selected_floating_object->_com, _selected_floating_object->_alpha, scale);
		update();
		return true;
	}
	// mouvement + ESPACE = déplacement visu grille track
	else if (input_state->get_key(SDLK_SPACE)) {
		_translation.x+= 0.02* number(input_state->_xrel);
		_translation.y-= 0.02* number(input_state->_yrel);
		update();
	}
	return false;
}


// wheel = zoom / dezoom
bool TrackEditor::mouse_wheel(InputState * input_state) {
	_scale+= 0.1* number(input_state->_y_wheel);
	update();
	return true;
}


// Editor -----------------------------------------------------
Editor::Editor() {

}


Editor::Editor(GLuint prog_simple, GLuint prog_texture, GLuint prog_font, ScreenGL * screengl) : _screengl(screengl) {
	//_font= new Font(prog_font, "../../fonts/Silom.ttf", 48, screengl);

	_track_editor= new TrackEditor(prog_simple, prog_texture, prog_font, screengl, 2.0);
	_tile_grid_editor= new GridEditor(prog_simple, prog_texture, prog_font, screengl, OBSTACLE_SETTING_COLOR, 1.0, VERTICAL_GRID);
	_floating_grid_editor= new GridEditor(prog_simple, prog_texture, prog_font, screengl, OBSTACLE_FLOATING_FOOTPRINT_COLOR, 1.0, HORIZONTAL_GRID);

	_track_editor->_translation= TRACK_ORIGIN;
	_tile_grid_editor->_translation= TILES_ORIGIN;
	_floating_grid_editor->_translation= FLOATING_OBJECTS_ORIGIN;

	_track_editor->_scale= DEFAULT_TRACK_EDITOR_SCALE;

	_tile_grid_editor->_grid->_width= TILE_GRID_WIDTH;
	_floating_grid_editor->_grid->_height= FLOATING_GRID_HEIGHT;

	for (auto model : _track_editor->_track->_models) {
		if (model.second->_type== OBSTACLE_TILE || model.second->_type== MATERIAL_TILE) {
			//std::cout << "loading OBSTACLE_SETTING : " << model.first << "\n";
			_tile_grid_editor->_grid->push_tile(model.second);
			//std::cout << "loaded\n";
		}
		else {
			//std::cout << "loading OBSTACLE_FLOATING : " << model.first << "\n";
			_floating_grid_editor->_grid->push_tile(model.second);
			//std::cout << "loaded\n";
		}
	}

	_tile_grid_editor->fill_texture_array();
	_floating_grid_editor->fill_texture_array();

	_track_editor->update();
	_tile_grid_editor->update();
	_floating_grid_editor->update();
}


Editor::~Editor() {
	
}


void Editor::draw() {
	// scissor permet de ne dessiner que sur un rectangle sans dépasser
	glScissor(0, 0, 800, 700);
	glEnable(GL_SCISSOR_TEST);
	_track_editor->show_info();
	_track_editor->draw();
	glDisable(GL_SCISSOR_TEST);
	_floating_grid_editor->show_info();
	_tile_grid_editor->draw();
	_floating_grid_editor->draw();
}


// update de la tuile courante de la track avec la tuile courante sélectionnée
void Editor::sync_track_with_tile() {
	StaticObject * current_tile= _tile_grid_editor->_grid->get_tile(_tile_grid_editor->_col_idx_select, _tile_grid_editor->_row_idx_select);
	StaticObjectModel * model= current_tile->_model;
	_track_editor->_track->_grid->set_tile(model, _track_editor->_col_idx_select, _track_editor->_row_idx_select);
	_track_editor->update();
}


// ajout d'un objet flottant
void Editor::add_floating_object(pt_type pos) {
	StaticObject * current_floating_object= _floating_grid_editor->_grid->get_tile(_floating_grid_editor->_col_idx_select, _floating_grid_editor->_row_idx_select);
	StaticObjectModel * model= current_floating_object->_model;

	// gestion des checkpoints
	if (model->_type== START) {
		CheckPoint * checkpoint= new CheckPoint(model, pt_type(pos.x, pos.y), 0.0, pt_type(1.0));
		_track_editor->_track->_start= checkpoint;
		_track_editor->_last_checkpoint= checkpoint;
		_track_editor->_track->_floating_objects.push_back(checkpoint);
	}
	else if (model->_type== CHECKPOINT) {
		if (_track_editor->_track->_start== NULL) {
			std::cerr << "pas de checkpoint sans start\n";
			return;
		}
		CheckPoint * checkpoint= new CheckPoint(model, pt_type(pos.x, pos.y), 0.0, pt_type(1.0));
		checkpoint->_next= _track_editor->_track->_start;
		_track_editor->_last_checkpoint->_next= checkpoint;
		_track_editor->_last_checkpoint= checkpoint;
		_track_editor->_track->_floating_objects.push_back(checkpoint);
	}
	else {
		_track_editor->_track->_floating_objects.push_back(new StaticObject(model, pt_type(pos.x, pos.y), 0.0, pt_type(1.0)));
	}
	
	_track_editor->update();
}


bool Editor::key_down(InputState * input_state, SDL_Keycode key) {
	// touches directionnelles : si LSHIFT on se déplace dans les tuiles dispos sinon dans la track
	if (key== SDLK_DOWN || key== SDLK_UP || key== SDLK_LEFT || key== SDLK_RIGHT) {
		if (input_state->get_key(SDLK_LSHIFT)) {
			_tile_grid_editor->key_down(input_state, key);
		}
		else if (input_state->get_key(SDLK_RSHIFT)) {
			_track_editor->key_down(input_state, key);
		}
		else {
			// si de plus w : write tuile track avec tuile sélectionnée
			_track_editor->key_down(input_state, key);
			if (input_state->get_key(SDLK_w)) {
				sync_track_with_tile();
			}
		}
		return true;
	}
	// w : write tuile track avec tuile sélectionnée
	else if (key== SDLK_w) {
		sync_track_with_tile();
		return true;
	}
	else if (_track_editor->key_down(input_state, key)) {
		return true;
	}
	return false;
}


bool Editor::key_up(InputState * input_state, SDL_Keycode key) {
	return false;
}


bool Editor::mouse_button_down(InputState * input_state) {
	// f = ajout d'un objet flottant
	if (input_state->get_key(SDLK_f)) {
		pt_type pos= _track_editor->screen2pt(input_state->_x, input_state->_y);
		if (pos.x>=0 && pos.x<number(_track_editor->_track->_grid->_width)* _track_editor->_track->_grid->_cell_size && pos.y>=0 && pos.y<number(_track_editor->_track->_grid->_height)* _track_editor->_track->_grid->_cell_size) {
			add_floating_object(pos);
			return true;
		}
	}

	if (_tile_grid_editor->mouse_button_down(input_state)) {
		return true;
	}
	else if (_floating_grid_editor->mouse_button_down(input_state)) {
		return true;
	}
	else if (_track_editor->mouse_button_down(input_state)) {
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
