#ifndef VORONOI_H
#define VORONOI_H

#include <string>
#include <vector>
#include <set>
#include <utility>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "dcel.h"
#include "bst.h"


typedef enum {CircleEvent, SiteEvent} EventType;
typedef enum {Arc, BreakPoint} BeachLineNodeType;

std::pair<glm::vec2, glm::vec2> parabols_intersections(glm::vec2 & site1, glm::vec2 & site2, float yline);
//glm::vec2 bisector_intersection(glm::vec2 & a, glm::vec2 & b, glm::vec2 & c);

struct Event;

struct BeachLineNode {
	BeachLineNodeType _type;

	// node feuille = arc, lié à un site; _circle_event est l'event qui fera disparaitre cet arc
	glm::vec2 _site = glm::vec2(0.0f, 0.0f);
	Event * _circle_event = NULL;
	
	// node interne = breakpoint, lié à 2 sites; _half_edge est l'un des 2 half-edges en train d'etre dessiné
	std::pair<glm::vec2, glm::vec2> _sites = std::make_pair(glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
	DCEL_HalfEdge * _half_edge = NULL;
};


struct Event {
	EventType _type;

	// SiteEvent : on ne stocke que le site lié à l'event
	glm::vec2 _site = glm::vec2(0.0f, 0.0f);

	// CircleEvent : on stocke le point le plus bas du cercle et l'arc qui sera supprimé par cet event
	glm::vec2 _circle_lowest_point = glm::vec2(0.0f, 0.0f);
	BeachLineNode * _leaf = NULL;
};


struct EventCmp {
	bool operator()(const Event& lhs, const Event& rhs) const {
		if (lhs._type== EventType::SiteEvent) {
			return lhs._site.y< rhs._site.y;
		}
		else if (lhs._type== EventType::CircleEvent) {
			return lhs._circle_lowest_point.y< rhs._circle_lowest_point.y;
		}
		return false; // n'arrive jamais
	}
};


/*class EventQueue {
public:
	EventQueue();
	~EventQueue();
	void insert_event(Event & e);
	bool empty();


	std::set<Event, EventCmp> _events;
};*/


class Voronoi {
public:
	Voronoi();
	Voronoi(std::vector<glm::vec2> sites);
	~Voronoi();
	void handle_site_event(Event e);
	void handle_circle_event(Event e);


	DCEL _diagram;
	BST<BeachLineNode> _beachline;
	//EventQueue _queue;
	std::set<Event, EventCmp> _queue;
	float _current_y;
};

#endif
