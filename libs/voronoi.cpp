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


std::string parabola_equation(glm::vec2 & site, float yline) {
	float a= 0.5f/ (site.y- yline);
	float b= site.x/ (yline- site.y);
	float c= 0.5f* site.x* site.x/ (site.y- yline)+ 0.5f* (site.y+ yline);
	return std::to_string(a)+ "x2 + "+ std::to_string(b)+ "x + "+ std::to_string(c);
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


bool breakpoints_converge(BeachLineNode * bkpt1, BeachLineNode * bkpt2) {
	std::cout << "DEBUG --- " <<  *bkpt1 << " ; " << *bkpt2 << "\n";
	std::cout << "DEBUG2 --- " <<  *bkpt1->_half_edge << " ; " << *bkpt2->_half_edge << "\n";
	glm::vec2 origin1= glm::vec2(bkpt1->_half_edge->_tmp_x, bkpt1->_half_edge->_tmp_y);
	glm::vec2 direction1= glm::vec2(bkpt1->_half_edge->_dx, bkpt1->_half_edge->_dy);
	glm::vec2 origin2= glm::vec2(bkpt2->_half_edge->_tmp_x, bkpt2->_half_edge->_tmp_y);
	glm::vec2 direction2= glm::vec2(bkpt2->_half_edge->_dx, bkpt2->_half_edge->_dy);
	glm::vec2 result;
	return ray_intersects_ray(origin1, direction1, origin2, direction2, &result);
}


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
		//os << " ; addr = " << &b;
	}
	else if (b._type== BreakPoint) {
		//os << "BreakPoint : sites = [ " << glm_to_string(b._sites.first) << " ; " << glm_to_string(b._sites.second) << " ] ; half_edge = " << b._half_edge;
		os << "BrkPt[" << glm_to_string(b._sites.first) << " , " << glm_to_string(b._sites.second) << "]";
		//os <<  " ; half_edge = " << b._half_edge;
		//os << " ; addr = " << &b;
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
		os << " ; leaf = " << *event._leaf;
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
		//ss << "aaa";
		return ss.str();
	};

	_beachline= new BST<BeachLineNode>(cmp, node_print);
	_diagram= new DCEL();
}


