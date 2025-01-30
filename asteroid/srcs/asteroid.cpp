#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <SDL2/SDL_image.h>

#include "asteroid.h"
#include "utile.h"

using json = nlohmann::json;


Mix_Music * Asteroid::_music= NULL;
std::string Asteroid::_next_music_path= "";


// action -----------------------------------------------------------
Action::Action() {

}


Action::Action(glm::vec2 direction, int t, std::string bullet_name, unsigned int t_shooting, std::string texture_name) :
	_direction(direction), _t(t), _bullet_name(bullet_name), _bullet_model(NULL), _t_shooting(t_shooting), _texture_name(texture_name) {

}


Action::~Action() {
	
}


std::ostream & operator << (std::ostream & os, const Action & action) {
	os << "direction=" << glm::to_string(action._direction) << " ; t=" << action._t << " ; " << "t_shooting=" << action._t_shooting;
	os << " ; bullet_name=" << action._bullet_name << " ; texture_name=" << action._texture_name;
	return os;
}


// action_texture ---------------------------------------------------
ActionTexture::ActionTexture() {

}


ActionTexture::ActionTexture(std::vector<std::string> & pngs, std::vector<unsigned int> & t_anims, AABB_2D & footprint) :
	_first_idx(0), _footprint(footprint) {
	for (auto png : pngs) {
		_pngs.push_back(png);
	}
	for (auto t : t_anims) {
		_t_anims.push_back(t);
	}
}


ActionTexture::~ActionTexture() {

}


std::ostream & operator << (std::ostream & os, const ActionTexture & at) {
	os << "pngs=";
	for (auto png : at._pngs) {
		os << png << " ; ";
	}
	os << "first_idx=" << at._first_idx;
	os << " ; footprint=" << at._footprint;
	return os;
}


// model -----------------------------------------------------------
ShipModel::ShipModel() {

}


ShipModel::ShipModel(std::string json_path) : _json_path(json_path) {
	std::string tmp_json_path= "../data/tmp/"+ basename(_json_path)+ "_tmp.json";

	// supprimer ce fichier si le json original a été modifié
	if (!file_exists(tmp_json_path)) {
		std::string cmd= "../srcs/flat_json_ship.py "+ json_path+ " "+ tmp_json_path;
		system(cmd.c_str());
	}

	std::ifstream ifs(tmp_json_path);
	json js= json::parse(ifs);
	ifs.close();

	//std::string cmd2= "rm "+ tmp_json_path;
	//system(cmd2.c_str());

	if (js["type"]== "hero") {
		_type= HERO;
	}
	else if (js["type"]== "enemy") {
		_type= ENEMY;
	}
	else if (js["type"]== "bullet") {
		_type= BULLET;
	}
	else {
		std::cerr << "type : " << js["type"] << " non reconnu\n";
	}

	_size= glm::vec2(js["size"][0], js["size"][1]);
	_lives= 1;
	if (js["lives"]!= nullptr) {
		_lives= js["lives"];
	}
	_score_hit= 0;
	if (js["score_hit"]!= nullptr) {
		_score_hit= js["score_hit"];
	}
	_score_death= 0;
	if (js["_score_death"]!= nullptr) {
		_score_death= js["score_death"];
	}

	// parcours des textures
	for (json::iterator it = js["textures"].begin(); it != js["textures"].end(); ++it) {
		auto & texture_name= it.key();
		auto & l_textures= it.value();
		std::vector<std::string> pngs;
		std::vector<unsigned int> t_anims;
		AABB_2D footprint(pt_type(0.0, 0.0), pt_type(1.0, 1.0));
		for (auto texture : l_textures) {
			std::string png= texture["png"];
			unsigned int t= texture["t"];
			pngs.push_back(png);
			t_anims.push_back(t);
			pt_type pos(texture["footprint"]["pos"][0], texture["footprint"]["pos"][1]);
			pt_type size(texture["footprint"]["size"][0], texture["footprint"]["size"][1]);
			// on prend l'intersection de tous les footprints
			if (pos.x> footprint._pos.x) {
				footprint._pos.x= pos.x;
			}
			if (pos.y> footprint._pos.y) {
				footprint._pos.y= pos.y;
			}
			if (size.x< footprint._size.x) {
				footprint._size.x= size.x;
			}
			if (size.y< footprint._size.y) {
				footprint._size.y= size.y;
			}
		}
		_textures[texture_name]= new ActionTexture(pngs, t_anims, footprint);
	}

	// parcours des actions
	for (json::iterator it = js["actions"].begin(); it != js["actions"].end(); ++it) {
		auto & action_name= it.key();
		auto & l_actions= it.value();

		_actions[action_name]= std::vector<Action *>();
		for (auto & action : l_actions) {

			glm::vec2 direction(0.0);
			if (action["direction"]!= nullptr) {
				direction= glm::vec2(action["direction"][0], action["direction"][1]);
			}
			
			int t= -1; // infini
			if (action["t"]!= nullptr) {
				t= action["t"];
			}
			
			std::string bullet_name= "";
			unsigned int t_shooting= 0;
			if (action["shooting"]!= nullptr) {
				bullet_name= action["shooting"];
				t_shooting= action["t_shooting"];
			}

			std::string texture_name= action["texture"];

			_actions[action_name].push_back(new Action(direction, t, bullet_name, t_shooting, texture_name));
		}
	}

	_hit_sound= Mix_LoadWAV("../data/sounds/hit.wav");
	_death_sound= Mix_LoadWAV("../data/sounds/death.wav");
	_shoot_sound= Mix_LoadWAV("../data/sounds/shoot.wav");
}


ShipModel::~ShipModel() {
	Mix_FreeChunk(_hit_sound);
	Mix_FreeChunk(_death_sound);
	Mix_FreeChunk(_shoot_sound);
}


std::ostream & operator << (std::ostream & os, const ShipModel & model) {
	os << "json = " << model._json_path << " ; type=" << model._type << " ; size=" << glm::to_string(model._size);
	os << " ; score_hit=" << model._score_hit << " ; score_death=" << model._score_death << " ; lives=" << model._lives;
	os << " ; actions = [";
	for (auto action : model._actions) {
		os << action.first << " : (";
		for (auto a : action.second) {
			os << *a << " | ";
		}
	}
	os << "]";
	os << " ; textures = [";
	for (auto texture : model._textures) {
		os << texture.first << " : " << *texture.second << " | ";
	}
	os << "]";
	return os;
}


