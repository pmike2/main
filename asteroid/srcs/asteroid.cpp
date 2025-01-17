#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "asteroid.h"
#include "utile.h"

using json = nlohmann::json;


// action -----------------------------------------------------------
Action::Action() {

}


Action::Action(glm::vec2 direction, int t, std::string bullet_name, unsigned int t_shooting) :
	_direction(direction), _t(t), _bullet_name(bullet_name), _bullet_model(NULL), _t_shooting(t_shooting) {

}


Action::~Action() {
	
}


// model -----------------------------------------------------------
ShipModel::ShipModel() {

}


ShipModel::ShipModel(std::string json_path) : _json_path(json_path) {
	std::string tmp_json_path= "./tmp.json";
	std::string cmd= "../srcs/flat_json.py "+ json_path+ " "+ tmp_json_path;
	system(cmd.c_str());

	std::ifstream ifs(tmp_json_path);
	json js= json::parse(ifs);
	ifs.close();

	std::string cmd2= "rm "+ tmp_json_path;
	system(cmd2.c_str());

	_size= glm::vec2(js["size"][0], js["size"][1]);
	_lives= 1;
	if (js["lives"]!= nullptr) {
		_lives= js["lives"];
	}
	_score= 0;
	if (js["score"]!= nullptr) {
		_score= js["score"];
	}

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

			_actions[action_name].push_back(new Action(direction, t, bullet_name, t_shooting));
		}
	}
}


ShipModel::~ShipModel() {

}


std::ostream & operator << (std::ostream & os, const ShipModel & model) {
	os << "json = " << model._json_path << " ; actions = [";
	for (auto action : model._actions) {
		os << action.first << " , ";
	}
	os << "]";
	return os;
}


// bullet -------------------------------------------------------------
/*Bullet::Bullet() {

}


Bullet::Bullet(const AABB_2D & aabb, bool friendly, glm::vec2 velocity) :
	_aabb(aabb), _friendly(friendly), _velocity(velocity), _dead(false) {
	
}


Bullet::~Bullet() {
	
}


void Bullet::anim() {
	_aabb._pos+= _velocity;
}
*/

// ship -------------------------------------------------------------
Ship::Ship() {

}


Ship::Ship(ShipModel * model, pt_type pos, bool friendly) :
	_model(model), _friendly(friendly), _dead(false), _shooting(false), _current_action_name("main"), _current_action_bullet_name("main"),
	_idx_action(0), _idx_action_bullet(0),
	_t_action_start(std::chrono::system_clock::now()), _t_last_bullet(std::chrono::system_clock::now()), _t_bullet_start(std::chrono::system_clock::now())
{
	_aabb= AABB_2D(pos, model->_size);
	_lives= _model->_lives;
}


Ship::~Ship() {
	
}


void Ship::anim(bool is_hero) {
	//std::cout << _current_action_name << "\n";

	std::chrono::system_clock::time_point now= std::chrono::system_clock::now();
	
	// chgmt action ; _t < 0 correspond à une action infinie
	auto d= std::chrono::duration_cast<std::chrono::milliseconds>(now- _t_action_start).count();
	if (_model->_actions[_current_action_name][_idx_action]->_t> 0 && d> _model->_actions[_current_action_name][_idx_action]->_t) {
		_t_action_start= now;
		_idx_action++;
		if (_idx_action>= _model->_actions[_current_action_name].size()) {
			_idx_action= 0;
		}
		_idx_action_bullet= 0;
		_t_bullet_start= now;
	}

	ShipModel * bullet_model= get_current_bullet_model();

	// chgmt action bullet
	if (bullet_model!= NULL) {
		auto d2= std::chrono::duration_cast<std::chrono::milliseconds>(now- _t_bullet_start).count();
		
		if (bullet_model->_actions[_current_action_bullet_name][_idx_action_bullet]->_t> 0 && d2> bullet_model->_actions[_current_action_bullet_name][_idx_action_bullet]->_t) {
			_t_bullet_start= now;
			_idx_action_bullet++;
			if (_idx_action_bullet>= bullet_model->_actions[_current_action_bullet_name].size()) {
				_idx_action_bullet= 0;
			}
		}
	}

	// faut-il tirer
	_shooting= false;
	if (bullet_model!= NULL) {
		auto d3= std::chrono::duration_cast<std::chrono::milliseconds>(now- _t_last_bullet).count();
		//std::cout << d3 << " ; " << bullet_model->_actions[_current_action_bullet_name][_idx_action_bullet]->_t_shooting << "\n";
		//if (d3> bullet_model->_actions[_current_action_bullet_name][_idx_action_bullet]->_t_shooting) {
		if (d3> _model->_actions[_current_action_name][_idx_action]->_t_shooting) {
			_t_last_bullet= now;
			_shooting= true;
		}
	}

	// maj vitesse et position
	if (!is_hero) {
		_velocity= _model->_actions[_current_action_name][_idx_action]->_direction;
	}
	_aabb._pos+= _velocity;
}


