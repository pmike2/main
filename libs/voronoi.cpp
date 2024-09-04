#include <iostream>
#include <sstream>
#include "math.h"

//#include <glm/gtx/string_cast.hpp>

#include "voronoi.h"
#include "geom_2d.h"
#include "utile.h"


// ---------------------------------------------------------------------------------------------
float y_parabola(glm::vec2 & site, float yline, float x) {
	if (site.y- yline< EPS) {
		std::cout << "y_parabola problème : site = " << glm_to_string(site) << " ; yline = " << yline << " ; x = " << x << "\n";
		return 0.0f;
	}
	return (x- site.x)* (x- site.x)* 0.5f/ (site.y- yline)+ 0.5f* (site.y+ yline);
}


float y_derivative_parabola(glm::vec2 & site, float yline, float x) {
	if (site.y- yline< EPS) {
		std::cout << "y_parabola problème : site = " << glm_to_string(site) << " ; yline = " << yline << " ; x = " << x << "\n";
		return 0.0f;
	}
	return (x- site.x)/ (site.y- yline);
}


glm::vec2 parabolas_intersection(glm::vec2 & site_left, glm::vec2 & site_right, float yline) {
	float a= 2.0f* (site_right.y- site_left.y);
	float b= 4.0f* ((site_left.y- yline)* site_right.x- (site_right.y- yline)* site_left.x);
	float c= 2.0f* (site_right.y- yline)* (site_left.x* site_left.x+ site_left.y* site_left.y- yline* yline)
			-2.0f* (site_left.y- yline)* (site_right.x* site_right.x+ site_right.y* site_right.y- yline* yline);

	if (a== 0.0f) {
		float x= -1.0f* c/ b;
		return glm::vec2(x, y_parabola(site_left, yline, x));
	}

	float delta= b* b- 4.0f* a* c;
	if (delta< 0.0f) {
		std::cout << "parabolas_intersection 0 pt : site_left=" << glm_to_string(site_left) << " ; site_right=" << glm_to_string(site_right) << " ; yline=" << yline << "\n";
		return glm::vec2(0.0f);
	}

	float x1= 0.5f* (-1.0f* b- sqrt(delta))/ a;
	float x2= 0.5f* (-1.0f* b+ sqrt(delta))/ a;
	float dleft_x1= y_derivative_parabola(site_left, yline, x1);
	float dright_x1= y_derivative_parabola(site_right, yline, x1);
	float dleft_x2= y_derivative_parabola(site_left, yline, x2);
	float dright_x2= y_derivative_parabola(site_right, yline, x2);

	if (dleft_x1>= dright_x1) {
		return glm::vec2(x1, y_parabola(site_left, yline, x1));
	}
	else if (dleft_x2>= dright_x2) {
		return glm::vec2(x2, y_parabola(site_left, yline, x2));
	}

	std::cout << "parabolas_intersection derivative problem : site_left=" << glm_to_string(site_left) << " ; site_right=" << glm_to_string(site_right) << " ; yline=" << yline << "\n";
	return glm::vec2(0.0f);
}


/*glm::vec2 bisector_intersection(glm::vec2 & a, glm::vec2 & b, glm::vec2 & c) {
	float mu= 0.5f* ((b.y- a.y)* (c.y- a.y)+ (a.x- b.x)* (c.x- a.x))/ ((b.y- c.y)* (b.x- a.x)+ (b.y- a.y)* (c.x- b.x));
	float x= 0.5f* (b.x+ c.x)+ mu* (b.y- c.y);
	float y= 0.5f* (b.y+ c.y)+ mu* (c.x- b.x);
	return glm::vec2(x, y);
}*/


// ---------------------------------------------------------------------------------------------
BeachLineNode::BeachLineNode() :
	_type(Arc), _site(glm::vec2(0.0f, 0.0f)), _circle_event(NULL), _sites(std::make_pair(glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f))), _half_edge(NULL) 
{

}


BeachLineNode::BeachLineNode(BeachLineNodeType type) :
	_type(type), _site(glm::vec2(0.0f, 0.0f)), _circle_event(NULL), _sites(std::make_pair(glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f))), _half_edge(NULL) 
{

}


