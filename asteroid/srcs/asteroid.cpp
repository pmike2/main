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


Action::Action(glm::vec2 direction, unsigned int t, bool shooting, unsigned int t_shooting) :
	_direction(direction), _t(t), _shooting(shooting), _t_shooting(t_shooting) {

}


Action::~Action() {
	
}


// model -----------------------------------------------------------
ShipModel::ShipModel() {

}


ShipModel::ShipModel(std::string json_path) {
	std::string tmp_json_path= "./tmp.json";
	std::string cmd= "../srcs/flat_json.py "+ json_path+ " "+ tmp_json_path;
	//std::cout << cmd << "\n";
	system(cmd.c_str());

	std::ifstream ifs(tmp_json_path);
	json js= json::parse(ifs);
	ifs.close();

	std::string cmd2= "rm "+ tmp_json_path;
	system(cmd.c_str());

	//std::string name= js["name"];
	_size= glm::vec2(js["size"][0], js["size"][1]);

	for (auto & action : js["actions"]) {
		glm::vec2 direction(action["direction"][0], action["direction"][1]);
		unsigned int t= action["t"];
		bool shooting= false;
		unsigned int t_shooting= 0;
		if (action["shooting"]!= nullptr) {
			shooting= true;
			t_shooting= action["shooting"];
		}

		_actions.push_back(new Action(direction, t, shooting, t_shooting));
	}

	/*while (true) {
		bool modified= false;
		unsigned int action_idx= 0;
		for (json::iterator it = js["actions"].begin(); it != js["actions"].end(); ++it) {
			//std::cout << it.key() << " : " << it.value() << "\n";
			auto & action_name= it.key();
			auto & l_actions= it.value();
			if (modified) {
				break;
			}
			for (auto & action :l_actions) {
				if (action["name"]!= nullptr) {
					//std::cout << action["name"] << "\n";
					unsigned int n= 1;
					if (action["n"]!= nullptr) {
						n= action["n"];
					}
					json::iterator it_ok;
					for (json::iterator it2 = js["actions"].begin(); it2 != js["actions"].end(); ++it2) {
						auto & action_name2= it.key();
						auto & l_actions2= it.value();
						if (action_name2== action["name"]) {
							it_ok= it2;
							break;
						}
					}
					modified= true;
					js["actions"][action_idx]= [];
					break;
				}
				action_idx++;
			}
		}
		if (!modified) {
			break;
		}
	}*/
}


ShipModel::~ShipModel() {
	for (auto action : _actions) {
		delete action;
	}
	_actions.clear();
}


// bullet -------------------------------------------------------------
Bullet::Bullet() {

}


Bullet::Bullet(const AABB_2D & aabb, bool friendly, glm::vec2 velocity) :
	_aabb(aabb), _friendly(friendly), _velocity(velocity), _dead(false) {
	
}


Bullet::~Bullet() {
	
}


void Bullet::anim() {
	_aabb._pos+= _velocity;
}


// ship -------------------------------------------------------------
Ship::Ship() {

}


Ship::Ship(ShipModel * model, pt_type pos, bool friendly, glm::vec2 velocity) :
	_model(model), _friendly(friendly), _velocity(velocity), _dead(false), _idx_action(0),
	_t_action_start(std::chrono::system_clock::now()), _t_last_bullet(std::chrono::system_clock::now())
{
	_aabb= AABB_2D(pos, model->_size);
}


Ship::~Ship() {
	
}


Bullet * Ship::anim() {
	std::chrono::system_clock::time_point now= std::chrono::system_clock::now();
	
	auto d= std::chrono::duration_cast<std::chrono::milliseconds>(now- _t_action_start).count();
	if (d> _model->_actions[_idx_action]->_t) {
		_t_action_start= now;
		_idx_action++;
		if (_idx_action>= _model->_actions.size()) {
			_idx_action= 0;
		}
	}
	_velocity= _model->_actions[_idx_action]->_direction;

	_aabb._pos+= _velocity;

	Bullet * bullet= NULL;
	if (_model->_actions[_idx_action]->_shooting) {
		auto d2= std::chrono::duration_cast<std::chrono::milliseconds>(now- _t_last_bullet).count();
		if (d2> _model->_actions[_idx_action]->_t_shooting) {
			_t_last_bullet= now;
			bullet= new Bullet(AABB_2D(_aabb.center(), glm::vec2(0.1, 0.2)), false, glm::vec2(0.0, -0.2));
		}
	}
	return bullet;
}


// level -------------------------------------------------------------
Level::Level() {

}


