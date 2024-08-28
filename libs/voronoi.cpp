#include <iostream>
#include "math.h"

//#include <glm/gtx/string_cast.hpp>

#include "voronoi.h"
#include "geom_2d.h"


// -------------------------------
std::pair<glm::vec2, glm::vec2> parabols_intersections(glm::vec2 & site1, glm::vec2 & site2, float yline) {
	auto y= [& site1, & yline](float x) {
		return 0.5f* (x* x- 2.0f* site1.x* x+ site1.x* site1.x+ site1.y* site1.y- yline* yline)/ (site1.y- yline);
	};
	float a= 2.0f* (site2.y- site1.y);
	float b= 4.0f* ((site1.y- yline)* site2.x- (site2.y- yline)* site1.x);
	float c= 2.0f* (site2.y- yline)* (site1.x* site1.x+ site1.y* site1.y- yline* yline)
			-2.0f* (site1.y- yline)* (site2.x* site2.x+ site2.y* site2.y- yline* yline);
	if (a== 0.0f) {
		//std::cout << "parabols_intersections 1 seul pt\n";
		float x= -1.0f* c/ b;
		glm::vec2 inter(x, y(x));
		return std::make_pair(inter, inter);
	}
	float delta= b* b- 4.0f* a* c;
	if (delta< 0.0f) {
		std::cout << "parabols_intersections 0 pt\n";
		return std::make_pair(glm::vec2(0.0f), glm::vec2(0.0f));
	}
	std::cout << "parabols_intersections 2 pts\n";
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
Voronoi::Voronoi() {

}


Voronoi::Voronoi(std::vector<glm::vec2> sites) :
	_beachline([this](BeachLineNode lhs, BeachLineNode rhs) {
		float lhx= 0.0f;
		float rhx= 0.0f;
		
		// dans le cas d'un Arc (feuille du BST) on considère le x du site associé
		// sinon on calcule le x du breakpoint = intersection des 2 arcs définissant le breakpoint
		if (lhs._type== Arc) {
			lhx= lhs._site.x;
		}
		else if (lhs._type== BreakPoint) {
			std::pair<glm::vec2, glm::vec2> inter_pair= parabols_intersections(lhs._sites.first, lhs._sites.second, _current_y);
			// a priori une seule intersection, on prend la 1ere
			lhx= inter_pair.first.x;
		}

		if (rhs._type== Arc) {
			rhx= rhs._site.x;
		}
		else if (lhs._type== BreakPoint) {
			std::pair<glm::vec2, glm::vec2> inter_pair= parabols_intersections(rhs._sites.first, rhs._sites.second, _current_y);
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
	}), _current_y(0.0f)
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
		_beachline.insert({Arc, e._site});
		return;
	}

	// recherche de l'arc au dessus du site
	BeachLineNode tmp{Arc, e._site};
	Node<BeachLineNode> * arc_above_site= _beachline.search(tmp, false);

	// si cet arc a un circle event associé, on supprime cet event, les 2 edges ne se rencontront jamais
	if (arc_above_site->_data._circle_event!= NULL) {
		_queue.erase(*arc_above_site->_data._circle_event);
	}

	// remplacer arc_above_site par 3 arcs dont les sites sont (arc_above_site->_site, e._site, arc_above_site->_site)
	// avec les breakpoints qui vont bien
	std::pair<Node<BeachLineNode> *, Node<BeachLineNode> *> nl= _beachline.neighbours_leaf(b);
	Node<BeachLineNode> * prev_site= nl.first;
	Node<BeachLineNode> * next_site= nl.second;

	std::pair<glm::vec2, glm::vec2> breakpoints= parabols_intersections(b->_data._site, e._site, e._site.y);
	Node<BeachLineNode> * left_site= new Node<BeachLineNode>(b->_data);
	Node<BeachLineNode> * middle_site= new Node<BeachLineNode>({e._site, NULL, NULL});
	Node<BeachLineNode> * right_site= new Node<BeachLineNode>(b->_data);
	DCEL_HalfEdge * he_left= new DCEL_HalfEdge();
	DCEL_HalfEdge * he_right= new DCEL_HalfEdge();
	Node<BeachLineNode> * left_middle_parent= new Node<BeachLineNode>({breakpoints.first, NULL, he_left}, left_site, middle_site);
	b->_data= {breakpoints.second, NULL, he_right};
	b->_left= left_middle_parent;
	b->_right= right_site;
	// _beachline.balance();

	//_diagram._half_edges.push_back(*he_left);
	//_diagram._half_edges.push_back(*he_right);

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

