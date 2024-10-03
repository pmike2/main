#include <iostream>
#include <sstream>
#include <algorithm>
#include "math.h"

#include "voronoi.h"
#include "geom_2d.h"
#include "utile.h"


// ---------------------------------------------------------------------------------------------
bool float_equals_strict(float x, float y) {
	return x== y;
}


bool float_equals_epsilon(float x, float y) {
	return abs(x- y)< 1e-11;
}


float y_parabola(const glm::vec2 & site, float yline, float x) {
	if (float_equals_strict(site.y, yline)) {
		std::cout << "y_parabola problème : site = " << glm_to_string(site) << " ; yline = " << yline << " ; x = " << x << "\n";
		return 0.0f;
	}
	return (x- site.x)* (x- site.x)* 0.5f/ (site.y- yline)+ 0.5f* (site.y+ yline);
}


float y_derivative_parabola(const glm::vec2 & site, float yline, float x) {
	if (float_equals_strict(site.y, yline)) {
		std::cout << "y_derivative_parabola problème : site = " << glm_to_string(site) << " ; yline = " << yline << " ; x = " << x << "\n";
		return 0.0f;
	}
	return (x- site.x)/ (site.y- yline);
}


std::string parabola_equation(const glm::vec2 & site, float yline) {
	if (float_equals_strict(site.y, yline)) {
		std::cout << "parabola_equation problème : site = " << glm_to_string(site) << " ; yline = " << yline << "\n";
		return "";
	}
	float a= 0.5f/ (site.y- yline);
	float b= site.x/ (yline- site.y);
	float c= 0.5f* site.x* site.x/ (site.y- yline)+ 0.5f* (site.y+ yline);
	return std::to_string(a)+ "x2 + "+ std::to_string(b)+ "x + "+ std::to_string(c);
}