BeachLineNode::~BeachLineNode() {

}


std::ostream & operator << (std::ostream & os, const BeachLineNode & b) {
	if (b._type== Arc) {
		//os << "Arc : event = " << b._circle_event << " ; site = " << glm_to_string(b._site);
		os << "Arc" << glm_to_string(b._site);
	}
	else if (b._type== BreakPoint) {
		//os << "BreakPoint : sites = [ " << glm_to_string(b._sites.first) << " ; " << glm_to_string(b._sites.second) << " ] ; half_edge = " << b._half_edge;
		os << "BrkPt[" << glm_to_string(b._sites.first) << " , " << glm_to_string(b._sites.second) << "]";
	}
	return os;
}


// ---------------------------------------------------------------------------------------------
Event::Event() :
	_type(CircleEvent), _site(glm::vec2(0.0f, 0.0f)), _circle_center(glm::vec2(0.0f, 0.0f)), _circle_radius(0.0f), _leaf(NULL)
{

}


Event::Event(EventType type) :
	_type(type), _site(glm::vec2(0.0f, 0.0f)), _circle_center(glm::vec2(0.0f, 0.0f)), _circle_radius(0.0f), _leaf(NULL)
{

}


Event::~Event() {

}


std::ostream & operator << (std::ostream & os, const Event & event) {
	if (event._type== CircleEvent) {
		os << "CircleEvent : circle = " << glm_to_string(event._circle_center) << " ; radius = " << event._circle_radius;
	}
	else if (event._type== SiteEvent) {
		os << "SiteEvent : site = " << glm_to_string(event._site);
	}
	
	return os;
}


// ---------------------------------------------------------------------------------------------
Voronoi::Voronoi() : _current_y(0.0f) {
	std::function<int(BeachLineNode, BeachLineNode)> cmp= [this](BeachLineNode lhs, BeachLineNode rhs) {
		float lhx= 0.0f;
		float rhx= 0.0f;
		
		// dans le cas d'un Arc (feuille du BST) on considère le x du site associé
		// sinon on calcule le x du breakpoint = intersection des 2 arcs définissant le breakpoint
		if (lhs._type== Arc) {
			lhx= lhs._site.x;
		}
		else if (lhs._type== BreakPoint) {
			glm::vec2 inter= parabolas_intersection(lhs._sites.first, lhs._sites.second, _current_y);
			lhx= inter.x;
		}

		if (rhs._type== Arc) {
			rhx= rhs._site.x;
		}
		else if (rhs._type== BreakPoint) {
			glm::vec2 inter= parabolas_intersection(rhs._sites.first, rhs._sites.second, _current_y);
			rhx= inter.x;
		}

		if (lhx< rhx) {
			return -1;
		}
		else if (lhx> rhx) {
			return 1;
		}
		return 0;
	};

	/*std::function<std::string(BeachLineNode)> node_print= [](BeachLineNode node) {
		if (node._type== Arc) {
			std::stringstream ss;
			ss << node._circle_event;
			return "Arc : site = "+ glm_to_string(node._site)+ " circle_event = "+ ss.str();
		}
		else {
			std::stringstream ss;
			ss << node._half_edge;
			return "BreakPoint : sites = ["+ glm_to_string(node._sites.first)+ " ; "+ glm_to_string(node._sites.second)+ " ] ; half_edge = "+ ss.str();
		}
	};*/
	std::function<std::string(BeachLineNode)> node_print= [](BeachLineNode node) {
		std::stringstream ss;
		ss << node;
		return ss.str();
	};

	_beachline= new BST<BeachLineNode>(cmp, node_print);
	_diagram= new DCEL();
}