ShipModel * Ship::get_current_bullet_model() {
	return _model->_actions[_current_action_name][_idx_action]->_bullet_model;
}


void Ship::set_current_action(std::string action_name) {
	std::chrono::system_clock::time_point now= std::chrono::system_clock::now();

	_current_action_name= action_name;
	_current_action_bullet_name= "main";
	_idx_action= 0;
	_idx_action_bullet= 0;
	_t_action_start= now;
	_t_bullet_start= now;
}

std::ostream & operator << (std::ostream & os, const Ship & ship) {
	os << "model = " << *ship._model << " ; aabb = [" << ship._aabb << "]";
	return os;
}


// level -------------------------------------------------------------
Level::Level() {

}


Level::Level(GLuint prog_draw_simple, GLuint prog_draw_texture) 
	: _pt_min(-10.0, -10.0), _pt_max(10.0, 10.0), _draw_aabb(true), _draw_texture(false),
	_key_left(false), _key_right(false), _key_up(false), _key_down(false),
	_gameover(false), _score(0) {

	std::vector<std::string> jsons= list_files("../data", "json");
	for (auto json_path : jsons) {
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

	/*_models["hero"]= new ShipModel("../data/hero.json");
	_models["cruiser"]= new ShipModel("../data/cruiser.json");
	_models["fatman"]= new ShipModel("../data/fatman.json");*/

	reinit();

	unsigned int n_buffers= 3;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	_contexts["border_aabb"]= new DrawContext(prog_draw_simple, _buffers[0],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});

	_contexts["ship_aabb"]= new DrawContext(prog_draw_simple, _buffers[1],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});
	
	_contexts["bullet_aabb"]= new DrawContext(prog_draw_simple, _buffers[2],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});

	/*_contexts["texture"]= new DrawContext(prog_draw_texture, _buffers[],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in"},
		std::vector<std::string>{"world2clip_matrix", "diffuse_texture_array"});*/
	update_border_aabb();
	update_ship_aabb();
	//update_bullet_aabb();
}