glm::vec2 parabolas_intersection(const glm::vec2 & site_left, const glm::vec2 & site_right, float yline) {
	// cas limite des sites situés sur yline
	if (float_equals_epsilon(site_left.y, yline)) {
		if (float_equals_epsilon(site_right.y,yline)) {
			std::cout << "parabolas_intersection 0 pt (2 sites sur yline) : site_left=" << glm_to_string(site_left) << " ; site_right=" << glm_to_string(site_right) << " ; yline=" << yline << "\n";
		}
		else {
			return glm::vec2(site_left.x, y_parabola(site_right, yline, site_left.x));
		}
	}
	else if (float_equals_epsilon(site_right.y, yline)) {
		return glm::vec2(site_right.x, y_parabola(site_left, yline, site_right.x));
	}

	float a= 2.0f* (site_right.y- site_left.y);
	float b= 4.0f* ((site_left.y- yline)* site_right.x- (site_right.y- yline)* site_left.x);
	float c= 2.0f* (site_right.y- yline)* (site_left.x* site_left.x+ site_left.y* site_left.y- yline* yline)
			-2.0f* (site_left.y- yline)* (site_right.x* site_right.x+ site_right.y* site_right.y- yline* yline);

	if (float_equals_epsilon(a, 0.0f)) {
		float x= -1.0f* c/ b;
		return glm::vec2(x, y_parabola(site_left, yline, x));
	}

	float delta= b* b- 4.0f* a* c;
	if (delta< 0.0f) {
		std::cout << "parabolas_intersection 0 pt : site_left=" << glm_to_string(site_left) << " ; site_right=" << glm_to_string(site_right) << " ; yline=" << yline << "\n";
		return glm::vec2(0.0f);
	}

	// 2 intersections dans la majorité des cas
	float x1= 0.5f* (-1.0f* b- sqrt(delta))/ a;
	float x2= 0.5f* (-1.0f* b+ sqrt(delta))/ a;

	// je compare les tangentes aux 2 pts pour savoir quel pt respecte left-right
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


bool breakpoints_converge(DCEL_HalfEdge * he1, DCEL_HalfEdge * he2) {
	/*glm::vec2 origin1= glm::vec2(he1->_tmp_x, he1->_tmp_y);
	glm::vec2 direction1= glm::vec2(he1->_dx, he1->_dy);
	glm::vec2 origin2= glm::vec2(he2->_tmp_x, he2->_tmp_y);
	glm::vec2 direction2= glm::vec2(he2->_dx, he2->_dy);
	glm::vec2 result;

	const float EPS= 1e-6;
	origin1-= EPS* direction1;
	origin2-= EPS* direction2;

	bool is_inter= ray_intersects_ray(origin1, direction1, origin2, direction2, &result);*/


	glm::vec2 result;
	glm::vec2 he1_origin, he2_origin;
	DCEL_HalfEdgeData * he1_data= (DCEL_HalfEdgeData *)(he1->_data);
	DCEL_HalfEdgeData * he2_data= (DCEL_HalfEdgeData *)(he2->_data);

	if (he1_data->_is_full_line) {
		he1_origin= he1_data->_center;
	}
	else {
		he1_origin= he1->_origin->_coords;
	}

	if (he2_data->_is_full_line) {
		he2_origin= he2_data->_center;
	}
	else {
		he2_origin= he2->_origin->_coords;
	}

	//std::cout << "DEBUG : " << *he1 << " | " << *he2 << "\n";
	//std::cout << "DEBUG2 : " << *he1_data << " | " << *he2_data << "\n";
	//std::cout << "DEBUG3 : " << glm_to_string(he1_origin) << " | " << glm_to_string(he2_origin) << "\n";

	bool is_inter= segment_intersects_segment(
		he1_origin, he1->destination()->_coords,
		he2_origin, he2->destination()->_coords, 
	&result);

	/*if ((float_equals_strict(he1_origin.x, he2_origin.x)) || (float_equals_strict(he1_origin.y, he2_origin.y))) {
		return false;
	}*/

	// faire un truc ici peut-être
	/*if ((he1->_origin!= NULL) && (he2->_origin!= NULL)) {
		if ((float_equals_strict(he1->_origin->_x, he2->_origin->_x)) || (float_equals_strict(he1->_origin->_y, he2->_origin->_y))) {
			return false;
		}
	}*/

	return is_inter;
}

// ---------------------------------------------------------------------------------------------
DCEL_HalfEdgeData::DCEL_HalfEdgeData() : _is_full_line(false), _center(glm::vec2(0.0f)) {

}


DCEL_HalfEdgeData::DCEL_HalfEdgeData(bool is_full_line) : _is_full_line(is_full_line), _center(glm::vec2(0.0f)) {

}


DCEL_HalfEdgeData::DCEL_HalfEdgeData(bool is_full_line, glm::vec2 center) : _is_full_line(is_full_line), _center(center) {
	
}


DCEL_HalfEdgeData::~DCEL_HalfEdgeData() {

}


std::ostream & operator << (std::ostream & os, const DCEL_HalfEdgeData & d) {
	os << "is_full_line = " << d._is_full_line << " ; center = " << glm_to_string(d._center);
	return os;
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
		os << "Arc" << glm_to_string(b._site);
	}
	else if (b._type== BreakPoint) {
		os << "BrkPt[" << glm_to_string(b._sites.first) << " , " << glm_to_string(b._sites.second) << "]";
		//os <<  " ; half_edge = " << *b._half_edge;
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

		//std::cout << "DEBUG : " << lhs << " || " << rhs << " || " << lhx << " ; " << rhx << "\n";
		
		if (lhx< rhx) {
			return -1;
		}
		else if (lhx> rhx) {
			return 1;
		}
		return 0;
	};

	_beachline= new BST<BeachLineNode>(cmp);
	_diagram= new DCEL();
}


