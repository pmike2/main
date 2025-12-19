#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/gtx/string_cast.hpp>
#include "json.hpp"

#include "utile.h"

#include "ship_model.h"

using json = nlohmann::json;


// action -----------------------------------------------------------
Action::Action() {

}


Action::Action(glm::vec2 direction, int t, std::string bullet_name, unsigned int t_shooting, std::string texture_name, Mix_Chunk * shoot_sound) :
	_direction(direction), _t(t), _bullet_name(bullet_name), _bullet_model(NULL), _t_shooting(t_shooting), _texture_name(texture_name),
	_shoot_sound(shoot_sound)
{

}


Action::~Action() {
	if (_shoot_sound!= NULL) {
		Mix_FreeChunk(_shoot_sound);
	}
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
	if (js["score_death"]!= nullptr) {
		_score_death= js["score_death"];
	}

	_hit_sound= NULL;
	if (js["hit_sound"]!= nullptr) {
		std::string hit_sound_path= js["hit_sound"];
		_hit_sound= Mix_LoadWAV(hit_sound_path.c_str());
	}

	_death_sound= NULL;
	if (js["death_sound"]!= nullptr) {
		std::string death_sound_path= js["death_sound"];
		_death_sound= Mix_LoadWAV(death_sound_path.c_str());
	}

	// parcours des textures
	for (json::iterator it = js["textures"].begin(); it != js["textures"].end(); ++it) {
		auto & texture_name= it.key();
		auto & l_textures= it.value();
		std::vector<std::string> pngs;
		std::vector<unsigned int> t_anims;
		AABB_2D footprint(pt_2d(0.0, 0.0), pt_2d(1.0, 1.0));
		for (auto texture : l_textures) {
			std::string png= texture["png"];
			unsigned int t= texture["t"];
			pngs.push_back(png);
			t_anims.push_back(t);
			pt_2d pos(texture["footprint"]["pos"][0], texture["footprint"]["pos"][1]);
			pt_2d size(texture["footprint"]["size"][0], texture["footprint"]["size"][1]);
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

			Mix_Chunk * shoot_sound= NULL;
			if (action["shoot_sound"]!= nullptr) {
				std::string shoot_sound_path= action["shoot_sound"];
				shoot_sound= Mix_LoadWAV(shoot_sound_path.c_str());
			}

			_actions[action_name].push_back(new Action(direction, t, bullet_name, t_shooting, texture_name, shoot_sound));
		}
	}
}


ShipModel::~ShipModel() {
	if (_hit_sound!= NULL) {
		Mix_FreeChunk(_hit_sound);
	}
	if (_death_sound!= NULL) {
		Mix_FreeChunk(_death_sound);
	}
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

