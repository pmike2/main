/*

Implémentation diagramme Voronoi

TODO : faire un BST plus malin car pour l'instant je ne peux pas faire de balance
cf https://pvigier.github.io/2018/11/18/fortune-algorithm-details.html
cf https://stackoverflow.com/questions/8688251/fortunes-algorithm-beach-line-data-structure

*/


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


// marge ajoutée pour faire une bbox englobante
const number BBOX_MARGIN_PERCENT= 0.1;

// type d'événement
typedef enum {CircleEvent, SiteEvent} EventType;

// type de noeud dans la beachline
typedef enum {Arc, BreakPoint} BeachLineNodeType;


class Event;


number y_parabola(const pt_type & site, number yline, number x);
number y_derivative_parabola(const pt_type & site, number yline, number x);
std::string parabola_equation(const pt_type & site, number yline);
number parabolas_intersection(const pt_type & site1, const pt_type & site2, number yline);
bool breakpoints_converge(DCEL_HalfEdge * he1, DCEL_HalfEdge * he2);
bool events_are_equal(Event * lhs, Event * rhs);


// données ajoutées à chaque half-edge
class DCEL_HalfEdgeData {
public:
	DCEL_HalfEdgeData();
	DCEL_HalfEdgeData(bool is_full_line, pt_type direction);
	DCEL_HalfEdgeData(bool is_full_line, pt_type center, pt_type direction);
	~DCEL_HalfEdgeData();
	friend std::ostream & operator << (std::ostream & os, const DCEL_HalfEdgeData & d);


	bool _is_full_line;
	pt_type _center;
	pt_type _direction;
};


// noeud de la beachline
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


// Evénement
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


// utilisée pour la comparaison d'événements lors de leur insertion dans la queue
struct EventCmp {
	bool operator()(const Event * lhs, const Event * rhs) const;
};


// classe principale
class Voronoi {
public:
	Voronoi();
	Voronoi(const std::vector<pt_type> & sites, bool verbose=false, std::string debug_path="", number bbox_expand=2.0f);
	~Voronoi();
	DCEL_HalfEdge * add_full_line(pt_type position, pt_type direction);
	DCEL_HalfEdge * add_half_line(pt_type position, pt_type direction);
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
	bool _verbose;
	pt_type _bbox_min;
	pt_type _bbox_max;

	// pour debug, optimisation
	unsigned int _debug_count= 0;
	std::string _debug_path;
	std::vector<number> _site_times;
	std::vector<number> _circle_times;
	std::vector<std::pair<unsigned int, unsigned int> > _tree_stats;
};

#endif