Voronoi::Voronoi(const std::vector<glm::vec2> & sites, bool verbose, std::string debug_path, float bbox_expand) : Voronoi()
{
	_sites= sites;
	_debug_path= debug_path;
	_verbose= verbose;

	if (_debug_path!= "") {
		std::string cmd= "rm -rf "+ _debug_path+ " && mkdir -p "+ _debug_path;
		system(cmd.c_str());
	}

	_bbox_min.x= 1e8; _bbox_min.y= 1e8; _bbox_max.x= -1e8; _bbox_max.y= -1e8;
	for (unsigned int i=0; i<sites.size(); ++i) {
		Event * e= new Event(SiteEvent);
		e->_site= sites[i];
		_queue.push(e);
		if (sites[i].x< _bbox_min.x) { _bbox_min.x= sites[i].x; }
		if (sites[i].x> _bbox_max.x) { _bbox_max.x= sites[i].x; }
		if (sites[i].y< _bbox_min.y) { _bbox_min.y= sites[i].y; }
		if (sites[i].y> _bbox_max.y) { _bbox_max.y= sites[i].y; }
	}

	_debug_count= 0;
	float last_site_x= 1e8;
	float last_site_y= 1e8;
	while (!_queue.empty()) {
		Event * e= _queue.top();
		_queue.pop();
		if (!e->_is_valid) {
			continue;
		}
		if ((float_equals_strict(e->_site.x, last_site_x)) && (float_equals_strict(e->_site.y, last_site_y))) {
			continue;
		}

		//std::cout << *e << "\n";

		if (_verbose) {
			std::cout << "------------------------------------\n";
			std::cout << "count= " << _debug_count << "\n";
			std::cout << *e;
			std::cout << "\n";
		}

		if (e->_type== SiteEvent) {
			last_site_x= e->_site.x;
			last_site_y= e->_site.y;
			_current_y= e->_site.y;
			if (_debug_count== 0) {
				_first_y= e->_site.y;
			}
			if (float_equals_strict(_current_y, _first_y)) {
				handle_first_sites_event(e);
			}
			else {
				handle_site_event(e);
			}
		}
		else if (e->_type== CircleEvent) {
			_current_y= e->_circle_center.y- e->_circle_radius;
			handle_circle_event(e);
		}

		/*if (_verbose) {
			std::cout << "beachline =\n" << *_beachline;
			
			for (auto s : sites) {
				if (s.y<= _current_y) {
					continue;
				}
				std::cout << glm_to_string(s) << " : " << parabola_equation(s, _current_y) << " ; ";
			}
			std::cout << "\n";
		}*/

		if (_debug_path!= "") {
			_beachline->export_html(_debug_path+ "/beachline"+ std::to_string(_debug_count)+ ".html");
			export_debug_html(_debug_path+ "/debug"+ std::to_string(_debug_count)+ ".html");
		}

		_debug_count++;
	}

	if (_verbose) {
		std::cout << "------------------------------------\n";
	}
	
	for (auto v : _diagram->_vertices) {
		if (v->_incident_edge== NULL) {
			_diagram->add2queue({VERTEX, v});
		}
	}
	_diagram->delete_queue();

	std::cout << *_diagram << "\n";

	/*if (_verbose) {
		std::cout << "diagram_valid= " << _diagram->is_valid() << "\n";
	}*/

	if (_verbose) {
		std::cout << "ajout BBOX\n";
	}

	float max_size= std::max(_bbox_max.x- _bbox_min.x, _bbox_max.y- _bbox_min.y);
	glm::vec2 bbox_min= glm::vec2(_bbox_min.x- BBOX_MARGIN_PERCENT* max_size, _bbox_min.y- BBOX_MARGIN_PERCENT* max_size);
	glm::vec2 bbox_max= glm::vec2(_bbox_max.x+ BBOX_MARGIN_PERCENT* max_size, _bbox_max.y+ BBOX_MARGIN_PERCENT* max_size);
	_diagram->add_bbox(bbox_min, bbox_max);

	//std::cout << *_diagram << "\n";
}


Voronoi::~Voronoi() {
	delete _diagram;
	delete _beachline;
}


DCEL_HalfEdge * Voronoi::add_full_segment(glm::vec2 position, glm::vec2 direction) {
	float max_size= std::max(_bbox_max.x- _bbox_min.x, _bbox_max.y- _bbox_min.y);
	float size= (1.0f+ BBOX_MARGIN_PERCENT)* max_size* 2.0f;
	DCEL_HalfEdge * he= _diagram->add_edge(position- size* direction, position+ size* direction);
	he->_data= new DCEL_HalfEdgeData(true, position);
	he->_twin->_data= new DCEL_HalfEdgeData(true, position);
	return he;
}


