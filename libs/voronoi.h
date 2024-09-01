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

float y_parabola(glm::vec2 & site, float yline, float x);
std::pair<glm::vec2, glm::vec2> parabolas_intersection(glm::vec2 & site1, glm::vec2 & site2, float yline);
//glm::vec2 bisector_intersection(glm::vec2 & a, glm::vec2 & b, glm::vec2 & c);

class Event;

class BeachLineNode {
public:
	BeachLineNode();
	BeachLineNode(BeachLineNodeType type);
	~BeachLineNode();

	BeachLineNodeType _type;

	// node feuille = arc, lié à un site; _circle_event est l'event qui fera disparaitre cet arc
	glm::vec2 _site;
	Event * _circle_event;
	
	// node interne = breakpoint, lié à 2 sites; _half_edge est l'un des 2 half-edges en train d'etre dessiné
	std::pair<glm::vec2, glm::vec2> _sites;
	DCEL_HalfEdge * _half_edge;
};


class Event {
public:
	Event();
	Event(EventType type);
	~Event();
	friend std::ostream & operator << (std::ostream & os, const Event & event);


	EventType _type;

	// SiteEvent : on ne stocke que le site lié à l'event
	glm::vec2 _site;

	// CircleEvent : on stocke le centre du cercle, son rayon et l'arc qui sera supprimé par cet event
	glm::vec2 _circle_center;
	float _circle_radius;
	Node<BeachLineNode> * _leaf;
};


struct EventCmp {
	bool operator()(const Event * lhs, const Event * rhs) const {
		float ly= 0.0f;
		float ry= 0.0f;
		if (lhs->_type== EventType::SiteEvent) {
			ly= lhs->_site.y;
		}
		else if (lhs->_type== EventType::CircleEvent) {
			return ly= lhs->_circle_center.y+ lhs->_circle_radius;
		}
		if (rhs->_type== EventType::SiteEvent) {
			ry= rhs->_site.y;
		}
		else if (rhs->_type== EventType::CircleEvent) {
			return ry= rhs->_circle_center.y+ rhs->_circle_radius;
		}
		
		return ly< ry;
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
	void handle_site_event(Event * e);
	void handle_circle_event(Event * e);


	DCEL _diagram;
	BST<BeachLineNode *> _beachline;
	//EventQueue _queue;
	std::set<Event *, EventCmp> _queue;
	float _current_y;
};

#endif
