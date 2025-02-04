#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/gtx/string_cast.hpp>
#include "json.hpp"

#include "utile.h"

#include "level.h"

using json = nlohmann::json;


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
