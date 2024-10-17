#include <iostream>
#include <sstream>
#include <algorithm>
#include "math.h"
#include <chrono>

#include "voronoi.h"
#include "geom_2d.h"
#include "utile.h"



// renvoie le y du pt de la parabole définie par site et yline, en x
number y_parabola(const pt_type & site, number yline, number x) {
	bool verbose= false;

	if (number_equals_strict(site.y, yline)) {
		std::cerr << "y_parabola problème : site = " << glm_to_string(site) << " ; yline = " << yline << " ; x = " << x << "\n";
		return 0.0;
	}
	number result= (x- site.x)* (x- site.x)* 0.5/ (site.y- yline)+ 0.5* (site.y+ yline);

	if (verbose) {
		std::cout << "y_parabola : site = " << glm_to_string(site) << " ; yline = " << yline << " ; x = " << x << " ; result = " << result << "\n";
	}

	return result;
}


// renvoie la dérivée de la parabole définie par site et yline, en x
number y_derivative_parabola(const pt_type & site, number yline, number x) {
	if (number_equals_strict(site.y, yline)) {
		std::cerr << "y_derivative_parabola problème : site = " << glm_to_string(site) << " ; yline = " << yline << " ; x = " << x << "\n";
		return 0.0;
	}
	return (x- site.x)/ (site.y- yline);
}


// string pour debug de l'équation de la parabole définie par site et yline
std::string parabola_equation(const pt_type & site, number yline) {
	if (number_equals_strict(site.y, yline)) {
		std::cerr << "parabola_equation problème : site = " << glm_to_string(site) << " ; yline = " << yline << "\n";
		return "";
	}
	number a= 0.5/ (site.y- yline);
	number b= site.x/ (yline- site.y);
	number c= 0.5* site.x* site.x/ (site.y- yline)+ 0.5* (site.y+ yline);
	return std::to_string(a)+ "x2 + "+ std::to_string(b)+ "x + "+ std::to_string(c);
}


// x de l'intersection de 2 paraboles définies par site_left, site_right et yline
number parabolas_intersection(const pt_type & site_left, const pt_type & site_right, number yline) {
	bool verbose= false;

	// cas limite des sites situés sur yline
	if (number_equals_epsilon(site_left.y, yline)) {
		if (number_equals_epsilon(site_right.y, yline)) {
			std::cerr << "parabolas_intersection 0 pt (2 sites sur yline) : site_left=" << glm_to_string(site_left) << " ; site_right=" << glm_to_string(site_right) << " ; yline=" << yline << "\n";
			return 0.0;
		}
		else {
			if (verbose) {
				std::cout << "parabolas_intersection site_left sur yline : site_left=" << glm_to_string(site_left) << " ; site_right=" << glm_to_string(site_right) << " ; yline=" << yline << "\n";
			}
			return site_left.x;
		}
	}
	else if (number_equals_epsilon(site_right.y, yline)) {
		if (verbose) {
			std::cout << "parabolas_intersection site_right sur yline : site_left=" << glm_to_string(site_left) << " ; site_right=" << glm_to_string(site_right) << " ; yline=" << yline << "\n";
		}
		return site_right.x;
	}

	number a= 2.0* (site_right.y- site_left.y);
	number b= 4.0* ((site_left.y- yline)* site_right.x- (site_right.y- yline)* site_left.x);
	number c= 2.0* (site_right.y- yline)* (site_left.x* site_left.x+ site_left.y* site_left.y- yline* yline)
			-2.0* (site_left.y- yline)* (site_right.x* site_right.x+ site_right.y* site_right.y- yline* yline);

	if (number_equals_epsilon(a, 0.0)) {
		if (verbose) {
			std::cout << "parabolas_intersection site_left.y == site_right.y : site_left=" << glm_to_string(site_left) << " ; site_right=" << glm_to_string(site_right) << " ; yline=" << yline << "\n";
		}
		return -1.0* c/ b;
	}

	number delta= b* b- 4.0* a* c;
	if (delta< 0.0) {
		std::cerr << "parabolas_intersection 0 pt : site_left=" << glm_to_string(site_left) << " ; site_right=" << glm_to_string(site_right) << " ; yline=" << yline << "\n";
		return 0.0;
	}

	// 2 intersections dans la majorité des cas
	number delta_sqroot= sqrt(delta);
	number x1= 0.5* (-1.0* b- delta_sqroot)/ a;
	number x2= 0.5* (-1.0* b+ delta_sqroot)/ a;

	// je compare les tangentes aux 2 pts pour savoir quel pt respecte left-right
	// je ne passe pas par y_derivative_parabola pour éviter les divisions
	//number dleft_x1= y_derivative_parabola(site_left, yline, x1);
	//number dright_x1= y_derivative_parabola(site_right, yline, x1);
	//if (dleft_x1>= dright_x1) {
	if ((x1- site_left.x)* (site_right.y- yline)> (x1- site_right.x)* (site_left.y- yline)) {
		return x1;
	}
	else {
		return x2;
	}
}


