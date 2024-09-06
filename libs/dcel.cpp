#include <algorithm>
#include <fstream>
#include <cmath>
#include <tuple>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "dcel.h"
#include "geom_2d.h"
#include "utile.h"


// ----------------------------------------------------------------------
DCEL_Vertex::DCEL_Vertex() : _x(0.0f), _y(0.0f), _incident_edge(NULL) {

}


DCEL_Vertex::DCEL_Vertex(float x, float y) : _x(x), _y(y), _incident_edge(NULL) {

}


DCEL_Vertex::~DCEL_Vertex() {

}


std::vector<DCEL_HalfEdge *> DCEL_Vertex::get_incident_edges() {
	std::vector<DCEL_HalfEdge *> result;
	result.push_back(_incident_edge);
	DCEL_HalfEdge * current= _incident_edge;
	while(true) {
		current= current->_next;
		if (current->destination()== this) {
			current= current->_twin;
			if (current== _incident_edge) {
				break;
			}
			result.push_back(current);
		}
	}
	return result;
}


std::ostream & operator << (std::ostream & os, DCEL_Vertex & v) {
	os << "(" << v._x << " ; " << v._y << ")";
	return os;
}


// ----------------------------------------------------------------------
DCEL_HalfEdge::DCEL_HalfEdge() : 
_origin(NULL), _twin(NULL), _next(NULL), _previous(NULL), _incident_face(NULL), _dx(0.0f), _dy(0.0f), _tmp_x(0.0f), _tmp_y(0.0f)
{

}


DCEL_HalfEdge::DCEL_HalfEdge(DCEL_Vertex * origin) :
_origin(origin), _twin(NULL), _next(NULL), _previous(NULL), _incident_face(NULL), _dx(0.0f), _dy(0.0f), _tmp_x(0.0f), _tmp_y(0.0f)
{

}


DCEL_HalfEdge::~DCEL_HalfEdge() {

}


DCEL_Vertex * DCEL_HalfEdge::destination() {
	return _twin->_origin;
}


DCEL_Face * DCEL_HalfEdge::opposite_face() {
	return _twin->_incident_face;
}


void DCEL_HalfEdge::set_twin(DCEL_HalfEdge * hedge) {
	_twin= hedge;
	hedge->_twin= this;
}


void DCEL_HalfEdge::set_next(DCEL_HalfEdge * hedge) {
	_next= hedge;
	hedge->_previous= this;
}


void DCEL_HalfEdge::set_tmp_data(glm::vec2 direction, glm::vec2 position) {
	_dx= direction.x;
	_dy= direction.y;
	_tmp_x= position.x;
	_tmp_y= position.y;
	if (_twin!= NULL) {
		_twin->_dx= -direction.x;
		_twin->_dy= -direction.y;
		_twin->_tmp_x= position.x;
		_twin->_tmp_y= position.y;
	}
}


std::ostream & operator << (std::ostream & os, DCEL_HalfEdge & e) {
	if (e._origin!= NULL) {
		os << *e._origin;
	}
	else {
		os << "NULL";
	}
	os << " -> ";
	if (e.destination()!= NULL) {
		os << *e.destination();
	}
	else {
		os << "NULL";
	}
	os << " ; dx = " << e._dx << " ; dy = " << e._dy << " ; _tmp_x = " << e._tmp_x << " ; _tmp_y = " << e._tmp_y;
	
	return os;
}


// ----------------------------------------------------------------------
DCEL_Face::DCEL_Face() : _outer_edge(NULL) {

}


/*DCEL_Face::DCEL_Face(DCEL_HalfEdge * outer_edge) : _outer_edge(outer_edge) {
	
}*/


DCEL_Face::~DCEL_Face() {

}


std::vector<DCEL_Vertex *> DCEL_Face::get_vertices() {
	std::vector<DCEL_Vertex *> result;
	DCEL_HalfEdge * first_edge= _outer_edge;
	DCEL_HalfEdge * current_edge= _outer_edge;
	do {
		result.push_back(current_edge->_origin);
		current_edge= current_edge->_next;
	} while (current_edge!= first_edge);

	return result;
}


