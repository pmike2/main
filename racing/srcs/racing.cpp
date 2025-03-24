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


Racing::Racing(std::map<std::string, GLuint> progs, ScreenGL * screengl, InputState * input_state, std::chrono::system_clock::time_point t) :
	_draw_bbox(false), _draw_force(false), _draw_texture(true), _show_debug_info(false),
	_cam_mode(TRANSLATE), _screengl(screengl), _input_state(input_state),
	_mode(CHOOSE_TRACK)
	{
	// caméras
	_camera2clip= glm::ortho(float(-screengl->_gl_width)* 0.5f, float(screengl->_gl_width)* 0.5f, -float(screengl->_gl_height)* 0.5f, float(screengl->_gl_height)* 0.5f, Z_NEAR, Z_FAR);
	_world2camera= glm::mat4(1.0);
	
	// font
	_font= new Font(progs["font"], "../../fonts/Silom.ttf", 48, screengl);
	_font->_z= 100.0f; // pour que l'affichage des infos se fassent par dessus le reste

	// buffers
	unsigned int n_buffers= 7;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	// textures
	unsigned int n_textures= 5;
	_textures= new GLuint[n_textures];
	glGenTextures(n_textures, _textures);
	_texture_idx_model= 0;
	_texture_idx_bump= 1;
	_texture_idx_smoke= 2;
	_texture_idx_choose_track= 3;
	_texture_idx_tire_track= 4;

	// contextes de dessin
	_contexts["bbox"]= new DrawContext(progs["simple"], _buffers[0],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["footprint"]= new DrawContext(progs["simple"], _buffers[1],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["force"]= new DrawContext(progs["simple"], _buffers[2],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["texture"]= new DrawContext(progs["texture"], _buffers[3],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in", "bump_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "texture_array", "texture_array_bump", "gray_blend"});

	_contexts["smoke"]= new DrawContext(progs["smoke"], _buffers[4],
		std::vector<std::string>{"position_in", "tex_coord_in", "alpha_in", "current_layer_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "texture_array"});

	_contexts["choose_track"]= new DrawContext(progs["choose_track"], _buffers[5],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in", "selection_in"},
		std::vector<std::string>{"camera2clip_matrix", "texture_array"});

	_contexts["tire_track"]= new DrawContext(progs["tire_track"], _buffers[6],
		std::vector<std::string>{"position_in", "tex_coord_in", "alpha_in", "current_layer_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "texture_array", "z"});

	// piste
	_track= new Track();
	_tire_track_system= new TireTrackSystem();

	fill_texture_array_models();
	fill_texture_array_bump();
	fill_texture_array_smoke();
	fill_texture_array_tire_track();

	_idx_chosen_track= 1;
	fill_texture_choose_track();
	update_choose_track();
}


Racing::~Racing() {
	delete _track;
	for (auto ss : _smoke_systems) {
		delete ss;
	}
	_smoke_systems.clear();
	delete _tire_track_system;
	for (auto context : _contexts) {
		delete context.second;
	}
	_contexts.clear();
	delete _buffers;
	delete _textures;
	delete _font;
}


void Racing::choose_track(unsigned int idx_track, std::chrono::system_clock::time_point t) {
	// chargement json
	_track->load_json("../data/tracks/track"+ std::to_string(idx_track)+ ".json");

	// reinit fumée par voiture
	for (auto ss : _smoke_systems) {
		delete ss;
	}
	_smoke_systems.clear();
	std::vector<std::string> pngs= list_files("../data/smoke", "png");
	for (auto car : _track->_sorted_cars) {
		_smoke_systems.push_back(new SmokeSystem(car, pngs.size(), t));
	}

	// reinit traces de pneu
	_tire_track_system->reinit();

	// init données
	Car * hero= _track->get_hero();
	_com_camera= hero->_com;
	
	update_bbox();
	update_footprint();
	update_force();
	update_texture();
	update_smoke();
	update_tire_track();

	_mode= RACING;
	_track->start(t);
}


void Racing::fill_texture_choose_track() {
	std::vector<std::string> pngs= list_files("../data/tracks/quicklooks", "png");
	fill_texture_array(0, _textures[_texture_idx_choose_track], 1024, pngs);
	_n_available_tracks= pngs.size();
}


void Racing::fill_texture_array_models() {
	std::vector<std::string> pngs;
	unsigned int idx_model= 0;
	for (auto m : _track->_models) {
		StaticObjectModel * model= m.second;
		model->_texture_idx= float(idx_model++);
		std::string png= dirname(model->_json_path)+ "/textures/"+ model->_name+ ".png";
		pngs.push_back(png);
	}
	fill_texture_array(0, _textures[_texture_idx_model], 1024, pngs);
}


void Racing::fill_texture_array_bump() {
	std::vector<std::string> pngs;
	for (auto m : _track->_models) {
		StaticObjectModel * model= m.second;
		std::string png_normal= dirname(model->_json_path)+ "/textures/"+ model->_name+ ".png";
		std::string png_bump= dirname(model->_json_path)+ "/textures/"+ model->_name+ "_bump.png";
		
		if (file_exists(png_bump)) {
			pngs.push_back(png_bump);
		}
		else {
			pngs.push_back(png_normal);
		}
	}
	fill_texture_array(1, _textures[_texture_idx_bump], 1024, pngs);
}


void Racing::fill_texture_array_smoke() {
	std::vector<std::string> pngs= list_files("../data/smoke", "png");
	fill_texture_array(0, _textures[_texture_idx_smoke], 1024, pngs);
}


void Racing::fill_texture_array_tire_track() {
	std::vector<std::string> pngs= list_files("../data/tire_track", "png");
	fill_texture_array(0, _textures[_texture_idx_tire_track], 1024, pngs);
	for (unsigned int i=0; i<pngs.size(); ++i) {
		_track->_materials[basename(pngs[i])]->_tire_track_texture_idx= float(i);
	}
}


void Racing::draw_choose_track() {
	DrawContext * context= _contexts["choose_track"];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textures[_texture_idx_choose_track]);
	glActiveTexture(0);

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);

	glUniform1i(context->_locs_uniform["texture_array"], 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));

	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["tex_coord_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["current_layer_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(4* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["selection_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(5* sizeof(float)));
	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Racing::draw_bbox() {
	DrawContext * context= _contexts["bbox"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	glUniform1f(context->_locs_uniform["z"], Z_BBOX);
	
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
	glUniform1f(context->_locs_uniform["z"], Z_FOOTPRINT);
	
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
	glUniform1f(context->_locs_uniform["z"], Z_FORCE);
	
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
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textures[_texture_idx_model]);
	glActiveTexture(0);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textures[_texture_idx_bump]);
	glActiveTexture(0);

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);

	glUniform1i(context->_locs_uniform["texture_array"], 0);
	glUniform1i(context->_locs_uniform["texture_array_bump"], 1);
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));

	if (_track->_mode== TRACK_LIVE) {
		glUniform1f(context->_locs_uniform["gray_blend"], 0.0f);
	}
	else {
		glUniform1f(context->_locs_uniform["gray_blend"], GRAY_BLEND);
	}
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["tex_coord_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["bump_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(5* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["current_layer_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(6* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Racing::draw_smoke() {
	DrawContext * context= _contexts["smoke"];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textures[_texture_idx_smoke]);
	glActiveTexture(0);

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);

	glUniform1i(context->_locs_uniform["texture_array"], 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));

	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["tex_coord_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["alpha_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(5* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["current_layer_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(6* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Racing::draw_tire_track() {
	DrawContext * context= _contexts["tire_track"];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textures[_texture_idx_tire_track]);
	glActiveTexture(0);

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);

	glUniform1i(context->_locs_uniform["texture_array"], 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	glUniform1f(context->_locs_uniform["z"], Z_TIRE_TRACK);

	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["tex_coord_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["alpha_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(4* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["current_layer_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(5* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Racing::show_info() {
	std::vector<Text> texts;

	if (_mode== CHOOSE_TRACK) {
		texts.push_back(Text("Track "+ std::to_string(_idx_chosen_track), glm::vec2(-1.5f, 4.0f), 0.02, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
	}

	else if (_mode== RACING) {
		Car * hero= _track->get_hero();

		if (_show_debug_info) {
			const float font_scale= 0.007f;
			const glm::vec4 text_color(1.0, 1.0, 1.0, 1.0);

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
		}

		else {
			// 3, 2, 1
			if (_track->_mode== TRACK_PRECOUNT) {
				texts.push_back(Text(std::to_string(_track->_precount), glm::vec2(-1.0f, 0.0f), 0.06f, glm::vec4(1.0f, 1.0f, 0.5f, 1.0f)));
			}
			
			// classement joueur arrivée
			if (_track->_mode== TRACK_FINISHED) {
				texts.push_back(Text(std::to_string(hero->_rank) + " / "+ std::to_string(_track->_sorted_cars.size()), glm::vec2(-2.0f, 0.0f), 0.03f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)));
			}

			// nombre de tours
			if (_track->_mode== TRACK_LIVE) {
				glm::vec4 nlaps_color(NLAPS_COLOR);
				if (hero->_n_laps== _track->_n_laps) {
					nlaps_color= NLAPS_LAST_COLOR;
				}
				texts.push_back(Text("LAP "+ std::to_string(hero->_n_laps)+ " / "+ std::to_string(_track->_n_laps), glm::vec2(5.0f, 5.0f), 0.01f, nlaps_color));
			}

			// vitesse joueur
			if (_track->_mode== TRACK_LIVE) {
				int hero_speed= (int)(sqrt(hero->_velocity.x* hero->_velocity.x+ hero->_velocity.y* hero->_velocity.y)* 60.0);
				glm::vec4 speed_color(LOW_SPEED_COLOR);
				if (hero_speed> 200) {
					speed_color= HIGH_SPEED_COLOR;
				}
				texts.push_back(Text(std::to_string(hero_speed)+ " km/h", glm::vec2(-7.2f, 5.0f), 0.01f, speed_color));
			}

			// classement toutes voitures
			if (_track->_mode== TRACK_LIVE || _track->_mode== TRACK_FINISHED) {
				for (unsigned int idx_car=0; idx_car<_track->_sorted_cars.size(); ++idx_car) {
					Car * car= _track->_sorted_cars[idx_car];
					glm::vec4 ranking_color(RANKING_ENNEMY_COLOR);
					if (car== hero) {
						ranking_color= RANKING_HERO_COLOR;
					}
					texts.push_back(Text(std::to_string(idx_car+ 1)+  " - "+ car->_name, glm::vec2(-7.2f, 4.0f- float(idx_car)* 0.5f), 0.005f, ranking_color));
				}
			}

			// temps tours joueur
			if (_track->_mode== TRACK_LIVE || _track->_mode== TRACK_FINISHED) {
				for (unsigned int idx_time=0; idx_time<hero->_lap_times.size(); ++idx_time) {
					glm::vec4 lap_time_color(PAST_LAP_TIME_COLOR);
					if (idx_time== hero->_lap_times.size()- 1) {
						lap_time_color= CURRENT_LAP_TIME_COLOR;
					}
					std::ostringstream oss;
					oss << std::fixed << std::setprecision(2) << hero->_lap_times[idx_time];
					texts.push_back(Text("LAP "+ std::to_string(idx_time+ 1)+  " - "+ oss.str(), glm::vec2(5.0f, 4.0f- float(idx_time)* 0.5f), 0.005f, lap_time_color));
				}
			}

			// temps total joueur arrivée
			if (_track->_mode== TRACK_FINISHED) {
				number total_time= 0.0;
				for (auto lap_time : hero->_lap_times) {
					total_time+= lap_time;
				}
				std::ostringstream oss;
				oss << std::fixed << std::setprecision(2) << total_time;
				texts.push_back(Text("TOTAL - "+ oss.str(), glm::vec2(5.0f, 3.0f- 0.5f* float(_track->_n_laps+ 1)), 0.005f, TOTAL_LAP_TIME_COLOR));
			}
		}
	}

	_font->set_text(texts);
	_font->draw();
}


void Racing::draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (_mode== CHOOSE_TRACK) {
		draw_choose_track();
	}
	else if (_mode== RACING) {
		if (_draw_force) {
			draw_force();
		}
		if (_draw_bbox) {
			draw_bbox();
			draw_footprint();
		}
		if (_draw_texture) {
			draw_texture();
			draw_tire_track();
			draw_smoke();
		}
	}

	show_info();
}


void Racing::update_choose_track() {
	const unsigned int n_pts_per_obj= 6;
	const float MARGIN= 1.0;

	DrawContext * context= _contexts["choose_track"];
	context->_n_pts= _n_available_tracks* n_pts_per_obj;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	float size= (_screengl->_gl_width- float(_n_available_tracks+ 1)* MARGIN)/ float(_n_available_tracks);
	unsigned int compt= 0;
	float selection;
	for (unsigned int i=0; i<_n_available_tracks; ++i) {
		if (i== _idx_chosen_track- 1) {
			selection= 0.2f;
		}
		else {
			selection= 0.0f;
		}
		data[compt++]= -0.5f* _screengl->_gl_width+ MARGIN+ float(i)* (size+ MARGIN); data[compt++]= -0.5f* size;
		data[compt++]= 0.0f; data[compt++]= 1.0f; data[compt++]= float(i); data[compt++]= selection;
		data[compt++]= -0.5f* _screengl->_gl_width+ MARGIN+ float(i)* (size+ MARGIN)+ size; data[compt++]= -0.5f* size;
		data[compt++]= 1.0f; data[compt++]= 1.0f; data[compt++]= float(i); data[compt++]= selection;
		data[compt++]= -0.5f* _screengl->_gl_width+ MARGIN+ float(i)* (size+ MARGIN)+ size; data[compt++]= 0.5f* size;
		data[compt++]= 1.0f; data[compt++]= 0.0f; data[compt++]= float(i); data[compt++]= selection;
		
		data[compt++]= -0.5f* _screengl->_gl_width+ MARGIN+ float(i)* (size+ MARGIN); data[compt++]= -0.5f* size;
		data[compt++]= 0.0f; data[compt++]= 1.0f; data[compt++]= float(i); data[compt++]= selection;
		data[compt++]= -0.5f* _screengl->_gl_width+ MARGIN+ float(i)* (size+ MARGIN)+ size; data[compt++]= 0.5f* size;
		data[compt++]= 1.0f; data[compt++]= 0.0f; data[compt++]= float(i); data[compt++]= selection;
		data[compt++]= -0.5f* _screengl->_gl_width+ MARGIN+ float(i)* (size+ MARGIN); data[compt++]= 0.5f* size;
		data[compt++]= 0.0f; data[compt++]= 0.0f; data[compt++]= float(i); data[compt++]= selection;
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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
	const unsigned int n_pts_per_obj= 12;

	unsigned int n_grid_objects= _track->_grid->_objects.size();
	unsigned int n_floating_objects= _track->_floating_objects.size();

	DrawContext * context= _contexts["texture"];
	context->_n_pts= n_pts_per_obj* (n_grid_objects+ n_floating_objects);
	context->_n_attrs_per_pts= 7;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	for (unsigned int idx_obj=0; idx_obj<n_grid_objects+ n_floating_objects; ++idx_obj) {
		StaticObject * obj;
		
		if (idx_obj< n_grid_objects) {
			obj= _track->_grid->_objects[idx_obj];
		}
		else if (idx_obj< n_grid_objects+ n_floating_objects) {
			obj= _track->_floating_objects[idx_obj- n_grid_objects];
		}
		
		// à cause du système de reference opengl il faut inverser les 0 et les 1 des y des textures
		/*number positions[n_pts_per_obj* 5]= {
			obj->_bbox->_pts[0].x, obj->_bbox->_pts[0].y, obj->_z, 0.0, 1.0,
			obj->_bbox->_pts[1].x, obj->_bbox->_pts[1].y, obj->_z, 1.0, 1.0,
			obj->_bbox->_pts[2].x, obj->_bbox->_pts[2].y, obj->_z, 1.0, 0.0,

			obj->_bbox->_pts[0].x, obj->_bbox->_pts[0].y, obj->_z, 0.0, 1.0,
			obj->_bbox->_pts[2].x, obj->_bbox->_pts[2].y, obj->_z, 1.0, 0.0,
			obj->_bbox->_pts[3].x, obj->_bbox->_pts[3].y, obj->_z, 0.0, 0.0
		};*/
		
		number positions[n_pts_per_obj* 6]= {
			obj->_bbox->_pts[0].x, obj->_bbox->_pts[0].y, obj->_z, 0.0, 1.0, obj->_bumps[0],
			obj->_bbox->_pts[1].x, obj->_bbox->_pts[1].y, obj->_z, 1.0, 1.0, obj->_bumps[1],
			obj->_bbox->_center.x, obj->_bbox->_center.y, obj->_z, 0.5, 0.5, obj->_bumps[8],

			obj->_bbox->_pts[1].x, obj->_bbox->_pts[1].y, obj->_z, 1.0, 1.0, obj->_bumps[2],
			obj->_bbox->_pts[2].x, obj->_bbox->_pts[2].y, obj->_z, 1.0, 0.0, obj->_bumps[3],
			obj->_bbox->_center.x, obj->_bbox->_center.y, obj->_z, 0.5, 0.5, obj->_bumps[8],

			obj->_bbox->_pts[2].x, obj->_bbox->_pts[2].y, obj->_z, 1.0, 0.0, obj->_bumps[4],
			obj->_bbox->_pts[3].x, obj->_bbox->_pts[3].y, obj->_z, 0.0, 0.0, obj->_bumps[5],
			obj->_bbox->_center.x, obj->_bbox->_center.y, obj->_z, 0.5, 0.5, obj->_bumps[8],

			obj->_bbox->_pts[3].x, obj->_bbox->_pts[3].y, obj->_z, 0.0, 0.0, obj->_bumps[6],
			obj->_bbox->_pts[0].x, obj->_bbox->_pts[0].y, obj->_z, 0.0, 1.0, obj->_bumps[7],
			obj->_bbox->_center.x, obj->_bbox->_center.y, obj->_z, 0.5, 0.5, obj->_bumps[8]
		};

		for (unsigned int idx_pt=0; idx_pt<n_pts_per_obj; ++idx_pt) {
			for (unsigned int i=0; i<6; ++i) {
				data[idx_obj* n_pts_per_obj* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ i]= float(positions[6* idx_pt+ i]);
			}
			data[idx_obj* n_pts_per_obj* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 6]= obj->_model->_texture_idx;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::update_smoke() {
	const unsigned int n_pts_per_obj= 6;

	DrawContext * context= _contexts["smoke"];
	context->_n_pts= n_pts_per_obj* _smoke_systems.size()* N_SMOKES_PER_CAR;
	context->_n_attrs_per_pts= 7;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;
	for (unsigned int idx_smoke_system=0; idx_smoke_system<_smoke_systems.size(); ++idx_smoke_system) {
		for (unsigned int idx_smoke=0; idx_smoke<N_SMOKES_PER_CAR; ++idx_smoke) {
			Smoke * smoke= _smoke_systems[idx_smoke_system]->_smokes[idx_smoke];
		
			// à cause du système de reference opengl il faut inverser les 0 et les 1 des y des textures
			data[compt++]= smoke->_bbox->_pts[0].x; data[compt++]= smoke->_bbox->_pts[0].y; 
			data[compt++]= smoke->_z; data[compt++]= 0.0; data[compt++]= 1.0; data[compt++]= smoke->_opacity; data[compt++]= smoke->_idx_texture;
			data[compt++]= smoke->_bbox->_pts[1].x; data[compt++]= smoke->_bbox->_pts[1].y; 
			data[compt++]= smoke->_z; data[compt++]= 1.0; data[compt++]= 1.0; data[compt++]= smoke->_opacity; data[compt++]= smoke->_idx_texture;
			data[compt++]= smoke->_bbox->_pts[2].x; data[compt++]= smoke->_bbox->_pts[2].y; 
			data[compt++]= smoke->_z; data[compt++]= 1.0; data[compt++]= 0.0; data[compt++]= smoke->_opacity; data[compt++]= smoke->_idx_texture;

			data[compt++]= smoke->_bbox->_pts[0].x; data[compt++]= smoke->_bbox->_pts[0].y;
			data[compt++]= smoke->_z; data[compt++]= 0.0; data[compt++]= 1.0; data[compt++]= smoke->_opacity; data[compt++]= smoke->_idx_texture;
			data[compt++]= smoke->_bbox->_pts[2].x; data[compt++]= smoke->_bbox->_pts[2].y;
			data[compt++]= smoke->_z; data[compt++]= 1.0; data[compt++]= 0.0; data[compt++]= smoke->_opacity; data[compt++]= smoke->_idx_texture;
			data[compt++]= smoke->_bbox->_pts[3].x; data[compt++]= smoke->_bbox->_pts[3].y;
			data[compt++]= smoke->_z; data[compt++]= 0.0; data[compt++]= 0.0; data[compt++]= smoke->_opacity; data[compt++]= smoke->_idx_texture;
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::update_tire_track() {
	const unsigned int n_pts_per_obj= 6;

	DrawContext * context= _contexts["tire_track"];
	context->_n_pts= n_pts_per_obj* N_MAX_TIRE_TRACKS;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	unsigned int compt= 0;
	for (unsigned int idx_tire_track=0; idx_tire_track<N_MAX_TIRE_TRACKS; ++idx_tire_track) {
		TireTrack * tt= _tire_track_system->_tracks[idx_tire_track];
		
		if (!tt->_is_alive) {
			for (int i=0; i<n_pts_per_obj* context->_n_attrs_per_pts; ++i) {
				data[compt+ i]= 0.0;
			}
			compt+= n_pts_per_obj* context->_n_attrs_per_pts;
			continue;
		}

		// à cause du système de reference opengl il faut inverser les 0 et les 1 des y des textures
		data[compt++]= tt->_bbox->_pts[0].x; data[compt++]= tt->_bbox->_pts[0].y; 
		data[compt++]= 0.0; data[compt++]= 1.0; data[compt++]= tt->_opacity; data[compt++]= tt->_idx_texture;
		data[compt++]= tt->_bbox->_pts[1].x; data[compt++]= tt->_bbox->_pts[1].y; 
		data[compt++]= 1.0; data[compt++]= 1.0; data[compt++]= tt->_opacity; data[compt++]= tt->_idx_texture;
		data[compt++]= tt->_bbox->_pts[2].x; data[compt++]= tt->_bbox->_pts[2].y; 
		data[compt++]= 1.0; data[compt++]= 0.0; data[compt++]= tt->_opacity; data[compt++]= tt->_idx_texture;

		data[compt++]= tt->_bbox->_pts[0].x; data[compt++]= tt->_bbox->_pts[0].y;
		data[compt++]= 0.0; data[compt++]= 1.0; data[compt++]= tt->_opacity; data[compt++]= tt->_idx_texture;
		data[compt++]= tt->_bbox->_pts[2].x; data[compt++]= tt->_bbox->_pts[2].y;
		data[compt++]= 1.0; data[compt++]= 0.0; data[compt++]= tt->_opacity; data[compt++]= tt->_idx_texture;
		data[compt++]= tt->_bbox->_pts[3].x; data[compt++]= tt->_bbox->_pts[3].y;
		data[compt++]= 0.0; data[compt++]= 0.0; data[compt++]= tt->_opacity; data[compt++]= tt->_idx_texture;
	}
	/*for (int i=0;i<context->_n_pts* context->_n_attrs_per_pts; ++i) {
		std::cout << data[i] << " ; ";
	}
	std::cout << "\n";*/
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::anim(std::chrono::system_clock::time_point t) {
	if (_mode== CHOOSE_TRACK) {

	}
	else if (_mode== RACING) {
		_track->anim(t, _input_state);

		for (auto smoke_system : _smoke_systems) {
			smoke_system->anim(t);
		}

		_tire_track_system->anim(t, _track->_sorted_cars);

		if (_draw_bbox) {
			update_bbox();
			update_footprint();
		}
		if (_draw_force) {
			update_force();
		}
		if (_draw_texture) {
			update_texture();
			update_smoke();
			update_tire_track();
		}

		camera();
	}
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


bool Racing::key_down(SDL_Keycode key, std::chrono::system_clock::time_point t) {
	if (_mode== CHOOSE_TRACK) {
		if (key== SDLK_LEFT) {
			_idx_chosen_track--;
			if (_idx_chosen_track< 1) {
				_idx_chosen_track= 1;
			}
			update_choose_track();
		}
		else if (key== SDLK_RIGHT) {
			_idx_chosen_track++;
			if (_idx_chosen_track> _n_available_tracks) {
				_idx_chosen_track= _n_available_tracks; // on commence à 1
			}
			update_choose_track();
		}
		else if (key== SDLK_RETURN) {
			choose_track(_idx_chosen_track, t);
		}
	}
	else if (_mode== RACING) {
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
			return true;
		}

		// t : dessiner textures
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

		// retour au choix des circuits
		else if (key== SDLK_RETURN) {
			_mode= CHOOSE_TRACK;
			return true;
		}
	}

	return false;
}