// est-ce que les 2 breakpoints représentés par 2 half-edges convergent
bool breakpoints_converge(DCEL_HalfEdge * he1, DCEL_HalfEdge * he2) {
	bool verbose= false;

	if (verbose) {
		std::cout << "breakpoints_converge he1 = " << *he1 << " ; he2 = " << *he2 << "\n";
	}

	pt_type result;
	pt_type he1_origin, he2_origin;
	DCEL_HalfEdgeData * he1_data= (DCEL_HalfEdgeData *)(he1->_data);
	DCEL_HalfEdgeData * he2_data= (DCEL_HalfEdgeData *)(he2->_data);

	if (verbose) {
		std::cout << "breakpoints_converge he1_data = " << *he1_data << " ; he2_data = " << *he2_data << "\n";
	}

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

	bool is_inter= ray_intersects_ray(
		he1_origin, he1_data->_direction,
		he2_origin, he2_data->_direction, 
	&result);

	if (verbose) {
		std::cout << "breakpoints_converge is_inter = " << is_inter << " ; result = " << glm_to_string(result) << "\n";
	}

	// pourquoi cela arrive t'il ?
	if (number_equals_strict(he1_origin.x, he2_origin.x) && number_equals_strict(he1_origin.y, he2_origin.y)) {
		return false;
	}

	return is_inter;
}


// ---------------------------------------------------------------------------------------------
DCEL_HalfEdgeData::DCEL_HalfEdgeData() : _is_full_line(false), _center(pt_type(0.0)) {

}


DCEL_HalfEdgeData::DCEL_HalfEdgeData(bool is_full_line, pt_type direction) :
_is_full_line(is_full_line), _center(pt_type(0.0)), _direction(direction)
{

}


DCEL_HalfEdgeData::DCEL_HalfEdgeData(bool is_full_line, pt_type center, pt_type direction) :
	_is_full_line(is_full_line), _center(center), _direction(direction)
{
	
}


DCEL_HalfEdgeData::~DCEL_HalfEdgeData() {

}


std::ostream & operator << (std::ostream & os, const DCEL_HalfEdgeData & d) {
	os << "is_full_line = " << d._is_full_line << " ; center = " << glm_to_string(d._center);
	return os;
}

// ---------------------------------------------------------------------------------------------
BeachLineNode::BeachLineNode() :
	_type(Arc), _site(pt_type(0.0, 0.0)), _circle_event(NULL), _sites(std::make_pair(pt_type(0.0, 0.0), pt_type(0.0, 0.0))), _half_edge(NULL) 
{

}


BeachLineNode::BeachLineNode(BeachLineNodeType type) :
	_type(type), _site(pt_type(0.0, 0.0)), _circle_event(NULL), _sites(std::make_pair(pt_type(0.0, 0.0), pt_type(0.0, 0.0))), _half_edge(NULL) 
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
	}
	return os;
}