DCEL_HalfEdge * Voronoi::add_half_segment(glm::vec2 position, glm::vec2 direction) {
	float max_size= std::max(_bbox_max.x- _bbox_min.x, _bbox_max.y- _bbox_min.y);
	float size= (1.0f+ BBOX_MARGIN_PERCENT)* max_size* 2.0f;
	DCEL_HalfEdge * he= _diagram->add_edge(position, position+ size* direction/ sqrt(direction.x* direction.x+ direction.y* direction.y));
	he->_data= new DCEL_HalfEdgeData(false);
	he->_twin->_data= new DCEL_HalfEdgeData(false);
	return he;
}


void Voronoi::set_halfedge_origin(DCEL_HalfEdge * he, DCEL_Vertex * v) {
	he->set_origin(v);
	DCEL_HalfEdgeData * he_data= (DCEL_HalfEdgeData *)(he->_data);
	he_data->_is_full_line= false;
}


void Voronoi::handle_first_sites_event(Event * e) {
	if (_beachline->empty()) {
		BeachLineNode * new_arc= new BeachLineNode(Arc);
		new_arc->_site= e->_site;
		_beachline->insert(*new_arc);
	}
	else {
		Node<BeachLineNode> * max_node= _beachline->maximum();

		BeachLineNode * new_arc= new BeachLineNode(Arc);
		new_arc->_site= e->_site;
		BeachLineNode * new_bkpt= new BeachLineNode(BreakPoint);
		new_bkpt->_sites= std::make_pair(max_node->_data._site, e->_site);

		/*DCEL_HalfEdge * he= _diagram->add_edge(NULL, NULL);
		float x= (max_node->_data._site.x+ e->_site.x)* 0.5f;
		float y= _current_y;
		he->set_tmp_data(glm::vec2(0.0f, -1.0f), glm::vec2(x, y));*/

		/*DCEL_HalfEdge * he= _diagram->add_edge(
			glm::vec2((max_node->_data._site.x+ e->_site.x)* 0.5f, _bbox_max.y+ 0.1),
			glm::vec2((max_node->_data._site.x+ e->_site.x)* 0.5f, _bbox_min.y- 0.1)
		);*/

		glm::vec2 position= glm::vec2((max_node->_data._site.x+ e->_site.x)* 0.5f, _current_y);
		glm::vec2 direction= glm::vec2(0.0f, -1.0f);
		DCEL_HalfEdge * he= add_full_segment(position, direction);

		new_bkpt->_half_edge= he;

		Node<BeachLineNode> * new_arc_node= _beachline->gen_node(*new_arc);
		Node<BeachLineNode> * new_bkpt_node= _beachline->gen_node(*new_bkpt);
		
		if (max_node->_parent!= NULL) {
			max_node->_parent->set_right(new_bkpt_node);
		}
		else {
			_beachline->_root= new_bkpt_node;
		}
		new_bkpt_node->set_left(max_node);
		new_bkpt_node->set_right(new_arc_node);
	}
}