Voronoi::Voronoi(std::vector<glm::vec2> sites) : Voronoi()
{
	for (unsigned int i=0; i<sites.size(); ++i) {
		Event * e= new Event(SiteEvent);
		e->_site= sites[i];
		_queue.insert(e);
	}

	while (!_queue.empty()) {
		std::set<Event *>::iterator it= _queue.begin();
		Event * e= *it;

		if (VERBOSE) {
			std::cout << "------------------------------------\n";
			std::cout << *e;
			std::cout << "\n";
		}

		if (e->_type== SiteEvent) {
			_current_y= e->_site.y;
			handle_site_event(e);
		}
		else if (e->_type== CircleEvent) {
			_current_y= e->_circle_center.y- e->_circle_radius;
			handle_circle_event(e);
		}

		if (VERBOSE) {
			std::cout << "beachline =\n" << *_beachline;
		}

		_queue.erase(it);
	}

	if (VERBOSE) {
		std::cout << "ajout BBOX\n";
	}
	_diagram->add_bbox(BBOX_EXPAND);

	
	if (VERBOSE) {
		std::cout << "calcul faces DCEL\n";
	}
	_diagram->create_faces_from_half_edges();
}


Voronoi::~Voronoi() {
	delete _diagram;
	delete _beachline;
}