Level::~Level() {
	for (auto ship : _ships) {
		delete ship;
	}
	_ships.clear();

	/*for (auto bullet : _bullets) {
		delete bullet;
	}
	_bullets.clear();*/

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


void Level::draw_border_aabb(const glm::mat4 & world2clip) {
	DrawContext * context= _contexts["border_aabb"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
	glEnableVertexAttribArray(context->_locs["position_in"]);
	glEnableVertexAttribArray(context->_locs["color_in"]);

	glVertexAttribPointer(context->_locs["position_in"], 3, GL_FLOAT, GL_FALSE, 7* sizeof(GL_FLOAT), (void*)0);
	glVertexAttribPointer(context->_locs["color_in"], 4, GL_FLOAT, GL_FALSE, 7* sizeof(GL_FLOAT), (void*)(3* sizeof(GL_FLOAT)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	glDisableVertexAttribArray(context->_locs["position_in"]);
	glDisableVertexAttribArray(context->_locs["color_in"]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Level::draw_ship_aabb(const glm::mat4 & world2clip) {
	DrawContext * context= _contexts["ship_aabb"];
	
	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
	glEnableVertexAttribArray(context->_locs["position_in"]);
	glEnableVertexAttribArray(context->_locs["color_in"]);

	glVertexAttribPointer(context->_locs["position_in"], 3, GL_FLOAT, GL_FALSE, 7* sizeof(GL_FLOAT), (void*)0);
	glVertexAttribPointer(context->_locs["color_in"], 4, GL_FLOAT, GL_FALSE, 7* sizeof(GL_FLOAT), (void*)(3* sizeof(GL_FLOAT)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	glDisableVertexAttribArray(context->_locs["position_in"]);
	glDisableVertexAttribArray(context->_locs["color_in"]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


/*void Level::draw_bullet_aabb(const glm::mat4 & world2clip) {
	DrawContext * context= _contexts["bullet_aabb"];
	
	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
	glEnableVertexAttribArray(context->_locs["position_in"]);
	glEnableVertexAttribArray(context->_locs["color_in"]);

	glVertexAttribPointer(context->_locs["position_in"], 3, GL_FLOAT, GL_FALSE, 7* sizeof(GL_FLOAT), (void*)0);
	glVertexAttribPointer(context->_locs["color_in"], 4, GL_FLOAT, GL_FALSE, 7* sizeof(GL_FLOAT), (void*)(3* sizeof(GL_FLOAT)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	glDisableVertexAttribArray(context->_locs["position_in"]);
	glDisableVertexAttribArray(context->_locs["color_in"]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}*/


void Level::draw_texture(const glm::mat4 & world2clip) {

}


void Level::draw(const glm::mat4 & world2clip) {
	if (_draw_texture) {
		draw_texture(world2clip);
	}
	if (_draw_aabb) {
		draw_border_aabb(world2clip);
		draw_ship_aabb(world2clip);
		//draw_bullet_aabb(world2clip);
	}
}


void Level::update_border_aabb() {
	DrawContext * context= _contexts["border_aabb"];
	context->_n_pts= 8;
	context->_n_attrs_per_pts= 7;

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
		data[context->_n_attrs_per_pts* idx_pt+ 2]= 0.0;
		for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
			data[context->_n_attrs_per_pts* idx_pt+ 3+ idx_color]= border_color[idx_color];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Level::update_ship_aabb() {
	DrawContext * context= _contexts["ship_aabb"];
	context->_n_pts= 0;
	context->_n_attrs_per_pts= 7;

	const unsigned int n_pts_per_ship= 8;

	for (auto ship : _ships) {
		context->_n_pts+= n_pts_per_ship;
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];

	for (unsigned int idx_ship=0; idx_ship<_ships.size(); ++idx_ship) {
		glm::vec4 color;
		if (_ships[idx_ship]->_friendly) {
			color= friendly_color;
		}
		else {
			color= unfriendly_color;
		}
		number positions[n_pts_per_ship* 2]= {
			_ships[idx_ship]->_aabb._pos.x, _ships[idx_ship]->_aabb._pos.y,
			_ships[idx_ship]->_aabb._pos.x+ _ships[idx_ship]->_aabb._size.x, _ships[idx_ship]->_aabb._pos.y,

			_ships[idx_ship]->_aabb._pos.x+ _ships[idx_ship]->_aabb._size.x, _ships[idx_ship]->_aabb._pos.y,
			_ships[idx_ship]->_aabb._pos.x+ _ships[idx_ship]->_aabb._size.x, _ships[idx_ship]->_aabb._pos.y+ _ships[idx_ship]->_aabb._size.y,

			_ships[idx_ship]->_aabb._pos.x+ _ships[idx_ship]->_aabb._size.x, _ships[idx_ship]->_aabb._pos.y+ _ships[idx_ship]->_aabb._size.y,
			_ships[idx_ship]->_aabb._pos.x, _ships[idx_ship]->_aabb._pos.y+ _ships[idx_ship]->_aabb._size.y,

			_ships[idx_ship]->_aabb._pos.x, _ships[idx_ship]->_aabb._pos.y+ _ships[idx_ship]->_aabb._size.y,
			_ships[idx_ship]->_aabb._pos.x, _ships[idx_ship]->_aabb._pos.y
		};

		for (unsigned int idx_pt=0; idx_pt<n_pts_per_ship; ++idx_pt) {
			data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 0]= float(positions[2* idx_pt]);
			data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 1]= float(positions[2* idx_pt+ 1]);
			data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 2]= 0.0; // z
			for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
				data[idx_ship* n_pts_per_ship* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 3+ idx_color]= color[idx_color];
			}
		}
	}
	/*for (int i=0; i<_n_pts_aabb* n_attrs_per_pts; ++i) {
		std::cout << data[i] << " ; ";
	}
	std::cout << "\n";*/

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Level::anim() {
	if (_gameover) {
		return;
	}

	if (rand_int(0, 100)> 98) {
		add_rand_ennemy();
	}

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

	if (_ships[0]->_aabb._pos.x> _pt_max.x- _ships[0]->_aabb._size.x) {
		_ships[0]->_aabb._pos.x= _pt_max.x- _ships[0]->_aabb._size.x;
	}
	if (_ships[0]->_aabb._pos.x< _pt_min.x) {
		_ships[0]->_aabb._pos.x= _pt_min.x;
	}
	if (_ships[0]->_aabb._pos.y> _pt_max.x- _ships[0]->_aabb._size.y) {
		_ships[0]->_aabb._pos.y= _pt_max.x- _ships[0]->_aabb._size.y;
	}
	if (_ships[0]->_aabb._pos.y< _pt_min.y) {
		_ships[0]->_aabb._pos.y= _pt_min.y;
	}

	for (unsigned int idx_ship=0; idx_ship<_ships.size(); ++idx_ship) {
		bool is_hero= false;
		if (idx_ship== 0) {
			is_hero= true;
		}
		bool friendly= is_hero;
		_ships[idx_ship]->anim(is_hero);
		if (_ships[idx_ship]->_shooting) {
			_ships.push_back(new Ship(_ships[idx_ship]->get_current_bullet_model(), _ships[idx_ship]->_aabb.center(), friendly));
		}
	}

	for (unsigned int idx_ship=1; idx_ship<_ships.size(); ++idx_ship) {
		if (_ships[idx_ship]->_aabb._pos.x> _pt_max.x) {
			_ships[idx_ship]->_dead= true;
		}
		else if (_ships[idx_ship]->_aabb._pos.x< _pt_min.x- _ships[idx_ship]->_aabb._size.x) {
			_ships[idx_ship]->_dead= true;
		}
		if (_ships[idx_ship]->_aabb._pos.y> _pt_max.y) {
			_ships[idx_ship]->_dead= true;
		}
		else if (_ships[idx_ship]->_aabb._pos.y< _pt_min.y- _ships[idx_ship]->_aabb._size.y) {
			_ships[idx_ship]->_dead= true;
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
			if (aabb_intersects_aabb(&_ships[idx_ship]->_aabb, &_ships[idx_ship2]->_aabb)) {
				_ships[idx_ship]->_lives--;
				_ships[idx_ship2]->_lives--;
				if (_ships[idx_ship]->_lives<= 0) {
					_ships[idx_ship]->_dead= true;
					if (!_ships[idx_ship]->_friendly) {
						_score+= _ships[idx_ship]->_model->_score;
					}
				}
				if (_ships[idx_ship2]->_lives<= 0) {
					_ships[idx_ship2]->_dead= true;
					if (!_ships[idx_ship2]->_friendly) {
						_score+= _ships[idx_ship2]->_model->_score;
					}
				}
			}
		}
	}

	if (_ships[0]->_dead) {
		_gameover= true;
		std::cout << "gameover\n";
		return;
	}

	_ships.erase(std::remove_if(_ships.begin(), _ships.end(), [](Ship * s){
		return s->_dead;
	}), _ships.end());

	update_ship_aabb();
}


bool Level::key_down(InputState * input_state, SDL_Keycode key) {
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
		_ships[0]->set_current_action("shoot");
		return true;
	}
	return false;
}


bool Level::key_up(InputState * input_state, SDL_Keycode key) {
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
		reinit();
	}
	return false;
}


bool Level::joystick_down(unsigned int button_idx) {
	//std::cout << button_idx << "\n";
	if (button_idx== 0) {
		_ships[0]->_shooting= true;
		return true;
	}
	return false;
}


bool Level::joystick_up(unsigned int button_idx) {
	//std::cout << button_idx << "\n";
	if (button_idx== 0) {
		_ships[0]->_shooting= false;
		return true;
	}
	return false;
}


bool Level::joystick_axis(unsigned int axis_idx, int value) {
	//std::cout << "axis_idx=" << axis_idx << " ; value=" << value << "\n";
	float fvalue= float(value)/ 32768.0;
	_joystick[axis_idx]= fvalue;
	return true;
}


void Level::add_rand_ennemy() {
	//pt_type pos= pt_type(rand_float(_pt_min.x, _pt_max.x), rand_float(_pt_min.y, _pt_max.y));
	pt_type pos= pt_type(rand_float(_pt_min.x, _pt_max.x), _pt_max.y);
	//pt_type size= pt_type(rand_float(0.5, 4.0), rand_float(0.5, 4.0));
	//AABB_2D aabb(pos, size);

	std::string model_name= "";
	int n= rand_int(0, 1);
	if (n== 0) {
		model_name= "cruiser";
	}
	else if (n== 1) {
		model_name= "fatman";
	}
	//model_name= "cruiser";

	Ship * ennemy= new Ship(_models[model_name], pos, false);
	bool ok= true;
	for (auto ship : _ships) {
		if (aabb_intersects_aabb(&ship->_aabb, &ennemy->_aabb)) {
			ok= false;
			break;
		}
	}
	if (ok) {
		_ships.push_back(ennemy);
	}
	else {
		delete ennemy;
	}
}


void Level::reinit() {
	_gameover= false;
	_score= 0;
	
	for (auto ship : _ships) {
		delete ship;
	}
	_ships.clear();

	/*for (auto bullet : _bullets) {
		delete bullet;
	}
	_bullets.clear();*/

	_ships.push_back(new Ship(_models["hero"], glm::vec2(0.0, _pt_min.y+ 2.0), true));

	unsigned int n_ennemies= rand_int(10, 20);
	//unsigned int n_ennemies= 1;
	/*for (unsigned int i=0; i<n_ennemies; ++i) {
		add_rand_ennemy();
	}*/
}