void Voronoi::handle_site_event(Event * e) {
	// recherche de l'arc au dessus du site
	BeachLineNode * new_arc= new BeachLineNode(Arc);
	new_arc->_site= glm::vec2(e->_site);
	Node<BeachLineNode> * node_above_site= _beachline->search(*new_arc, false);

	Node<BeachLineNode> * prev_site= _beachline->predecessor_leaf(node_above_site);
	Node<BeachLineNode> * next_site= _beachline->successor_leaf(node_above_site);
	Node<BeachLineNode> * prev_bkpt= _beachline->predecessor(node_above_site);
	Node<BeachLineNode> * next_bkpt= _beachline->successor(node_above_site);

	if (_verbose) {
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
		if (_verbose) {
			std::cout << "suppression circle event : " << *node_above_site->_data._circle_event << "\n";
		}
		//_queue.erase(node_above_site->_data._circle_event);
		node_above_site->_data._circle_event->_is_valid= false;
	}

	// ajout des 2 half-edge
	// pente he = tangente à la parabole
	float y= y_parabola(node_above_site->_data._site, _current_y, e->_site.x);
	float dy= y_derivative_parabola(node_above_site->_data._site, _current_y, e->_site.x);

	/*DCEL_HalfEdge * he= _diagram->add_edge(NULL, NULL);
	he->set_tmp_data(glm::vec2(1.0f, dy), glm::vec2(e->_site.x, y));*/

	// doit être + grand que la diagonale
	/*float lambda= 2.0f* std::max(_bbox_max.x- _bbox_min.x, _bbox_max.y- _bbox_min.y);
	DCEL_HalfEdge * he= _diagram->add_edge(
		glm::vec2(e->_site.x, y)- lambda* glm::vec2(1.0f, dy),
		glm::vec2(e->_site.x, y)+ lambda* glm::vec2(1.0f, dy)
	);*/

	glm::vec2 position= glm::vec2(e->_site.x, y);
	glm::vec2 direction= glm::vec2(1.0f, dy);
	DCEL_HalfEdge * he= add_full_segment(position, direction);

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
	breakpoint_right_node->set_left(new_node);
	breakpoint_right_node->set_right(node_above_site_right_copy);
	
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
	if ((prev_site!= NULL) && (prev_bkpt!= NULL) && (breakpoints_converge(prev_bkpt->_data._half_edge, breakpoint_left->_half_edge))) {

		std::pair<glm::vec2, float> circle= circumcircle(prev_site->_data._site, node_above_site->_data._site, new_arc->_site);
		glm::vec2 center= circle.first;
		float radius= circle.second;

		Event * new_circle_event= new Event(CircleEvent);
		new_circle_event->_circle_center= center;
		new_circle_event->_circle_radius= radius;
		new_circle_event->_leaf= node_above_site_left_copy;
		node_above_site_left_copy->_data._circle_event= new_circle_event;
		/*if (auto search = _queue.find(new_circle_event); search != _queue.end()) {
			std::cout << "Event deja en queue 1 : " << *new_circle_event << " || " << *(*search) << "\n";
		}*/
		//_queue.insert(new_circle_event);
		_queue.push(new_circle_event);
		if (_verbose) {
			std::cout << "bkpt converge : " << *prev_bkpt->_data._half_edge << " ; " << *breakpoint_left->_half_edge << "\n";
			//std::cout << "circumcircle prev " << prev_site->_data << " ; " << node_above_site->_data << " ; " << *new_arc;
			//std::cout << " ; center = " << glm_to_string(center) << " ; radius = " << radius << "\n";
			std::cout << "New CircleEvent prev : " << *new_circle_event << "\n";
		}
	}

	if ((next_site!= NULL) && (next_bkpt!= NULL) && (breakpoints_converge(next_bkpt->_data._half_edge, breakpoint_right->_half_edge))) {
		std::pair<glm::vec2, float> circle= circumcircle(new_arc->_site, node_above_site->_data._site, next_site->_data._site);
		glm::vec2 center= circle.first;
		float radius= circle.second;

		Event * new_circle_event= new Event(CircleEvent);
		new_circle_event->_circle_center= center;
		new_circle_event->_circle_radius= radius;
		new_circle_event->_leaf= node_above_site_right_copy;
		node_above_site_right_copy->_data._circle_event= new_circle_event;
		/*if (auto search = _queue.find(new_circle_event); search != _queue.end()) {
			std::cout << "Event deja en queue 2 : " << *new_circle_event << " || " << *(*search) << "\n";
		}*/
		//_queue.insert(new_circle_event);
		_queue.push(new_circle_event);
		if (_verbose) {
			std::cout << "bkpt converge : " << *next_bkpt->_data._half_edge << " ; " << *breakpoint_right->_half_edge << "\n";
			//std::cout << "circumcircle next " << *new_arc << " ; " << node_above_site->_data << " ; " << next_site->_data;
			//std::cout << " ; center = " << glm_to_string(center) << " ; radius = " << radius << "\n";
			std::cout << "New CircleEvent next : " << *new_circle_event << "\n";
		}
	}
}