void Voronoi::handle_site_event(Event * e) {
	// si la beachline est vide on insère un nouvel arc et on sort
	if (_beachline->empty()) {
		BeachLineNode * new_arc= new BeachLineNode(Arc);
		new_arc->_site= e->_site;
		_beachline->insert(*new_arc);
		return;
	}

	// recherche de l'arc au dessus du site
	BeachLineNode * new_arc= new BeachLineNode(Arc);
	new_arc->_site= e->_site;
	Node<BeachLineNode> * node_above_site= _beachline->search(*new_arc, false);

	if (VERBOSE) {
		std::cout << "node_above_site =\n" << *node_above_site << "\n";
	}

	// si cet arc a un circle event associé, on supprime cet event, les 2 edges ne se rencontront jamais
	if (node_above_site->_data._circle_event!= NULL) {
		if (VERBOSE) {
			std::cout << "suppression circle event\n";
		}
		_queue.erase(node_above_site->_data._circle_event);
	}

	// ajout des 2 half-edge
	/*DCEL_Vertex * v= _diagram->add_vertex(e->_site.x, y_parabola(node_above_site->_data._site, _current_y, e->_site.x));
	if (VERBOSE) {
		std::cout << "ajout vertex : " << *v << "\n";
	}*/
	DCEL_HalfEdge * he= _diagram->add_edge(NULL, NULL);
	// pente he = tangente à la parabole
	he->_dx= 1.0f;
	he->_dy= y_derivative_parabola(node_above_site->_data._site, _current_y, e->_site.x);
	he->_tmp_x= e->_site.x;
	he->_tmp_y= y_parabola(node_above_site->_data._site, _current_y, e->_site.x);
	/*if (VERBOSE) {
		std::cout << "ajout half edge : " << *he << " ; twin = " << *he->_twin << "\n";
	}*/

	// création des 2 nouveaux breakpoints
	/*Node<BeachLineNode> * prev_site= _beachline->predecessor_leaf(node_above_site);
	Node<BeachLineNode> * next_site= _beachline->successor_leaf(node_above_site);
	BeachLineNode * breakpoint_left= new BeachLineNode(BreakPoint);
	breakpoint_left->_sites= std::make_pair(prev_site->_data._site, new_arc->_site);
	breakpoint_left->_half_edge= he;
	BeachLineNode * breakpoint_right= new BeachLineNode(BreakPoint);
	breakpoint_right->_sites= std::make_pair(new_arc->_site, next_site->_data._site);
	breakpoint_right->_half_edge= he; // twin ?*/

	// remplacer arc_above_site par 3 arcs dont les sites sont (arc_above_site->_site, e._site, arc_above_site->_site)
	// avec les breakpoints qui vont bien
	/*BST<BeachLineNode> subtree(_beachline._cmp, _beachline._node_print);
	subtree.insert(breakpoint1);
	subtree.insert(breakpoint2);
	subtree.insert(prev_site->_data);
	subtree.insert(arc_above_site->_data);
	subtree.insert(next_site->_data);
	_beachline.transplant(arc_above_site, subtree._root);*/

	Node<BeachLineNode> * node_above_site_left_copy= _beachline->gen_node(node_above_site->_data);
	Node<BeachLineNode> * node_above_site_right_copy= _beachline->gen_node(node_above_site->_data);

	BeachLineNode * breakpoint_left= new BeachLineNode(BreakPoint);
	breakpoint_left->_sites= std::make_pair(node_above_site_left_copy->_data._site, new_arc->_site);
	breakpoint_left->_half_edge= he;
	if (VERBOSE) {
		std::cout << "breakpoint_left =  " << *breakpoint_left << "\n";
	}

	BeachLineNode * breakpoint_right= new BeachLineNode(BreakPoint);
	breakpoint_right->_sites= std::make_pair(new_arc->_site, node_above_site_right_copy->_data._site);
	breakpoint_right->_half_edge= he;
	if (VERBOSE) {
		std::cout << "breakpoint_right = " << *breakpoint_right << "\n";
	}

	Node<BeachLineNode> * breakpoint_left_node= _beachline->gen_node(*breakpoint_left);
	Node<BeachLineNode> * breakpoint_right_node= _beachline->gen_node(*breakpoint_right);
	Node<BeachLineNode> * new_node= _beachline->gen_node(*new_arc);
	breakpoint_left_node->_left= node_above_site_left_copy;
	breakpoint_left_node->_right= breakpoint_right_node;
	node_above_site_left_copy->_parent= breakpoint_left_node;
	breakpoint_right_node->_parent= breakpoint_left_node;
	breakpoint_right_node->_left= new_node;
	breakpoint_right_node->_right= node_above_site_right_copy;
	new_node->_parent= breakpoint_right_node;
	node_above_site_right_copy->_parent= breakpoint_right_node;
	_beachline->transplant(node_above_site, breakpoint_left_node);

	if (VERBOSE) {
		_debug_count++;
		_beachline->export_html("../data/beachline"+ std::to_string(_debug_count)+ ".html");
	}

	// rebalance ; pour l'instant non j'ai peur que cela mette en l'air l'arbre vu la fonction de comparaison basée sur _site.x
	//_beachline.balance();

	// voir si les breakpoints convergent
	Node<BeachLineNode> * prev_site= _beachline->predecessor_leaf(node_above_site);
	Node<BeachLineNode> * next_site= _beachline->successor_leaf(node_above_site);
	if (prev_site!= NULL) {
		std::pair<glm::vec2, float> circle= circumcircle(prev_site->_data._site, node_above_site->_data._site, new_arc->_site);
		glm::vec2 center= circle.first;
		float radius= circle.second;
		if (center.y< _current_y) {
			Event * new_circle_event= new Event(CircleEvent);
			new_circle_event->_circle_center= center;
			new_circle_event->_circle_radius= radius;
			new_circle_event->_leaf= node_above_site_left_copy;
			_queue.insert(new_circle_event);
			if (VERBOSE) {
				std::cout << "New CircleEvent : ";
				std::cout << "left = " << prev_site->_data;
				std::cout << " ; middle = " << node_above_site->_data;
				std::cout << " ; right = " << *new_arc;
				std::cout << " ; " << *new_circle_event << "\n";
			}
		}
	}

	if (next_site!= NULL) {
		std::pair<glm::vec2, float> circle= circumcircle(new_arc->_site, node_above_site->_data._site, next_site->_data._site);
		glm::vec2 center= circle.first;
		float radius= circle.second;
		if (center.y< _current_y) {
			Event * new_circle_event= new Event(CircleEvent);
			new_circle_event->_circle_center= center;
			new_circle_event->_circle_radius= radius;
			new_circle_event->_leaf= node_above_site_right_copy;
			_queue.insert(new_circle_event);
			if (VERBOSE) {
				std::cout << "New CircleEvent(2) : " << *new_circle_event << "\n";
			}
		}
	}
}


