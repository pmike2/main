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


void Voronoi::handle_site_event(Event e) {
	// si la beachline est vide on insère un nouvel arc et on sort
	if (_beachline.empty()) {
		BeachLineNode new_arc{Arc};
		new_arc._site= e._site;
		_beachline.insert(new_arc);
		return;
	}

	// recherche de l'arc au dessus du site
	BeachLineNode new_arc{Arc};
	new_arc._site= e._site;
	Node<BeachLineNode> * node_above_site= _beachline.search(new_arc, false);

	// si cet arc a un circle event associé, on supprime cet event, les 2 edges ne se rencontront jamais
	if (node_above_site->_data._circle_event!= NULL) {
		_queue.erase(*node_above_site->_data._circle_event);
	}

	// ajout du half-edge
	DCEL_Vertex * v= _diagram.add_vertex(e._site.x, y_parabola(node_above_site->_data._site, _current_y, e._site.x));
	DCEL_HalfEdge * he= _diagram.add_edge(v, NULL);

	// création des 2 nouveaux breakpoints
	Node<BeachLineNode> * prev_site= _beachline.predecessor_leaf(node_above_site);
	Node<BeachLineNode> * next_site= _beachline.successor_leaf(node_above_site);
	BeachLineNode breakpoint_left{BreakPoint};
	breakpoint_left._sites= std::make_pair(prev_site->_data._site, new_arc._site);
	breakpoint_left._half_edge= he;
	BeachLineNode breakpoint_right{BreakPoint};
	breakpoint_right._sites= std::make_pair(new_arc._site, next_site->_data._site);
	breakpoint_right._half_edge= he;

	// remplacer arc_above_site par 3 arcs dont les sites sont (arc_above_site->_site, e._site, arc_above_site->_site)
	// avec les breakpoints qui vont bien
	/*BST<BeachLineNode> subtree(_beachline._cmp, _beachline._node_print);
	subtree.insert(breakpoint1);
	subtree.insert(breakpoint2);
	subtree.insert(prev_site->_data);
	subtree.insert(arc_above_site->_data);
	subtree.insert(next_site->_data);
	_beachline.transplant(arc_above_site, subtree._root);*/

	Node<BeachLineNode> * breakpoint_left_node= new Node<BeachLineNode>(breakpoint_left, _beachline._node_print);
	Node<BeachLineNode> * breakpoint_right_node= new Node<BeachLineNode>(breakpoint_right, _beachline._node_print);
	Node<BeachLineNode> * new_node= new Node<BeachLineNode>(new_arc, _beachline._node_print);
	Node<BeachLineNode> * node_above_site_left_copy= new Node<BeachLineNode>(node_above_site->_data, _beachline._node_print);
	Node<BeachLineNode> * node_above_site_right_copy= new Node<BeachLineNode>(node_above_site->_data, _beachline._node_print);
	breakpoint_left_node->_left= node_above_site_left_copy;
	breakpoint_left_node->_right= breakpoint_right_node;
	node_above_site_left_copy->_parent= breakpoint_left_node;
	breakpoint_right_node->_parent= breakpoint_left_node;
	breakpoint_right_node->_left= new_node;
	breakpoint_right_node->_right= node_above_site_right_copy;
	new_node->_parent= breakpoint_right_node;
	node_above_site_right_copy->_parent= breakpoint_right_node;
	_beachline.transplant(node_above_site, breakpoint_left_node);

	// rebalance ; pour l'instant non j'ai peur que cela mette en l'air l'arbre vu la fonction de comparaison basée sur _site.x
	//_beachline.balance();

	// voir si les breakpoints convergent
	if (prev_site!= NULL) {
		std::pair<glm::vec2, float> circle= circumcircle(prev_site->_data._site, node_above_site->_data._site, new_arc._site);
		glm::vec2 center= circle.first;
		float radius= circle.second;
		if (center.y< _current_y) {
			Event new_circle_event{EventType::CircleEvent};
			new_circle_event._circle_lowest_point= center- glm::vec2(0.0f, radius);
			new_circle_event._leaf= node_above_site_left_copy;
			_queue.insert(new_circle_event);
		}
	}

	if (next_site!= NULL) {
		std::pair<glm::vec2, float> circle= circumcircle(new_arc._site, node_above_site->_data._site, next_site->_data._site);
		glm::vec2 center= circle.first;
		float radius= circle.second;
		if (center.y< _current_y) {
			Event new_circle_event{EventType::CircleEvent};
			new_circle_event._circle_lowest_point= center- glm::vec2(0.0f, radius);
			new_circle_event._leaf= node_above_site_right_copy;
			_queue.insert(new_circle_event);
		}
	}
}


void Voronoi::handle_circle_event(Event e) {
	Node<BeachLineNode> * node= _beachline.search(e._leaf->_data);
	_beachline.remove(node);

	std::pair<Node<BeachLineNode> *, Node<BeachLineNode> *> neighbours= _beachline.neighbours_leaf(node);
	if (neighbours.first!= NULL) {
		_beachline.remove(neighbours.first->_data);
	}
	if (neighbours.second!= NULL) {
		_beachline.remove(neighbours.second->_data);
	}
}