// ship -------------------------------------------------------------
Ship::Ship() {

}


Ship::Ship(ShipModel * model, pt_type pos, bool friendly, std::chrono::system_clock::time_point t) :
	_model(model), _friendly(friendly), _dead(false), _shooting(false), _current_action_name(MAIN_ACTION_NAME),
	_idx_action(0), _velocity(glm::vec2(0.0)), _idx_anim(0), 
	_hit(false), _hit_value(0.0), _delete(false), _alpha(1.0), _lives(_model->_lives),
	_rotation(0.0), _scale(1.0)
{
	_aabb= AABB_2D(pos, model->_size);
	ActionTexture * current_texture= get_current_texture();
	_footprint= AABB_2D(
		pt_type(pos.x+ current_texture->_footprint._pos.x* _model->_size.x, pos.y+ current_texture->_footprint._pos.y* _model->_size.y),
		pt_type(_model->_size.x* current_texture->_footprint._size.x, _model->_size.y* current_texture->_footprint._size.y)
	);
	_t_action_start= _t_last_hit= _t_die= _t_last_bullet= _t_anim_start= t;
}


Ship::~Ship() {
	
}


void Ship::anim(std::chrono::system_clock::time_point t) {
	_shooting= false;

	if (_dead) {
		//_hit= false;
		//_hit_value= 0.0;
		auto d_death= std::chrono::duration_cast<std::chrono::milliseconds>(t- _t_die).count();
		if (d_death> DEATH_MS) {
			_delete= true;
			_alpha= 0.0;
			_scale= 1.0;
			return;
		}
		else {
			_alpha= float(DEATH_MS- d_death)/ float(DEATH_MS);
			if (_model->_type== ENEMY) {
				_scale+= DEATH_SCALE_INC;
			}
		}
	}

	if (_hit) {
		auto d_hit= std::chrono::duration_cast<std::chrono::milliseconds>(t- _t_last_hit).count();
		if (d_hit> HIT_UNTOUCHABLE_MS) {
			_hit= false;
			_hit_value= 0.0;
			_rotation= 0.0;
		}
		else {
			_hit_value= float(HIT_UNTOUCHABLE_MS- d_hit)/ float(HIT_UNTOUCHABLE_MS);
			int i= d_hit/ HIT_ROTATION_MS;
			if (i% 2== 0) {
				_rotation+= HIT_ROTATION_INC;
			}
			else {
				_rotation-= HIT_ROTATION_INC;
			}
		}
	}
	else {
		_hit_value= 0.0;
	}
	
	// chgmt action ; _t < 0 correspond à une action infinie
	auto d_action= std::chrono::duration_cast<std::chrono::milliseconds>(t- _t_action_start).count();
	if (_model->_actions[_current_action_name][_idx_action]->_t> 0 && d_action> _model->_actions[_current_action_name][_idx_action]->_t) {
		_t_action_start= t;
		_idx_action++;
		if (_idx_action>= _model->_actions[_current_action_name].size()) {
			_idx_action= 0;
		}
		_idx_anim= 0;
	}

	ShipModel * bullet_model= get_current_bullet_model();
	ActionTexture * texture= get_current_texture();

	// faut-il tirer
	if (bullet_model!= NULL) {
		auto d_shoot= std::chrono::duration_cast<std::chrono::milliseconds>(t- _t_last_bullet).count();
		if (d_shoot> _model->_actions[_current_action_name][_idx_action]->_t_shooting) {
			_t_last_bullet= t;
			_shooting= true;
			if (_model->_type== HERO) {
				Mix_PlayChannel(-1, _model->_shoot_sound, 0);
			}
		}
	}

	// maj vitesse et position
	if (_model->_type== ENEMY || _model->_type== BULLET) {
		_velocity= _model->_actions[_current_action_name][_idx_action]->_direction;
	}
	
	_aabb._pos+= _velocity;

	_footprint._pos.x= _aabb._pos.x+ texture->_footprint._pos.x* _aabb._size.x;
	_footprint._pos.y= _aabb._pos.y+ texture->_footprint._pos.y* _aabb._size.y;
	_footprint._size.x= texture->_footprint._size.x*_aabb._size.x;
	_footprint._size.y= texture->_footprint._size.y*_aabb._size.y;

	// anim texture
	auto d_anim= std::chrono::duration_cast<std::chrono::milliseconds>(t- _t_anim_start).count();
	if (d_anim> texture->_t_anims[_idx_anim]) {
		_t_anim_start= t;
		_idx_anim++;
		if (_idx_anim>= texture->_t_anims.size()) {
			_idx_anim= 0;
		}
	}
}


ShipModel * Ship::get_current_bullet_model() {
	return _model->_actions[_current_action_name][_idx_action]->_bullet_model;
}


ActionTexture * Ship::get_current_texture() {
	std::string current_tex_name= _model->_actions[_current_action_name][_idx_action]->_texture_name;
	return _model->_textures[current_tex_name];
}


void Ship::set_current_action(std::string action_name, std::chrono::system_clock::time_point t) {
	_current_action_name= action_name;
	_idx_action= 0;
	_t_action_start= _t_anim_start= t;
	_idx_anim= 0;
}


bool Ship::hit(std::chrono::system_clock::time_point t) {
	if (_hit) {
		return false;
	}

	_lives--;
	if (_lives<= 0) {
		_dead= true;
		_t_die= t;
		if (_model->_type== ENEMY) {
			Mix_PlayChannel(-1, _model->_death_sound, 0);
		}
	}
	else {
		_hit= true;
		_t_last_hit= t;
		if (_model->_type== ENEMY) {
			Mix_PlayChannel(-1, _model->_hit_sound, 0);
		}
	}
	return true;
}


std::ostream & operator << (std::ostream & os, const Ship & ship) {
	os << "model = " << *ship._model << " ; aabb = [" << ship._aabb << "]";
	return os;
}


// event -------------------------------------------------------------
Event::Event() {

}


Event::Event(EventType type, unsigned int t, glm::vec2 position, std::string enemy) :
	_type(type), _t(t), _position(position), _enemy(enemy)
{

}


