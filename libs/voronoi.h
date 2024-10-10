#ifndef VORONOI_H
#define VORONOI_H

#include <string>
#include <vector>
#include <queue>
#include <utility>
#include <functional>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "typedefs.h"
#include "dcel.h"
#include "bst.h"


const number BBOX_MARGIN_PERCENT= 0.4;


typedef enum {CircleEvent, SiteEvent} EventType;
typedef enum {Arc, BreakPoint} BeachLineNodeType;

class Event;


inline bool number_equals_strict(number x, number y);
inline bool number_equals_epsilon(number x, number y);
number y_parabola(const pt_type & site, number yline, number x);
number y_derivative_parabola(const pt_type & site, number yline, number x);
std::string parabola_equation(const pt_type & site, number yline);
pt_type parabolas_intersection(const pt_type & site1, const pt_type & site2, number yline);
bool breakpoints_converge(DCEL_HalfEdge * he1, DCEL_HalfEdge * he2);
bool events_are_equal(Event * lhs, Event * rhs);


class DCEL_HalfEdgeData {
public:
	DCEL_HalfEdgeData();
	DCEL_HalfEdgeData(bool is_full_line);
	DCEL_HalfEdgeData(bool is_full_line, pt_type center);
	~DCEL_HalfEdgeData();
	friend std::ostream & operator << (std::ostream & os, const DCEL_HalfEdgeData & d);


	bool _is_full_line;
	pt_type _center;
};


class BeachLineNode {
public:
	BeachLineNode();
	BeachLineNode(BeachLineNodeType type);
	~BeachLineNode();
	friend std::ostream & operator << (std::ostream & os, const BeachLineNode & b);


	BeachLineNodeType _type;

	// node feuille = arc, lié à un site; _circle_event est l'event qui fera disparaitre cet arc
	pt_type _site;
	Event * _circle_event;
	
	// node interne = breakpoint, lié à 2 sites; _half_edge est l'un des 2 half-edges en train d'etre dessiné
	std::pair<pt_type, pt_type> _sites;
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
	pt_type _site;

	// CircleEvent : on stocke le centre du cercle, son rayon et l'arc qui sera supprimé par cet event
	pt_type _circle_center;
	number _circle_radius;
	Node<BeachLineNode> * _leaf;
};


struct EventCmp {
	bool operator()(const Event * lhs, const Event * rhs) const {
		number lx= 0.0f;
		number rx= 0.0f;
		number ly= 0.0f;
		number ry= 0.0f;

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
		}
		if ((lhs->_type== EventType::SiteEvent) && (rhs->_type== EventType::SiteEvent)) {
			return true;
		}
		if ((lhs->_type== EventType::CircleEvent) && (rhs->_type== EventType::CircleEvent)) {
			return true;
		}*/

		return false;
	}
};


class Voronoi {
public:
	Voronoi();
	Voronoi(const std::vector<pt_type> & sites, bool verbose=false, std::string debug_path="", number bbox_expand=2.0f);
	~Voronoi();
	DCEL_HalfEdge * add_full_segment(pt_type position, pt_type direction);
	DCEL_HalfEdge * add_half_segment(pt_type position, pt_type direction);
	void set_halfedge_origin(DCEL_HalfEdge * he, DCEL_Vertex * v);
	void handle_first_sites_event(Event * e);
	void handle_site_event(Event * e);
	void handle_circle_event(Event * e);
	void export_debug_html(std::string html_path);


	std::vector<pt_type> _sites;
	DCEL * _diagram;
	BST<BeachLineNode> * _beachline;
	std::priority_queue<Event *, std::vector<Event *>, EventCmp> _queue;
	number _current_y;
	number _first_y;
	unsigned int _debug_count= 0;
	std::string _debug_path;
	bool _verbose;
	pt_type _bbox_min;
	pt_type _bbox_max;
};

#endif
