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


Racing::Racing(std::map<std::string, GLuint> progs, ScreenGL * screengl, InputState * input_state, time_point t) :
	_draw_bbox(false), _draw_force(false), _draw_texture(true), _show_debug_info(false),
	_cam_mode(TRANSLATE), _screengl(screengl), _input_state(input_state),
	_mode(CHOOSE_DRIVER), _joystick_is_input(false), _help(false)
	{
	// caméras
	_camera2clip= glm::ortho(float(-screengl->_gl_width)* 0.5f, float(screengl->_gl_width)* 0.5f, -float(screengl->_gl_height)* 0.5f, float(screengl->_gl_height)* 0.5f, Z_NEAR, Z_FAR);
	_world2camera= glm::mat4(1.0);
	
	// font
	_font= new Font(progs["font"], "../../fonts/Silom.ttf", 48, screengl);
	_font->_z= 100.0f; // pour que l'affichage des infos se fassent par dessus le reste

	// aide
	std::ifstream ifs("../data/main_help.txt");
	std::string line;
	while (std::getline(ifs, line)) {
		_help_data.push_back(line);
	}

	// buffers
	uint n_buffers= 13;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	// textures
	uint n_textures= 8;
	_textures= new GLuint[n_textures];
	glGenTextures(n_textures, _textures);
	_texture_idx_model= 0;
	_texture_idx_bump= 1;
	_texture_idx_smoke= 2;
	_texture_idx_driver_face= 3;
	_texture_idx_choose_track= 4;
	_texture_idx_tire_track= 5;
	_texture_idx_map= 6;
	_texture_idx_water= 7;

	// contextes de dessin
	_contexts["bbox"]= new GLDrawContext(progs["simple"], _buffers[0],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["footprint"]= new GLDrawContext(progs["simple"], _buffers[1],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["force"]= new GLDrawContext(progs["simple"], _buffers[2],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["texture"]= new GLDrawContext(progs["texture"], _buffers[3],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in", "bump_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "texture_array", "texture_array_bump", "gray_blend"});

	_contexts["smoke"]= new GLDrawContext(progs["smoke"], _buffers[4],
		std::vector<std::string>{"position_in", "tex_coord_in", "alpha_in", "current_layer_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "texture_array"});

	_contexts["choose_driver"]= new GLDrawContext(progs["choose_track"], _buffers[5],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in", "selection_in"},
		std::vector<std::string>{"camera2clip_matrix", "texture_array"});

	_contexts["choose_track"]= new GLDrawContext(progs["choose_track"], _buffers[6],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in", "selection_in"},
		std::vector<std::string>{"camera2clip_matrix", "texture_array"});

	_contexts["tire_track"]= new GLDrawContext(progs["tire_track"], _buffers[7],
		std::vector<std::string>{"position_in", "tex_coord_in", "alpha_in", "current_layer_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "texture_array", "z"});

	_contexts["spark"]= new GLDrawContext(progs["spark"], _buffers[8],
		std::vector<std::string>{"position_in", "alpha_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["driver_face"]= new GLDrawContext(progs["driver_face"], _buffers[9],
		std::vector<std::string>{"position_in", "tex_coord_in", "alpha_in", "current_layer_in"},
		std::vector<std::string>{"camera2clip_matrix", "texture_array", "z"});

	_contexts["barrier"]= new GLDrawContext(progs["barrier"], _buffers[10],
		std::vector<std::string>{"position_in", "color_in", "lambda_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "z"});

	_contexts["map"]= new GLDrawContext(progs["map"], _buffers[11],
		std::vector<std::string>{"position_in", "tex_coord_in", "alpha_in", "current_layer_in"},
		std::vector<std::string>{"camera2clip_matrix", "texture_array"});

	_contexts["water"]= new GLDrawContext(progs["water"], _buffers[12],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in"},
		std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix", "texture_array", "gray_blend"});

	_track_info= new TrackInfo();
	_track= new Track();
	_tire_track_system= new TireTrackSystem();
	_spark_system= new SparkSystem();
	_water_system= new WaterSystem(_track->_grid->_cell_size);

	_idx_chosen_driver= 0;
	_idx_chosen_track= 1; // commence à 1 pas 0

	// récupération d'infos globale sur la piste courante (nombre de tours, records, ...)
	_track_info->parse_json("../data/tracks/track"+ std::to_string(_idx_chosen_track)+ ".json");

	// remplissage des GL_TEXTURE_2D_ARRAY
	fill_texture_array_models();
	fill_texture_array_smoke();
	fill_texture_array_tire_track();
	fill_texture_driver();
	fill_texture_choose_track();
	fill_texture_map();
	fill_texture_water();
}


Racing::~Racing() {
	delete _track;
	delete _track_info;
	for (auto ss : _smoke_systems) {
		delete ss;
	}
	_smoke_systems.clear();
	delete _tire_track_system;
	delete _spark_system;
	delete _water_system;
	for (auto context : _contexts) {
		delete context.second;
	}
	_contexts.clear();
	delete _buffers;
	delete _textures;
	delete _font;
}


/*void Racing::choose_driver(uint idx_driver) {
	_mode= CHOOSE_TRACK;
}*/


void Racing::choose_track(uint idx_track, time_point t) {
	// chargement json
	_track->load_json("../data/tracks/track"+ std::to_string(idx_track)+ ".json");

	// set hero
	_track->set_hero(_idx_chosen_driver);

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
	
	// reinit étincelles
	_spark_system->reinit();

	// eau background
	_water_system->set_track_grid(_track->_grid);

	// on récupère les records de la piste
	_track_lap_record= _track->_info->_best_lap[0].second;
	_track_overall_record= _track->_info->_best_overall[0].second;

	// init caméra
	_com_camera= _track->_hero->_com;

	// maj une fois pour toutes du dessin des barrières
	update_barrier();

	// et c'est parti
	_mode= RACING;
	_track->start(t);
}


void Racing::fill_texture_driver() {
	std::vector<std::string> pngs;
	uint compt= 0;
	for (auto driver : _track->_drivers) {
		for (auto expression : driver->_expressions) {
			for (auto expression_texture : expression.second->_textures) {
				pngs.push_back(expression_texture->_texture_path);
				expression_texture->_texture_idx= float(compt++);
			}
		}
	}
	fill_texture_array(0, _textures[_texture_idx_driver_face], 1024, pngs);
}


void Racing::fill_texture_choose_track() {
	std::vector<std::string> pngs= list_files("../data/tracks/quicklooks", "png");
	fill_texture_array(0, _textures[_texture_idx_choose_track], 1024, pngs);
	_n_available_tracks= pngs.size();
}


void Racing::fill_texture_array_models() {
	std::vector<std::string> pngs;
	std::vector<std::string> pngs_bump;
	uint compt= 0;
	for (auto m : _track->_models) {
		StaticObjectModel * model= m.second;
		std::vector<Action *> actions= model->get_unduplicated_actions();

		for (auto action : actions) {
			for (auto action_texture : action->_textures) {
				pngs.push_back(action_texture->_texture_path);
				pngs_bump.push_back(action_texture->_texture_path_bump);
				action_texture->_texture_idx= float(compt++);
			}
		}
	}
	fill_texture_array(0, _textures[_texture_idx_model], 1024, pngs);
	fill_texture_array(1, _textures[_texture_idx_bump], 1024, pngs_bump);
}


void Racing::fill_texture_array_smoke() {
	std::vector<std::string> pngs= list_files("../data/smoke", "png");
	fill_texture_array(0, _textures[_texture_idx_smoke], 1024, pngs);
}


void Racing::fill_texture_array_tire_track() {
	std::vector<std::string> pngs= list_files("../data/tire_track", "png");
	fill_texture_array(0, _textures[_texture_idx_tire_track], 1024, pngs);
	for (uint i=0; i<pngs.size(); ++i) {
		_track->_materials[basename(pngs[i])]->_tire_track_texture_idx= float(i);
	}
}


void Racing::fill_texture_map() {
	std::vector<std::string> pngs= list_files("../data/tracks/quicklooks", "png");
	for (auto driver : _track->_drivers) {
		for (auto expression : driver->_expressions) {
			for (auto expression_texture : expression.second->_textures) {
				pngs.push_back(expression_texture->_texture_path);
			}
		}
	}

	fill_texture_array(0, _textures[_texture_idx_map], 1024, pngs);
}


void Racing::fill_texture_water() {
	std::vector<std::string> pngs= list_files("../data/water", "png");
	fill_texture_array(0, _textures[_texture_idx_water], 1024, pngs);
	_water_system->set_pngs(pngs);
}


void Racing::draw_choose_driver() {
	GLDrawContext * context= _contexts["choose_driver"];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textures[_texture_idx_driver_face]);
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


void Racing::draw_choose_track() {
	GLDrawContext * context= _contexts["choose_track"];

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
	GLDrawContext * context= _contexts["bbox"];

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
	GLDrawContext * context= _contexts["footprint"];

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
	GLDrawContext * context= _contexts["force"];

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
	GLDrawContext * context= _contexts["texture"];

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
	GLDrawContext * context= _contexts["smoke"];

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
	GLDrawContext * context= _contexts["tire_track"];

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


void Racing::draw_spark() {
	GLDrawContext * context= _contexts["spark"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);

	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	glUniform1f(context->_locs_uniform["z"], Z_SPARK);

	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["alpha_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Racing::draw_driver_face() {
	GLDrawContext * context= _contexts["driver_face"];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textures[_texture_idx_driver_face]);
	glActiveTexture(0);

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);

	glUniform1i(context->_locs_uniform["texture_array"], 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniform1f(context->_locs_uniform["z"], Z_DRIVER_FACE);

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


void Racing::draw_barrier() {
	GLDrawContext * context= _contexts["barrier"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	glUniform1f(context->_locs_uniform["z"], Z_BARRIER);
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["lambda_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Racing::draw_map() {
	GLDrawContext * context= _contexts["map"];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textures[_texture_idx_map]);
	glActiveTexture(0);

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);

	glUniform1i(context->_locs_uniform["texture_array"], 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));

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


void Racing::draw_water() {
	GLDrawContext * context= _contexts["water"];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textures[_texture_idx_water]);
	glActiveTexture(0);

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);

	glUniform1i(context->_locs_uniform["texture_array"], 0); //Sampler refers to texture unit 0
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

	if (_help) {
		for (uint idx_line=0; idx_line<_help_data.size(); ++idx_line) {
			texts.push_back(Text(_help_data[idx_line], glm::vec2(-7.0f, 5.0f- float(idx_line)* 0.4), 0.006, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
		}
	}
	else if (_mode== CHOOSE_DRIVER) {
		texts.push_back(Text("touche h pour l'aide", glm::vec2(-7.0f, -5.0f), 0.004, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
		texts.push_back(Text(_track->_drivers[_idx_chosen_driver]->_name, glm::vec2(-1.5f, 4.0f), 0.02, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
	}
	else if (_mode== CHOOSE_TRACK) {
		texts.push_back(Text("touche h pour l'aide", glm::vec2(-7.0f, -5.0f), 0.004, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
		texts.push_back(Text("Track "+ std::to_string(_idx_chosen_track), glm::vec2(-1.5f, 4.0f), 0.02, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
		
		texts.push_back(Text("NLAPS = "+ std::to_string(_track_info->_n_laps), glm::vec2(4.0f, 4.0f), 0.01, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
		
		texts.push_back(Text("BEST LAP", glm::vec2(-7.0f, 4.5f), 0.005, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
		for (uint idx_best_lap=0; idx_best_lap<_track_info->_best_lap.size(); ++idx_best_lap) {
			std::ostringstream oss;
			oss << std::fixed << std::setprecision(2) << _track_info->_best_lap[idx_best_lap].second;
			texts.push_back(Text(_track_info->_best_lap[idx_best_lap].first+ " : "+ oss.str(), glm::vec2(-7.0f, 4.0f- float(idx_best_lap)* 0.5f), 0.005, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
		}
		
		texts.push_back(Text("BEST OVERALL", glm::vec2(-4.0f, 4.5f), 0.005, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
		for (uint idx_best_overall=0; idx_best_overall<_track_info->_best_overall.size(); ++idx_best_overall) {
			std::ostringstream oss;
			oss << std::fixed << std::setprecision(2) << _track_info->_best_overall[idx_best_overall].second;
			texts.push_back(Text(_track_info->_best_overall[idx_best_overall].first+ " : "+ oss.str(), glm::vec2(-4.0f, 4.0f- float(idx_best_overall)* 0.5f), 0.005, glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)));
		}
	}

	else if (_mode== RACING) {
		if (_show_debug_info) {
			const float font_scale= 0.005f;
			const glm::vec4 text_color(1.0, 1.0, 1.0, 1.0);

			texts.push_back(Text("COM PT", glm::vec2(-7.0f, 5.0f), font_scale, COM_CROSS_COLOR));
			texts.push_back(Text("FORCE_FWD PT", glm::vec2(-7.0f, 4.0f), font_scale, FORCE_FWD_CROSS_COLOR));
			texts.push_back(Text("FORCE_BWD PT", glm::vec2(-7.0f, 3.0f), font_scale, FORCE_BWD_CROSS_COLOR));
			texts.push_back(Text("FORCE_FWD VEC", glm::vec2(-7.0f, 2.0f), font_scale, FORCE_FWD_ARROW_COLOR));
			texts.push_back(Text("FORCE_BWD VEC", glm::vec2(-7.0f, 1.0f), font_scale, FORCE_BWD_ARROW_COLOR));
			texts.push_back(Text("ACCELERATION VEC", glm::vec2(-7.0f, 0.0f), font_scale, ACCELERATION_ARROW_COLOR));
			texts.push_back(Text("VELOCITY VEC", glm::vec2(-7.0f, -1.0f), font_scale, VELOCITY_ARROW_COLOR));
			texts.push_back(Text("FORWARD VEC", glm::vec2(-7.0f, -2.0f), font_scale, FORWARD_ARROW_COLOR));
			texts.push_back(Text("RIGHT VEC", glm::vec2(-7.0f, -3.0f), font_scale, RIGHT_ARROW_COLOR));

			texts.push_back(Text("thrust="+ std::to_string(_track->_hero->_thrust), glm::vec2(4.0f, 5.0f), font_scale, text_color));
			texts.push_back(Text("wheel="+ std::to_string(_track->_hero->_wheel), glm::vec2(4.0f, 4.0f), font_scale, text_color));
			texts.push_back(Text("force_fwd="+ std::to_string(norm(_track->_hero->_force_fwd)), glm::vec2(4.0f, 3.0f), font_scale, text_color));
			texts.push_back(Text("force_bwd="+ std::to_string(norm(_track->_hero->_force_bwd)), glm::vec2(4.0f, 2.0f), font_scale, text_color));
			texts.push_back(Text("acc="+ std::to_string(norm(_track->_hero->_acceleration)), glm::vec2(4.0f, 1.0f), font_scale, text_color));
			texts.push_back(Text("vel="+ std::to_string(norm(_track->_hero->_velocity)), glm::vec2(4.0f, 0.0f), font_scale, text_color));
			texts.push_back(Text("torque="+ std::to_string(_track->_hero->_torque), glm::vec2(4.0f, -1.0f), font_scale, text_color));
			texts.push_back(Text("ang acc="+ std::to_string(_track->_hero->_angular_acceleration), glm::vec2(4.0f, -2.0f), font_scale, text_color));
			texts.push_back(Text("ang vel="+ std::to_string(_track->_hero->_angular_velocity), glm::vec2(4.0f, -3.0f), font_scale, text_color));
			texts.push_back(Text("alpha="+ std::to_string(_track->_hero->_alpha), glm::vec2(4.0f, -4.0f), font_scale, text_color));
			texts.push_back(Text("drift="+ std::to_string(_track->_hero->_drift), glm::vec2(4.0f, -5.0f), font_scale, text_color));
		}

		else {
			// 3, 2, 1
			if (_track->_mode== TRACK_PRECOUNT) {
				texts.push_back(Text(std::to_string(_track->_precount), glm::vec2(-1.0f, 0.0f), 0.06f, glm::vec4(1.0f, 1.0f, 0.5f, 1.0f)));
			}
			
			// nombre de tours
			if (_track->_mode== TRACK_LIVE) {
				glm::vec4 nlaps_color(NLAPS_COLOR);
				if (_track->_hero->_n_laps== _track->_info->_n_laps) {
					nlaps_color= NLAPS_LAST_COLOR;
				}
				texts.push_back(Text("LAP "+ std::to_string(_track->_hero->_n_laps)+ " / "+ std::to_string(_track->_info->_n_laps), glm::vec2(4.5f, 4.5f), 0.01f, nlaps_color));
			}

			// vitesse joueur
			if (_track->_mode== TRACK_LIVE) {
				// on multiplie par un facteur qui rende le truc cohérent
				int hero_speed= (int)(_track->_hero->_speed* 60.0);
				glm::vec4 speed_color(LOW_SPEED_COLOR);
				if (hero_speed> 200) {
					speed_color= HIGH_SPEED_COLOR;
				}
				std::string speed_str= std::to_string(hero_speed);
				texts.push_back(Text(std::to_string(hero_speed), glm::vec2(4.5f, -5.0f), 0.01f, speed_color));
				texts.push_back(Text(" km/h", glm::vec2(5.5f, -5.0f), 0.01f, speed_color));
			}

			// adversaires
			if (_track->_mode== TRACK_LIVE || _track->_mode== TRACK_PRECOUNT) {
				for (uint idx_car=0; idx_car<_track->_sorted_cars.size(); ++idx_car) {
					Car * car= _track->_sorted_cars[idx_car];
					glm::vec4 position= _world2camera* glm::vec4(float(car->_com.x), float(car->_com.y), float(car->_z), 1.0f);
					glm::vec4 color= RANKING_ENNEMY_COLOR;
					if (car== _track->_hero) {
						color= RANKING_HERO_COLOR;
					}
					float scale;
					if (idx_car< IN_RACE_TEXT_SCALE.size()) {
						scale= IN_RACE_TEXT_SCALE[idx_car];
					}
					else {
						scale= IN_RACE_TEXT_SCALE.back();
					}
					texts.push_back(Text(std::to_string(car->_rank)+  " - "+ car->_driver->_name, glm::vec2(position.x, position.y)+ IN_RACE_TEXT_OFFSET, scale, color));
				}
			}

			// classement toutes voitures
			if (_track->_mode== TRACK_LIVE || _track->_mode== TRACK_FINISHED) {
				float x0= RANKING_ORIGIN.x;
				float y0= RANKING_ORIGIN.y- RANKING_FACE_TEXT_MARGIN;

				for (uint idx_car=0; idx_car<_track->_sorted_cars.size(); ++idx_car) {
					float face_size, x_inc, scale;
					if (idx_car< RANKING_FACE_SIZE.size()) {
						face_size= RANKING_FACE_SIZE[idx_car];
						if (idx_car< RANKING_FACE_SIZE.size()- 1) {
							x_inc= 0.5f* (RANKING_FACE_SIZE[idx_car]- RANKING_FACE_SIZE[idx_car+ 1]);
						}
						else {
							x_inc= 0.0f;
						}
						scale= RANKING_TEXT_SCALE[idx_car];
					}
					else {
						face_size= RANKING_FACE_SIZE.back();
						x_inc= 0.0f;
						scale= RANKING_TEXT_SCALE.back();
					}
					Car * car= _track->_sorted_cars[idx_car];
					glm::vec4 ranking_color(RANKING_ENNEMY_COLOR);
					if (car== _track->_hero) {
						ranking_color= RANKING_HERO_COLOR;
					}
					texts.push_back(Text(std::to_string(car->_rank)+  " - "+ car->_driver->_name, glm::vec2(x0, y0), scale, ranking_color));
					
					x0+= x_inc;
					y0-= face_size* RANKING_FACE_MARGIN_FACTOR;
				}
			}

			// temps tours joueur
			if (_track->_mode== TRACK_LIVE || _track->_mode== TRACK_FINISHED) {
				for (uint idx_time=0; idx_time<_track->_hero->_lap_times.size(); ++idx_time) {
					glm::vec4 lap_time_color(PAST_LAP_TIME_COLOR);
					if (idx_time== _track->_hero->_lap_times.size()- 1) {
						lap_time_color= CURRENT_LAP_TIME_COLOR;
					}
					std::ostringstream oss;
					oss << std::fixed << std::setprecision(2) << _track->_hero->_lap_times[idx_time];
					texts.push_back(Text("LAP "+ std::to_string(idx_time+ 1)+  " - "+ oss.str(), glm::vec2(4.5f, 4.0f- float(idx_time)* 0.5f), 0.005f, lap_time_color));

					// affichage diff par tour avec record du tour
					if (idx_time< _track->_hero->_lap_times.size()- 1 || _track->_hero->_finished) {
						number diff_best_lap= _track->_hero->_lap_times[idx_time]- _track_lap_record;
						glm::vec4 diff_lap_time_color(NOT_BEST_LAP_TIME_COLOR);
						if (diff_best_lap< 0.0) {
							diff_lap_time_color= BEST_LAP_TIME_COLOR;
						}
						std::ostringstream oss2;
						oss2 << std::fixed << std::setprecision(2) << diff_best_lap;
						std::string sign= "";
						if (diff_best_lap> 0.0) {
							sign= "+";
						}
						texts.push_back(Text(sign+ oss2.str(), glm::vec2(6.5f, 4.0f- float(idx_time)* 0.5f), 0.005f, diff_lap_time_color));
					}
				}
			}

			// temps total joueur arrivée
			if (_track->_mode== TRACK_FINISHED) {
				std::ostringstream oss;
				oss << std::fixed << std::setprecision(2) << _track->_hero->_total_time;
				texts.push_back(Text("TOTAL - "+ oss.str(), glm::vec2(4.5f, 3.0f- 0.5f* float(_track->_info->_n_laps+ 1)), 0.005f, TOTAL_LAP_TIME_COLOR));

				// affichage diff avec record
				number diff_best_overall= _track->_hero->_total_time- _track_overall_record;
				glm::vec4 diff_lap_time_color(NOT_BEST_LAP_TIME_COLOR);
				if (diff_best_overall< 0.0) {
					diff_lap_time_color= BEST_LAP_TIME_COLOR;
				}
				std::ostringstream oss2;
				oss2 << std::fixed << std::setprecision(2) << diff_best_overall;
				std::string sign= "";
				if (diff_best_overall> 0.0) {
					sign= "+";
				}
				texts.push_back(Text(sign+ oss2.str(), glm::vec2(6.5f, 3.0f- 0.5f* float(_track->_info->_n_laps+ 1)), 0.005f, diff_lap_time_color));
			}

			// classement joueur arrivée
			if (_track->_mode== TRACK_FINISHED) {
				texts.push_back(Text(std::to_string(_track->_hero->_rank) + " / "+ std::to_string(_track->_sorted_cars.size()), glm::vec2(-2.0f, 0.0f), 0.03f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)));
			}

			// y a t'il un record ?
			if (_track->_mode== TRACK_FINISHED) {
				if (_track->_new_best_lap) {
					texts.push_back(Text("RECORD DU TOUR !", glm::vec2(-4.5f, 3.0f), 0.02f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)));
				}
				if (_track->_new_best_overall) {
					texts.push_back(Text("RECORD PISTE !", glm::vec2(-4.0f, 2.0f), 0.02f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)));
				}
			}
		}
	}

	_font->set_text(texts);
	_font->draw();
}


void Racing::draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (_mode== CHOOSE_DRIVER && !_help) {
		draw_choose_driver();
	}
	else if (_mode== CHOOSE_TRACK && !_help) {
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
			draw_spark();
			draw_driver_face();
			draw_barrier();
			draw_map();
			draw_water();
		}
	}

	show_info();
}


void Racing::update_choose_driver() {
	const uint n_pts_per_obj= 6;
	const float MARGIN= 1.0;

	GLDrawContext * context= _contexts["choose_driver"];
	context->_n_pts= _track->_drivers.size()* n_pts_per_obj;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	float size= (_screengl->_gl_width- float(_track->_drivers.size()+ 1)* MARGIN)/ float(_track->_drivers.size());
	uint compt= 0;
	float selection, texture_idx;
	for (uint idx_driver=0; idx_driver<_track->_drivers.size(); ++idx_driver) {
		if (idx_driver== _idx_chosen_driver) {
			selection= 0.2f;
		}
		else {
			selection= 0.0f;
		}
		texture_idx= _track->_drivers[idx_driver]->_expressions["normal"]->_textures[0]->_texture_idx;

		data[compt++]= -0.5f* _screengl->_gl_width+ MARGIN+ float(idx_driver)* (size+ MARGIN); data[compt++]= -0.5f* size;
		data[compt++]= 0.0f; data[compt++]= 1.0f; data[compt++]= texture_idx; data[compt++]= selection;
		data[compt++]= -0.5f* _screengl->_gl_width+ MARGIN+ float(idx_driver)* (size+ MARGIN)+ size; data[compt++]= -0.5f* size;
		data[compt++]= 1.0f; data[compt++]= 1.0f; data[compt++]= texture_idx; data[compt++]= selection;
		data[compt++]= -0.5f* _screengl->_gl_width+ MARGIN+ float(idx_driver)* (size+ MARGIN)+ size; data[compt++]= 0.5f* size;
		data[compt++]= 1.0f; data[compt++]= 0.0f; data[compt++]= texture_idx; data[compt++]= selection;
		
		data[compt++]= -0.5f* _screengl->_gl_width+ MARGIN+ float(idx_driver)* (size+ MARGIN); data[compt++]= -0.5f* size;
		data[compt++]= 0.0f; data[compt++]= 1.0f; data[compt++]= texture_idx; data[compt++]= selection;
		data[compt++]= -0.5f* _screengl->_gl_width+ MARGIN+ float(idx_driver)* (size+ MARGIN)+ size; data[compt++]= 0.5f* size;
		data[compt++]= 1.0f; data[compt++]= 0.0f; data[compt++]= texture_idx; data[compt++]= selection;
		data[compt++]= -0.5f* _screengl->_gl_width+ MARGIN+ float(idx_driver)* (size+ MARGIN); data[compt++]= 0.5f* size;
		data[compt++]= 0.0f; data[compt++]= 0.0f; data[compt++]= texture_idx; data[compt++]= selection;
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::update_choose_track() {
	const uint n_pts_per_obj= 6;
	const float MARGIN= 1.0;

	GLDrawContext * context= _contexts["choose_track"];
	context->_n_pts= _n_available_tracks* n_pts_per_obj;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	float size= (_screengl->_gl_width- float(_n_available_tracks+ 1)* MARGIN)/ float(_n_available_tracks);
	uint compt= 0;
	float selection;
	for (uint i=0; i<_n_available_tracks; ++i) {
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
	const uint n_pts_per_obj= 8;

	GLDrawContext * context= _contexts["bbox"];
	context->_n_pts= n_pts_per_obj* (_track->_grid->_objects.size()+ _track->_floating_objects.size());
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	float * ptr= data;
	for (auto obj : _track->_grid->_objects) {
		std::vector<pt_2d> pts(obj->_bbox->_pts, obj->_bbox->_pts+ 4);
		ptr= draw_polygon(ptr, pts, BBOX_COLOR);
	}
	for (auto obj : _track->_floating_objects) {
		std::vector<pt_2d> pts(obj->_bbox->_pts, obj->_bbox->_pts+ 4);
		ptr= draw_polygon(ptr, pts, BBOX_COLOR);
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::update_footprint() {
	GLDrawContext * context= _contexts["footprint"];
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

	uint compt= 0;
	
	for (auto obj : _track->_floating_objects) {
		for (auto tri : obj->_footprint->_triangles_idx) {
			for (int i=0; i<3; ++i) {
				pt_2d pt= obj->_footprint->_pts[tri[i]];
				data[compt++]= float(pt.x);
				data[compt++]= float(pt.y);
				compt+= 4;
			}
		}
	}

	for (auto obj : _track->_grid->_objects) {
		for (auto tri : obj->_footprint->_triangles_idx) {
			for (int i=0; i<3; ++i) {
				pt_2d pt= obj->_footprint->_pts[tri[i]];
				data[compt++]= float(pt.x);
				data[compt++]= float(pt.y);

				compt+= 4;
			}
		}
	}

	for (uint idx_pt=0; idx_pt<context->_n_pts; ++idx_pt) {
		for (uint idx_color=0; idx_color<4; ++idx_color) {
			data[idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= FOOTPRINT_COLOR[idx_color];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::update_force() {
	const uint n_pts_per_cross= 4;
	const uint n_pts_per_arrow= 6;
	const uint n_pts_per_obj= 3* n_pts_per_cross+ 6* n_pts_per_arrow; // 3 croix ; 6 fleches

	GLDrawContext * context= _contexts["force"];
	context->_n_pts= n_pts_per_obj* (_track->_floating_objects.size()+ _track->_grid->_objects.size());
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr= data;

	// dessins des forces; voir gl_utils
	for (auto obj : _track->_floating_objects) {
		if (obj->_model->_type== CAR) {
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
	const uint n_pts_per_obj= 12;

	uint n_grid_objects= _track->_grid->_objects.size();
	uint n_floating_objects= _track->_floating_objects.size();

	GLDrawContext * context= _contexts["texture"];
	context->_n_pts= n_pts_per_obj* (n_grid_objects+ n_floating_objects);
	context->_n_attrs_per_pts= 7;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	for (uint idx_obj=0; idx_obj<n_grid_objects+ n_floating_objects; ++idx_obj) {
		StaticObject * obj;

		if (idx_obj< n_grid_objects) {
			obj= _track->_grid->_objects[idx_obj];
		}
		else if (idx_obj< n_grid_objects+ n_floating_objects) {
			obj= _track->_floating_objects[idx_obj- n_grid_objects];
		}
		Action * action= obj->get_current_action();
		
		// on prend + de points que d'ahbitude pour les bumps car
		/*number positions[n_pts_per_obj* 5]= {
			obj->_bbox->_pts[0].x, obj->_bbox->_pts[0].y, obj->_z, 0.0, 1.0,
			obj->_bbox->_pts[1].x, obj->_bbox->_pts[1].y, obj->_z, 1.0, 1.0,
			obj->_bbox->_pts[2].x, obj->_bbox->_pts[2].y, obj->_z, 1.0, 0.0,

			obj->_bbox->_pts[0].x, obj->_bbox->_pts[0].y, obj->_z, 0.0, 1.0,
			obj->_bbox->_pts[2].x, obj->_bbox->_pts[2].y, obj->_z, 1.0, 0.0,
			obj->_bbox->_pts[3].x, obj->_bbox->_pts[3].y, obj->_z, 0.0, 0.0
		};*/

		number left_u= 0.0; number right_u= 1.0;
		if (obj->_flipped) {
			left_u= 1.0;
			right_u= 0.0;
		}
		
		// à cause du système de reference opengl il faut inverser les 0 et les 1 des y des textures
		number positions[n_pts_per_obj* 6]= {
			obj->_bbox->_pts[0].x, obj->_bbox->_pts[0].y, obj->_z, left_u, 1.0, obj->_bumps[0],
			obj->_bbox->_pts[1].x, obj->_bbox->_pts[1].y, obj->_z, right_u, 1.0, obj->_bumps[1],
			obj->_bbox->_center.x, obj->_bbox->_center.y, obj->_z, 0.5, 0.5, obj->_bumps[8],

			obj->_bbox->_pts[1].x, obj->_bbox->_pts[1].y, obj->_z, right_u, 1.0, obj->_bumps[2],
			obj->_bbox->_pts[2].x, obj->_bbox->_pts[2].y, obj->_z, right_u, 0.0, obj->_bumps[3],
			obj->_bbox->_center.x, obj->_bbox->_center.y, obj->_z, 0.5, 0.5, obj->_bumps[8],

			obj->_bbox->_pts[2].x, obj->_bbox->_pts[2].y, obj->_z, right_u, 0.0, obj->_bumps[4],
			obj->_bbox->_pts[3].x, obj->_bbox->_pts[3].y, obj->_z, left_u, 0.0, obj->_bumps[5],
			obj->_bbox->_center.x, obj->_bbox->_center.y, obj->_z, 0.5, 0.5, obj->_bumps[8],

			obj->_bbox->_pts[3].x, obj->_bbox->_pts[3].y, obj->_z, left_u, 0.0, obj->_bumps[6],
			obj->_bbox->_pts[0].x, obj->_bbox->_pts[0].y, obj->_z, left_u, 1.0, obj->_bumps[7],
			obj->_bbox->_center.x, obj->_bbox->_center.y, obj->_z, 0.5, 0.5, obj->_bumps[8]
		};

		for (uint idx_pt=0; idx_pt<n_pts_per_obj; ++idx_pt) {
			for (uint i=0; i<6; ++i) {
				data[idx_obj* n_pts_per_obj* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ i]= float(positions[6* idx_pt+ i]);
			}
			data[idx_obj* n_pts_per_obj* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 6]= action->_textures[obj->_current_action_texture_idx]->_texture_idx;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::update_smoke() {
	const uint n_pts_per_obj= 6;

	GLDrawContext * context= _contexts["smoke"];
	context->_n_pts= n_pts_per_obj* _smoke_systems.size()* N_SMOKES_PER_CAR;
	context->_n_attrs_per_pts= 7;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	uint compt= 0;
	for (uint idx_smoke_system=0; idx_smoke_system<_smoke_systems.size(); ++idx_smoke_system) {
		for (uint idx_smoke=0; idx_smoke<N_SMOKES_PER_CAR; ++idx_smoke) {
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
	const uint n_pts_per_obj= 6;

	GLDrawContext * context= _contexts["tire_track"];
	context->_n_pts= n_pts_per_obj* N_MAX_TIRE_TRACKS;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	uint compt= 0;
	for (uint idx_tire_track=0; idx_tire_track<N_MAX_TIRE_TRACKS; ++idx_tire_track) {
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

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::update_spark() {
	const uint n_pts_per_obj= 6;

	GLDrawContext * context= _contexts["spark"];
	context->_n_pts= n_pts_per_obj* N_MAX_SPARKS;
	context->_n_attrs_per_pts= 3;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	uint compt= 0;
	for (uint idx_spark=0; idx_spark<N_MAX_SPARKS; ++idx_spark) {
		Spark * spark= _spark_system->_sparks[idx_spark];
		
		if (!spark->_is_alive) {
			for (int i=0; i<n_pts_per_obj* context->_n_attrs_per_pts; ++i) {
				data[compt+ i]= 0.0;
			}
			compt+= n_pts_per_obj* context->_n_attrs_per_pts;
			continue;
		}

		data[compt++]= spark->_bbox->_pts[0].x; data[compt++]= spark->_bbox->_pts[0].y; data[compt++]= spark->_opacity;
		data[compt++]= spark->_bbox->_pts[1].x; data[compt++]= spark->_bbox->_pts[1].y; data[compt++]= spark->_opacity;
		data[compt++]= spark->_bbox->_pts[2].x; data[compt++]= spark->_bbox->_pts[2].y; data[compt++]= spark->_opacity;

		data[compt++]= spark->_bbox->_pts[0].x; data[compt++]= spark->_bbox->_pts[0].y; data[compt++]= spark->_opacity;
		data[compt++]= spark->_bbox->_pts[2].x; data[compt++]= spark->_bbox->_pts[2].y; data[compt++]= spark->_opacity;
		data[compt++]= spark->_bbox->_pts[3].x; data[compt++]= spark->_bbox->_pts[3].y; data[compt++]= spark->_opacity;
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::update_driver_face() {
	const uint n_pts_per_obj= 6;

	GLDrawContext * context= _contexts["driver_face"];
	context->_n_pts= _track->_drivers.size()* 2* n_pts_per_obj; // * 2 : 1 pour le ranking, 1 pour dans la course
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	uint compt= 0;

	// ranking ----------------------------
	float x0= RANKING_ORIGIN.x;
	float y0= RANKING_ORIGIN.y;
	for (uint idx_car=0; idx_car<_track->_sorted_cars.size(); ++idx_car) {
		Car * car= _track->_sorted_cars[idx_car];
		Driver * driver= car->_driver;

		float face_size, alpha, x_inc, texture_idx;
		if (idx_car< RANKING_FACE_SIZE.size()) {
			face_size= RANKING_FACE_SIZE[idx_car];
			alpha= RANKING_FACE_ALPHA[idx_car];
			if (idx_car< RANKING_FACE_SIZE.size()- 1) {
				x_inc= 0.5f* (RANKING_FACE_SIZE[idx_car]- RANKING_FACE_SIZE[idx_car+ 1]);
			}
			else {
				x_inc= 0.0f;
			}
		}
		else {
			face_size= RANKING_FACE_SIZE.back();
			alpha= RANKING_FACE_ALPHA.back();
			x_inc= 0.0f;
		}

		texture_idx= driver->_expressions[driver->_current_expression_name]->_textures[driver->_current_expression_texture_idx]->_texture_idx;

		data[compt++]= x0; data[compt++]= y0; data[compt++]= 0.0; data[compt++]= 1.0; data[compt++]= alpha; data[compt++]= texture_idx;
		data[compt++]= x0+ face_size; data[compt++]= y0; data[compt++]= 1.0; data[compt++]= 1.0; data[compt++]= alpha; data[compt++]= texture_idx;
		data[compt++]= x0+ face_size; data[compt++]= y0+ face_size; data[compt++]= 1.0; data[compt++]= 0.0; data[compt++]= alpha; data[compt++]= texture_idx;

		data[compt++]= x0; data[compt++]= y0; data[compt++]= 0.0; data[compt++]= 1.0; data[compt++]= alpha; data[compt++]= texture_idx;
		data[compt++]= x0+ face_size; data[compt++]= y0+ face_size; data[compt++]= 1.0; data[compt++]= 0.0; data[compt++]= alpha; data[compt++]= texture_idx;
		data[compt++]= x0; data[compt++]= y0+ face_size; data[compt++]= 0.0; data[compt++]= 0.0; data[compt++]= alpha; data[compt++]= texture_idx;

		x0+= x_inc;
		y0-= face_size* RANKING_FACE_MARGIN_FACTOR;
	}

	// dans la course
	for (uint idx_car=0; idx_car<_track->_sorted_cars.size(); ++idx_car) {
		Car * car= _track->_sorted_cars[idx_car];
		Driver * driver= car->_driver;
		glm::vec4 position= _world2camera* glm::vec4(float(car->_com.x), float(car->_com.y), float(car->_z), 1.0f);
		float face_size, alpha, texture_idx;
		if (idx_car< IN_RACE_FACE_SIZE.size()) {
			face_size= IN_RACE_FACE_SIZE[idx_car];
			alpha= IN_RACE_FACE_ALPHA[idx_car];
		}
		else {
			face_size= IN_RACE_FACE_SIZE.back();
			alpha= IN_RACE_FACE_ALPHA.back();
		}

		float x0= position.x+ IN_RACE_FACE_OFFSET.x;
		float y0= position.y+ IN_RACE_FACE_OFFSET.y;
		
		texture_idx= driver->_expressions[driver->_current_expression_name]->_textures[driver->_current_expression_texture_idx]->_texture_idx;

		data[compt++]= x0; data[compt++]= y0; data[compt++]= 0.0; data[compt++]= 1.0; data[compt++]= alpha; data[compt++]= texture_idx;
		data[compt++]= x0+ face_size; data[compt++]= y0; data[compt++]= 1.0; data[compt++]= 1.0; data[compt++]= alpha; data[compt++]= texture_idx;
		data[compt++]= x0+ face_size; data[compt++]= y0+ face_size; data[compt++]= 1.0; data[compt++]= 0.0; data[compt++]= alpha; data[compt++]= texture_idx;

		data[compt++]= x0; data[compt++]= y0; data[compt++]= 0.0; data[compt++]= 1.0; data[compt++]= alpha; data[compt++]= texture_idx;
		data[compt++]= x0+ face_size; data[compt++]= y0+ face_size; data[compt++]= 1.0; data[compt++]= 0.0; data[compt++]= alpha; data[compt++]= texture_idx;
		data[compt++]= x0; data[compt++]= y0+ face_size; data[compt++]= 0.0; data[compt++]= 0.0; data[compt++]= alpha; data[compt++]= texture_idx;
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::update_barrier() {
	const uint n_pts_per_obj= 6;

	GLDrawContext * context= _contexts["barrier"];
	uint n_barriers= 0;
	for (auto poly : _track->_barriers) {
		n_barriers+= poly.size();
	}

	context->_n_pts= n_pts_per_obj* n_barriers;
	context->_n_attrs_per_pts= 7;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	const glm::vec4 BARRIER_COLOR(1.0, 1.0, 1.0, 0.8);
	const number BARRIER_WIDTH= 0.1;

	uint compt= 0;
	for (auto poly : _track->_barriers) {
		for (uint idx_pt=0; idx_pt<poly.size(); ++idx_pt) {
			pt_2d pt1(poly[idx_pt]);
			pt_2d pt2;
			if (idx_pt< poly.size()- 1) {
				pt2= pt_2d(poly[idx_pt+ 1]);
			}
			else {
				pt2= pt_2d(poly[0]);
			}
			pt_2d vrot90= pt_2d(pt1.y- pt2.y, pt2.x- pt1.x);
			vrot90= normalized(vrot90);
			pt_2d p0= pt1- 0.5* BARRIER_WIDTH* vrot90;
			pt_2d p1= pt2- 0.5* BARRIER_WIDTH* vrot90;
			pt_2d p2= pt2+ 0.5* BARRIER_WIDTH* vrot90;
			pt_2d p3= pt1+ 0.5* BARRIER_WIDTH* vrot90;
			
			data[compt++]= p0.x; data[compt++]= p0.y; data[compt++]= 0.0f; compt+= 4;
			data[compt++]= p1.x; data[compt++]= p1.y; data[compt++]= 0.0f; compt+= 4;
			data[compt++]= p2.x; data[compt++]= p2.y; data[compt++]= 1.0f; compt+= 4;
			data[compt++]= p0.x; data[compt++]= p0.y; data[compt++]= 0.0f; compt+= 4;
			data[compt++]= p2.x; data[compt++]= p2.y; data[compt++]= 1.0f; compt+= 4;
			data[compt++]= p3.x; data[compt++]= p3.y; data[compt++]= 1.0f; compt+= 4;
		}
	}
	for (int i=0; i<context->_n_pts; ++i) {
		data[i* context->_n_attrs_per_pts+ 3]= BARRIER_COLOR[0];
		data[i* context->_n_attrs_per_pts+ 4]= BARRIER_COLOR[1];
		data[i* context->_n_attrs_per_pts+ 5]= BARRIER_COLOR[2];
		data[i* context->_n_attrs_per_pts+ 6]= BARRIER_COLOR[3];
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::update_map() {
	const uint n_pts_per_obj= 6;

	GLDrawContext * context= _contexts["map"];
	context->_n_pts= (1+ _track->_drivers.size())* n_pts_per_obj;
	context->_n_attrs_per_pts= 7;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	uint compt= 0;

	float xmap0= MAP_ORIGIN.x; float ymap0= MAP_ORIGIN.y;
	float xmap1= MAP_ORIGIN.x+ MAP_SIZE.x; float ymap1= MAP_ORIGIN.y;
	float xmap2= MAP_ORIGIN.x+ MAP_SIZE.x; float ymap2= MAP_ORIGIN.y+ MAP_SIZE.y;
	float xmap3= MAP_ORIGIN.x; float ymap3= MAP_ORIGIN.y+ MAP_SIZE.y;

	data[compt++]= xmap0; data[compt++]= ymap0; data[compt++]= Z_MAP; data[compt++]= 0.0; data[compt++]= 1.0; data[compt++]= MAP_OPACITY; data[compt++]= _idx_chosen_track- 1;
	data[compt++]= xmap1; data[compt++]= ymap1; data[compt++]= Z_MAP; data[compt++]= 1.0; data[compt++]= 1.0; data[compt++]= MAP_OPACITY; data[compt++]= _idx_chosen_track- 1;
	data[compt++]= xmap2; data[compt++]= ymap2; data[compt++]= Z_MAP; data[compt++]= 1.0; data[compt++]= 0.0; data[compt++]= MAP_OPACITY; data[compt++]= _idx_chosen_track- 1;
	data[compt++]= xmap0; data[compt++]= ymap0; data[compt++]= Z_MAP; data[compt++]= 0.0; data[compt++]= 1.0; data[compt++]= MAP_OPACITY; data[compt++]= _idx_chosen_track- 1;
	data[compt++]= xmap2; data[compt++]= ymap2; data[compt++]= Z_MAP; data[compt++]= 1.0; data[compt++]= 0.0; data[compt++]= MAP_OPACITY; data[compt++]= _idx_chosen_track- 1;
	data[compt++]= xmap3; data[compt++]= ymap3; data[compt++]= Z_MAP; data[compt++]= 0.0; data[compt++]= 0.0; data[compt++]= MAP_OPACITY; data[compt++]= _idx_chosen_track- 1;
	
	for (uint idx_car=0; idx_car<_track->_sorted_cars.size(); ++idx_car) {
		Car * car= _track->_sorted_cars[idx_car];
		Driver * driver= car->_driver;

		float face_size, texture_idx, x0, y0;
		if (idx_car< MAP_FACE_SIZE.size()) {
			face_size= MAP_FACE_SIZE[idx_car];
		}
		else {
			face_size= MAP_FACE_SIZE.back();
		}
		x0= MAP_ORIGIN.x+ MAP_SIZE.x* car->_com.x/ (number(_track->_grid->_width)* _track->_grid->_cell_size)- 0.5* face_size;
		y0= MAP_ORIGIN.y+ MAP_SIZE.y* car->_com.y/ (number(_track->_grid->_height)* _track->_grid->_cell_size)- 0.5* face_size;

		texture_idx= _n_available_tracks+ driver->_expressions[driver->_current_expression_name]->_textures[driver->_current_expression_texture_idx]->_texture_idx;

		data[compt++]= x0; data[compt++]= y0; 			 			data[compt++]= Z_MAP+ 0.1f; data[compt++]= 0.0; data[compt++]= 1.0; data[compt++]= MAP_FACE_OPACITY; data[compt++]= texture_idx;
		data[compt++]= x0+ face_size; data[compt++]= y0; 			data[compt++]= Z_MAP+ 0.1f; data[compt++]= 1.0; data[compt++]= 1.0; data[compt++]= MAP_FACE_OPACITY; data[compt++]= texture_idx;
		data[compt++]= x0+ face_size; data[compt++]= y0+ face_size; data[compt++]= Z_MAP+ 0.1f; data[compt++]= 1.0; data[compt++]= 0.0; data[compt++]= MAP_FACE_OPACITY; data[compt++]= texture_idx;

		data[compt++]= x0; data[compt++]= y0; 						data[compt++]= Z_MAP+ 0.1f; data[compt++]= 0.0; data[compt++]= 1.0; data[compt++]= MAP_FACE_OPACITY; data[compt++]= texture_idx;
		data[compt++]= x0+ face_size; data[compt++]= y0+ face_size; data[compt++]= Z_MAP+ 0.1f; data[compt++]= 1.0; data[compt++]= 0.0; data[compt++]= MAP_FACE_OPACITY; data[compt++]= texture_idx;
		data[compt++]= x0; data[compt++]= y0+ face_size; 			data[compt++]= Z_MAP+ 0.1f; data[compt++]= 0.0; data[compt++]= 0.0; data[compt++]= MAP_FACE_OPACITY; data[compt++]= texture_idx;
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::update_water() {
	const uint n_pts_per_obj= 6;

	GLDrawContext * context= _contexts["water"];
	context->_n_pts= _water_system->_tiles.size()* n_pts_per_obj;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	uint compt= 0;
	for (auto tile : _water_system->_tiles) {
		data[compt++]= tile->_aabb->_pos.x; data[compt++]= tile->_aabb->_pos.y; data[compt++]= tile->_z; data[compt++]= 0.0; data[compt++]= 1.0; data[compt++]= float(tile->_idx_texture);
		data[compt++]= tile->_aabb->_pos.x+ tile->_aabb->_size.x; data[compt++]= tile->_aabb->_pos.y; data[compt++]= tile->_z; data[compt++]= 1.0; data[compt++]= 1.0; data[compt++]= float(tile->_idx_texture);
		data[compt++]= tile->_aabb->_pos.x+ tile->_aabb->_size.x; data[compt++]= tile->_aabb->_pos.y+ tile->_aabb->_size.y; data[compt++]= tile->_z; data[compt++]= 1.0; data[compt++]= 0.0; data[compt++]= float(tile->_idx_texture);

		data[compt++]= tile->_aabb->_pos.x; data[compt++]= tile->_aabb->_pos.y; data[compt++]= tile->_z; data[compt++]= 0.0; data[compt++]= 1.0; data[compt++]= float(tile->_idx_texture);
		data[compt++]= tile->_aabb->_pos.x+ tile->_aabb->_size.x; data[compt++]= tile->_aabb->_pos.y+ tile->_aabb->_size.y; data[compt++]= tile->_z; data[compt++]= 1.0; data[compt++]= 0.0; data[compt++]= float(tile->_idx_texture);
		data[compt++]= tile->_aabb->_pos.x; data[compt++]= tile->_aabb->_pos.y+ tile->_aabb->_size.y; data[compt++]= tile->_z; data[compt++]= 0.0; data[compt++]= 0.0; data[compt++]= float(tile->_idx_texture);
	}
	/*for (int i=0; i<context->_n_pts* context->_n_attrs_per_pts; ++i) {
		std::cout << data[i] << " ; ";
	}
	std::cout << "\n";*/

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::anim(time_point t) {
	if (_mode== CHOOSE_DRIVER) {
		update_choose_driver();
	}
	else if (_mode== CHOOSE_TRACK) {
		update_choose_track();
	}
	else if (_mode== RACING) {
		_track->anim(t, _input_state, _joystick_is_input);

		for (auto smoke_system : _smoke_systems) {
			smoke_system->anim(t);
		}

		_tire_track_system->anim(t, _track->_sorted_cars);

		_spark_system->anim(t, _track->_collisions);

		_water_system->anim(t);

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
			update_spark();
			update_driver_face();
			//update_barrier(); // fait 1 fois au début suffit
			update_map();
			update_water();
		}

		camera();
	}
}


void Racing::camera() {
	_com_camera.x+= CAM_INC* (_track->_hero->_com.x- _com_camera.x);
	_com_camera.y+= CAM_INC* (_track->_hero->_com.y- _com_camera.y);
	number diff_alpha= _track->_hero->_alpha- _alpha_camera;
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


bool Racing::key_down(SDL_Keycode key, time_point t) {
	// aide
	if (key== SDLK_h) {
		_help= !_help;
		return true;
	}

	if (_mode== CHOOSE_DRIVER) {
		if (key== SDLK_LEFT) {
			_idx_chosen_driver--;
			if (_idx_chosen_driver< 0) {
				_idx_chosen_driver= 0;
			}
			return true;
		}
		else if (key== SDLK_RIGHT) {
			_idx_chosen_driver++;
			if (_idx_chosen_driver>= _track->_drivers.size()) {
				_idx_chosen_driver= _track->_drivers.size()- 1;
			}
			return true;
		}
		else if (key== SDLK_RETURN) {
			//choose_driver(_idx_chosen_driver);
			_mode= CHOOSE_TRACK;
			return true;
		}
	}
	else if (_mode== CHOOSE_TRACK) {
		if (key== SDLK_LEFT) {
			_idx_chosen_track--;
			if (_idx_chosen_track< 1) {
				_idx_chosen_track= 1;
			}
			_track_info->parse_json("../data/tracks/track"+ std::to_string(_idx_chosen_track)+ ".json");
			return true;
		}
		else if (key== SDLK_RIGHT) {
			_idx_chosen_track++;
			if (_idx_chosen_track> _n_available_tracks) {
				_idx_chosen_track= _n_available_tracks; // on commence à 1
			}
			_track_info->parse_json("../data/tracks/track"+ std::to_string(_idx_chosen_track)+ ".json");
			return true;
		}
		else if (key== SDLK_RETURN) {
			choose_track(_idx_chosen_track, t);
			return true;
		}
		else if (key== SDLK_ESCAPE) {
			_mode= CHOOSE_DRIVER;
			return true;
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

		// t : dessiner textures
		else if (key== SDLK_t) {
			_draw_texture= !_draw_texture;
			return true;
		}

		// i : switcher infos normales / infos debug
		else if (key== SDLK_i) {
			_show_debug_info= !_show_debug_info;
			return true;
		}

		// j : activer / désactiver joystick
		else if (key== SDLK_j) {
			_joystick_is_input= !_joystick_is_input;
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
		else if (key== SDLK_ESCAPE) {
			// maj potentielle des meilleurs temps
			_track_info->parse_json("../data/tracks/track"+ std::to_string(_idx_chosen_track)+ ".json");
			_mode= CHOOSE_TRACK;
			return true;
		}
	}

	return false;
}