Event::~Event() {

}


std::ostream & operator << (std::ostream & os, const Event & event) {
	if (event._type== NEW_ENEMY) {
		os << "NEW_ENEMY ; ";
	}
	else if (event._type== LEVEL_END) {
		os << "LEVEL_END ; ";
	}
	os << "t = " << event._t << " ; position = " << glm::to_string(event._position) << " ; enemy = " << event._enemy;
	return os;
}


// Level ----------------------------------------------------------------
Level::Level() {

}


Level::Level(std::string json_path, std::chrono::system_clock::time_point t) : _t_start(t), _json_path(json_path) {
	std::string tmp_json_path= "../data/tmp/"+ basename(_json_path)+ "_tmp.json";

	// supprimer ce fichier si le json original a été modifié
	if (!file_exists(tmp_json_path)) {
		std::string cmd= "../srcs/flat_json_level.py "+ json_path+ " "+ tmp_json_path;
		system(cmd.c_str());
	}

	std::ifstream ifs(tmp_json_path);
	json js= json::parse(ifs);
	ifs.close();

	//std::string cmd2= "rm "+ tmp_json_path;
	//system(cmd2.c_str());

	_music_path= js["music"];

	//for (json::iterator it = js["events"].begin(); it != js["events"].end(); ++it) {
	//	auto & event_name= it.key();
	//	auto & l_textures= it.value();
	for (auto event : js["events"]) {
		unsigned int t= event["t"];
		if (event["type"]== "enemy") {
			float x= event["position"][0];
			float y= event["position"][1];
			std::string enemy_name= event["enemy"];
			_events.push_back(new Event(NEW_ENEMY, t, glm::vec2(x, y), enemy_name));
		}
		else if (event["type"]== "end") {
			_events.push_back(new Event(LEVEL_END, t, glm::vec2(0.0), ""));
		}
	}

	// les évenements sont classés du dernier au 1er, afin que l'on puisse faire un pop_back (pop_front n'existe pas pour les vector)
	std::sort(_events.begin(), _events.end(), [](const Event * a, const Event * b) { return a->_t > b->_t; });
	// on commence donc avec le dernier
	_current_event_idx= _events.size()- 1;
}


Level::~Level() {
}


void Level::reinit(std::chrono::system_clock::time_point t) {
	_current_event_idx= _events.size()- 1;
	_t_start= t;
}


std::ostream & operator << (std::ostream & os, const Level & level) {
	os << "json_path=" << level._json_path << " ; _events=[";
	for (auto event : level._events) {
		os << *event << " ; ";
	}
	os << "_current_event_idx=" << level._current_event_idx;
	return os;
}


// Asteroid -------------------------------------------------------------
Asteroid::Asteroid() {

}


Asteroid::Asteroid(GLuint prog_aabb, GLuint prog_texture, GLuint prog_font, ScreenGL * screengl, std::chrono::system_clock::time_point t) 
	: _draw_aabb(false), _draw_footprint(false), _draw_texture(true),
	_key_left(false), _key_right(false), _key_up(false), _key_down(false),
	_mode(INACTIVE), _score(0), _current_level_idx(0) {

	_pt_min= glm::vec2(-screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f);
	_pt_max= glm::vec2(screengl->_gl_width* 0.5f, screengl->_gl_height* 0.5f);
	_camera2clip= glm::ortho(-screengl->_gl_width* 0.5f, screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f, screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
	_font= new Font(prog_font, "../../fonts/Silom.ttf", 48, screengl);

	load_models();
	/*for (auto model : _models) {
		std::cout << model.first << " : " << *model.second << "\n";
	}*/

	load_levels(t);
	/*for (auto level : _levels) {
		std::cout << *level << "\n";
	}*/
	fill_texture_array();

	read_highest_scores();

	reinit(t);

	unsigned int n_buffers= 4;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	_contexts["border_aabb"]= new DrawContext(prog_aabb, _buffers[0],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"camera2clip_matrix"});

	_contexts["ship_aabb"]= new DrawContext(prog_aabb, _buffers[1],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"camera2clip_matrix"});

	_contexts["ship_footprint"]= new DrawContext(prog_aabb, _buffers[2],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"camera2clip_matrix"});

	_contexts["ship_texture"]= new DrawContext(prog_texture, _buffers[3],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in", "hit_in", "alpha_in",
		"rotation_in", "scale_in", "center_in"},
		std::vector<std::string>{"camera2clip_matrix", "texture_array"});

	update_border_aabb();
	update_ship_aabb();
	update_ship_footprint();
	update_ship_texture();

	set_music("../data/sounds/music_inactive.wav");
}


Asteroid::~Asteroid() {
	for (auto ship : _ships) {
		delete ship;
	}
	_ships.clear();

	for (auto context : _contexts) {
		delete context.second;
	}
	_contexts.clear();

	for (auto model : _models) {
		delete model.second;
	}
	_models.clear();

	delete _buffers;

	Mix_HaltMusic();
	Mix_FreeMusic(Asteroid::_music);
}


void Asteroid::load_models() {
	std::vector<std::string> jsons_total;
	for (auto ship_type : std::vector<std::string>{"bullets", "enemies", "heroes"}) {
		std::vector<std::string> jsons= list_files("../data/"+ ship_type, "json");
		jsons_total.insert(jsons_total.end(), jsons.begin(), jsons.end());
	}
	
	for (auto json_path : jsons_total) {
		_models[basename(json_path)]= new ShipModel(json_path);
	}
	
	for (auto model : _models) {
		for (auto l_action : model.second->_actions) {
			for (auto action : l_action.second) {
				if (action->_bullet_name!= "") {
					bool found= false;
					for (auto bullet_model : _models) {
						if (bullet_model.first== action->_bullet_name) {
							action->_bullet_model= bullet_model.second;
							found= true;
							break;
						}
					}
					if (!found) {
						std::cerr << "bullet model : " << action->_bullet_name << " non trouvé\n";
					}
				}
			}
		}
	}
}