// ---------------------------------------------------------------------------------------------
Event::Event() :
	_type(CircleEvent), _site(pt_type(0.0, 0.0)), _circle_center(pt_type(0.0, 0.0)), _circle_radius(0.0), _leaf(NULL)
{

}


Event::Event(EventType type) :
	_type(type), _site(pt_type(0.0, 0.0)), _circle_center(pt_type(0.0, 0.0)), _circle_radius(0.0), _leaf(NULL)
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
// utilisé par la queue d'événements lors de l'insertion des sites puis des CircleEvent
bool EventCmp::operator()(const Event * lhs, const Event * rhs) const {
	number lx= 0.0;
	number rx= 0.0;
	number ly= 0.0;
	number ry= 0.0;

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

	std::cerr << "EventCmp equality : " << *lhs << " || " << *rhs << "\n";

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


// ---------------------------------------------------------------------------------------------
Voronoi::Voronoi() : _current_y(0.0), _max_height(0), _max_height_n_nodes(0), _max_imbalance(0) {

	// fonction de comparaison utilisée lors de la recherche dans le BST _beachline
	std::function<int(BeachLineNode, BeachLineNode)> cmp= [this](BeachLineNode lhs, BeachLineNode rhs) {
		bool verbose= false;

		number lhx= 0.0;
		number rhx= 0.0;

		// dans le cas d'un Arc (feuille du BST) on considère le x du site associé
		// sinon on calcule le x du breakpoint = intersection des 2 arcs définissant le breakpoint
		if (lhs._type== Arc) {
			lhx= lhs._site.x;
		}
		else if (lhs._type== BreakPoint) {
			lhx= parabolas_intersection(lhs._sites.first, lhs._sites.second, _current_y);
		}

		if (rhs._type== Arc) {
			rhx= rhs._site.x;
		}
		else if (rhs._type== BreakPoint) {
			rhx= parabolas_intersection(rhs._sites.first, rhs._sites.second, _current_y);
		}

		if (verbose) {
			std::cout << "cmp : " << lhs << " || " << rhs << " || " << lhx << " ; " << rhx << "\n";
		}
		
		if (lhx< rhx) {
			return -1;
		}
		else if (lhx> rhx) {
			return 1;
		}

		// on ne veut pas d'égalité entre un Arc et un Bkpt (jeu test1/site_bkpt_x_align)
		//return 0;
		return -1;
	};

	_beachline= new BST<BeachLineNode>(cmp);
	_diagram= new DCEL();
}


Voronoi::Voronoi(const std::vector<pt_type> & sites, bool verbose, bool output_beachline, bool output_intermediate, bool output_stat, std::string debug_path) : Voronoi()
{
	_sites= sites;
	_debug_path= debug_path;
	_verbose= verbose;

	if (_debug_path!= "") {
		std::string cmd= "rm -rf "+ _debug_path+ " && mkdir -p "+ _debug_path;
		system(cmd.c_str());
	}

	// _bbox est la bounding box des sites, utilisée lors de l'ajout de la bbox englobante du DCEL _diagram
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
	number last_site_x= 1e8;
	number last_site_y= 1e8;
	
	const auto t1= std::chrono::high_resolution_clock::now();
	while (!_queue.empty()) {
		Event * e= _queue.top();
		_queue.pop();
		
		// un evnmt peut être rendu invalide par un autre evnmt
		if (!e->_is_valid) {
			continue;
		}
		
		// on évite les sites doublons
		if ((number_equals_strict(e->_site.x, last_site_x)) && (number_equals_strict(e->_site.y, last_site_y))) {
			continue;
		}

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

			// il faut une méthode spéciale pour les sites qui ont tous le même y et sont les 1ers à être traités
			if (number_equals_strict(_current_y, _first_y)) {
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

		// pour débugger on exporte l'état du BST _beachline ainsi qu'un état intermédiaire du _diagram
		if (output_beachline) {
			_beachline->export_html(_debug_path+ "/beachline"+ std::to_string(_debug_count)+ ".html");
		}
		
		if (output_intermediate) {
			export_debug_html(_debug_path+ "/debug"+ std::to_string(_debug_count)+ ".html");
		}

		if (output_stat) {
			unsigned height= _beachline->height();
			if (height> _max_height) {
				_max_height= height;
				_max_height_n_nodes= _beachline->n_nodes();
				if (_debug_path!= "") {
					_beachline->draw(_debug_path+ "/beachline_max_height.pbm");
				}
			}
			int max_imbalance= _beachline->max_imbalance();
			if (abs(max_imbalance)> abs(_max_imbalance)) {
				_max_imbalance= max_imbalance;
				_max_imbalance_n_nodes= _beachline->n_nodes();
				if (_debug_path!= "") {
					_beachline->draw(_debug_path+ "/beachline_max_imbalance.pbm");
				}
			}
		}

		_debug_count++;
	}
	const auto t2= std::chrono::high_resolution_clock::now();
	const std::chrono::duration<number, std::milli> dt= t2- t1;
	std::cout << "temps traitement queue = " << dt.count() << "\n";


	if (_verbose) {
		std::cout << "------------------------------------\n";
	}

	const auto t3= std::chrono::high_resolution_clock::now();

	// suppression des sommets isolés
	_diagram->delete_isolated_vertices();
	// suppression des edges ton la destination == origine
	_diagram->delete_loop_edge();

	if (_verbose) {
		std::cout << "diagram_valid= " << _diagram->is_valid() << "\n";
	}

	if (_verbose) {
		std::cout << "ajout BBOX\n";
	}

	// aojut bbox englobante
	number max_size= std::max(_bbox_max.x- _bbox_min.x, _bbox_max.y- _bbox_min.y);
	pt_type bbox_min= pt_type(_bbox_min.x- BBOX_MARGIN_PERCENT* max_size, _bbox_min.y- BBOX_MARGIN_PERCENT* max_size);
	pt_type bbox_max= pt_type(_bbox_max.x+ BBOX_MARGIN_PERCENT* max_size, _bbox_max.y+ BBOX_MARGIN_PERCENT* max_size);
	_diagram->add_bbox(bbox_min, bbox_max);

	if (output_stat) {
		const auto t4= std::chrono::high_resolution_clock::now();
		const std::chrono::duration<number, std::milli> dt2= t4- t3;
		std::cout << "temps post-traitement (bbox) = " << dt2.count() << "\n";

		std::cout << "n sites = " << _site_times.size() << " ; n circles = " << _circle_times.size() << "\n";
		number t_sites= 0.0;
		for (auto t : _site_times) {
			t_sites+= t;
		}
		t_sites/= _site_times.size();

		number t_circles= 0.0;
		for (auto t : _circle_times) {
			t_circles+= t;
		}
		t_circles/= _circle_times.size();
		std::cout << "t_sites = " << t_sites << " ; t_circles = " << t_circles << "\n";

		std::cout << "max_height = " << _max_height;
		std::cout << " ; max_height_n_nodes = " << _max_height_n_nodes;
		std::cout << " ; max_imbalance = " << _max_imbalance;
		std::cout << " ; max_imbalance_n_nodes = " << _max_imbalance_n_nodes;
		std::cout << "\n";
	}
}


Voronoi::~Voronoi() {
	delete _diagram;
	delete _beachline;
}


// dans ces 2 fonctions que choisir pour size ? on veut être surs que ces lignes intersecteront la bbox 
// mais position peut être n'importe où
// si je prends size trop faible ca bugge
DCEL_HalfEdge * Voronoi::add_full_line(pt_type position, pt_type direction) {
	// dans le cas add_full_segment on sait que norm(direction)> 1, donc pas besoin de diviser pas sa norme
	number size= 1e9;
	DCEL_HalfEdge * he= _diagram->add_edge(position- size* direction, position+ size* direction);
	//DCEL_HalfEdge * he= _diagram->add_edge(position- direction, position+ direction);
	he->_data= new DCEL_HalfEdgeData(true, position, direction);
	he->_twin->_data= new DCEL_HalfEdgeData(true, position, -direction);
	return he;
}


// ajout d'une demi-ligne
DCEL_HalfEdge * Voronoi::add_half_line(pt_type position, pt_type direction) {
	number size= 1e9;
	DCEL_HalfEdge * he= _diagram->add_edge(position, position+ size* direction/ sqrt(direction.x* direction.x+ direction.y* direction.y));
	he->_data= new DCEL_HalfEdgeData(false, direction);
	he->_twin->_data= new DCEL_HalfEdgeData(false, -direction);
	return he;
}


// set origine d'un half-edge
void Voronoi::set_halfedge_origin(DCEL_HalfEdge * he, DCEL_Vertex * v) {
	if (_verbose) {
		std::cout << "set_halfedge_origin : he = " << *he << " ; v = " << *v << "\n";
	}
	he->set_origin(v);
	DCEL_HalfEdgeData * he_data= (DCEL_HalfEdgeData *)(he->_data);
	he_data->_is_full_line= false;
	DCEL_HalfEdgeData * he_twin_data= (DCEL_HalfEdgeData *)(he->_twin->_data);
	he_twin_data->_is_full_line= false;
}


// gestion des 1ers sites qui ont tous le même y
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

		pt_type position= pt_type((max_node->_data._site.x+ e->_site.x)* 0.5, _current_y);
		pt_type direction= pt_type(0.0, -1.0);
		DCEL_HalfEdge * he= add_full_line(position, direction);

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


// gestion des énévements de type site
void Voronoi::handle_site_event(Event * e) {
	const auto t1= std::chrono::high_resolution_clock::now();

	// recherche de l'arc au dessus du site
	BeachLineNode * new_arc= new BeachLineNode(Arc);
	new_arc->_site= pt_type(e->_site);
	Node<BeachLineNode> * node_above_site= _beachline->search(*new_arc, false);

	// sites et breakpoints voisins
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
		node_above_site->_data._circle_event->_is_valid= false;
	}

	// ajout des 2 half-edge jumeaux qui sont pour l'instant 2 lignes
	// y = valeur de la parabole au dessus du site en site.x ; pente = tangente à la parabole
	number y= y_parabola(node_above_site->_data._site, _current_y, e->_site.x);
	number dy= y_derivative_parabola(node_above_site->_data._site, _current_y, e->_site.x);
	pt_type position= pt_type(e->_site.x, y);
	pt_type direction= pt_type(1.0, dy);
	DCEL_HalfEdge * he= add_full_line(position, direction);

	if (_verbose) {
		std::cout << "New line : position = " << glm_to_string(position) << " ; direction = " << glm_to_string(direction) << "\n";
	}

	// on crée 2 copies de node_above_site qui seront les 2 portions du même arc séparées par le nouvel arc
	Node<BeachLineNode> * node_above_site_left_copy= _beachline->gen_node(node_above_site->_data);
	Node<BeachLineNode> * node_above_site_right_copy= _beachline->gen_node(node_above_site->_data);

	// on associe au bkpt gauche le half-edge qui va vers la gauche
	BeachLineNode * breakpoint_left= new BeachLineNode(BreakPoint);
	breakpoint_left->_sites= std::make_pair(node_above_site_left_copy->_data._site, new_arc->_site);
	breakpoint_left->_half_edge= he->_twin;

	// on associe au bkpt droit le half-edge qui va vers la droite
	BeachLineNode * breakpoint_right= new BeachLineNode(BreakPoint);
	breakpoint_right->_sites= std::make_pair(new_arc->_site, node_above_site_right_copy->_data._site);
	breakpoint_right->_half_edge= he;

	// voir si les nouveaux breakpoints consécutifs convergent ; si c'est le cas on crée un circle event
	if (prev_site!= NULL && prev_bkpt!= NULL && breakpoints_converge(prev_bkpt->_data._half_edge, breakpoint_left->_half_edge)) {
		std::pair<pt_type, number> circle= circumcircle(prev_site->_data._site, node_above_site->_data._site, new_arc->_site);
		pt_type center= circle.first;
		number radius= circle.second;

		Event * new_circle_event= new Event(CircleEvent);
		new_circle_event->_circle_center= center;
		new_circle_event->_circle_radius= radius;
		new_circle_event->_leaf= node_above_site_left_copy;
		node_above_site_left_copy->_data._circle_event= new_circle_event;
		_queue.push(new_circle_event);
		if (_verbose) {
			std::cout << "bkpt converge : " << *prev_bkpt->_data._half_edge << " ; " << *breakpoint_left->_half_edge << "\n";
			std::cout << "New CircleEvent prev : " << *new_circle_event << "\n";
		}
	}

	if ((next_site!= NULL) && (next_bkpt!= NULL) && (breakpoints_converge(next_bkpt->_data._half_edge, breakpoint_right->_half_edge))) {
		std::pair<pt_type, number> circle= circumcircle(new_arc->_site, node_above_site->_data._site, next_site->_data._site);
		pt_type center= circle.first;
		number radius= circle.second;

		Event * new_circle_event= new Event(CircleEvent);
		new_circle_event->_circle_center= center;
		new_circle_event->_circle_radius= radius;
		new_circle_event->_leaf= node_above_site_right_copy;
		node_above_site_right_copy->_data._circle_event= new_circle_event;
		_queue.push(new_circle_event);
		if (_verbose) {
			std::cout << "bkpt converge : " << *next_bkpt->_data._half_edge << " ; " << *breakpoint_right->_half_edge << "\n";
			std::cout << "New CircleEvent next : " << *new_circle_event << "\n";
		}
	}

	// construction du sous-arbre à insérer dans la beachline
	Node<BeachLineNode> * breakpoint_left_node= _beachline->gen_node(*breakpoint_left);
	Node<BeachLineNode> * breakpoint_right_node= _beachline->gen_node(*breakpoint_right);
	Node<BeachLineNode> * new_node= _beachline->gen_node(*new_arc);
	breakpoint_left_node->set_left(node_above_site_left_copy);
	breakpoint_left_node->set_right(breakpoint_right_node);
	breakpoint_right_node->set_left(new_node);
	breakpoint_right_node->set_right(node_above_site_right_copy);
	
	// insertion du sous-arbre
	_beachline->transplant(node_above_site, breakpoint_left_node);
	
	// ne fonctionne pas pour l'instant, cf commentaires voronoi.h
	//_beachline->balance();
	if (_beachline->_root->_right->_data._type== BreakPoint && _beachline->height(_beachline->_root->_right)> _beachline->height(_beachline->_root->_left)) {
		_beachline->rotate_left(_beachline->_root);
	}
	else if (_beachline->_root->_left->_data._type== BreakPoint && _beachline->height(_beachline->_root->_right)< _beachline->height(_beachline->_root->_left)) {
		_beachline->rotate_right(_beachline->_root);
	}

	const auto t2= std::chrono::high_resolution_clock::now();
	const std::chrono::duration<number, std::milli> dt= t2- t1;
	_site_times.push_back(dt.count());
}


// gestion événement de type cercle (ie quand des edges du _diagram convergent en un point)
void Voronoi::handle_circle_event(Event * e) {
	const auto t1= std::chrono::high_resolution_clock::now();

	/*std::cout << "---\n";
	std::cout << *_beachline;
	std::cout << "---\n";*/

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
	// direction du nouveau he perpendiculaire à site_droit - site_gauche et pointant vers les y< 0
	pt_type v= successor_leaf->_data._site- predecessor_leaf->_data._site;
	DCEL_HalfEdge * he= add_half_line(e->_circle_center, pt_type(v.y, -v.x));
	DCEL_Vertex * circle_center_vertex= he->_origin;
	
	if (_verbose) {
		std::cout << "predecessor_leaf = " << *predecessor_leaf << "\n";
		std::cout << "successor_leaf = " << *successor_leaf << "\n";
		std::cout << "New Edge = " << *he << "\n";
	}

	// suppression des CircleEvent des arcs voisins
	if ((predecessor_leaf->_data._circle_event!= NULL)) {
		if (_verbose) {
			std::cout << "suppression CircleEvent predecessor : " << *predecessor_leaf->_data._circle_event << "\n";
		}
		predecessor_leaf->_data._circle_event->_is_valid= false;
	}
	if ((successor_leaf->_data._circle_event!= NULL)) {
		if (_verbose) {
			std::cout << "suppression CircleEvent successor : " << *successor_leaf->_data._circle_event << "\n";
		}
		successor_leaf->_data._circle_event->_is_valid= false;
	}

	// rattacher le nouveau vertex et les 2 nouveaux he aux he existants
	set_halfedge_origin(predecessor->_data._half_edge->_twin, circle_center_vertex);
	set_halfedge_origin(successor->_data._half_edge->_twin, circle_center_vertex);
	he->_twin->set_next(predecessor->_data._half_edge->_twin);
	successor->_data._half_edge->set_next(he);
	predecessor->_data._half_edge->set_next(successor->_data._half_edge->_twin);

	// on regarde si le nouveau half-edge converge avec ses voisins ; si oui création d'un nouveau circle event
	if ((predecessor_predecessor_leaf!= NULL) && (breakpoints_converge(predecessor_bkpt->_data._half_edge, he))) {
		std::pair<pt_type, number> circle= circumcircle(predecessor_predecessor_leaf->_data._site, predecessor_leaf->_data._site, successor_leaf->_data._site);
		pt_type center= circle.first;
		number radius= circle.second;

		Event * new_circle_event= new Event(CircleEvent);
		new_circle_event->_circle_center= center;
		new_circle_event->_circle_radius= radius;
		new_circle_event->_leaf= predecessor_leaf;
		predecessor_leaf->_data._circle_event= new_circle_event;
		_queue.push(new_circle_event);
		if (_verbose) {
			std::cout << "New CircleEvent pp : " << *new_circle_event << "\n";
		}
	}

	if ((successor_successor_leaf!= NULL) && (breakpoints_converge(successor_bkpt->_data._half_edge, he))) {
		std::pair<pt_type, number> circle= circumcircle(predecessor_leaf->_data._site, successor_leaf->_data._site, successor_successor_leaf->_data._site);
		pt_type center= circle.first;
		number radius= circle.second;

		Event * new_circle_event= new Event(CircleEvent);
		new_circle_event->_circle_center= center;
		new_circle_event->_circle_radius= radius;
		new_circle_event->_leaf= successor_leaf;
		successor_leaf->_data._circle_event= new_circle_event;
		_queue.push(new_circle_event);
		if (_verbose) {
			std::cout << "New CircleEvent ss : " << *new_circle_event << "\n";
		}
	}

	// suppression de l'arc dans le BST
	if ((e->_leaf->is_left()) && (parent->_right!= NULL) && (grand_parent!= NULL)) {
		if (parent->is_left()) {
			grand_parent->set_left(parent->_right);
		}
		else {
			grand_parent->set_right(parent->_right);
		}
		predecessor->_data._sites.second= pt_type(successor_leaf->_data._site);
		predecessor->_data._half_edge= he;
	}
	else if ((e->_leaf->is_right()) && (parent->_left!= NULL) && (grand_parent!= NULL)) {
		if (parent->is_left()) {
			grand_parent->set_left(parent->_left);
		}
		else {
			grand_parent->set_right(parent->_left);
		}
		successor->_data._sites.first= pt_type(predecessor_leaf->_data._site);
		successor->_data._half_edge= he;
	}

	// ne fonctionne pas pour l'instant, cf commentaires voronoi.h
	//_beachline.balance();
	if (_beachline->_root->_right->_data._type== BreakPoint && _beachline->height(_beachline->_root->_right)> _beachline->height(_beachline->_root->_left)) {
		_beachline->rotate_left(_beachline->_root);
	}
	else if (_beachline->_root->_left->_data._type== BreakPoint && _beachline->height(_beachline->_root->_right)< _beachline->height(_beachline->_root->_left)) {
		_beachline->rotate_right(_beachline->_root);
	}

	const auto t2= std::chrono::high_resolution_clock::now();
	const std::chrono::duration<number, std::milli> dt= t2- t1;
	_circle_times.push_back(dt.count());
}


// export sous forme de HTML pour debug
void Voronoi::export_debug_html(std::string html_path) {
	const unsigned int SVG_WIDTH= 1000;
	const unsigned int SVG_HEIGHT= 800;
	
	number max_size= std::max(_bbox_max.x- _bbox_min.x, _bbox_max.y- _bbox_min.y);
	const number VIEW_XMIN= _bbox_min.x- BBOX_MARGIN_PERCENT* 5.0* max_size;
	const number VIEW_YMIN= _bbox_min.y- BBOX_MARGIN_PERCENT* 5.0* max_size;
	const number VIEW_XMAX= _bbox_max.x+ BBOX_MARGIN_PERCENT* 5.0* max_size;
	const number VIEW_YMAX= _bbox_max.y+ BBOX_MARGIN_PERCENT* 5.0* max_size;
	
	const number SIZE= std::max(VIEW_XMAX- VIEW_XMIN, VIEW_YMAX- VIEW_YMIN);
	const number POINT_RADIUS= 0.006* SIZE;
	const number STROKE_WIDTH= 0.004* SIZE;
	const number STEP= 0.01;

	const number OPACITY_MIN= 0.2;
	const number OPACITY_MAX= 0.6;

	auto y_html= [VIEW_YMIN, VIEW_YMAX](number y) -> number {return VIEW_YMIN+ VIEW_YMAX- y;};

	auto site_color= [](pt_type s) -> glm::ivec3 {
		number intpart;
		number fractpartx= modf(s.x, &intpart);
		number fractparty= modf(s.y, &intpart);
		int x= int(255.0* abs(cos(fractpartx* 6)));
		int y= int(255.0* fractparty);
		int z= int(255.0* fractpartx* fractparty);
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

		number x= VIEW_XMIN;
		std::string poly= "";
		while (true) {
			number y= y_parabola(s, _current_y, x);
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
		number x1= he->_origin->_coords.x;
		number y1= he->_origin->_coords.y;
		number x2= he->destination()->_coords.x;
		number y2= he->destination()->_coords.y;
		DCEL_HalfEdgeData * he_data= (DCEL_HalfEdgeData *)(he->_data);

		f << "<circle class=\"half_edge_origin_point_class\" cx=\"" << x1 << "\" cy=\"" << y_html(y1) << "\" r=\"" << POINT_RADIUS << "\" />\n";
		f << "<line class=\"complete_half_edge_class\" x1=\"" << x1 << "\" y1=\"" << y_html(y1) << "\" x2=\"" << x2 << "\" y2=\"" << y_html(y2) << "\" />\n";
	}

	f << "</svg>\n";
	f << "</body>\n</html>\n";
	f.close();
}