Voronoi::Voronoi(std::vector<glm::vec2> & sites) : Voronoi()
{
	_sites= sites;

	for (unsigned int i=0; i<sites.size(); ++i) {
		Event * e= new Event(SiteEvent);
		e->_site= sites[i];
		_queue.insert(e);
	}

	_debug_count= 0;
	while (!_queue.empty()) {
		std::set<Event *>::iterator it= _queue.begin();
		Event * e= *it;

		if (VERBOSE) {
			std::cout << "------------------------------------\n";
			std::cout << "count= " << _debug_count << "\n";
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
			_beachline->export_html("../data/beachline"+ std::to_string(_debug_count)+ ".html");

			export_html("../data/debug"+ std::to_string(_debug_count)+ ".html");

			/*for (auto s : sites) {
				if (s.y<= _current_y) {
					continue;
				}
				std::cout << glm_to_string(s) << " : " << parabola_equation(s, _current_y) << " ; ";
			}
			std::cout << "\n";*/

			_debug_count++;
		}

		_queue.erase(it);
	}

	std::cout << "------------------------------------\n";
	std::cout << *_diagram;

	if (VERBOSE) {
		std::cout << "------------------------------------\n";
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
	new_arc->_site= glm::vec2(e->_site);
	Node<BeachLineNode> * node_above_site= _beachline->search(*new_arc, false);

	Node<BeachLineNode> * prev_site= _beachline->predecessor_leaf(node_above_site);
	Node<BeachLineNode> * next_site= _beachline->successor_leaf(node_above_site);
	Node<BeachLineNode> * prev_bkpt= _beachline->predecessor(node_above_site);
	Node<BeachLineNode> * next_bkpt= _beachline->successor(node_above_site);

	if (VERBOSE) {
		std::cout << "node_above_site = " << *node_above_site << "\n";
		if (prev_bkpt!= NULL) {
			std::cout << "prev_bkpt = " << *prev_bkpt << "\n";
		}
		else {
			std::cout << "prev_bkpt = NULL\n";
		}
		if (next_bkpt!= NULL) {
			std::cout << "next_bkpt = " << *next_bkpt << "\n";
		}
		else {
			std::cout << "next_bkpt = NULL\n";
		}
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
	he->set_tmp_data(
		glm::vec2(1.0f, y_derivative_parabola(node_above_site->_data._site, _current_y, e->_site.x)),
		glm::vec2(e->_site.x, y_parabola(node_above_site->_data._site, _current_y, e->_site.x))
	);
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

	BeachLineNode * above_site_copy1= new BeachLineNode(Arc);
	above_site_copy1->_site= glm::vec2(node_above_site->_data._site);
	above_site_copy1->_circle_event= node_above_site->_data._circle_event;
	BeachLineNode * above_site_copy2= new BeachLineNode(Arc);
	above_site_copy2->_site= glm::vec2(node_above_site->_data._site);
	above_site_copy2->_circle_event= node_above_site->_data._circle_event;

	Node<BeachLineNode> * node_above_site_left_copy= _beachline->gen_node(*above_site_copy1);
	Node<BeachLineNode> * node_above_site_right_copy= _beachline->gen_node(*above_site_copy2);

	// on associe au bkpt gauche le half-edge qui va vers la gauche
	BeachLineNode * breakpoint_left= new BeachLineNode(BreakPoint);
	breakpoint_left->_sites= std::make_pair(node_above_site_left_copy->_data._site, new_arc->_site);
	breakpoint_left->_half_edge= he->_twin;

	// on associe au bkpt droit le half-edge qui va vers la droite
	BeachLineNode * breakpoint_right= new BeachLineNode(BreakPoint);
	breakpoint_right->_sites= std::make_pair(new_arc->_site, node_above_site_right_copy->_data._site);
	breakpoint_right->_half_edge= he;

	Node<BeachLineNode> * breakpoint_left_node= _beachline->gen_node(*breakpoint_left);
	Node<BeachLineNode> * breakpoint_right_node= _beachline->gen_node(*breakpoint_right);
	Node<BeachLineNode> * new_node= _beachline->gen_node(*new_arc);
	breakpoint_left_node->set_left(node_above_site_left_copy);
	breakpoint_left_node->set_right(breakpoint_right_node);
	//node_above_site_left_copy->_parent= breakpoint_left_node;
	//breakpoint_right_node->_parent= breakpoint_left_node;
	breakpoint_right_node->set_left(new_node);
	breakpoint_right_node->set_right(node_above_site_right_copy);
	//new_node->_parent= breakpoint_right_node;
	//node_above_site_right_copy->_parent= breakpoint_right_node;
	
	//_beachline->transplant(node_above_site, breakpoint_left_node);

	if (node_above_site->is_root()) {
		_beachline->_root= breakpoint_left_node;
	}
	else {
		if (node_above_site->is_left()) {
			node_above_site->_parent->set_left(breakpoint_left_node);
		}
		else {
			node_above_site->_parent->set_right(breakpoint_left_node);
		}
	}

	// rebalance ; pour l'instant non j'ai peur que cela mette en l'air l'arbre vu la fonction de comparaison basée sur _site.x
	//_beachline.balance();

	// voir si les breakpoints convergent
	if ((prev_site!= NULL) && (prev_bkpt!= NULL) && (breakpoints_converge(&prev_bkpt->_data, breakpoint_left))) {
		
		std::pair<glm::vec2, float> circle= circumcircle(prev_site->_data._site, node_above_site->_data._site, new_arc->_site);
		glm::vec2 center= circle.first;
		float radius= circle.second;

		if (VERBOSE) {
			std::cout << "circumcircle prev " << prev_site->_data << " ; " << node_above_site->_data << " ; " << *new_arc;
			std::cout << " ; center = " << glm_to_string(center) << " ; radius = " << radius << "\n";
		}

		if (center.y- radius< _current_y) {
			Event * new_circle_event= new Event(CircleEvent);
			new_circle_event->_circle_center= center;
			new_circle_event->_circle_radius= radius;
			new_circle_event->_leaf= node_above_site_left_copy;
			_queue.insert(new_circle_event);
			if (VERBOSE) {
				std::cout << "New CircleEvent prev : " << *new_circle_event << "\n";
			}
		}
	}

	if ((next_site!= NULL) && (next_bkpt!= NULL) && (breakpoints_converge(&next_bkpt->_data, breakpoint_right))) {
		std::pair<glm::vec2, float> circle= circumcircle(new_arc->_site, node_above_site->_data._site, next_site->_data._site);
		glm::vec2 center= circle.first;
		float radius= circle.second;

		if (VERBOSE) {
			std::cout << "circumcircle next " << *new_arc << " ; " << node_above_site->_data << " ; " << next_site->_data;
			std::cout << " ; center = " << glm_to_string(center) << " ; radius = " << radius << "\n";
		}

		if (center.y- radius< _current_y) {
			Event * new_circle_event= new Event(CircleEvent);
			new_circle_event->_circle_center= center;
			new_circle_event->_circle_radius= radius;
			new_circle_event->_leaf= node_above_site_right_copy;
			_queue.insert(new_circle_event);
			if (VERBOSE) {
				std::cout << "New CircleEvent next : " << *new_circle_event << "\n";
			}
		}
	}
}


void Voronoi::handle_circle_event(Event * e) {
	// ici e->_leaf est l'arc qui disparait de la beachline
	
	//Node<BeachLineNode> * event_node= _beachline->search(e->_leaf->_data);

	Node<BeachLineNode> * predecessor_leaf= _beachline->predecessor_leaf(e->_leaf); // arc à gauche
	Node<BeachLineNode> * successor_leaf= _beachline->successor_leaf(e->_leaf); // arc à droite
	Node<BeachLineNode> * predecessor= _beachline->predecessor(e->_leaf); // breakpoint à gauche
	Node<BeachLineNode> * successor= _beachline->successor(e->_leaf); // breakpoint à droite
	Node<BeachLineNode> * parent= e->_leaf->_parent;
	Node<BeachLineNode> * grand_parent= parent->_parent;
	Node<BeachLineNode> * sibling= e->_leaf->sibling();
	Node<BeachLineNode> * predecessor_predecessor_leaf= _beachline->predecessor_leaf(predecessor_leaf);
	Node<BeachLineNode> * successor_successor_leaf= _beachline->successor_leaf(successor_leaf);

	// suppression de l'arc dans le BST
	if ( (e->_leaf->is_left()) && (parent->_right!= NULL) && (grand_parent!= NULL)) {
		if (parent->is_left()) {
			grand_parent->set_left(parent->_right);
		}
		else {
			grand_parent->set_right(parent->_right);
		}
		predecessor->_data._sites.second= glm::vec2(successor_leaf->_data._site);
	}
	else if ( (e->_leaf->is_right()) && (parent->_left!= NULL) && (grand_parent!= NULL)) {
		if (parent->is_left()) {
			grand_parent->set_left(parent->_left);
		}
		else {
			grand_parent->set_right(parent->_left);
		}
		successor->_data._sites.first= glm::vec2(predecessor_leaf->_data._site);
	}

	/*if (predecessor_leaf== sibling) {
		successor_leaf->_parent->_data._sites.first= predecessor_leaf->_data._site;
		successor_leaf->_parent->_data._sites.second= successor_leaf->_data._site;
	}
	else if (successor_leaf== sibling) {
		predecessor_leaf->_parent->_data._sites.first= predecessor_leaf->_data._site;
		predecessor_leaf->_parent->_data._sites.second= successor_leaf->_data._site;
	}*/

	// ici il manque du nettoyage...
	//_beachline.transplant(event_node, NULL);
	//_beachline.remove(event_node);

	// a voir après
	//_beachline.balance();

	// suppression des CircleEvent des arcs voisins
	if (predecessor_leaf->_data._circle_event!= NULL) {
		_queue.erase(predecessor_leaf->_data._circle_event);
	}
	if (successor_leaf->_data._circle_event!= NULL) {
		_queue.erase(successor_leaf->_data._circle_event);
	}

	// ajout du centre du cercle comme sommet du diagram et d'un nouveau he délimitant les breakpoints voisins
	DCEL_Vertex * vertex= _diagram->add_vertex(e->_circle_center.x, e->_circle_center.y);
	DCEL_HalfEdge * he= _diagram->add_edge(vertex, NULL);
	// direction du nouveau he perpendiculaire à site_droit - site_gauche et pointant vers les y< 0
	glm::vec2 v= successor_leaf->_data._site- predecessor_leaf->_data._site;
	he->set_tmp_data(glm::vec2(v.y, -v.x), e->_circle_center);
	if (VERBOSE) {
		std::cout << "New Vertex = " << *vertex << "\n";
		std::cout << "New Edge = " << *he << "\n";
	}

	// rattacher le nouveau vertex et les 2 nouveaux he aux he existants
	predecessor->_data._half_edge->_twin->_origin= vertex;
	successor->_data._half_edge->_twin->_origin= vertex;
	he->_twin->set_next(predecessor->_data._half_edge->_twin);
	successor->_data._half_edge->set_next(he);
	predecessor->_data._half_edge->set_next(successor->_data._half_edge->_twin);


	if (predecessor_predecessor_leaf!= NULL) {
		std::pair<glm::vec2, float> circle= circumcircle(predecessor_predecessor_leaf->_data._site, predecessor_leaf->_data._site, successor_leaf->_data._site);
		glm::vec2 center= circle.first;
		float radius= circle.second;

		if (VERBOSE) {
			std::cout << "circumcircle pp " << predecessor_predecessor_leaf->_data << " ; " << predecessor_leaf->_data << " ; " << successor_leaf->_data;
			std::cout << " ; center = " << glm_to_string(center) << " ; radius = " << radius << "\n";
		}

		if (center.y< _current_y) {
			Event * new_circle_event= new Event(CircleEvent);
			new_circle_event->_circle_center= center;
			new_circle_event->_circle_radius= radius;
			new_circle_event->_leaf= predecessor_leaf;
			_queue.insert(new_circle_event);
			if (VERBOSE) {
				std::cout << "New CircleEvent pp : " << *new_circle_event << "\n";
			}
		}
	}

	if (successor_successor_leaf!= NULL) {
		std::pair<glm::vec2, float> circle= circumcircle(predecessor_leaf->_data._site, successor_leaf->_data._site, successor_successor_leaf->_data._site);
		glm::vec2 center= circle.first;
		float radius= circle.second;

		if (VERBOSE) {
			std::cout << "circumcircle ss " << predecessor_leaf->_data << " ; " << successor_leaf->_data << " ; " << successor_successor_leaf->_data;
			std::cout << " ; center = " << glm_to_string(center) << " ; radius = " << radius << "\n";
		}

		if (center.y< _current_y) {
			Event * new_circle_event= new Event(CircleEvent);
			new_circle_event->_circle_center= center;
			new_circle_event->_circle_radius= radius;
			new_circle_event->_leaf= successor_leaf;
			_queue.insert(new_circle_event);
			if (VERBOSE) {
				std::cout << "New CircleEvent ss : " << *new_circle_event << "\n";
			}
		}
	}
}


void Voronoi::export_html(std::string html_path) {
	float xmin= 1e8;
	float ymin= 1e8;
	float xmax= -1e8;
	float ymax= -1e8;
	for (auto s : _sites) {
		if (s.x< xmin) {
			xmin= s.x;
		}
		if (s.x> xmax) {
			xmax= s.x;
		}
		if (s.y< ymin) {
			ymin= s.y;
		}
		if (s.y> ymax) {
			ymax= s.y;
		}
	}

	const unsigned int SVG_WIDTH= 1000;
	const unsigned int SVG_HEIGHT= 800;
	
	/*const float MARGIN_FACTOR= 1.5f;
	const float VIEW_XMIN= (xmin- 0.5f* (xmin+ xmax))* MARGIN_FACTOR+ 0.5f* (xmin+ xmax);
	const float VIEW_YMIN= (ymin- 0.5f* (ymin+ ymax))* MARGIN_FACTOR+ 0.5f* (ymin+ ymax);
	const float VIEW_XMAX= (xmax- 0.5f* (xmin+ xmax))* MARGIN_FACTOR+ 0.5f* (xmin+ xmax);
	const float VIEW_YMAX= (ymax- 0.5f* (ymin+ ymax))* MARGIN_FACTOR+ 0.5f* (ymin+ ymax);*/

	const float VIEW_XMIN= -1.0f;
	const float VIEW_XMAX= 2.0f;
	const float VIEW_YMIN= -1.0f;
	const float VIEW_YMAX= 2.0f;
	
	const float SIZE= std::max(xmax- xmin, ymax- ymin);
	const float POINT_RADIUS= 0.02f* SIZE;
	const float STROKE_WIDTH= 0.01f* SIZE;
	const float STEP= 0.01f;

	const float OPACITY_MIN= 0.2f;
	const float OPACITY_MAX= 0.6f;

	auto y_html= [VIEW_YMIN, VIEW_YMAX](float y) -> float {return VIEW_YMIN+ VIEW_YMAX- y;};

	std::ofstream f;
	f.open(html_path);
	f << "<!DOCTYPE html>\n<html>\n<head>\n";
	f << "<style>\n";
	
	f << ".site_point_class {fill: black; fill-opacity:" << OPACITY_MAX << "}\n";
	//f << ".repere_point_class {fill: red;}\n";
	f << ".half_edge_origin_point_class {fill: green; fill-opacity:" << OPACITY_MAX << "}\n";
	f << ".half_edge_destination_point_class {fill: royalblue; fill-opacity:" << OPACITY_MAX << "}\n";
	f << ".half_edge_tmp_point_class {fill: grey; fill-opacity:" << OPACITY_MAX << "}\n";

	f << ".parabola_class {fill: transparent; stroke: black; stroke-width: " << STROKE_WIDTH << "; stroke-opacity:" << OPACITY_MIN << "}\n";
	f << ".repere_line_class {fill: transparent; stroke: red; stroke-width: " << STROKE_WIDTH << "; stroke-opacity:" << OPACITY_MIN << "}\n";
	f << ".current_line_class {fill: transparent; stroke: purple; stroke-width: " << STROKE_WIDTH << "; stroke-opacity:" << OPACITY_MIN << "}\n";
	f << ".complete_half_edge_class {fill: transparent; stroke: green; stroke-width: " << STROKE_WIDTH << "; stroke-opacity:" << OPACITY_MAX << "}\n";
	f << ".origincomplete_half_edge_class {fill: transparent; stroke: olive; stroke-width: " << STROKE_WIDTH << "; stroke-opacity:" << OPACITY_MAX << "}\n";
	f << ".destinationcomplete_half_edge_class {fill: transparent; stroke: royalblue; stroke-width: " << STROKE_WIDTH << "; stroke-opacity:" << OPACITY_MAX << "}\n";
	f << ".incomplete_half_edge_class {fill: transparent; stroke: grey; stroke-width: " << STROKE_WIDTH << "; stroke-opacity:" << OPACITY_MAX << "}\n";
	
	f << "</style>\n</head>\n<body>\n";
	f << "<svg width=\"" << SVG_WIDTH << "\" height=\"" << SVG_HEIGHT << "\" ";
	// viewbox = xmin, ymin, width, height
	f << "viewbox=\"" << VIEW_XMIN << " " << VIEW_YMIN << " " << VIEW_XMAX- VIEW_XMIN << " " << VIEW_YMAX- VIEW_YMIN << "\" ";
	f << "style=\"background-color:rgb(220,240,230)\"";
	f << "\">\n";

	// repère
	//f << "<circle class=\"repere_point_class\" cx=\"" << 0 << "\" cy=\"" << y_html(0) << "\" r=\"" << POINT_RADIUS << "\" />\n";
	//f << "<circle class=\"repere_point_class\" cx=\"" << 1 << "\" cy=\"" << y_html(0) << "\" r=\"" << POINT_RADIUS << "\" />\n";
	//f << "<circle class=\"repere_point_class\" cx=\"" << 0 << "\" cy=\"" << y_html(1) << "\" r=\"" << POINT_RADIUS << "\" />\n";
	f << "<line class=\"repere_line_class\" x1=\"" << 0 << "\" y1=\"" << y_html(0) << "\" x2=\"" << 1 << "\" y2=\"" << y_html(0) << "\" />\n";
	f << "<line class=\"repere_line_class\" x1=\"" << 0 << "\" y1=\"" << y_html(0) << "\" x2=\"" << 0 << "\" y2=\"" << y_html(1) << "\" />\n";

	// current line
	f << "<line class=\"current_line_class\" x1=\"" << VIEW_XMIN << "\" y1=\"" << y_html(_current_y) << "\" x2=\"" << VIEW_XMAX << "\" y2=\"" << y_html(_current_y) << "\" />\n";

	for (auto s : _sites) {
		f << "<circle class=\"site_point_class\" cx=\"" << s.x << "\" cy=\"" << y_html(s.y) << "\" r=\"" << POINT_RADIUS << "\" />\n";

		if (s.y<= _current_y) {
			continue;
		}

		float x= VIEW_XMIN;
		std::string poly= "";
		while (true) {
			float y= y_parabola(s, _current_y, x);
			poly+= std::to_string(x)+ ","+ std::to_string(y_html(y))+ " ";
			x+= STEP;
			if (x> VIEW_XMAX) {
				break;
			}
		}
		f << "<polyline class=\"parabola_class\" points=\""+ poly+ "\" />\n";
	}

	for (auto he : _diagram->_half_edges) {
		if (he->_twin== NULL) {
			std::cout << "twin == NULL !\n";
			continue;
		}

		if (he->_origin!= NULL) {
			f << "<circle class=\"half_edge_origin_point_class\" cx=\"" << he->_origin->_x << "\" cy=\"" << y_html(he->_origin->_y) << "\" r=\"" << POINT_RADIUS << "\" />\n";
			if (he->destination()!= NULL) {
				f << "<line class=\"complete_half_edge_class\" x1=\"" << he->_origin->_x << "\" y1=\"" << y_html(he->_origin->_y) << "\" x2=\"" << he->_twin->_origin->_x << "\" y2=\"" << y_html(he->_twin->_origin->_y) << "\" />\n";
			}
			else {
				f << "<line class=\"origincomplete_half_edge_class\" x1=\"" << he->_origin->_x << "\" y1=\"" << y_html(he->_origin->_y) << "\" x2=\"" << he->_origin->_x+ he->_dx << "\" y2=\"" << y_html(he->_origin->_y+ he->_dy) << "\" />\n";
			}
		}
		else {
			if (he->destination()!= NULL) {
				//f << "<circle class=\"half_edge_destination_point_class\" cx=\"" << he->_twin->_origin->_x << "\" cy=\"" << y_html(he->_twin->_origin->_y) << "\" r=\"" << POINT_RADIUS << "\" />\n";
				//f << "<line class=\"destinationcomplete_half_edge_class\" x1=\"" << he->_twin->_origin->_x << "\" y1=\"" << y_html(he->_twin->_origin->_y) << "\" x2=\"" << he->_twin->_origin->_x- he->_dx << "\" y2=\"" << y_html(he->_twin->_origin->_y- he->_dy) << "\" />\n";
			}
			else {
				f << "<circle class=\"half_edge_tmp_point_class\" cx=\"" << he->_tmp_x << "\" cy=\"" << y_html(he->_tmp_y) << "\" r=\"" << POINT_RADIUS << "\" />\n";
				f << "<line class=\"incomplete_half_edge_class\" x1=\"" << he->_tmp_x- he->_dx << "\" y1=\"" << y_html(he->_tmp_y- he->_dy) << "\" x2=\"" << he->_tmp_x+ he->_dx << "\" y2=\"" << y_html(he->_tmp_y+ he->_dy) << "\" />\n";
			}
		}
	}

	f << "</svg>\n";
	f << "</body>\n</html>\n";
	f.close();
}