void Voronoi::handle_circle_event(Event * e) {
	// ici e->_leaf est l'arc qui disparait de la beachline
	Node<BeachLineNode> * predecessor_leaf= _beachline->predecessor_leaf(e->_leaf); // arc à gauche
	Node<BeachLineNode> * successor_leaf= _beachline->successor_leaf(e->_leaf); // arc à droite
	Node<BeachLineNode> * predecessor= _beachline->predecessor(e->_leaf); // breakpoint à gauche
	Node<BeachLineNode> * successor= _beachline->successor(e->_leaf); // breakpoint à droite
	Node<BeachLineNode> * parent= e->_leaf->_parent;
	Node<BeachLineNode> * grand_parent= parent->_parent;
	Node<BeachLineNode> * predecessor_predecessor_leaf= _beachline->predecessor_leaf(predecessor_leaf);
	Node<BeachLineNode> * successor_successor_leaf= _beachline->successor_leaf(successor_leaf);
	Node<BeachLineNode> * predecessor_bkpt= _beachline->predecessor(predecessor_leaf);
	Node<BeachLineNode> * successor_bkpt= _beachline->successor(successor_leaf);

	// ajout du centre du cercle comme sommet du diagram et d'un nouveau he délimitant les breakpoints voisins
	//DCEL_Vertex * vertex= _diagram->add_vertex(e->_circle_center);

	/*DCEL_HalfEdge * he= _diagram->add_edge(vertex, NULL);
	he->set_tmp_data(glm::vec2(v.y, -v.x), e->_circle_center);*/

	/*float lambda= std::max(_bbox_max.x- _bbox_min.x, _bbox_max.y- _bbox_min.y)/ sqrt(v.x* v.x+ v.y* v.y);
	DCEL_Vertex * vertex2= _diagram->add_vertex(e->_circle_center+ lambda* glm::vec2(v.y, -v.x));
	DCEL_HalfEdge * he= _diagram->add_edge(vertex, vertex2);*/

	// direction du nouveau he perpendiculaire à site_droit - site_gauche et pointant vers les y< 0
	glm::vec2 v= successor_leaf->_data._site- predecessor_leaf->_data._site;
	DCEL_HalfEdge * he= add_half_segment(e->_circle_center, glm::vec2(v.y, -v.x));
	DCEL_Vertex * circle_center_vertex= he->_origin;
	
	if (_verbose) {
		//std::cout << "New Vertex = " << *vertex << "\n";
		std::cout << "New Edge = " << *he << "\n";
	}

	// ici il manque du nettoyage...
	//_beachline.transplant(event_node, NULL);
	//_beachline.remove(event_node);

	// a voir après
	//_beachline.balance();

	// suppression des CircleEvent des arcs voisins
	if ((predecessor_leaf->_data._circle_event!= NULL)) {
		if (_verbose) {
			std::cout << "suppression CircleEvent predecessor : " << *predecessor_leaf->_data._circle_event << "\n";
		}
		//_queue.erase(predecessor_leaf->_data._circle_event);
		predecessor_leaf->_data._circle_event->_is_valid= false;
	}
	if ((successor_leaf->_data._circle_event!= NULL)) {
		if (_verbose) {
			std::cout << "suppression CircleEvent successor : " << *successor_leaf->_data._circle_event << "\n";
		}
		//_queue.erase(successor_leaf->_data._circle_event);
		successor_leaf->_data._circle_event->_is_valid= false;
	}

	// rattacher le nouveau vertex et les 2 nouveaux he aux he existants
	//std::cout << "DDDD : " << glm_to_string(circle_center_vertex->_coords) << " ; " << glm_to_string(predecessor->_data._half_edge->_twin->destination()->_coords) << "\n";
	//std::cout << "EEEE : " << glm_to_string(circle_center_vertex->_coords) << " ; " << glm_to_string(successor->_data._half_edge->_twin->destination()->_coords) << "\n";
	//predecessor->_data._half_edge->_twin->set_origin(circle_center_vertex);
	set_halfedge_origin(predecessor->_data._half_edge->_twin, circle_center_vertex);
	//successor->_data._half_edge->_twin->set_origin(circle_center_vertex);
	set_halfedge_origin(successor->_data._half_edge->_twin, circle_center_vertex);
	he->_twin->set_next(predecessor->_data._half_edge->_twin);
	successor->_data._half_edge->set_next(he);
	predecessor->_data._half_edge->set_next(successor->_data._half_edge->_twin);

	if ((predecessor_predecessor_leaf!= NULL) && (breakpoints_converge(predecessor_bkpt->_data._half_edge, he))) {
		std::pair<glm::vec2, float> circle= circumcircle(predecessor_predecessor_leaf->_data._site, predecessor_leaf->_data._site, successor_leaf->_data._site);
		glm::vec2 center= circle.first;
		float radius= circle.second;

		Event * new_circle_event= new Event(CircleEvent);
		new_circle_event->_circle_center= center;
		new_circle_event->_circle_radius= radius;
		new_circle_event->_leaf= predecessor_leaf;
		predecessor_leaf->_data._circle_event= new_circle_event;
		/*if (auto search = _queue.find(new_circle_event); search != _queue.end()) {
			std::cout << "Event deja en queue 3 : " << *new_circle_event << " || " << *(*search) << "\n";
		}*/
		//_queue.insert(new_circle_event);
		_queue.push(new_circle_event);
		if (_verbose) {
			//std::cout << "circumcircle pp " << predecessor_predecessor_leaf->_data << " ; " << predecessor_leaf->_data << " ; " << successor_leaf->_data;
			//std::cout << " ; center = " << glm_to_string(center) << " ; radius = " << radius << "\n";
			std::cout << "New CircleEvent pp : " << *new_circle_event << "\n";
		}
	}

	if ((successor_successor_leaf!= NULL) && (breakpoints_converge(successor_bkpt->_data._half_edge, he))) {
		std::pair<glm::vec2, float> circle= circumcircle(predecessor_leaf->_data._site, successor_leaf->_data._site, successor_successor_leaf->_data._site);
		glm::vec2 center= circle.first;
		float radius= circle.second;

		Event * new_circle_event= new Event(CircleEvent);
		new_circle_event->_circle_center= center;
		new_circle_event->_circle_radius= radius;
		new_circle_event->_leaf= successor_leaf;
		successor_leaf->_data._circle_event= new_circle_event;
		/*if (auto search = _queue.find(new_circle_event); search != _queue.end()) {
			std::cout << "Event deja en queue 4 : " << *new_circle_event << " || " << *search << "\n";
		}*/
		//_queue.insert(new_circle_event);
		_queue.push(new_circle_event);
		if (_verbose) {
			//std::cout << "circumcircle ss " << predecessor_leaf->_data << " ; " << successor_leaf->_data << " ; " << successor_successor_leaf->_data;
			//std::cout << " ; center = " << glm_to_string(center) << " ; radius = " << radius << "\n";
			std::cout << "New CircleEvent ss : " << *new_circle_event << "\n";
		}
	}

	// suppression de l'arc dans le BST
	if ( (e->_leaf->is_left()) && (parent->_right!= NULL) && (grand_parent!= NULL)) {
		if (parent->is_left()) {
			grand_parent->set_left(parent->_right);
		}
		else {
			grand_parent->set_right(parent->_right);
		}
		predecessor->_data._sites.second= glm::vec2(successor_leaf->_data._site);
		predecessor->_data._half_edge= he;
	}
	else if ( (e->_leaf->is_right()) && (parent->_left!= NULL) && (grand_parent!= NULL)) {
		if (parent->is_left()) {
			grand_parent->set_left(parent->_left);
		}
		else {
			grand_parent->set_right(parent->_left);
		}
		successor->_data._sites.first= glm::vec2(predecessor_leaf->_data._site);
		successor->_data._half_edge= he;
	}
}


