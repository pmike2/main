#include <iomanip>
#include <fstream>
#include <sstream>
#include <functional>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "json.hpp"

#include "utile.h"

#include "racing.h"


using json = nlohmann::json;



// Racing ---------------------------------------------------------
Racing::Racing() {

}


Racing::Racing(GLuint prog_simple, GLuint prog_texture, GLuint prog_font, ScreenGL * screengl, InputState * input_state) :
	_draw_bbox(false), _draw_force(false), _draw_texture(true), _show_debug_info(false), _show_info(true),
	_cam_mode(TRANSLATE), _screengl(screengl), _input_state(input_state)
	{
	// caméras
	_camera2clip= glm::ortho(float(-screengl->_gl_width)* 0.5f, float(screengl->_gl_width)* 0.5f, -float(screengl->_gl_height)* 0.5f, float(screengl->_gl_height)* 0.5f, Z_NEAR, Z_FAR);
	_world2camera= glm::mat4(1.0);
	
	// font
	_font= new Font(prog_font, "../../fonts/Silom.ttf", 48, screengl);
	_font->_z= 100.0f; // pour que l'affichage des infos se fassent par dessus le reste

	// buffers
	unsigned int n_buffers= 4;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	// textures
	unsigned int n_textures= 1;
	_textures= new GLuint(n_textures);
	glGenTextures(n_textures, _textures);

	// contextes de dessin
	_contexts["bbox"]= new DrawContext(prog_simple, _buffers[0],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["footprint"]= new DrawContext(prog_simple, _buffers[1],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["force"]= new DrawContext(prog_simple, _buffers[2],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["texture"]= new DrawContext(prog_texture, _buffers[3],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "texture_array"});

	// piste
	_track= new Track();
	_track->load_json("../data/tracks/track1.json");

	// remplissage textures
	fill_texture_array();

	// init données
	Car * hero= _track->get_hero();
	_com_camera= hero->_com;
	update_bbox();
	update_footprint();
	update_force();
	update_texture();
}


Racing::~Racing() {
	delete _buffers;
}


void Racing::fill_texture_array() {
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
		_model_tex_idxs[model.first]= compt;
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


void Racing::draw_bbox() {
	DrawContext * context= _contexts["bbox"];

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

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Racing::draw_footprint() {
	DrawContext * context= _contexts["footprint"];

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


void Racing::draw_force() {
	DrawContext * context= _contexts["force"];

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


void Racing::draw_texture() {
	DrawContext * context= _contexts["texture"];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textures[0]);
	glActiveTexture(0);

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);

	glUniform1i(context->_locs_uniform["texture_array"], 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	//glUniform1f(context->_locs_uniform["z"], -15.0f);
	
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


void Racing::draw() {
	if (_track->_mode== TRACK_LIVE) {
		if (_draw_force) {
			draw_force();
		}
		if (_draw_bbox) {
			draw_bbox();
			draw_footprint();
		}
		if (_draw_texture) {
			draw_texture();
		}
		if (_show_debug_info) {
			show_debug_info();
		}
		if (_show_info) {
			show_info();
		}
	}
	else if (_track->_mode== TRACK_WON) {
		std::vector<Text> texts;
		texts.push_back(Text("YOU WON !", glm::vec2(-4.0f, 0.0f), 0.03f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)));
		_font->set_text(texts);
		_font->draw();
	}
	else if (_track->_mode== TRACK_LOST) {
		std::vector<Text> texts;
		texts.push_back(Text("YOU LOST !", glm::vec2(-4.0f, 0.0f), 0.03f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)));
		_font->set_text(texts);
		_font->draw();
	}
}


void Racing::show_info() {
	Car * hero= _track->get_hero();
	std::vector<Car *> cars= _track->get_sorted_cars();

	std::vector<Text> texts;

	glm::vec4 color(NLAPS_COLOR);
	if (hero->_n_laps== _track->_n_laps) {
		color= NLAPS_LAST_COLOR;
	}
	texts.push_back(Text("LAP "+ std::to_string(hero->_n_laps)+ " / "+ std::to_string(_track->_n_laps), glm::vec2(6.0f, 7.0f), 0.015f, NLAPS_COLOR));

	for (unsigned int idx_car=0; idx_car<cars.size(); ++idx_car) {
		glm::vec4 color(ENNEMY_COLOR);
		if (cars[idx_car]== hero) {
			color= HERO_COLOR;
		}
		texts.push_back(Text(std::to_string(idx_car+ 1)+  " - "+ cars[idx_car]->_name, glm::vec2(-9.0f, 7.0f- float(idx_car)* 0.5f), 0.007f, color));
	}

	/*if (hero->_drift) {
		texts.push_back(Text("DRIFT !", glm::vec2(0.0f, 0.0f), 0.02, glm::vec4(1.0, 1.0, 0.0, 0.1)));
	}*/

	_font->set_text(texts);
	_font->draw();
}


void Racing::show_debug_info() {
	const float font_scale= 0.007f;
	const glm::vec4 text_color(1.0, 1.0, 1.0, 1.0);

	Car * hero= _track->get_hero();

	std::vector<Text> texts;

	texts.push_back(Text("COM PT", glm::vec2(-9.0f, 7.0f), font_scale, COM_CROSS_COLOR));
	texts.push_back(Text("FORCE_FWD PT", glm::vec2(-9.0f, 6.0f), font_scale, FORCE_FWD_CROSS_COLOR));
	texts.push_back(Text("FORCE_BWD PT", glm::vec2(-9.0f, 5.0f), font_scale, FORCE_BWD_CROSS_COLOR));
	texts.push_back(Text("FORCE_FWD VEC", glm::vec2(-9.0f, 3.0f), font_scale, FORCE_FWD_ARROW_COLOR));
	texts.push_back(Text("FORCE_BWD VEC", glm::vec2(-9.0f, 2.0f), font_scale, FORCE_BWD_ARROW_COLOR));
	texts.push_back(Text("ACCELERATION VEC", glm::vec2(-9.0f, 1.0f), font_scale, ACCELERATION_ARROW_COLOR));
	texts.push_back(Text("VELOCITY VEC", glm::vec2(-9.0f, 0.0f), font_scale, VELOCITY_ARROW_COLOR));
	texts.push_back(Text("FORWARD VEC", glm::vec2(-9.0f, -1.0f), font_scale, FORWARD_ARROW_COLOR));
	texts.push_back(Text("RIGHT VEC", glm::vec2(-9.0f, -2.0f), font_scale, RIGHT_ARROW_COLOR));

	texts.push_back(Text("thrust="+ std::to_string(hero->_thrust), glm::vec2(6.0, 7.0), font_scale, text_color));
	texts.push_back(Text("wheel="+ std::to_string(hero->_wheel), glm::vec2(6.0, 6.0), font_scale, text_color));
	
	texts.push_back(Text("force_fwd="+ std::to_string(norm(hero->_force_fwd)), glm::vec2(6.0, 4.0), font_scale, text_color));
	texts.push_back(Text("force_bwd="+ std::to_string(norm(hero->_force_bwd)), glm::vec2(6.0, 3.0), font_scale, text_color));
	texts.push_back(Text("acc="+ std::to_string(norm(hero->_acceleration)), glm::vec2(6.0, 2.0), font_scale, text_color));
	texts.push_back(Text("vel="+ std::to_string(norm(hero->_velocity)), glm::vec2(6.0, 1.0), font_scale, text_color));

	texts.push_back(Text("torque="+ std::to_string(hero->_torque), glm::vec2(6.0, -1.0), font_scale, text_color));
	texts.push_back(Text("ang acc="+ std::to_string(hero->_angular_acceleration), glm::vec2(6.0, -2.0), font_scale, text_color));
	texts.push_back(Text("ang vel="+ std::to_string(hero->_angular_velocity), glm::vec2(6.0, -3.0), font_scale, text_color));
	texts.push_back(Text("alpha="+ std::to_string(hero->_alpha), glm::vec2(6.0, -4.0), font_scale, text_color));

	texts.push_back(Text("drift="+ std::to_string(hero->_drift), glm::vec2(6.0, -6.0), font_scale, text_color));

	_font->set_text(texts);
	_font->draw();
}


void Racing::update_bbox() {
	const unsigned int n_pts_per_obj= 8;

	DrawContext * context= _contexts["bbox"];
	context->_n_pts= n_pts_per_obj* (_track->_grid->_objects.size()+ _track->_floating_objects.size());
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	float * ptr= data;
	for (auto obj : _track->_grid->_objects) {
		std::vector<pt_type> pts(obj->_bbox->_pts, obj->_bbox->_pts+ 4);
		ptr= draw_polygon(ptr, pts, BBOX_COLOR);
	}
	for (auto obj : _track->_floating_objects) {
		std::vector<pt_type> pts(obj->_bbox->_pts, obj->_bbox->_pts+ 4);
		ptr= draw_polygon(ptr, pts, BBOX_COLOR);
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::update_footprint() {
	DrawContext * context= _contexts["footprint"];
	context->_n_pts= 0;
	context->_n_attrs_per_pts= 6;

	// pour dessiner les polygones footprint on utilise la triangulation de ces polygones
	for (auto obj : _track->_floating_objects) {
		context->_n_pts+= 3* obj->_footprint->_triangles_idx.size();
	}
	for (auto obj : _track->_grid->_objects) {
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
			data[idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= FOOTPRINT_COLOR[idx_color];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::update_force() {
	const unsigned int n_pts_per_cross= 4;
	const unsigned int n_pts_per_arrow= 6;
	const unsigned int n_pts_per_obj= 3* n_pts_per_cross+ 6* n_pts_per_arrow; // 3 croix ; 6 fleches

	DrawContext * context= _contexts["force"];
	context->_n_pts= n_pts_per_obj* (_track->_floating_objects.size()+ _track->_grid->_objects.size());
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr= data;

	// dessins des forces; voir gl_utils
	for (auto obj : _track->_floating_objects) {
		if (obj->_model->_type== HERO_CAR || obj->_model->_type== ENNEMY_CAR) {
			Car * car= (Car *)(obj);
			ptr= draw_cross(ptr, car->_com, CROSS_SIZE, COM_CROSS_COLOR);
			ptr= draw_cross(ptr, car->_com+ car->_com2force_fwd, CROSS_SIZE, FORCE_FWD_CROSS_COLOR);
			ptr= draw_cross(ptr, car->_com+ car->_com2force_bwd, CROSS_SIZE, FORCE_BWD_CROSS_COLOR);
			ptr= draw_arrow(ptr, car->_com+ car->_com2force_fwd, car->_com+ car->_com2force_fwd+ car->_force_fwd, ARROW_TIP_SIZE, ARROW_ANGLE, FORCE_FWD_ARROW_COLOR);
			ptr= draw_arrow(ptr, car->_com+ car->_com2force_bwd, car->_com+ car->_com2force_bwd+ car->_force_bwd, ARROW_TIP_SIZE, ARROW_ANGLE, FORCE_BWD_ARROW_COLOR);
			ptr= draw_arrow(ptr, car->_com, car->_com+ car->_acceleration, ARROW_TIP_SIZE, ARROW_ANGLE, ACCELERATION_ARROW_COLOR);
			ptr= draw_arrow(ptr, car->_com, car->_com+ car->_velocity, ARROW_TIP_SIZE, ARROW_ANGLE, VELOCITY_ARROW_COLOR);
			ptr= draw_arrow(ptr, car->_com, car->_com+ car->_forward, ARROW_TIP_SIZE, ARROW_ANGLE, FORWARD_ARROW_COLOR);
			ptr= draw_arrow(ptr, car->_com, car->_com+ car->_right, ARROW_TIP_SIZE, ARROW_ANGLE, RIGHT_ARROW_COLOR);
		}
		else {
			ptr= draw_cross(ptr, obj->_com, CROSS_SIZE, COM_CROSS_COLOR);
			ptr= draw_nothing(ptr, context->_n_attrs_per_pts, n_pts_per_cross);
			ptr= draw_nothing(ptr, context->_n_attrs_per_pts, n_pts_per_cross);
			ptr= draw_arrow(ptr, obj->_com, obj->_com+ obj->_force, ARROW_TIP_SIZE, ARROW_ANGLE, FORCE_FWD_ARROW_COLOR);
			ptr= draw_arrow(ptr, obj->_com, obj->_com+ obj->_acceleration, ARROW_TIP_SIZE, ARROW_ANGLE, ACCELERATION_ARROW_COLOR);
			ptr= draw_arrow(ptr, obj->_com, obj->_com+ obj->_velocity, ARROW_TIP_SIZE, ARROW_ANGLE, VELOCITY_ARROW_COLOR);
			ptr= draw_nothing(ptr, context->_n_attrs_per_pts, n_pts_per_arrow);
			ptr= draw_nothing(ptr, context->_n_attrs_per_pts, n_pts_per_arrow);
			ptr= draw_nothing(ptr, context->_n_attrs_per_pts, n_pts_per_arrow);
		}
	}

	for (auto obj : _track->_grid->_objects) {
		ptr= draw_cross(ptr, obj->_com, CROSS_SIZE, COM_CROSS_COLOR);
		ptr= draw_nothing(ptr, context->_n_attrs_per_pts, n_pts_per_cross);
		ptr= draw_nothing(ptr, context->_n_attrs_per_pts, n_pts_per_cross);
		ptr= draw_nothing(ptr, context->_n_attrs_per_pts, n_pts_per_arrow);
		ptr= draw_nothing(ptr, context->_n_attrs_per_pts, n_pts_per_arrow);
		ptr= draw_nothing(ptr, context->_n_attrs_per_pts, n_pts_per_arrow);
		ptr= draw_nothing(ptr, context->_n_attrs_per_pts, n_pts_per_arrow);
		ptr= draw_nothing(ptr, context->_n_attrs_per_pts, n_pts_per_arrow);
		ptr= draw_nothing(ptr, context->_n_attrs_per_pts, n_pts_per_arrow);
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::update_texture() {
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
			data[idx_obj* n_pts_per_obj* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 5]= float(_model_tex_idxs[basename(obj->_model->_json_path)]);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::anim(std::chrono::system_clock::time_point t) {
	_track->anim(t, _input_state);

	update_bbox();
	update_footprint();
	update_force();
	update_texture();
	
	camera();
}


void Racing::camera() {
	Car * hero= _track->get_hero();

	_com_camera.x+= CAM_INC* (hero->_com.x- _com_camera.x);
	_com_camera.y+= CAM_INC* (hero->_com.y- _com_camera.y);
	number diff_alpha= hero->_alpha- _alpha_camera;
	if (diff_alpha> M_PI) {
		_alpha_camera+= 2.0* M_PI;
		diff_alpha-= 2.0* M_PI;
	}
	if (diff_alpha< -1.0* M_PI) {
		_alpha_camera-= 2.0* M_PI;
		diff_alpha+= 2.0* M_PI;
	}
	_alpha_camera+= CAM_INC_ALPHA* diff_alpha;

	if (_cam_mode== FIXED) {
		_world2camera= glm::translate(glm::mat4(1.0f), glm::vec3(-0.5* _screengl->_gl_width+ 1.0, -0.5* _screengl->_gl_height+ 1.0, 0.0));
	}
	else if (_cam_mode== TRANSLATE) {
		_world2camera= glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f* float(_com_camera.x), -1.0f* float(_com_camera.y), 1.0f));
	}
	else if (_cam_mode== TRANSLATE_AND_ROTATE) {
		_world2camera= glm::translate(glm::rotate(glm::mat4(1.0f), -1.0f* float(_alpha_camera), glm::vec3(0.0, 0.0, 1.0f)), glm::vec3(-1.0f* float(_com_camera.x), -1.0f* float(_com_camera.y), 1.0f));
	}
}


bool Racing::key_down(SDL_Keycode key) {
	// b : dessiner bbox
	if (key== SDLK_b) {
		_draw_bbox= !_draw_bbox;
		return true;
	}

	// f : dessiner forces
	else if (key== SDLK_f) {
		_draw_force= !_draw_force;
		return true;
	}

	// i : switcher infos normales / infos debug
	else if (key== SDLK_i) {
		_show_debug_info= !_show_debug_info;
		_show_info= !_show_info;
		return true;
	}

	else if (key== SDLK_t) {
		_draw_texture= !_draw_texture;
		return true;
	}

	// c : chgmt mode caméra
	else if (key== SDLK_c) {
		if (_cam_mode== FIXED) {
			_cam_mode= TRANSLATE;
		}
		else if (_cam_mode== TRANSLATE) {
			_cam_mode= TRANSLATE_AND_ROTATE;
		}
		else if (_cam_mode== TRANSLATE_AND_ROTATE) {
			_cam_mode= FIXED;
		}
		return true;
	}

	return false;
}
