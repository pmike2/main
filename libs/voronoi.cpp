#include <iostream>
#include <sstream>
#include "math.h"

#include <glm/gtx/string_cast.hpp>

#include "voronoi.h"
#include "geom_2d.h"


// -------------------------------
float y_parabola(glm::vec2 & site, float yline, float x) {
	return (x- site.x)* (x- site.x)* 0.5f/ (site.y- yline)+ 0.5f* (site.y+ yline);
}


std::pair<glm::vec2, glm::vec2> parabolas_intersection(glm::vec2 & site1, glm::vec2 & site2, float yline) {
	auto y= [& site1, & yline](float x) {
		return 0.5f* (x* x- 2.0f* site1.x* x+ site1.x* site1.x+ site1.y* site1.y- yline* yline)/ (site1.y- yline);
	};
	float a= 2.0f* (site2.y- site1.y);
	float b= 4.0f* ((site1.y- yline)* site2.x- (site2.y- yline)* site1.x);
	float c= 2.0f* (site2.y- yline)* (site1.x* site1.x+ site1.y* site1.y- yline* yline)
			-2.0f* (site1.y- yline)* (site2.x* site2.x+ site2.y* site2.y- yline* yline);
	if (a== 0.0f) {
		//std::cout << "parabolas_intersection 1 seul pt\n";
		float x= -1.0f* c/ b;
		glm::vec2 inter(x, y(x));
		return std::make_pair(inter, inter);
	}
	float delta= b* b- 4.0f* a* c;
	if (delta< 0.0f) {
		std::cout << "parabolas_intersection 0 pt\n";
		return std::make_pair(glm::vec2(0.0f), glm::vec2(0.0f));
	}
	std::cout << "parabolas_intersection 2 pts\n";
	float x1= 0.5f* (-1.0f* b- sqrt(delta))/ a;
	float x2= 0.5f* (-1.0f* b+ sqrt(delta))/ a;
	return std::make_pair(glm::vec2(x1, y(x1)), glm::vec2(x2, y(x2)));
}

/*glm::vec2 bisector_intersection(glm::vec2 & a, glm::vec2 & b, glm::vec2 & c) {
	float mu= 0.5f* ((b.y- a.y)* (c.y- a.y)+ (a.x- b.x)* (c.x- a.x))/ ((b.y- c.y)* (b.x- a.x)+ (b.y- a.y)* (c.x- b.x));
	float x= 0.5f* (b.x+ c.x)+ mu* (b.y- c.y);
	float y= 0.5f* (b.y+ c.y)+ mu* (c.x- b.x);
	return glm::vec2(x, y);
}*/


// -------------------------------
std::ostream & operator << (std::ostream & os, const Event & event) {
	if (event._type== CircleEvent) {
		os << "CircleEvent : circle_lowest_point = " << glm::to_string(event._circle_lowest_point);
	}
	else if (event._type== SiteEvent) {
		os << "SiteEvent : site = " << glm::to_string(event._site);
	}
	
	return os;
}


// -------------------------------
Voronoi::Voronoi() {

}


Voronoi::Voronoi(std::vector<glm::vec2> sites) :
	_beachline(
		[this](BeachLineNode lhs, BeachLineNode rhs) {
			float lhx= 0.0f;
			float rhx= 0.0f;
			
			// dans le cas d'un Arc (feuille du BST) on considère le x du site associé
			// sinon on calcule le x du breakpoint = intersection des 2 arcs définissant le breakpoint
			if (lhs._type== Arc) {
				lhx= lhs._site.x;
			}
			else if (lhs._type== BreakPoint) {
				std::pair<glm::vec2, glm::vec2> inter_pair= parabolas_intersection(lhs._sites.first, lhs._sites.second, _current_y);
				// a priori une seule intersection, on prend la 1ere
				lhx= inter_pair.first.x;
			}

			if (rhs._type== Arc) {
				rhx= rhs._site.x;
			}
			else if (rhs._type== BreakPoint) {
				std::pair<glm::vec2, glm::vec2> inter_pair= parabolas_intersection(rhs._sites.first, rhs._sites.second, _current_y);
				// a priori une seule intersection, on prend la 1ere
				rhx= inter_pair.first.x;
			}

			if (lhx< rhx) {
				return -1;
			}
			else if (lhx> rhx) {
				return 1;
			}
			return 0;
		},
		[](BeachLineNode node) {
			if (node._type== Arc) {
				//std::ostream * os;
				//*os << node._circle_event;
				std::stringstream ss;
				//ss << os->rdbuf();
				ss << node._circle_event;
				return "Arc : site = "+ glm::to_string(node._site)+ " circle_event = "+ ss.str();
			}
			else {
				std::stringstream ss;
				ss << node._half_edge;
				return "BreakPoint : sites = ["+ glm::to_string(node._sites.first)+ " ; "+ glm::to_string(node._sites.second)+ " ] ; half_edge = "+ ss.str();
			}
		}
	), _current_y(0.0f)
{
	for (unsigned int i=0; i<sites.size(); ++i) {
		Event e{EventType::SiteEvent, sites[i]};
		_queue.insert(e);
	}
	while (!_queue.empty()) {
		std::set<Event>::iterator it= _queue.begin();
		Event e= *it;
		_queue.erase(it);

		if (e._type== EventType::SiteEvent) {
			handle_site_event(e);
		}
		else if (e._type== EventType::CircleEvent) {
			handle_circle_event(e);
		}
	}
}