void Voronoi::export_debug_html(std::string html_path) {
	const unsigned int SVG_WIDTH= 1000;
	const unsigned int SVG_HEIGHT= 800;
	
	//const float MARGIN_FACTOR= 2.0f;
	float max_size= std::max(_bbox_max.x- _bbox_min.x, _bbox_max.y- _bbox_min.y);
	const float VIEW_XMIN= _bbox_min.x- BBOX_MARGIN_PERCENT* 5.0f* max_size;
	const float VIEW_YMIN= _bbox_min.y- BBOX_MARGIN_PERCENT* 5.0f* max_size;
	const float VIEW_XMAX= _bbox_max.x+ BBOX_MARGIN_PERCENT* 5.0f* max_size;
	const float VIEW_YMAX= _bbox_max.y+ BBOX_MARGIN_PERCENT* 5.0f* max_size;
	
	const float SIZE= std::max(VIEW_XMAX- VIEW_XMIN, VIEW_YMAX- VIEW_YMIN);
	const float POINT_RADIUS= 0.006f* SIZE;
	const float STROKE_WIDTH= 0.004f* SIZE;
	const float STEP= 0.01f;

	const float OPACITY_MIN= 0.2f;
	const float OPACITY_MAX= 0.6f;

	auto y_html= [VIEW_YMIN, VIEW_YMAX](float y) -> float {return VIEW_YMIN+ VIEW_YMAX- y;};

	auto site_color= [](glm::vec2 s) -> glm::ivec3 {
		float intpart;
		float fractpartx= modf(s.x, &intpart);
		float fractparty= modf(s.y, &intpart);
		int x= int(255.0f* abs(cos(fractpartx* 6)));
		int y= int(255.0f* fractparty);
		int z= int(255.0f* fractpartx* fractparty);
		return glm::vec3(x, y, z);
	};

	std::ofstream f;
	f.open(html_path);
	f << "<!DOCTYPE html>\n<html>\n<head>\n";
	f << "<style>\n";
	
	f << ".site_point_class {fill: black; fill-opacity:" << OPACITY_MAX << "}\n";
	//f << ".repere_point_class {fill: red;}\n";
	f << ".half_edge_origin_point_class {fill: green; fill-opacity:" << OPACITY_MIN << "}\n";
	f << ".half_edge_destination_point_class {fill: royalblue; fill-opacity:" << OPACITY_MIN << "}\n";
	f << ".half_edge_tmp_point_class {fill: grey; fill-opacity:" << OPACITY_MIN << "}\n";

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
		//std::string str_color= "rgb("+ std::to_string(rand_int(0, 200))+ ", "+ std::to_string(rand_int(0, 200))+ ", "+ std::to_string(rand_int(0, 200))+ ")";
		glm::ivec3 color= site_color(s);
		std::string str_color= "rgb("+ std::to_string(color.x)+ ", "+ std::to_string(color.y)+ ", "+ std::to_string(color.z)+ ")";
		
		f << "<circle class=\"site_point_class\" cx=\"" << s.x << "\" cy=\"" << y_html(s.y) << "\" r=\"" << POINT_RADIUS << "\" style=\"fill:" << str_color << "\" />\n";

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
		f << "<polyline class=\"parabola_class\" points=\""+ poly+ "\" style=\"stroke:" << str_color << "\" />\n";
	}

	for (auto he : _diagram->_half_edges) {
		if (he->_twin== NULL) {
			std::cout << "twin == NULL !\n";
			continue;
		}
		float x1= he->_origin->_coords.x;
		float y1= he->_origin->_coords.y;
		float x2= he->destination()->_coords.x;
		float y2= he->destination()->_coords.y;

		f << "<circle class=\"half_edge_origin_point_class\" cx=\"" << x1 << "\" cy=\"" << y_html(y1) << "\" r=\"" << POINT_RADIUS << "\" />\n";
		f << "<line class=\"complete_half_edge_class\" x1=\"" << x1 << "\" y1=\"" << y_html(y1) << "\" x2=\"" << x2 << "\" y2=\"" << y_html(y2) << "\" />\n";

		/*if (he->_origin!= NULL) {
			f << "<circle class=\"half_edge_origin_point_class\" cx=\"" << he->_origin->_coords.x << "\" cy=\"" << y_html(he->_origin->_coords.y) << "\" r=\"" << POINT_RADIUS << "\" />\n";
			if (he->destination()!= NULL) {
				f << "<line class=\"complete_half_edge_class\" x1=\"" << he->_origin->_coords.x << "\" y1=\"" << y_html(he->_origin->_coords.y) << "\" x2=\"" << he->_twin->_origin->_x << "\" y2=\"" << y_html(he->_twin->_origin->_y) << "\" />\n";
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
		}*/
	}

	f << "</svg>\n";
	f << "</body>\n</html>\n";
	f.close();
}