std::vector<DCEL_HalfEdge *> DCEL_Face::get_edges() {
	std::vector<DCEL_HalfEdge *> result;
	DCEL_HalfEdge * first_edge= _outer_edge;
	DCEL_HalfEdge * current_edge= _outer_edge;
	do {
		result.push_back(current_edge);
		current_edge= current_edge->_next;
	} while (current_edge!= first_edge);

	return result;
}


std::vector<DCEL_Face *> DCEL_Face::get_adjacent_faces() {
	std::vector<DCEL_Face *> result;
	DCEL_HalfEdge * first_edge= _outer_edge;
	DCEL_HalfEdge * current_edge= _outer_edge;
	do {
		DCEL_Face * face= current_edge->opposite_face();
		std::vector<DCEL_Face *>::iterator it= find(result.begin(), result.end(), face);
		if (it!= result.end()) {
			result.push_back(face);
		}
		current_edge= current_edge->_next;
	} while (current_edge!= first_edge);

	return result;
}


std::pair<float , float> DCEL_Face::get_gravity_center() {
	float x= 0.0f;
	float y= 0.0f;
	std::vector<DCEL_Vertex *> vertices= get_vertices();
	for (auto v : vertices) {
		x+= v->_x;
		y+= v->_y;
	}
	x/= vertices.size();
	y/= vertices.size();
	return std::make_pair(x, y);
}


std::ostream & operator << (std::ostream & os, DCEL_Face & f) {
	for (auto edge : f.get_edges()) {
		os << *edge << "\n";
	}
	return os;
}


// ----------------------------------------------------------------------
DCEL::DCEL() : _xmin(1e8), _xmax(-1e8), _ymin(1e8), _ymax(-1e8) {

}


DCEL::~DCEL() {
	for (auto & f : _faces) {
		delete f;
	}
	_faces.clear();

	for (auto & he : _half_edges) {
		delete he;
	}
	_half_edges.clear();

	for (auto & v : _vertices) {
		delete v;
	}
	_vertices.clear();
}


DCEL_Vertex * DCEL::add_vertex(float x, float y) {
	DCEL_Vertex * v= new DCEL_Vertex(x, y);
	_vertices.push_back(v);
	return v;
}


DCEL_HalfEdge * DCEL::add_edge(DCEL_Vertex * v1, DCEL_Vertex * v2) {
	DCEL_HalfEdge * hedge1= new DCEL_HalfEdge(v1);
	DCEL_HalfEdge * hedge2= new DCEL_HalfEdge(v2);
	hedge1->_twin= hedge2;
	hedge2->_twin= hedge1;
	_half_edges.push_back(hedge1);
	_half_edges.push_back(hedge2);
	return hedge1;
}


/*DCEL_Face * DCEL::add_face(std::vector<DCEL_HalfEdge *> edges) {
	DCEL_Face * face= new DCEL_Face(edges[0]);
	for (auto edge : edges) {
		edge->_incident_face= face;
	}
	for (unsigned int i=0; i<edges.size()- 1; ++i) {
		edges[i]->_next= edges[i+ 1];
		edges[i+ 1]->_previous= edges[i];
	}
	edges[edges.size()- 1]->_next= edges[0];
	edges[0]->_previous= edges[edges.size()- 1];

	_faces.push_back(face);
	return face;
}*/


DCEL_Face * DCEL::add_face() {
	DCEL_Face * face= new DCEL_Face();
	_faces.push_back(face);
	return face;
}


bool DCEL::is_empty() {
	return _vertices.empty() && _half_edges.empty();
}


void DCEL::compute_bbox() {
	for (auto v : _vertices) {
		if (v->_x< _xmin) {
			_xmin= v->_x;
		}
		if (v->_x> _xmax) {
			_xmax= v->_x;
		}
		if (v->_y< _ymin) {
			_ymin= v->_y;
		}
		if (v->_y> _ymax) {
			_ymax= v->_y;
		}
	}
	
	for (auto he : _half_edges) {
		if (he->_tmp_x< _xmin) {
			_xmin= he->_tmp_x;
		}
		if (he->_tmp_x> _xmax) {
			_xmax= he->_tmp_x;
		}
		if (he->_tmp_y< _ymin) {
			_ymin= he->_tmp_y;
		}
		if (he->_tmp_y> _ymax) {
			_ymax= he->_tmp_y;
		}
	}
}


