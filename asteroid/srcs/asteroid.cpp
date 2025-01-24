#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <SDL2/SDL_image.h>

#include "asteroid.h"
#include "utile.h"

using json = nlohmann::json;


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
	//std::cout << json_path << "\n";
	std::string tmp_json_path= "./tmp.json";
	std::string cmd= "../srcs/flat_json_ship.py "+ json_path+ " "+ tmp_json_path;
	system(cmd.c_str());

	std::ifstream ifs(tmp_json_path);
	json js= json::parse(ifs);
	ifs.close();

	std::string cmd2= "rm "+ tmp_json_path;
	system(cmd2.c_str());

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
	_score= 0;
	if (js["score"]!= nullptr) {
		_score= js["score"];
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
}


ShipModel::~ShipModel() {

}


std::ostream & operator << (std::ostream & os, const ShipModel & model) {
	os << "json = " << model._json_path << " ; type=" << model._type << " ; size=" << glm::to_string(model._size);
	os << " ; score=" << model._score << " ; lives=" << model._lives;
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


Ship::Ship(ShipModel * model, pt_type pos, bool friendly) :
	_model(model), _friendly(friendly), _dead(false), _shooting(false), _current_action_name("main"),
	_idx_action(0), _velocity(glm::vec2(0.0)),
	_t_action_start(std::chrono::system_clock::now()), _t_last_bullet(std::chrono::system_clock::now()),
	_idx_anim(0), _t_anim_start(std::chrono::system_clock::now()), _t_last_hit(std::chrono::system_clock::now()),
	_hit(false), _hit_value(0.0), _t_die(std::chrono::system_clock::now()), _delete(false), _alpha(1.0)
{
	_aabb= AABB_2D(pos, model->_size);
	ActionTexture * current_texture= get_current_texture();
	_footprint= AABB_2D(
		pt_type(pos.x+ current_texture->_footprint._pos.x* _model->_size.x, pos.y+ current_texture->_footprint._pos.y* _model->_size.y),
		pt_type(_model->_size.x* current_texture->_footprint._size.x, _model->_size.y* current_texture->_footprint._size.y)
	);
	_lives= _model->_lives;
}


Ship::~Ship() {
	
}


void Ship::anim() {
	std::chrono::system_clock::time_point now= std::chrono::system_clock::now();
	_shooting= false;

	if (_dead) {
		//_hit= false;
		//_hit_value= 0.0;
		auto d_death= std::chrono::duration_cast<std::chrono::milliseconds>(now- _t_die).count();
		if (d_death> DEATH_MS) {
			_delete= true;
			_alpha= 0.0;
			return;
		}
		else {
			_alpha= float(DEATH_MS- d_death)/ float(DEATH_MS);
		}
	}

	if (_hit) {
		auto d_hit= std::chrono::duration_cast<std::chrono::milliseconds>(now- _t_last_hit).count();
		if (d_hit> HIT_UNTOUCHABLE_MS) {
			_hit= false;
			_hit_value= 0.0;
		}
		else {
			_hit_value= float(HIT_UNTOUCHABLE_MS- d_hit)/ float(HIT_UNTOUCHABLE_MS);
		}
	}
	else {
		_hit_value= 0.0;
	}
	
	// chgmt action ; _t < 0 correspond à une action infinie
	auto d_action= std::chrono::duration_cast<std::chrono::milliseconds>(now- _t_action_start).count();
	if (_model->_actions[_current_action_name][_idx_action]->_t> 0 && d_action> _model->_actions[_current_action_name][_idx_action]->_t) {
		_t_action_start= now;
		_idx_action++;
		if (_idx_action>= _model->_actions[_current_action_name].size()) {
			_idx_action= 0;
		}
	}

	ShipModel * bullet_model= get_current_bullet_model();
	ActionTexture * texture= get_current_texture();

	// faut-il tirer
	if (bullet_model!= NULL) {
		auto d_shoot= std::chrono::duration_cast<std::chrono::milliseconds>(now- _t_last_bullet).count();
		if (d_shoot> _model->_actions[_current_action_name][_idx_action]->_t_shooting) {
			_t_last_bullet= now;
			_shooting= true;
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
	auto d_anim= std::chrono::duration_cast<std::chrono::milliseconds>(now- _t_anim_start).count();
	if (d_anim> texture->_t_anims[_idx_anim]) {
		_t_anim_start= now;
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


void Ship::set_current_action(std::string action_name) {
	std::chrono::system_clock::time_point now= std::chrono::system_clock::now();

	_current_action_name= action_name;
	_idx_action= 0;
	_t_action_start= now;
	_idx_anim= 0;
	_t_anim_start= now;
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


Level::Level(std::string json_path) : _t_start(std::chrono::system_clock::now()) {
	std::string tmp_json_path= "./tmp.json";
	std::string cmd= "../srcs/flat_json_level.py "+ json_path+ " "+ tmp_json_path;
	system(cmd.c_str());

	std::ifstream ifs(tmp_json_path);
	json js= json::parse(ifs);
	ifs.close();

	std::string cmd2= "rm "+ tmp_json_path;
	system(cmd2.c_str());

	for (auto event : js) {
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


void Level::reinit() {
	_current_event_idx= _events.size()- 1;
	_t_start= std::chrono::system_clock::now();
}


// Asteroid -------------------------------------------------------------
Asteroid::Asteroid() {

}


Asteroid::Asteroid(GLuint prog_aabb, GLuint prog_texture, GLuint prog_font, ScreenGL * screengl) 
	: _draw_aabb(false), _draw_footprint(false), _draw_texture(true),
	_key_left(false), _key_right(false), _key_up(false), _key_down(false),
	_mode(INACTIVE), _score(0), _current_level_idx(0) {

	_pt_min= glm::vec2(-screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f);
	_pt_max= glm::vec2(screengl->_gl_width* 0.5f, screengl->_gl_height* 0.5f);
	_camera2clip= glm::ortho(-screengl->_gl_width* 0.5f, screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f, screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
	_font= new Font(prog_font, "../../fonts/Silom.ttf", 48, screengl);

	load_models();

	for (auto model : _models) {
		std::cout << model.first << " : " << *model.second << "\n";
	}

	load_levels();

	read_highest_scores();

	reinit();

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
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in", "hit_in", "alpha_in"},
		std::vector<std::string>{"camera2clip_matrix", "texture_array"});

	update_border_aabb();
	update_ship_aabb();
	update_ship_footprint();
	update_ship_texture();
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

	glGenTextures(1, &_texture_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);

	unsigned int n_tex= 0;
	for (auto model : _models) {
		for (auto texture : model.second->_textures) {
			n_tex+= texture.second->_pngs.size();
		}
	}

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


void Asteroid::load_levels() {
	std::vector<std::string> jsons= list_files("../data/levels", "json");
	// les json_path sont triés par nom, donc level1, 2, ... devraient être dans l'ordre
	for (auto json_path : jsons) {
		_levels.push_back(new Level(json_path));
	}
}


void Asteroid::draw_border_aabb() {
	DrawContext * context= _contexts["border_aabb"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	
	glEnableVertexAttribArray(context->_locs["position_in"]);
	glEnableVertexAttribArray(context->_locs["color_in"]);

	glVertexAttribPointer(context->_locs["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(GL_FLOAT), (void*)0);
	glVertexAttribPointer(context->_locs["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(GL_FLOAT), (void*)(2* sizeof(GL_FLOAT)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	glDisableVertexAttribArray(context->_locs["position_in"]);
	glDisableVertexAttribArray(context->_locs["color_in"]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Asteroid::draw_ship_aabb() {
	DrawContext * context= _contexts["ship_aabb"];
	
	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	
	glEnableVertexAttribArray(context->_locs["position_in"]);
	glEnableVertexAttribArray(context->_locs["color_in"]);

	glVertexAttribPointer(context->_locs["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(GL_FLOAT), (void*)0);
	glVertexAttribPointer(context->_locs["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(GL_FLOAT), (void*)(2* sizeof(GL_FLOAT)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	glDisableVertexAttribArray(context->_locs["position_in"]);
	glDisableVertexAttribArray(context->_locs["color_in"]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Asteroid::draw_ship_footprint() {
	DrawContext * context= _contexts["ship_footprint"];
	
	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	
	glEnableVertexAttribArray(context->_locs["position_in"]);
	glEnableVertexAttribArray(context->_locs["color_in"]);

	glVertexAttribPointer(context->_locs["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(GL_FLOAT), (void*)0);
	glVertexAttribPointer(context->_locs["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(GL_FLOAT), (void*)(2* sizeof(GL_FLOAT)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	glDisableVertexAttribArray(context->_locs["position_in"]);
	glDisableVertexAttribArray(context->_locs["color_in"]);

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

	glUniform1i(context->_locs["texture_array"], 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(context->_locs["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	
	glEnableVertexAttribArray(context->_locs["position_in"]);
	glEnableVertexAttribArray(context->_locs["tex_coord_in"]);
	glEnableVertexAttribArray(context->_locs["current_layer_in"]);
	glEnableVertexAttribArray(context->_locs["hit_in"]);
	glEnableVertexAttribArray(context->_locs["alpha_in"]);

	glVertexAttribPointer(context->_locs["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs["tex_coord_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));
	glVertexAttribPointer(context->_locs["current_layer_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(4* sizeof(float)));
	glVertexAttribPointer(context->_locs["hit_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(5* sizeof(float)));
	glVertexAttribPointer(context->_locs["alpha_in"], 1, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(6* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	glDisableVertexAttribArray(context->_locs["position_in"]);
	glDisableVertexAttribArray(context->_locs["tex_coord_in"]);
	glDisableVertexAttribArray(context->_locs["current_layer_in"]);
	glDisableVertexAttribArray(context->_locs["hit_in"]);
	glDisableVertexAttribArray(context->_locs["alpha_in"]);

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

	_font->set_text_group(0, texts);
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
	_font->set_text_group(0, texts);
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

	_font->set_text_group(0, texts);
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
	context->_n_attrs_per_pts= 7;

	const unsigned int n_pts_per_ship= 6;

	for (auto ship : _ships) {
		context->_n_pts+= n_pts_per_ship;
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];

	std::chrono::system_clock::time_point now= std::chrono::system_clock::now();

	// à cause du système de reference opengl j'ai du inverser les 0 et les 1 des y des textures
	for (unsigned int idx_ship=0; idx_ship<_ships.size(); ++idx_ship) {
		Ship * ship= _ships[idx_ship];
		ActionTexture * texture= ship->get_current_texture();
		
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
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Asteroid::anim_playing() {
	/*if (rand_int(0, 100)> 98) {
		add_rand_enemy();
	}*/

	add_level_events();

	//std::cout << "joy=(" << _joystick[0] << " ; " << _joystick[1] << ") ; key_left=" << _key_left  << " ; key_right=" << _key_right << " ; key_down=" << _key_down  << " ; key_up=" << _key_up << "\n";

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

	// ne pas utiliser auto ici car les push_back dans le for modifie l'itérateur
	//for (auto ship : _ships) {
	for (unsigned int idx_ship=0; idx_ship<_ships.size(); ++idx_ship) {
		Ship * ship= _ships[idx_ship];
		ship->anim();

		if (ship->_shooting) {
			bool friendly= false;
			if (ship->_model->_type== HERO) {
				friendly= true;
			}
			ShipModel * bullet_model= ship->get_current_bullet_model();
			_ships.push_back(new Ship(bullet_model, ship->_footprint.center()- 0.5* bullet_model->_size, friendly));
		}
	}

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

	for (unsigned int idx_ship=0; idx_ship<_ships.size()- 1; ++idx_ship) {
		if (_ships[idx_ship]->_dead) {
			continue;
		}
		for (unsigned int idx_ship2=idx_ship+ 1; idx_ship2<_ships.size(); ++idx_ship2) {
			if (_ships[idx_ship2]->_dead) {
				continue;
			}
			if (
				(_ships[idx_ship]->_friendly && _ships[idx_ship2]->_friendly) ||
				(!_ships[idx_ship]->_friendly && !_ships[idx_ship2]->_friendly)
			) {
				continue;
			}

			// les footprint sont utilisés pour la détection de collision
			if (aabb_intersects_aabb(&_ships[idx_ship]->_footprint, &_ships[idx_ship2]->_footprint)) {
				if (!_ships[idx_ship]->_hit) {
					_ships[idx_ship]->_hit= true;
					_ships[idx_ship]->_t_last_hit= std::chrono::system_clock::now();
					_ships[idx_ship]->_lives--;
					if (_ships[idx_ship]->_lives<= 0) {
						_ships[idx_ship]->_dead= true;
						_ships[idx_ship]->_t_die= std::chrono::system_clock::now();
						if (!_ships[idx_ship]->_friendly) {
							_score+= _ships[idx_ship]->_model->_score;
						}
					}
				}

				if (!_ships[idx_ship2]->_hit) {
					_ships[idx_ship2]->_hit= true;
					_ships[idx_ship2]->_t_last_hit= std::chrono::system_clock::now();
					_ships[idx_ship2]->_lives--;
					if (_ships[idx_ship2]->_lives<= 0) {
						_ships[idx_ship2]->_dead= true;
						_ships[idx_ship2]->_t_die= std::chrono::system_clock::now();
						if (!_ships[idx_ship2]->_friendly) {
							_score+= _ships[idx_ship2]->_model->_score;
						}
					}
				}
			}
		}
	}

	if (_ships[0]->_dead) {
		_mode= INACTIVE;
		_new_highest_idx= 0;
		_new_highest_char_idx= 0;
		for (int i=0; i<_highest_scores.size(); ++i) {
			if (_score> _highest_scores[i].second) {
				_new_highest_idx= i;
				_mode= SET_SCORE_NAME;
				_highest_scores.insert(_highest_scores.begin()+ _new_highest_idx, std::make_pair("AAA", _score));
				_highest_scores.pop_back();
				break;
			}
		}
	}

	_ships.erase(std::remove_if(_ships.begin(), _ships.end(), [](Ship * s){
		return s->_delete;
	}), _ships.end());

	update_ship_aabb();
	update_ship_footprint();
	update_ship_texture();
}


void Asteroid::anim_inactive() {

}


void Asteroid::anim_set_score_name() {

}


void Asteroid::anim() {
	if (_mode== PLAYING) {
		anim_playing();
	}
	else if (_mode== INACTIVE) {
		anim_inactive();
	}
	else if (_mode== SET_SCORE_NAME) {
		anim_set_score_name();
	}
}


bool Asteroid::key_down(InputState * input_state, SDL_Keycode key) {
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
			_ships[0]->set_current_action("shoot_a");
			return true;
		}
		else if (key== SDLK_z) {
			_ships[0]->set_current_action("shoot_b");
			return true;
		}
	}
	else if (_mode== INACTIVE) {
		reinit();
		_mode= PLAYING;
		return true;
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
			write_highest_scores();
			return true;
		}
	}
	return false;
}


bool Asteroid::key_up(InputState * input_state, SDL_Keycode key) {
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
			_ships[0]->set_current_action("no_shoot");
			return true;
		}
		else if (key== SDLK_z) {
			_ships[0]->set_current_action("no_shoot");
			return true;
		}
	}
	else if (_mode== INACTIVE) {
	}
	else if (_mode== SET_SCORE_NAME) {
	}
	return false;
}


bool Asteroid::joystick_down(unsigned int button_idx) {
	if (_mode== PLAYING) {
		if (button_idx== 0) {
			_ships[0]->set_current_action("shoot_a");
			return true;
		}
		else if (button_idx== 1) {
			_ships[0]->set_current_action("shoot_b");
			return true;
		}
	}
	else if (_mode== INACTIVE) {
		reinit();
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


bool Asteroid::joystick_up(unsigned int button_idx) {
	if (_mode== PLAYING) {
		if (button_idx== 0) {
			_ships[0]->set_current_action("no_shoot");
			return true;
		}
		else if (button_idx== 1) {
			_ships[0]->set_current_action("no_shoot");
			return true;
		}
	}
	else if (_mode== INACTIVE) {
	}
	else if (_mode== SET_SCORE_NAME) {
	}
	return false;
}


bool Asteroid::joystick_axis(unsigned int axis_idx, int value) {
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


void Asteroid::add_level_events() {
	std::chrono::system_clock::time_point now= std::chrono::system_clock::now();
	auto d= std::chrono::duration_cast<std::chrono::milliseconds>(now- _levels[_current_level_idx]->_t_start).count();
	//std::cout << "add_level_events : " << _current_level_idx << " ; " << _levels[_current_level_idx]->_current_event_idx << "\n";
	for (int idx_event=_levels[_current_level_idx]->_current_event_idx; idx_event>=0; --idx_event) {
		Event * event= _levels[_current_level_idx]->_events[idx_event];
		//std::cout << d << " ; " << *event << "\n";
		if (d> event->_t) {
			if (event->_type== NEW_ENEMY) {
				_ships.push_back(new Ship(_models[event->_enemy], event->_position, false));
			}
			if (event->_type== LEVEL_END) {
				_current_level_idx++;
				//std::cout << _current_level_idx << " ; " << _levels.size() << "\n";
				if (_current_level_idx< _levels.size()) {
					// on ajuste le start
					_levels[_current_level_idx]->reinit();
				}
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


void Asteroid::add_rand_enemy() {
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

	_ships.push_back(new Ship(_models[model_name], pos, false));
}


void Asteroid::reinit() {
	_mode= INACTIVE;
	_score= 0;
	_current_level_idx= 0;
	for (auto level : _levels) {
		level->reinit();
	}
	
	for (auto ship : _ships) {
		delete ship;
	}
	_ships.clear();

	_ships.push_back(new Ship(_models["hero"], glm::vec2(0.0, _pt_min.y+ 2.0), true));
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
