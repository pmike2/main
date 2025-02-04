#ifndef LEVEL_H
#define LEVEL_H

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "constantes.h"


// Evénement dans le niveau
class Event {
public:
	Event();
	Event(EventType type, unsigned int t, glm::vec2 position, std::string enemy);
	~Event();
	friend std::ostream & operator << (std::ostream & os, const Event & event);


	EventType _type; // type
	unsigned int _t; // temps en ms en absolu à partir du début du niveau auquel déclencher cet événement
	glm::vec2 _position; // position de l'ennemi (dans le cas _type == NEW_ENEMY)
	std::string _enemy; // nom de l'ennemi (dans le cas _type == NEW_ENEMY)
};


// Niveau
class Level {
public:
	Level();
	Level(std::string json_path, std::chrono::system_clock::time_point t);
	~Level();
	void reinit(std::chrono::system_clock::time_point t);
	friend std::ostream & operator << (std::ostream & os, const Level & level);


	std::string _json_path; // json du niveau
	std::chrono::system_clock::time_point _t_start; // temps de début du niveau
	std::vector<Event *> _events; // liste d'événements du niveau
	unsigned int _current_event_idx; // indice de l'événement courant
	std::string _music_path; // chemin musique niveau
};


#endif