Level::Level(GLuint prog_draw_simple, GLuint prog_draw_texture) 
	: _pt_min(-10.0, -10.0), _pt_max(10.0, 10.0), _draw_aabb(true), _draw_texture(false),
	_key_left(false), _key_right(false), _key_up(false), _key_down(false),
	_shooting(false), _t_last_shooting(std::chrono::system_clock::now()), _gameover(false), _score(0) {

	std::vector<std::string> jsons= list_files("../data", "json");
	for (auto json_path : jsons) {
		_models[basename(json_path)]= new ShipModel(json_path);
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
	update_bullet_aabb();
}


Level::~Level() {
	for (auto ship : _ships) {
		delete ship;
	}
	_ships.clear();

	for (auto bullet : _bullets) {
		delete bullet;
	}
	_bullets.clear();

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


void Level::draw_bullet_aabb(const glm::mat4 & world2clip) {
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
}


void Level::draw_texture(const glm::mat4 & world2clip) {

}


void Level::draw(const glm::mat4 & world2clip) {
	if (_draw_texture) {
		draw_texture(world2clip);
	}
	if (_draw_aabb) {
		draw_border_aabb(world2clip);
		draw_ship_aabb(world2clip);
		draw_bullet_aabb(world2clip);
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


void Level::update_bullet_aabb() {
	DrawContext * context= _contexts["bullet_aabb"];
	context->_n_pts= 0;
	context->_n_attrs_per_pts= 7;

	const unsigned int n_pts_per_bullet= 8;

	for (auto bullet : _bullets) {
		context->_n_pts+= n_pts_per_bullet;
	}
	//std::cout << "_n_pts_aabb = " << _n_pts_aabb << "\n";

	float data[context->_n_pts* context->_n_attrs_per_pts];

	for (unsigned int idx_bullet=0; idx_bullet<_bullets.size(); ++idx_bullet) {
		glm::vec4 color;
		if (_bullets[idx_bullet]->_friendly) {
			color= friendly_color;
		}
		else {
			color= unfriendly_color;
		}
		number positions[n_pts_per_bullet* 2]= {
			_bullets[idx_bullet]->_aabb._pos.x, _bullets[idx_bullet]->_aabb._pos.y,
			_bullets[idx_bullet]->_aabb._pos.x+ _bullets[idx_bullet]->_aabb._size.x, _bullets[idx_bullet]->_aabb._pos.y,

			_bullets[idx_bullet]->_aabb._pos.x+ _bullets[idx_bullet]->_aabb._size.x, _bullets[idx_bullet]->_aabb._pos.y,
			_bullets[idx_bullet]->_aabb._pos.x+ _bullets[idx_bullet]->_aabb._size.x, _bullets[idx_bullet]->_aabb._pos.y+ _bullets[idx_bullet]->_aabb._size.y,

			_bullets[idx_bullet]->_aabb._pos.x+ _bullets[idx_bullet]->_aabb._size.x, _bullets[idx_bullet]->_aabb._pos.y+ _bullets[idx_bullet]->_aabb._size.y,
			_bullets[idx_bullet]->_aabb._pos.x, _bullets[idx_bullet]->_aabb._pos.y+ _bullets[idx_bullet]->_aabb._size.y,

			_bullets[idx_bullet]->_aabb._pos.x, _bullets[idx_bullet]->_aabb._pos.y+ _bullets[idx_bullet]->_aabb._size.y,
			_bullets[idx_bullet]->_aabb._pos.x, _bullets[idx_bullet]->_aabb._pos.y
		};

		for (unsigned int idx_pt=0; idx_pt<n_pts_per_bullet; ++idx_pt) {
			data[idx_bullet* n_pts_per_bullet* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 0]= float(positions[2* idx_pt]);
			data[idx_bullet* n_pts_per_bullet* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 1]= float(positions[2* idx_pt+ 1]);
			data[idx_bullet* n_pts_per_bullet* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 2]= 0.0; // z
			for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
				data[idx_bullet* n_pts_per_bullet* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 3+ idx_color]= color[idx_color];
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

	float HERO_VELOCITY= 0.1;
	if (_key_left) {
		_ships[0]->_aabb._pos.x-= HERO_VELOCITY;
	}
	if (_key_right) {
		_ships[0]->_aabb._pos.x+= HERO_VELOCITY;
	}
	if (_key_down) {
		_ships[0]->_aabb._pos.y-= HERO_VELOCITY;
	}
	if (_key_up) {
		_ships[0]->_aabb._pos.y+= HERO_VELOCITY;
	}

	// joystick vers le haut est négatif !
	_ships[0]->_aabb._pos.x+= HERO_VELOCITY* _joystick[0];
	_ships[0]->_aabb._pos.y-= HERO_VELOCITY* _joystick[1];

	
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

	for (unsigned int idx_ship=1; idx_ship<_ships.size(); ++idx_ship) {
		Bullet * bullet= _ships[idx_ship]->anim();
		if (bullet!= NULL) {
			_bullets.push_back(bullet);
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

		if (_ships[idx_ship]->_dead) {
			continue;
		}
		/*if (rand_int(0, 100)> 99) {
			_bullets.push_back(new Bullet(AABB_2D(_ships[idx_ship]->_aabb.center(), glm::vec2(0.1, 0.2)), false, glm::vec2(0.0, -0.1)));
		}*/
	}

	if (_shooting) {
		std::chrono::system_clock::time_point t2= std::chrono::system_clock::now();
		auto d= std::chrono::duration_cast<std::chrono::milliseconds>(t2- _t_last_shooting).count();
		if (d> DELTA_BULLET) {
			_t_last_shooting= t2;
			_bullets.push_back(new Bullet(AABB_2D(_ships[0]->_aabb.center(), glm::vec2(0.1, 0.2)), true, glm::vec2(0.0, 0.2)));
		}
	}

	for (auto bullet : _bullets) {
		bullet->anim();

		if (bullet->_aabb._pos.x> _pt_max.x) {
			bullet->_dead= true;
		}
		else if (bullet->_aabb._pos.x< _pt_min.x- bullet->_aabb._size.x) {
			bullet->_dead= true;
		}
		if (bullet->_aabb._pos.y> _pt_max.y) {
			bullet->_dead= true;
		}
		else if (bullet->_aabb._pos.y< _pt_min.y- bullet->_aabb._size.y) {
			bullet->_dead= true;
		}
	}

	for (auto ship : _ships) {
		if (ship->_dead) {
			continue;
		}
		for (auto bullet : _bullets) {
			if (bullet->_dead) {
				continue;
			}
			if (ship->_friendly && bullet->_friendly) {
				continue;
			}
			if (!ship->_friendly && !bullet->_friendly) {
				continue;
			}
			if (aabb_intersects_aabb(&ship->_aabb, &bullet->_aabb)) {
				ship->_dead= true;
				bullet->_dead= true;
				if (bullet->_friendly) {
					_score++;
				}
				break;
			}
		}
	}

	/*for (unsigned int idx_ship=1; idx_ship<_ships.size(); ++idx_ship) {
		if (aabb_intersects_aabb(&_ships[0]->_aabb, &_ships[idx_ship]->_aabb)) {
			_gameover= true;
			break;
		}
	}*/

	for (unsigned int idx_ship=0; idx_ship<_ships.size()- 1; ++idx_ship) {
		if (_ships[idx_ship]->_dead) {
			continue;
		}
		for (unsigned int idx_ship2=idx_ship+ 1; idx_ship2<_ships.size(); ++idx_ship2) {
			if (_ships[idx_ship2]->_dead) {
				continue;
			}
			if (aabb_intersects_aabb(&_ships[idx_ship]->_aabb, &_ships[idx_ship2]->_aabb)) {
				_ships[idx_ship]->_dead= true;
				_ships[idx_ship2]->_dead= true;
			}
		}
	}

	if (_ships[0]->_dead) {
		_gameover= true;
	}

	_ships.erase(std::remove_if(_ships.begin(), _ships.end(), [](Ship * s){
		return s->_dead;
	}), _ships.end());
	_bullets.erase(std::remove_if(_bullets.begin(), _bullets.end(), [](Bullet * b){
		return b->_dead;
	}), _bullets.end());

	update_ship_aabb();
	update_bullet_aabb();

	//std::cout << "_ships.size = " << _ships.size() << " ; _bullets.size = " << _bullets.size() << "\n";
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
		_shooting= true;
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
		_shooting= false;
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
		_shooting= true;
		return true;
	}
	return false;
}


bool Level::joystick_up(unsigned int button_idx) {
	//std::cout << button_idx << "\n";
	if (button_idx== 0) {
		_shooting= false;
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

	Ship * ennemy= new Ship(_models[model_name], pos, false, glm::vec2(0.0, -1.0* rand_float(0.02, 0.07)));
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

	for (auto bullet : _bullets) {
		delete bullet;
	}
	_bullets.clear();

	_ships.push_back(new Ship(_models["hero"], glm::vec2(0.0, _pt_min.y+ 2.0), true, glm::vec2(0.0)));
	//unsigned int n_ennemies= rand_int(10, 20);
	/*unsigned int n_ennemies= 5;
	for (unsigned int i=0; i<n_ennemies; ++i) {
		add_rand_ennemy();
	}*/
}