Voronoi::~Voronoi() {

}


Node<BeachLineNode> * Voronoi::prev_arc(Node<BeachLineNode> * node) {
	if (node->is_left()) {
		return node->_parent->_parent->_left;
	}
	return node->_parent->_left;
}


Node<BeachLineNode> * Voronoi::next_arc(Node<BeachLineNode> * node) {
	if (node->is_left()) {
		return node->_parent->_right;
	}
	return node->_parent->_parent->_right;
}


void Voronoi::handle_site_event(Event e) {
	// si la beachline est vide on insère un nouvel arc et on sort
	if (_beachline.empty()) {
		_beachline.insert({Arc, e._site});
		return;
	}

	// recherche de l'arc au dessus du site
	BeachLineNode tmp{Arc};
	tmp._site= e._site;
	Node<BeachLineNode> * arc_above_site= _beachline.search(tmp, false);

	// si cet arc a un circle event associé, on supprime cet event, les 2 edges ne se rencontront jamais
	if (arc_above_site->_data._circle_event!= NULL) {
		_queue.erase(*arc_above_site->_data._circle_event);
	}

	// remplacer arc_above_site par 3 arcs dont les sites sont (arc_above_site->_site, e._site, arc_above_site->_site)
	// avec les breakpoints qui vont bien
	Node<BeachLineNode> * prev_site= prev_arc(arc_above_site);
	Node<BeachLineNode> * next_site= next_arc(arc_above_site);

	DCEL_Vertex * v= _diagram.add_vertex(e._site.x, y_parabola(arc_above_site->_data._site, _current_y, e._site.x));
	DCEL_HalfEdge * he= _diagram.add_edge(v, NULL);

	BeachLineNode breakpoint1{BreakPoint};
	breakpoint1._sites= std::make_pair(prev_site->_data._site, arc_above_site->_data._site);
	breakpoint1._half_edge= he;
	BeachLineNode breakpoint2{BreakPoint};
	breakpoint2._sites= std::make_pair(arc_above_site->_data._site, next_site->_data._site);
	breakpoint2._half_edge= he;

	BST<BeachLineNode> subtree(_beachline._cmp, _beachline._node_print);
	subtree.insert(breakpoint1);
	subtree.insert(breakpoint2);
	subtree.insert(prev_site->_data);
	subtree.insert(arc_above_site->_data);
	subtree.insert(next_site->_data);
	_beachline.transplant(arc_above_site, subtree._root);
	_beachline.balance();

	if (prev_site!= NULL) {
		std::pair<glm::vec2, float> circle= circumcircle(prev_site->_data._site, b->_data._site, e._site);
		//glm::vec2 v= bisector_intersection(prev_site->_data._site, b->_data._site, e._site);
		glm::vec2 center= circle.first;
		float radius= circle.second;
		if (center.y< breakpoints.first.y) {
			_queue.insert({EventType::CircleEvent, glm::vec2(0.0f), center- glm::vec2(0.0f, radius), NULL});
		}
	}

	if (next_site!= NULL) {
		std::pair<glm::vec2, float> circle= circumcircle(next_site->_data._site, b->_data._site, e._site);
		glm::vec2 center= circle.first;
		float radius= circle.second;
		if (center.y< breakpoints.first.y) {
			_queue.insert({EventType::CircleEvent, glm::vec2(0.0f), center- glm::vec2(0.0f, radius), NULL});
		}
	}
}


void Voronoi::handle_circle_event(Event e) {
	Node<BeachLineNode> * node= _beachline.search(*e._leaf);
	std::pair<Node<BeachLineNode> *, Node<BeachLineNode> *> neighbours= _beachline.neighbours_leaf(node);
	_beachline.remove(*e._leaf);
	if (neighbours.first!= NULL) {
		_beachline.remove(neighbours.first->_data);
	}
	if (neighbours.second!= NULL) {
		_beachline.remove(neighbours.second->_data);
	}
}

