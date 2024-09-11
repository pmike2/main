#ifndef VORONOI_H
#define VORONOI_H

#include <string>
#include <vector>
//#include <set>
#include <queue>
#include <utility>
#include <functional>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "dcel.h"
#include "bst.h"


// pour comparaison à 0.0f
const float EPS= 1e-12;


typedef enum {CircleEvent, SiteEvent} EventType;
typedef enum {Arc, BreakPoint} BeachLineNodeType;


float y_parabola(const glm::vec2 & site, float yline, float x);
float y_derivative_parabola(const glm::vec2 & site, float yline, float x);
std::string parabola_equation(const glm::vec2 & site, float yline);
glm::vec2 parabolas_intersection(const glm::vec2 & site1, const glm::vec2 & site2, float yline);

class BeachLineNode;

bool breakpoints_converge(DCEL_HalfEdge * he1, DCEL_HalfEdge * he2);


class Event;

class BeachLineNode {
public:
	BeachLineNode();
	BeachLineNode(BeachLineNodeType type);
	~BeachLineNode();
	friend std::ostream & operator << (std::ostream & os, const BeachLineNode & b);


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


	// type d'événement
	EventType _type;

	// est-il encore valide
	bool _is_valid= true;

	// SiteEvent : on ne stocke que le site lié à l'event
	glm::vec2 _site;

	// CircleEvent : on stocke le centre du cercle, son rayon et l'arc qui sera supprimé par cet event
	glm::vec2 _circle_center;
	float _circle_radius;
	Node<BeachLineNode> * _leaf;
};

/*
struct EventCmp {
	bool operator()(const Event * lhs, const Event * rhs) const {
		float lx= 0.0f;
		float rx= 0.0f;
		float ly= 0.0f;
		float ry= 0.0f;

		if (lhs->_type== EventType::SiteEvent) {
			lx= lhs->_site.x;
			ly= lhs->_site.y;
		}
		else if (lhs->_type== EventType::CircleEvent) {
			lx= lhs->_circle_center.x;
			ly= lhs->_circle_center.y- lhs->_circle_radius;
		}
		
		if (rhs->_type== EventType::SiteEvent) {
			rx= rhs->_site.x;
			ry= rhs->_site.y;
		}
		else if (rhs->_type== EventType::CircleEvent) {
			rx= rhs->_circle_center.x;
			ry= rhs->_circle_center.y- rhs->_circle_radius;
		}
		
		// les y + grands doivent être traités en 1er
		if (ly> ry) {
			return true;
		}
		else if (ly< ry) {
			return false;
		}
		// en cas d'égalité des y on compare les x
		if (lx< rx) {
			return true;
		}
		else if (lx> rx) {
			return false;
		}

		std::cout << "equality : " << *lhs << " || " << *rhs << "\n";

		if ((lhs->_type== EventType::SiteEvent) && (rhs->_type== EventType::CircleEvent)) {
			return true;
		}
		if ((lhs->_type== EventType::CircleEvent) && (rhs->_type== EventType::SiteEvent)) {
			return false;
		}
		if ((lhs->_type== EventType::SiteEvent) && (rhs->_type== EventType::SiteEvent)) {
			// 2 sites identiques ; true fait planter je ne sais pquoi
			return true;
		}
		if ((lhs->_type== EventType::CircleEvent) && (rhs->_type== EventType::CircleEvent)) {
			return true;
		}

		return false;
	}
};
*/

struct EventCmp {
	bool operator()(const Event * lhs, const Event * rhs) const {
		float lx= 0.0f;
		float rx= 0.0f;
		float ly= 0.0f;
		float ry= 0.0f;

		if (lhs->_type== EventType::SiteEvent) {
			lx= lhs->_site.x;
			ly= lhs->_site.y;
		}
		else if (lhs->_type== EventType::CircleEvent) {
			lx= lhs->_circle_center.x;
			ly= lhs->_circle_center.y- lhs->_circle_radius;
		}
		
		if (rhs->_type== EventType::SiteEvent) {
			rx= rhs->_site.x;
			ry= rhs->_site.y;
		}
		else if (rhs->_type== EventType::CircleEvent) {
			rx= rhs->_circle_center.x;
			ry= rhs->_circle_center.y- rhs->_circle_radius;
		}
		
		// les y + grands doivent être traités en 1er
		if (ly< ry) {
			return true;
		}
		else if (ly> ry) {
			return false;
		}
		// en cas d'égalité des y on compare les x
		if (lx> rx) {
			return true;
		}
		else if (lx< rx) {
			return false;
		}

		std::cout << "equality : " << *lhs << " || " << *rhs << "\n";

		/*if ((lhs->_type== EventType::SiteEvent) && (rhs->_type== EventType::CircleEvent)) {
			return true;
		}
		if ((lhs->_type== EventType::CircleEvent) && (rhs->_type== EventType::SiteEvent)) {
			return false;
		}*/
		/*if ((lhs->_type== EventType::SiteEvent) && (rhs->_type== EventType::SiteEvent)) {
			return true;
		}*/
		/*if ((lhs->_type== EventType::CircleEvent) && (rhs->_type== EventType::CircleEvent)) {
			return true;
		}*/

		return false;
	}
};


class Voronoi {
public:
	Voronoi();
	Voronoi(const std::vector<glm::vec2> & sites, bool verbose=false, std::string debug_path="", float bbox_expand=2.0f);
	~Voronoi();
	void handle_first_sites_event(Event * e);
	void handle_site_event(Event * e);
	void handle_circle_event(Event * e);
	void export_debug_html(std::string html_path);


	std::vector<glm::vec2> _sites;
	DCEL * _diagram;
	BST<BeachLineNode> * _beachline;
	//std::multiset<Event *, EventCmp> _queue;
	std::priority_queue<Event *, std::vector<Event *>, EventCmp> _queue;
	float _current_y;
	float _first_y;
	unsigned int _debug_count= 0;
	std::string _debug_path;
	bool _verbose;
};

#endif