void Voronoi::handle_circle_event(Event * e) {
	Node<BeachLineNode> * event_node= _beachline->search(e->_leaf->_data);

	Node<BeachLineNode> * predecessor_leaf= _beachline->predecessor_leaf(event_node); // arc à gauche
	Node<BeachLineNode> * successor_leaf= _beachline->successor_leaf(event_node); // arc à droite
	Node<BeachLineNode> * predecessor= _beachline->predecessor(event_node); // breakpoint à gauche
	Node<BeachLineNode> * successor= _beachline->successor(event_node); // breakpoint à droite
	Node<BeachLineNode> * parent= event_node->_parent;
	Node<BeachLineNode> * grand_parent= parent->_parent;
	Node<BeachLineNode> * sibling= event_node->sibling();
	Node<BeachLineNode> * predecessor_predecessor_leaf= _beachline->predecessor_leaf(predecessor_leaf);
	Node<BeachLineNode> * successor_successor_leaf= _beachline->successor_leaf(successor_leaf);

	if ( (event_node->is_left()) && (parent->_right!= NULL) && (grand_parent!= NULL)) {
		if (parent->is_left()) {
			grand_parent->_left= parent->_right;
			parent->_right->_parent= grand_parent->_left;
		}
		else {
			grand_parent->_right= parent->_right;
			parent->_right->_parent= grand_parent->_right;
		}
	}
	else if ( (event_node->is_right()) && (parent->_left!= NULL) && (grand_parent!= NULL)) {
		if (parent->is_left()) {
			grand_parent->_left= parent->_left;
			parent->_left->_parent= grand_parent->_left;
		}
		else {
			grand_parent->_right= parent->_left;
			parent->_left->_parent= grand_parent->_right;
		}
	}

	if (predecessor_leaf== sibling) {
		successor_leaf->_parent->_data._sites.first= predecessor_leaf->_data._site;
		successor_leaf->_parent->_data._sites.second= successor_leaf->_data._site;
	}
	else if (successor_leaf== sibling) {
		predecessor_leaf->_parent->_data._sites.first= predecessor_leaf->_data._site;
		predecessor_leaf->_parent->_data._sites.second= successor_leaf->_data._site;
	}

	// ici il manque du nettoyage...
	//_beachline.transplant(event_node, NULL);
	//_beachline.remove(event_node);

	// a voir après
	//_beachline.balance();

	if (predecessor->_data._circle_event!= NULL) {
		_queue.erase(predecessor->_data._circle_event);
	}
	if (successor->_data._circle_event!= NULL) {
		_queue.erase(successor->_data._circle_event);
	}

	// ajout du centre du cercle comme sommet du diagram et d'un nouveau he délimitant les breakpoints voisins
	DCEL_Vertex * vertex= _diagram->add_vertex(e->_circle_center.x, e->_circle_center.y);
	DCEL_HalfEdge * he= _diagram->add_edge(vertex, NULL);
	// direction du nouveau he perpendiculaire à bkpt2-bkpt1 et pointant vers les y< 0
	glm::vec2 v= successor->_data._site- predecessor->_data._site;
	he->_dx= v.y;
	he->_dy= -v.x;
	//he->_tmp_x= e->_circle_center.x; // pas nécessaire car vertex référence déjà le centre du cercle
	//he->_tmp_y= e->_circle_center.y;

	// rattacher le nouveau vertex et les 2 nouveaux he aux he existants
	predecessor->_data._half_edge->_origin= vertex;
	successor->_data._half_edge->_twin->_origin= vertex;
	he->_twin->set_next(predecessor->_data._half_edge);
	successor->_data._half_edge->set_next(he);


	if (predecessor_predecessor_leaf!= NULL) {
		std::pair<glm::vec2, float> circle= circumcircle(predecessor_predecessor_leaf->_data._site, predecessor_leaf->_data._site, successor_leaf->_data._site);
		glm::vec2 center= circle.first;
		float radius= circle.second;
		if (center.y< _current_y) {
			Event * new_circle_event= new Event(CircleEvent);
			new_circle_event->_circle_center= center;
			new_circle_event->_circle_radius= radius;
			new_circle_event->_leaf= predecessor_leaf;
			_queue.insert(new_circle_event);
		}
	}

	if (successor_successor_leaf!= NULL) {
		std::pair<glm::vec2, float> circle= circumcircle(predecessor_leaf->_data._site, successor_leaf->_data._site, successor_successor_leaf->_data._site);
		glm::vec2 center= circle.first;
		float radius= circle.second;
		if (center.y< _current_y) {
			Event * new_circle_event= new Event(CircleEvent);
			new_circle_event->_circle_center= center;
			new_circle_event->_circle_radius= radius;
			new_circle_event->_leaf= successor_leaf;
			_queue.insert(new_circle_event);
		}
	}
}