void DCEL::add_bbox(float bbox_expand) {
	if (is_empty()) {
		DCEL_Vertex * v1= add_vertex(0.0f, 0.0f);
		DCEL_Vertex * v2= add_vertex(1.0f, 0.0f);
		DCEL_Vertex * v3= add_vertex(1.0f, 1.0f);
		DCEL_Vertex * v4= add_vertex(0.0f, 1.0f);
		DCEL_HalfEdge * he1= add_edge(v1, v2);
		DCEL_HalfEdge * he2= add_edge(v2, v3);
		DCEL_HalfEdge * he3= add_edge(v3, v4);
		DCEL_HalfEdge * he4= add_edge(v4, v1);
		he1->set_next(he2);
		he2->set_next(he3);
		he3->set_next(he4);
		he4->set_next(he1);
		create_faces_from_half_edges();
		return;
	}

	compute_bbox();

	/*float width= _xmax- _xmin;
	float height= _ymax- _ymin;
	if (width< 1e-7) {
		width= 1.0f;
	}
	if (height< 1e-7) {
		height= 1.0f;
	}
	_xmin-= width* bbox_expand;
	_xmax+= width* bbox_expand;
	_ymin-= height* bbox_expand;
	_ymax+= height* bbox_expand;*/

	_xmin= -10.0f;
	_xmax= 10.0f;
	_ymin= -10.0f;
	_ymax= 10.0f;

	DCEL_Vertex * bottom_left_corner= add_vertex(_xmin, _ymin);
	DCEL_Vertex * bottom_right_corner= add_vertex(_xmax, _ymin);
	DCEL_Vertex * top_left_corner= add_vertex(_xmin, _ymax);
	DCEL_Vertex * top_right_corner= add_vertex(_xmax, _ymax);

	std::vector<DCEL_HalfEdge *> origin_top;
	std::vector<DCEL_HalfEdge *> origin_bottom;
	std::vector<DCEL_HalfEdge *> origin_left;
	std::vector<DCEL_HalfEdge *> origin_right;

	glm::vec2 inter;
	bool is_inter;

	for (auto he : _half_edges) {
		if ((he->_origin== NULL) && (he->_twin->_origin!= NULL)) {
			glm::vec2 origin(he->_twin->_origin->_x, he->_twin->_origin->_y);
			glm::vec2 direction(he->_twin->_dx, he->_twin->_dy);

			//std::cout << "he= " << *he << " ; origin= " << glm_to_string(origin) << " ; direction= " << glm_to_string(direction) << "\n";

			is_inter= ray_intersects_segment(origin, direction,
				glm::vec2(bottom_left_corner->_x, bottom_left_corner->_y), glm::vec2(bottom_right_corner->_x, bottom_right_corner->_y),
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter.x, inter.y);
				origin_bottom.push_back(he);
				continue;
			}

			is_inter= ray_intersects_segment(origin, direction,
				glm::vec2(bottom_right_corner->_x, bottom_right_corner->_y), glm::vec2(top_right_corner->_x, top_right_corner->_y),
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter.x, inter.y);
				origin_right.push_back(he);
				continue;
			}

			is_inter= ray_intersects_segment(origin, direction,
				glm::vec2(top_right_corner->_x, top_right_corner->_y), glm::vec2(top_left_corner->_x, top_left_corner->_y),
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter.x, inter.y);
				origin_top.push_back(he);
				continue;
			}

			is_inter= ray_intersects_segment(origin, direction,
				glm::vec2(top_left_corner->_x, top_left_corner->_y), glm::vec2(bottom_left_corner->_x, bottom_left_corner->_y),
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter.x, inter.y);
				origin_left.push_back(he);
				continue;
			}

			std::cout << "add_bbox problème\n";
		}
	}

	for (auto he : _half_edges) {
		if ((he->_origin== NULL) && (he->_twin->_origin== NULL)) {
			glm::vec2 origin(he->_tmp_x, he->_tmp_y);
			glm::vec2 direction(he->_dx, he->_dy);
			glm::vec2 twin_direction(-he->_dx, -he->_dy);

			is_inter= ray_intersects_segment(origin, direction,
				glm::vec2(bottom_left_corner->_x, bottom_left_corner->_y), glm::vec2(bottom_right_corner->_x, bottom_right_corner->_y),
				&inter);
			if (is_inter) {
				he->_twin->_origin= add_vertex(inter.x, inter.y);
				origin_bottom.push_back(he->_twin);
			}
			is_inter= ray_intersects_segment(origin, twin_direction,
				glm::vec2(bottom_left_corner->_x, bottom_left_corner->_y), glm::vec2(bottom_right_corner->_x, bottom_right_corner->_y),
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter.x, inter.y);
				origin_bottom.push_back(he);
			}

			is_inter= ray_intersects_segment(origin, direction,
				glm::vec2(bottom_right_corner->_x, bottom_right_corner->_y), glm::vec2(top_right_corner->_x, top_right_corner->_y),
				&inter);
			if (is_inter) {
				he->_twin->_origin= add_vertex(inter.x, inter.y);
				origin_right.push_back(he->_twin);
			}
			is_inter= ray_intersects_segment(origin, twin_direction,
				glm::vec2(bottom_right_corner->_x, bottom_right_corner->_y), glm::vec2(top_right_corner->_x, top_right_corner->_y),
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter.x, inter.y);
				origin_right.push_back(he);
			}

			is_inter= ray_intersects_segment(origin, direction,
				glm::vec2(top_right_corner->_x, top_right_corner->_y), glm::vec2(top_left_corner->_x, top_left_corner->_y),
				&inter);
			if (is_inter) {
				he->_twin->_origin= add_vertex(inter.x, inter.y);
				origin_top.push_back(he->_twin);
			}
			is_inter= ray_intersects_segment(origin, twin_direction,
				glm::vec2(top_right_corner->_x, top_right_corner->_y), glm::vec2(top_left_corner->_x, top_left_corner->_y),
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter.x, inter.y);
				origin_top.push_back(he);
			}

			is_inter= ray_intersects_segment(origin, direction,
				glm::vec2(top_left_corner->_x, top_left_corner->_y), glm::vec2(bottom_left_corner->_x, bottom_left_corner->_y),
				&inter);
			if (is_inter) {
				he->_twin->_origin= add_vertex(inter.x, inter.y);
				origin_left.push_back(he->_twin);
			}
			is_inter= ray_intersects_segment(origin, twin_direction,
				glm::vec2(top_left_corner->_x, top_left_corner->_y), glm::vec2(bottom_left_corner->_x, bottom_left_corner->_y),
				&inter);
			if (is_inter) {
				he->_origin= add_vertex(inter.x, inter.y);
				origin_left.push_back(he);
			}
		}
	}

	sort(origin_bottom.begin(), origin_bottom.end(),
	[](const DCEL_HalfEdge * a, const DCEL_HalfEdge * b) -> bool {
		return a->_origin->_x< b->_origin->_x;
	});

	sort(origin_right.begin(), origin_right.end(),
	[](const DCEL_HalfEdge * a, const DCEL_HalfEdge * b) -> bool {
		return a->_origin->_y< b->_origin->_y;
	});

	sort(origin_top.begin(), origin_top.end(),
	[](const DCEL_HalfEdge * a, const DCEL_HalfEdge * b) -> bool {
		return a->_origin->_x> b->_origin->_x;
	});

	sort(origin_left.begin(), origin_left.end(),
	[](const DCEL_HalfEdge * a, const DCEL_HalfEdge * b) -> bool {
		return a->_origin->_y> b->_origin->_y;
	});

	DCEL_HalfEdge * bbox_he= NULL;
	DCEL_HalfEdge * last_he= NULL;
	DCEL_HalfEdge * first_he= NULL;

	for (int i=0; i<int(origin_bottom.size())- 1; ++i) {
		bbox_he= add_edge(origin_bottom[i]->_origin, origin_bottom[i+ 1]->_origin);
		origin_bottom[i]->_twin->set_next(bbox_he);
		bbox_he->set_next(origin_bottom[i+ 1]);
	}
	for (int i=0; i<int(origin_top.size())- 1; ++i) {
		bbox_he= add_edge(origin_top[i]->_origin, origin_top[i+ 1]->_origin);
		origin_top[i]->_twin->set_next(bbox_he);
		bbox_he->set_next(origin_top[i+ 1]);
	}
	for (int i=0; i<int(origin_right.size())- 1; ++i) {
		bbox_he= add_edge(origin_right[i]->_origin, origin_right[i+ 1]->_origin);
		origin_right[i]->_twin->set_next(bbox_he);
		bbox_he->set_next(origin_right[i+ 1]);
	}
	for (int i=0; i<int(origin_left.size())- 1; ++i) {
		bbox_he= add_edge(origin_left[i]->_origin, origin_left[i+ 1]->_origin);
		origin_left[i]->_twin->set_next(bbox_he);
		bbox_he->set_next(origin_left[i+ 1]);
	}

	if (!origin_bottom.empty()) {
		bbox_he= add_edge(bottom_left_corner, origin_bottom[0]->_origin);
		//last_he->set_next(bbox_he); // ici non last_he==NULL
		first_he= bbox_he;
		bbox_he->set_next(origin_bottom[0]);
		bbox_he= add_edge(origin_bottom[origin_bottom.size()- 1]->_origin, bottom_right_corner);
		origin_bottom[origin_bottom.size()- 1]->_twin->set_next(bbox_he);
	}
	else {
		bbox_he= add_edge(bottom_left_corner, bottom_right_corner);
		//last_he->set_next(bbox_he); // ici non last_he==NULL
		first_he= bbox_he;
	}
	last_he= bbox_he;
	
	if (!origin_right.empty()) {
		bbox_he= add_edge(bottom_right_corner, origin_right[0]->_origin);
		last_he->set_next(bbox_he);
		bbox_he->set_next(origin_right[0]);
		bbox_he= add_edge(origin_right[origin_right.size()- 1]->_origin, top_right_corner);
		origin_right[origin_right.size()- 1]->_twin->set_next(bbox_he);
	}
	else {
		bbox_he= add_edge(bottom_right_corner, top_right_corner);
		last_he->set_next(bbox_he);
	}
	last_he= bbox_he;
	
	if (!origin_top.empty()) {
		bbox_he= add_edge(top_right_corner, origin_top[0]->_origin);
		last_he->set_next(bbox_he);
		bbox_he->set_next(origin_top[0]);
		bbox_he= add_edge(origin_top[origin_top.size()- 1]->_origin, top_left_corner);
		origin_top[origin_top.size()- 1]->_twin->set_next(bbox_he);
	}
	else {
		bbox_he= add_edge(top_right_corner, top_left_corner);
		last_he->set_next(bbox_he);
	}
	last_he= bbox_he;
	
	if (!origin_left.empty()) {
		bbox_he= add_edge(top_left_corner, origin_left[0]->_origin);
		last_he->set_next(bbox_he);
		bbox_he->set_next(origin_left[0]);
		bbox_he= add_edge(origin_left[origin_left.size()- 1]->_origin, bottom_left_corner);
		origin_left[origin_left.size()- 1]->_twin->set_next(bbox_he);
	}
	else {
		bbox_he= add_edge(top_left_corner, bottom_left_corner);
		last_he->set_next(bbox_he);
	}
	last_he= bbox_he;
	
	last_he->set_next(first_he);
}