void Asteroid::fill_texture_array() {
	unsigned int n_tex= 0;
	for (auto model : _models) {
		for (auto texture : model.second->_textures) {
			n_tex+= texture.second->_pngs.size();
		}
	}

	glGenTextures(1, &_texture_id);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);
	
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, TEXTURE_SIZE, TEXTURE_SIZE, n_tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	unsigned int compt= 0;
	for (auto model : _models) {
		//std::cout << "\nmodel=" << model.first << "\n";
		for (auto texture : model.second->_textures) {
			//std::cout << "texture=" << texture.first << "\n";
			texture.second->_first_idx= compt;
			for (auto png : texture.second->_pngs) {
				std::string png_abs= splitext(model.second->_json_path).first+ "/"+ png;
				//std::cout << "png=" << png_abs << " ; compt=" << compt << "\n";
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
		}
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	/*glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);*/

	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}


void Asteroid::load_levels(std::chrono::system_clock::time_point t) {
	std::vector<std::string> jsons= list_files("../data/levels", "json");
	// les json_path sont triés par nom, donc level1, 2, ... devraient être dans l'ordre
	for (auto json_path : jsons) {
		_levels.push_back(new Level(json_path, t));
	}
}


void Asteroid::draw_border_aabb() {
	DrawContext * context= _contexts["border_aabb"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(GL_FLOAT), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(GL_FLOAT), (void*)(2* sizeof(GL_FLOAT)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Asteroid::draw_ship_aabb() {
	DrawContext * context= _contexts["ship_aabb"];
	
	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(GL_FLOAT), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(GL_FLOAT), (void*)(2* sizeof(GL_FLOAT)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Asteroid::draw_ship_footprint() {
	DrawContext * context= _contexts["ship_footprint"];
	
	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(GL_FLOAT), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(GL_FLOAT), (void*)(2* sizeof(GL_FLOAT)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Asteroid::draw_ship_texture() {
	DrawContext * context= _contexts["ship_texture"];
	//std::cout << _texture_id << " ; " << context->_locs["texture_array"] << " ; " << context->_locs["camera2clip_matrix"] << " ; " << context->_locs["position_in"] << " ; " << context->_locs["tex_coord_in"] << " ; " << context->_locs["current_layer_in"] << "\n";

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);
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
	glVertexAttribPointer(context->_locs_attrib["hit_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(5* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["alpha_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(6* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["rotation_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(7* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["scale_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(8* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["center_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(9* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Asteroid::draw() {
	if (_mode== PLAYING) {
		if (_draw_texture) {
			draw_ship_texture();
		}
		if (_draw_aabb) {
			draw_border_aabb();
			draw_ship_aabb();
		}
		if (_draw_footprint) {
			draw_ship_footprint();
		}
		show_playing_info();
	}
	else if (_mode== INACTIVE) {
		show_inactive_info();
	}
	else if (_mode== SET_SCORE_NAME) {
		show_set_score_name_info();
	}
}


void Asteroid::show_playing_info() {
	std::ostringstream font_str;

	float font_scale= 0.01f;
	glm::vec4 font_color= glm::vec4(1.0f, 1.0f, 0.0f, 0.5f);

	std::string s1= "score : "+ std::to_string(_score);
	glm::vec2 position1= glm::vec2(-9.0f, 7.0f);
	Text t1(s1, position1, font_scale, font_color);

	std::string s2= "level : "+ std::to_string(_current_level_idx+ 1);
	glm::vec2 position2= glm::vec2(-1.0f, 7.0f);
	Text t2(s2, position2, font_scale, font_color);

	std::string s3= "vies : "+ std::to_string(_ships[0]->_lives);
	glm::vec2 position3= glm::vec2(7.1f, 7.0f);
	Text t3(s3, position3, font_scale, font_color);

	std::vector<Text> texts;
	texts.push_back(t1);
	texts.push_back(t2);
	texts.push_back(t3);

	_font->set_text(texts);
	_font->draw();
}


void Asteroid::show_inactive_info() {
	float font_scale= 0.02f;
	glm::vec4 font_color= glm::vec4(1.0f, 1.0f, 0.0f, 0.5f);

	std::string s1= "score "+ std::to_string(_score);
	glm::vec2 position1= glm::vec2(-7.0f, 2.0f);
	Text t1(s1, position1, font_scale, font_color);

	std::string s2= "records ";
	glm::vec2 position2= glm::vec2(0.0f, 2.0f);
	Text t2(s2, position2, font_scale, font_color);

	std::vector<Text> texts;
	texts.push_back(t1);
	texts.push_back(t2);
	for (unsigned int i=0; i<_highest_scores.size(); ++i) {
		std::string s3= std::to_string(i+ 1)+ " "+ _highest_scores[i].first+ " " + std::to_string(_highest_scores[i].second);
		glm::vec2 position3= glm::vec2(0.0f, 1.0f- float(i)* 1.0);
		Text t3(s3, position3, font_scale, font_color);
		texts.push_back(t3);
	}
	_font->set_text(texts);
	_font->draw();
}


void Asteroid::show_set_score_name_info() {
	float font_scale= 0.02f;
	glm::vec4 font_color= glm::vec4(1.0f, 1.0f, 0.0f, 0.5f);

	std::string s1= "NOUVEAU RECORD";
	glm::vec2 position1= glm::vec2(-5.0f, 2.0f);
	Text t1(s1, position1, font_scale, font_color);

	std::vector<Text> texts;
	texts.push_back(t1);
	//texts.push_back(t2);
	for (unsigned int i=0; i<_highest_scores.size(); ++i) {
		if (i== _new_highest_idx) {
			font_color= glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);
		}
		else {
			font_color= glm::vec4(1.0f, 1.0f, 0.0f, 0.5f);
		}
		std::string s3= std::to_string(i+ 1)+ " "+ _highest_scores[i].first+ " " + std::to_string(_highest_scores[i].second);
		glm::vec2 position3= glm::vec2(-5.0f, 1.0f- float(i)* 1.0);
		Text t3(s3, position3, font_scale, font_color);
		texts.push_back(t3);
	}

	_font->set_text(texts);
	_font->draw();
}


void Asteroid::update_border_aabb() {
	DrawContext * context= _contexts["border_aabb"];
	context->_n_pts= 8;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];
	float positions[8* 2]= {
		_pt_min.x, _pt_min.y,
		_pt_max.x, _pt_min.y,

		_pt_max.x, _pt_min.y,
		_pt_max.x, _pt_max.y,

		_pt_max.x, _pt_max.y,
		_pt_min.x, _pt_max.y,

		_pt_min.x, _pt_max.y,
		_pt_min.x, _pt_min.y
	};
	for (unsigned int idx_pt=0; idx_pt<8; ++idx_pt) {
		data[context->_n_attrs_per_pts* idx_pt+ 0]= positions[2* idx_pt];
		data[context->_n_attrs_per_pts* idx_pt+ 1]= positions[2* idx_pt+ 1];
		for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
			data[context->_n_attrs_per_pts* idx_pt+ 2+ idx_color]= BORDER_COLOR[idx_color];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Asteroid::update_ship_aabb() {
	DrawContext * context= _contexts["ship_aabb"];
	context->_n_pts= 0;
	context->_n_attrs_per_pts= 6;

	const unsigned int n_pts_per_ship= 8;

	for (auto ship : _ships) {
		context->_n_pts+= n_pts_per_ship;
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];

	for (unsigned int idx_ship=0; idx_ship<_ships.size(); ++idx_ship) {
		Ship * ship= _ships[idx_ship];

		glm::vec4 color;
		if (ship->_friendly) {
			color= AABB_FRIENDLY_COLOR;
		}
		else {
			color= AABB_UNFRIENDLY_COLOR;
		}
		number positions[n_pts_per_ship* 2]= {
			ship->_aabb._pos.x, ship->_aabb._pos.y,
			ship->_aabb._pos.x+ ship->_aabb._size.x, ship->_aabb._pos.y,

			ship->_aabb._pos.x+ ship->_aabb._size.x, ship->_aabb._pos.y,
			ship->_aabb._pos.x+ ship->_aabb._size.x, ship->_aabb._pos.y+ ship->_aabb._size.y,

			ship->_aabb._pos.x+ ship->_aabb._size.x, ship->_aabb._pos.y+ ship->_aabb._size.y,
			ship->_aabb._pos.x, ship->_aabb._pos.y+ ship->_aabb._size.y,

			ship->_aabb._pos.x, ship->_aabb._pos.y+ ship->_aabb._size.y,
			ship->_aabb._pos.x, ship->_aabb._pos.y
		};

		for (unsigned int i=0; i<n_pts_per_ship; ++i) {
			if (positions[2* i]> _pt_max.x) {
				positions[2* i]= _pt_max.x;
			}
			if (positions[2* i]< _pt_min.x) {
				positions[2* i]= _pt_min.x;
			}
			if (positions[2* i+ 1]> _pt_max.y) {
				positions[2* i+ 1]= _pt_max.y;
			}
			if (positions[2* i+ 1]< _pt_min.y) {
				positions[2* i+ 1]= _pt_min.y;
			}
		}

		for (unsigned int idx_pt=0; idx_pt<n_pts_per_ship; ++idx_pt) {
			data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 0]= float(positions[2* idx_pt]);
			data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 1]= float(positions[2* idx_pt+ 1]);
			for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
				data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= color[idx_color];
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Asteroid::update_ship_footprint() {
	DrawContext * context= _contexts["ship_footprint"];
	context->_n_pts= 0;
	context->_n_attrs_per_pts= 6;

	const unsigned int n_pts_per_ship= 8;

	for (auto ship : _ships) {
		context->_n_pts+= n_pts_per_ship;
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];

	for (unsigned int idx_ship=0; idx_ship<_ships.size(); ++idx_ship) {
		Ship * ship= _ships[idx_ship];

		glm::vec4 color;
		if (ship->_friendly) {
			color= FOOTPRINT_FRIENDLY_COLOR;
		}
		else {
			color= FOOTPRINT_UNFRIENDLY_COLOR;
		}
		number positions[n_pts_per_ship* 2]= {
			ship->_footprint._pos.x, ship->_footprint._pos.y,
			ship->_footprint._pos.x+ ship->_footprint._size.x, ship->_footprint._pos.y,

			ship->_footprint._pos.x+ ship->_footprint._size.x, ship->_footprint._pos.y,
			ship->_footprint._pos.x+ ship->_footprint._size.x, ship->_footprint._pos.y+ ship->_footprint._size.y,

			ship->_footprint._pos.x+ ship->_footprint._size.x, ship->_footprint._pos.y+ ship->_footprint._size.y,
			ship->_footprint._pos.x, ship->_footprint._pos.y+ ship->_footprint._size.y,

			ship->_footprint._pos.x, ship->_footprint._pos.y+ ship->_footprint._size.y,
			ship->_footprint._pos.x, ship->_footprint._pos.y
		};

		for (unsigned int i=0; i<n_pts_per_ship; ++i) {
			if (positions[2* i]> _pt_max.x) {
				positions[2* i]= _pt_max.x;
			}
			if (positions[2* i]< _pt_min.x) {
				positions[2* i]= _pt_min.x;
			}
			if (positions[2* i+ 1]> _pt_max.y) {
				positions[2* i+ 1]= _pt_max.y;
			}
			if (positions[2* i+ 1]< _pt_min.y) {
				positions[2* i+ 1]= _pt_min.y;
			}
		}

		for (unsigned int idx_pt=0; idx_pt<n_pts_per_ship; ++idx_pt) {
			data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 0]= float(positions[2* idx_pt]);
			data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 1]= float(positions[2* idx_pt+ 1]);
			for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
				data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= color[idx_color];
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Asteroid::update_ship_texture() {
	DrawContext * context= _contexts["ship_texture"];
	context->_n_pts= 0;
	context->_n_attrs_per_pts= 11;

	const unsigned int n_pts_per_ship= 6;

	for (auto ship : _ships) {
		context->_n_pts+= n_pts_per_ship;
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];

	// à cause du système de reference opengl j'ai du inverser les 0 et les 1 des y des textures
	for (unsigned int idx_ship=0; idx_ship<_ships.size(); ++idx_ship) {
		Ship * ship= _ships[idx_ship];
		ActionTexture * texture= ship->get_current_texture();
		glm::vec2 center= ship->_footprint.center();
		
		number positions[n_pts_per_ship* 4]= {
			ship->_aabb._pos.x, ship->_aabb._pos.y, 0.0, 1.0,
			ship->_aabb._pos.x+ ship->_aabb._size.x, ship->_aabb._pos.y, 1.0, 1.0,
			ship->_aabb._pos.x+ ship->_aabb._size.x, ship->_aabb._pos.y+ ship->_aabb._size.y, 1.0, 0.0,

			ship->_aabb._pos.x, ship->_aabb._pos.y, 0.0, 1.0,
			ship->_aabb._pos.x+ ship->_aabb._size.x, ship->_aabb._pos.y+ ship->_aabb._size.y, 1.0, 0.0,
			ship->_aabb._pos.x, ship->_aabb._pos.y+ ship->_aabb._size.y, 0.0, 0.0
		};

		/*for (unsigned int i=0; i<n_pts_per_ship; ++i) {
			if (positions[4* i]> _pt_max.x) {
				positions[4* i]= _pt_max.x;
				positions[4* i+ 2]= 
			}
			if (positions[4* i]< _pt_min.x) {
				positions[4* i]= _pt_min.x;
			}
			if (positions[4* i+ 1]> _pt_max.y) {
				positions[4* i+ 1]= _pt_max.y;
			}
			if (positions[4* i+ 1]< _pt_min.y) {
				positions[4* i+ 1]= _pt_min.y;
			}
		}*/

		for (unsigned int idx_pt=0; idx_pt<n_pts_per_ship; ++idx_pt) {
			for (unsigned int i=0; i<4; ++i) {
				data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ i]= float(positions[4* idx_pt+ i]);
			}
			data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 4]= float(texture->_first_idx+ ship->_idx_anim); // current_layer_in
			data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 5]= ship->_hit_value;
			data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 6]= ship->_alpha;
			data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 7]= ship->_rotation;
			data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 8]= ship->_scale;
			data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 9]= center.x;
			data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 10]= center.y;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Asteroid::anim_playing(std::chrono::system_clock::time_point t) {
	/*if (rand_int(0, 100)> 98) {
		add_rand_enemy();
	}*/

	// ajout des événements du niveau courant
	add_level_events(t);

	// la vitesse se calque sur les entrées
	// joystick vers le haut est négatif !
	_ships[0]->_velocity.x= HERO_VELOCITY* _joystick[0];
	_ships[0]->_velocity.y= -1.0* HERO_VELOCITY* _joystick[1];

	if (_key_left) {
		_ships[0]->_velocity.x= -1.0* HERO_VELOCITY;
	}
	if (_key_right) {
		_ships[0]->_velocity.x= HERO_VELOCITY;
	}
	if (_key_down) {
		_ships[0]->_velocity.y= -1.0* HERO_VELOCITY;
	}
	if (_key_up) {
		_ships[0]->_velocity.y= HERO_VELOCITY;
	}

	// animation de chaque ship
	// ne pas utiliser auto ici car les push_back dans le for modifie l'itérateur
	//for (auto ship : _ships) {
	for (unsigned int idx_ship=0; idx_ship<_ships.size(); ++idx_ship) {
		Ship * ship= _ships[idx_ship];
		ship->anim(t);

		if (ship->_shooting) {
			bool friendly= false;
			if (ship->_model->_type== HERO) {
				friendly= true;
			}
			ShipModel * bullet_model= ship->get_current_bullet_model();
			_ships.push_back(new Ship(bullet_model, ship->_footprint.center()- 0.5* bullet_model->_size, friendly, t));
		}
	}

	// joueur contraint à l'écran
	if (_ships[0]->_aabb._pos.x> _pt_max.x- _ships[0]->_aabb._size.x) {
		_ships[0]->_aabb._pos.x= _pt_max.x- _ships[0]->_aabb._size.x;
	}
	if (_ships[0]->_aabb._pos.x< _pt_min.x) {
		_ships[0]->_aabb._pos.x= _pt_min.x;
	}
	if (_ships[0]->_aabb._pos.y> _pt_max.y- _ships[0]->_aabb._size.y) {
		_ships[0]->_aabb._pos.y= _pt_max.y- _ships[0]->_aabb._size.y;
	}
	if (_ships[0]->_aabb._pos.y< _pt_min.y) {
		_ships[0]->_aabb._pos.y= _pt_min.y;
	}

	// suppression des ships sortis de l'écran
	for (auto ship : _ships) {
		if (ship->_model->_type!= ENEMY && ship->_model->_type!= BULLET) {
			continue;
		}
		if (ship->_aabb._pos.x> _pt_max.x) {
			ship->_delete= true;
		}
		else if (ship->_aabb._pos.x< _pt_min.x- ship->_aabb._size.x) {
			ship->_delete= true;
		}
		// on se donne une marge pour les ennemis qui sont trop en haut afin que l'on puisse les créer sans qu'ils
		// apparaissent à l'écran au début
		if (ship->_aabb._pos.y> _pt_max.y+ 10.0) {
			ship->_delete= true;
		}
		else if (ship->_aabb._pos.y< _pt_min.y- ship->_aabb._size.y) {
			ship->_delete= true;
		}
	}

	// détection collisions
	for (unsigned int idx_ship1=0; idx_ship1<_ships.size()- 1; ++idx_ship1) {
		Ship * ship1= _ships[idx_ship1];
		if (ship1->_delete || ship1->_dead) {
			continue;
		}
		for (unsigned int idx_ship2=idx_ship1+ 1; idx_ship2<_ships.size(); ++idx_ship2) {
			Ship * ship2= _ships[idx_ship2];
			if (ship2->_delete || ship2->_dead) {
				continue;
			}
			// les alliés ne peuvent pas s'attaquer
			if ((ship1->_friendly && ship2->_friendly) || (!ship1->_friendly && !ship2->_friendly)) {
				continue;
			}

			// les footprint sont utilisés pour la détection de collision
			if (aabb_intersects_aabb(&ship1->_footprint, &ship2->_footprint)) {
				if (ship1->hit(t) && !ship1->_friendly) {
					if (ship1->_dead) {
						_score+= ship1->_model->_score_death;
					}
					else {
						_score+= ship1->_model->_score_hit;
					}
				}
				if (ship2->hit(t) && !ship2->_friendly) {
					if (ship2->_dead) {
						_score+= ship2->_model->_score_death;
					}
					else {
						_score+= ship2->_model->_score_hit;
					}
				}
			}
		}
	}

	// gamover
	if (_ships[0]->_dead) {
		// pas de high score
		if (_score< _highest_scores[2].second) {
			_mode= INACTIVE;
			set_music("../data/sounds/music_inactive.wav");
			_new_highest_idx= 0;
			_new_highest_char_idx= 0;
		}

		// high score
		else {
			for (int i=0; i<_highest_scores.size(); ++i) {
				if (_score> _highest_scores[i].second) {
					_new_highest_idx= i;
					_mode= SET_SCORE_NAME;
					set_music("../data/sounds/music_set_score_name.wav");
					_highest_scores.insert(_highest_scores.begin()+ _new_highest_idx, std::make_pair("AAA", _score));
					_highest_scores.pop_back();
					break;
				}
			}
		}
	}

	// suppression des ship _delete == true
	_ships.erase(std::remove_if(_ships.begin(), _ships.end(), [](Ship * s){
		return s->_delete;
	}), _ships.end());

	// maj des buffers
	update_ship_aabb();
	update_ship_footprint();
	update_ship_texture();
}


void Asteroid::anim_inactive() {

}


void Asteroid::anim_set_score_name() {

}


void Asteroid::anim(std::chrono::system_clock::time_point t) {
	if (_mode== PLAYING) {
		anim_playing(t);
	}
	else if (_mode== INACTIVE) {
		anim_inactive();
	}
	else if (_mode== SET_SCORE_NAME) {
		anim_set_score_name();
	}
}


bool Asteroid::key_down(InputState * input_state, SDL_Keycode key, std::chrono::system_clock::time_point t) {
	if (key== SDLK_w) {
		std::cout << "joy=(" << _joystick[0] << " ; " << _joystick[1] << ") ; key_left=" << _key_left  << " ; key_right=" << _key_right << " ; key_down=" << _key_down  << " ; key_up=" << _key_up << "\n";
	}
	if (key== SDLK_t) {
		_draw_texture= !_draw_texture;
		return true;
	}
	if (key== SDLK_y) {
		_draw_aabb= !_draw_aabb;
		return true;
	}
	if (key== SDLK_u) {
		_draw_footprint= !_draw_footprint;
		return true;
	}
	if (key== SDLK_m) {
		if (Mix_PlayingMusic()!= 0) {
			if (Mix_PausedMusic()== 1) {
				Mix_ResumeMusic();
			}
			else {
				Mix_PauseMusic();
			}
		}
	}

	if (_mode== PLAYING) {
		if (key== SDLK_LEFT) {
			_key_left= true;
			return true;
		}
		else if (key== SDLK_RIGHT) {
			_key_right= true;
			return true;
		}
		else if (key== SDLK_UP) {
			_key_up= true;
			return true;
		}
		else if (key== SDLK_DOWN) {
			_key_down= true;
			return true;
		}
		else if (key== SDLK_a) {
			_ships[0]->set_current_action("shoot_a", t);
			return true;
		}
		else if (key== SDLK_z) {
			_ships[0]->set_current_action("shoot_b", t);
			return true;
		}
	}
	
	else if (_mode== INACTIVE) {
		if (key== SDLK_RETURN) {
			reinit(t);
			_mode= PLAYING;
			set_level(0, t);
			return true;
		}
	}

	else if (_mode== SET_SCORE_NAME) {
		if (key== SDLK_LEFT) {
			_new_highest_char_idx--;
			if (_new_highest_char_idx< 0) {
				_new_highest_char_idx= 0;
			}
			return true;
		}
		else if (key== SDLK_RIGHT) {
			_new_highest_char_idx++;
			if (_new_highest_char_idx> 2) {
				_new_highest_char_idx= 2;
			}
			return true;
		}
		else if (key== SDLK_UP) {
			_highest_scores[_new_highest_idx].first[_new_highest_char_idx]++;
			if (_highest_scores[_new_highest_idx].first[_new_highest_char_idx]> 'Z') {
				_highest_scores[_new_highest_idx].first[_new_highest_char_idx]= 'Z';
			}
			return true;
		}
		else if (key== SDLK_DOWN) {
			_highest_scores[_new_highest_idx].first[_new_highest_char_idx]--;
			if (_highest_scores[_new_highest_idx].first[_new_highest_char_idx]< 'A') {
				_highest_scores[_new_highest_idx].first[_new_highest_char_idx]= 'A';
			}
			return true;
		}
		else if (key== SDLK_RETURN) {
			_mode= INACTIVE;
			set_music("../data/sounds/music_inactive.wav");
			write_highest_scores();
			return true;
		}
	}
	return false;
}


bool Asteroid::key_up(InputState * input_state, SDL_Keycode key, std::chrono::system_clock::time_point t) {
	if (_mode== PLAYING) {
		if (key== SDLK_LEFT) {
			_key_left= false;
			return true;
		}
		else if (key== SDLK_RIGHT) {
			_key_right= false;
			return true;
		}
		else if (key== SDLK_UP) {
			_key_up= false;
			return true;
		}
		else if (key== SDLK_DOWN) {
			_key_down= false;
			return true;
		}
		else if (key== SDLK_a) {
			_ships[0]->set_current_action("no_shoot", t);
			return true;
		}
		else if (key== SDLK_z) {
			_ships[0]->set_current_action("no_shoot", t);
			return true;
		}
	}
	else if (_mode== INACTIVE) {
	}
	else if (_mode== SET_SCORE_NAME) {
	}
	return false;
}


bool Asteroid::joystick_down(unsigned int button_idx, std::chrono::system_clock::time_point t) {
	if (_mode== PLAYING) {
		if (button_idx== 0) {
			_ships[0]->set_current_action("shoot_a", t);
			return true;
		}
		else if (button_idx== 1) {
			_ships[0]->set_current_action("shoot_b", t);
			return true;
		}
	}
	else if (_mode== INACTIVE) {
		reinit(t);
		_mode= PLAYING;
		return true;
	}
	else if (_mode== SET_SCORE_NAME) {
		_mode= INACTIVE;
		write_highest_scores();
		return true;
	}
	return false;
}


bool Asteroid::joystick_up(unsigned int button_idx, std::chrono::system_clock::time_point t) {
	if (_mode== PLAYING) {
		if (button_idx== 0) {
			_ships[0]->set_current_action("no_shoot", t);
			return true;
		}
		else if (button_idx== 1) {
			_ships[0]->set_current_action("no_shoot", t);
			return true;
		}
	}
	else if (_mode== INACTIVE) {
	}
	else if (_mode== SET_SCORE_NAME) {
	}
	return false;
}


bool Asteroid::joystick_axis(unsigned int axis_idx, int value, std::chrono::system_clock::time_point t) {
	// le joy droit a les axis_idx 2 et 3 qui ne sont pas gérés par Asteroid pour l'instant
	if (axis_idx> 1) {
		return false;
	}

	// -1 < fvalue < 1
	float fvalue= float(value)/ 32768.0;
	const float SET_SCORE_NAME_JOY_THRESH= 0.98;

	if (_mode== PLAYING) {
		_joystick[axis_idx]= fvalue;
		return true;
	}
	else if (_mode== INACTIVE) {
	}
	else if (_mode== SET_SCORE_NAME) {
		if (abs(fvalue)> SET_SCORE_NAME_JOY_THRESH) {
			if (axis_idx== 0 && fvalue> 0.0) {
				_new_highest_char_idx++;
				if (_new_highest_char_idx> 2) {
					_new_highest_char_idx= 2;
				}
			}
			else if (axis_idx== 0 && fvalue< 0.0) {
				_new_highest_char_idx--;
				if (_new_highest_char_idx< 0) {
					_new_highest_char_idx= 0;
				}
			}
			else if (axis_idx== 1 && fvalue< 0.0) {
				_highest_scores[_new_highest_idx].first[_new_highest_char_idx]++;
				if (_highest_scores[_new_highest_idx].first[_new_highest_char_idx]> 'Z') {
					_highest_scores[_new_highest_idx].first[_new_highest_char_idx]= 'Z';
				}
			}
			else if (axis_idx== 1 && fvalue> 0.0) {
				_highest_scores[_new_highest_idx].first[_new_highest_char_idx]--;
				if (_highest_scores[_new_highest_idx].first[_new_highest_char_idx]< 'A') {
					_highest_scores[_new_highest_idx].first[_new_highest_char_idx]= 'A';
				}
			}
		}
	}
	return false;
}


void Asteroid::add_level_events(std::chrono::system_clock::time_point t) {
	auto d= std::chrono::duration_cast<std::chrono::milliseconds>(t- _levels[_current_level_idx]->_t_start).count();
	for (int idx_event=_levels[_current_level_idx]->_current_event_idx; idx_event>=0; --idx_event) {
		Event * event= _levels[_current_level_idx]->_events[idx_event];
		if (d> event->_t) {
			if (event->_type== NEW_ENEMY) {
				_ships.push_back(new Ship(_models[event->_enemy], event->_position, false, t));
			}

			// fin du niveau courant
			if (event->_type== LEVEL_END) {
				// passage niveau suivant
				if (_current_level_idx< _levels.size()- 1) {
					set_level(_current_level_idx+ 1, t);
				}
				// fin jeu
				else {
					_mode= INACTIVE;
				}
				return;
			}
		}
		else {
			_levels[_current_level_idx]->_current_event_idx= idx_event;
			return;
		}
	}
}


void Asteroid::set_level(unsigned int level_idx, std::chrono::system_clock::time_point t) {
	_current_level_idx= level_idx;
	
	// on ajuste le start
	_levels[_current_level_idx]->reinit(t);
	
	set_music_with_fadeout(_levels[_current_level_idx]->_music_path, 2000);
}


void Asteroid::add_rand_enemy(std::chrono::system_clock::time_point t) {
	pt_type pos= pt_type(rand_float(_pt_min.x, _pt_max.x), _pt_max.y);

	std::string model_name= "";
	std::vector<std::string> enemies;
	for (auto model : _models) {
		if (model.second->_type== ENEMY) {
			enemies.push_back(model.first);
		}
	}
	int n= rand_int(0, enemies.size()- 1);
	model_name= enemies[n];

	_ships.push_back(new Ship(_models[model_name], pos, false, t));
}


void Asteroid::reinit(std::chrono::system_clock::time_point t) {
	_mode= INACTIVE;
	_score= 0;
	_current_level_idx= 0;
	for (auto level : _levels) {
		level->reinit(t);
	}
	
	for (auto ship : _ships) {
		delete ship;
	}
	_ships.clear();

	_ships.push_back(new Ship(_models["hero"], glm::vec2(0.0, _pt_min.y+ 2.0), true, t));

	_key_left= _key_right= _key_down= _key_up= false;
	_joystick[0]= _joystick[1]= 0.0;
}


void Asteroid::read_highest_scores() {
	std::ifstream ifs("../data/scores/highest_scores.json");
	json js= json::parse(ifs);
	ifs.close();
	for (auto & score : js) {
		_highest_scores.push_back(std::make_pair(score["name"], score["score"]));
	}
}


void Asteroid::write_highest_scores() {
	json js;
	for (auto score : _highest_scores) {
		json entry;
		entry["name"]= score.first;
		entry["score"]= score.second;
		js.push_back(entry);
	}
	std::ofstream ofs("../data/scores/highest_scores.json");
	ofs << js << "\n";
}


void Asteroid::set_music(std::string music_path, unsigned int music_fade_in_ms) {
	
	if (Asteroid::_music!= NULL) {
		Mix_FreeMusic(Asteroid::_music);
	}
	Asteroid::_music= Mix_LoadMUS(music_path.c_str());
	//Mix_PlayMusic(_music, -1);
	Mix_FadeInMusic(Asteroid::_music, -1, music_fade_in_ms);
}


void Asteroid::music_finished_callback() {
	std::cout << "music_finished_callback : _next_music_path=" << _next_music_path << "\n";
	set_music(_next_music_path);
}


void Asteroid::set_music_with_fadeout(std::string music_path, unsigned int music_fade_out_ms, unsigned int music_fade_in_ms) {
	Mix_FadeOutMusic(music_fade_out_ms);
	_next_music_path= music_path;
	Mix_HookMusicFinished(Asteroid::music_finished_callback);
}
