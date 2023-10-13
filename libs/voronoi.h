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

std::pair<glm::vec2, glm::vec2> parabols_intersections(glm::vec2 & site1, glm::vec2 & site2, float yline);
//glm::vec2 bisector_intersection(glm::vec2 & a, glm::vec2 & b, glm::vec2 & c);

struct Event;

struct BeachLineNode {
	glm::vec2 _site;
	Event * _circle_event; // pour nodes feuilles
	DCEL_HalfEdge * _half_edge; // pour nodes internes
};


struct Event {
	EventType _type;
	glm::vec2 _site;
	glm::vec2 _circle_lowest_point;
	BeachLineNode * _leaf;
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
};

#endif