void DCEL::create_faces_from_half_edges() {
	bool found_unattached_he= false;
	while (true) {
		found_unattached_he= false;
		for (auto he : _half_edges) {
			// le cas he->_next== NULL devrait correspondre forcément au cas de la face infinie
			if ((he->_incident_face== NULL) && (he->_next!= NULL)) {
				std::cout << "----\n";
				std::cout << *he << "\n";
				found_unattached_he= true;
				DCEL_Face * face= add_face();
				face->_outer_edge= he;
				DCEL_HalfEdge * he2= he;
				do {
					if (he2->_next== NULL) {
						std::cout << "DCEL::create_faces_from_half_edges : face non close\n";
						return;
					}
					he2->_incident_face= face;
					he2= he2->_next;
					std::cout << *he2 << "\n";
				} while (he2!= he);
				std::cout << "done\n";
			}
		}
		if (!found_unattached_he) {
			break;
		}
	}
}


void DCEL::export_html(std::string html_path) {
	compute_bbox();

	const unsigned int SVG_WIDTH= 700;
	const unsigned int SVG_HEIGHT= 700;
	const float MARGIN_FACTOR= 1.5f;
	const float VIEW_XMIN= (_xmin- 0.5f* (_xmin+ _xmax))* MARGIN_FACTOR+ 0.5f* (_xmin+ _xmax);
	const float VIEW_YMIN= (_ymin- 0.5f* (_ymin+ _ymax))* MARGIN_FACTOR+ 0.5f* (_ymin+ _ymax);
	const float VIEW_XMAX= (_xmax- 0.5f* (_xmin+ _xmax))* MARGIN_FACTOR+ 0.5f* (_xmin+ _xmax);
	const float VIEW_YMAX= (_ymax- 0.5f* (_ymin+ _ymax))* MARGIN_FACTOR+ 0.5f* (_ymin+ _ymax);
	const float SIZE= std::max(_xmax- _xmin, _ymax- _ymin);
	const float POINT_RADIUS= 0.005f* SIZE;
	const float DELTA_BUFFER= 0.05f* SIZE;
	const float ARROW_SIZE= 0.05f* SIZE;
	const float ARROW_SIZE_2= 0.01f* SIZE;
	const float STROKE_WIDTH= 0.002f* SIZE;

	auto y_html= [VIEW_YMIN, VIEW_YMAX](float y) -> float {return VIEW_YMIN+ VIEW_YMAX- y;};

	std::ofstream f;
	f.open(html_path);
	f << "<!DOCTYPE html>\n<html>\n<head>\n";
	f << "<style>\n";
	f << ".point_class {fill: black;}\n";
	f << ".repere_point_class {fill: red;}\n";
	f << ".line_class {fill: transparent; stroke: black; stroke-width: " << STROKE_WIDTH << "; stroke-opacity: 0.3;}\n";
	f << ".repere_line_class {fill: transparent; stroke: red; stroke-width: " << STROKE_WIDTH << "; stroke-opacity: 0.3;}\n";
	f << ".arrow_line_class {fill: transparent; stroke: rgb(100,100,100); stroke-width: " << STROKE_WIDTH << "; stroke-opacity: 0.3;}\n";
	f << "</style>\n</head>\n<body>\n";
	f << "<svg width=\"" << SVG_WIDTH << "\" height=\"" << SVG_HEIGHT << "\" ";
	// viewbox = xmin, ymin, width, height
	f << "viewbox=\"" << VIEW_XMIN << " " << VIEW_YMIN << " " << VIEW_XMAX- VIEW_XMIN << " " << VIEW_YMAX- VIEW_YMIN << "\" ";
	f << "style=\"background-color:rgb(220,240,230)\"";
	f << "\">\n";

	// repère
	f << "<circle class=\"repere_point_class\" cx=\"" << 0 << "\" cy=\"" << y_html(0) << "\" r=\"" << POINT_RADIUS << "\" />\n";
	f << "<circle class=\"repere_point_class\" cx=\"" << 1 << "\" cy=\"" << y_html(0) << "\" r=\"" << POINT_RADIUS << "\" />\n";
	f << "<circle class=\"repere_point_class\" cx=\"" << 0 << "\" cy=\"" << y_html(1) << "\" r=\"" << POINT_RADIUS << "\" />\n";
	f << "<line class=\"repere_line_class\" x1=\"" << 0 << "\" y1=\"" << y_html(0) << "\" x2=\"" << 1 << "\" y2=\"" << y_html(0) << "\" />\n";
	f << "<line class=\"repere_line_class\" x1=\"" << 0 << "\" y1=\"" << y_html(0) << "\" x2=\"" << 0 << "\" y2=\"" << y_html(1) << "\" />\n";

	for (auto face : _faces) {
		std::vector<DCEL_HalfEdge *> edges= face->get_edges();
		std::pair<float, float> gravity_center= face->get_gravity_center();
		float xg= gravity_center.first;
		float yg= gravity_center.second;
		//std::cout << "-----\n" << xg << " ; " << yg << "\n";
		
		for (auto edge : edges) {
			DCEL_Vertex * v1= edge->_origin;
			DCEL_Vertex * v2= edge->destination();
			
			float x1= v1->_x;
			float y1= v1->_y;
			float x2= v2->_x;
			float y2= v2->_y;

			// buffer inversé vers centre de gravité (moche mais bon)
			x1+= DELTA_BUFFER* (xg- x1)/ sqrt((xg- x1)* (xg- x1)+ (yg- y1)* (yg- y1));
			y1+= DELTA_BUFFER* (yg- y1)/ sqrt((xg- x1)* (xg- x1)+ (yg- y1)* (yg- y1));
			x2+= DELTA_BUFFER* (xg- x2)/ sqrt((xg- x2)* (xg- x2)+ (yg- y2)* (yg- y2));
			y2+= DELTA_BUFFER* (yg- y2)/ sqrt((xg- x2)* (xg- x2)+ (yg- y2)* (yg- y2));

			f << "<circle class=\"point_class\" cx=\"" << x1 << "\" cy=\"" << y_html(y1) << "\" r=\"" << POINT_RADIUS << "\" />\n";
			f << "<circle class=\"point_class\" cx=\"" << x2 << "\" cy=\"" << y_html(y2) << "\" r=\"" << POINT_RADIUS << "\" />\n";
			f << "<line class=\"line_class\" x1=\"" << x1 << "\" y1=\"" << y_html(y1) << "\" x2=\"" << x2 << "\" y2=\"" << y_html(y2) << "\" />\n";

			// fleche
			float u= (x2- x1)/ sqrt((x2- x1)* (x2- x1)+ (y2- y1)* (y2- y1));
			float v= (y2- y1)/ sqrt((x2- x1)* (x2- x1)+ (y2- y1)* (y2- y1));
			float x_arrow_1= 0.5f* (x1+ x2)+ 0.5f* ARROW_SIZE* u;
			float y_arrow_1= 0.5f* (y1+ y2)+ 0.5f* ARROW_SIZE* v;
			float x_arrow_2= x_arrow_1- ARROW_SIZE* u- ARROW_SIZE_2* v;
			float y_arrow_2= y_arrow_1- ARROW_SIZE* v+ ARROW_SIZE_2* u;
			float x_arrow_3= x_arrow_1- ARROW_SIZE* u+ ARROW_SIZE_2* v;
			float y_arrow_3= y_arrow_1- ARROW_SIZE* v- ARROW_SIZE_2* u;
			f << "<line class=\"arrow_line_class\" x1=\"" << x_arrow_1 << "\" y1=\"" << y_html(y_arrow_1) << "\" x2=\"" << x_arrow_2 << "\" y2=\"" << y_html(y_arrow_2) << "\" />\n";
			f << "<line class=\"arrow_line_class\" x1=\"" << x_arrow_2 << "\" y1=\"" << y_html(y_arrow_2) << "\" x2=\"" << x_arrow_3 << "\" y2=\"" << y_html(y_arrow_3) << "\" />\n";
			f << "<line class=\"arrow_line_class\" x1=\"" << x_arrow_3 << "\" y1=\"" << y_html(y_arrow_3) << "\" x2=\"" << x_arrow_1 << "\" y2=\"" << y_html(y_arrow_1) << "\" />\n";
		}
	}

	f << "</svg>\n";
	f << "</body>\n</html>\n";
	f.close();
}


std::ostream & operator << (std::ostream & os, DCEL & d) {
	/*for (auto face : d._faces) {
		os << *face << "\n";
	}*/
	for (auto he : d._half_edges) {
		os << *he << "\n";
	}

	return os;
}

