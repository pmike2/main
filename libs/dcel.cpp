#include <algorithm>
#include <fstream>
#include <cmath>

#include "dcel.h"


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
DCEL_HalfEdge::DCEL_HalfEdge() : _origin(NULL), _twin(NULL), _next(NULL), _previous(NULL), _incident_face(NULL) {

}


DCEL_HalfEdge::DCEL_HalfEdge(DCEL_Vertex * origin) : _origin(origin), _twin(NULL), _next(NULL), _previous(NULL), _incident_face(NULL) {
	//std::cout << "DCEL_HalfEdge constr : " << _origin << "\n";
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
	
	return os;
}


// ----------------------------------------------------------------------
DCEL_Face::DCEL_Face() : _outer_edge(NULL) {

}


DCEL_Face::DCEL_Face(DCEL_HalfEdge * outer_edge) : _outer_edge(outer_edge) {
	
}


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
	if (x< _xmin) {
		_xmin= x;
	}
	if (x> _xmax) {
		_xmax= x;
	}
	if (y< _ymin) {
		_ymin= y;
	}
	if (y> _ymax) {
		_ymax= y;
	}
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


DCEL_Face * DCEL::add_face(std::vector<DCEL_HalfEdge *> edges) {
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
}


void DCEL::create_faces_from_half_edges() {
	bool found_unattached_he= false;
	while (true) {
		found_unattached_he= false;
		for (auto he : _half_edges) {
			if (he->_incident_face== NULL) {
				found_unattached_he= true;
				DCEL_Face * face= add_face();
				face->_outer_edge= he;
				DCEL_HalfEdge * he2= he;
				while (he2!= he) {
					he2->_incident_face= face;
					he2= he2->_next;
				}
			}
		}
		if (!found_unattached_he) {
			break;
		}
	}
}


void DCEL::export_html(std::string html_path) {
	const unsigned int SVG_WIDTH= 700;
	const unsigned int SVG_HEIGHT= 700;
	const float MARGIN_FACTOR= 1.5f;
	const float VIEW_XMIN= (_xmin- 0.5f* (_xmin+ _xmax))* MARGIN_FACTOR+ 0.5f* (_xmin+ _xmax);
	const float VIEW_YMIN= (_ymin- 0.5f* (_ymin+ _ymax))* MARGIN_FACTOR+ 0.5f* (_ymin+ _ymax);
	const float VIEW_XMAX= (_xmax- 0.5f* (_xmin+ _xmax))* MARGIN_FACTOR+ 0.5f* (_xmin+ _xmax);
	const float VIEW_YMAX= (_ymax- 0.5f* (_ymin+ _ymax))* MARGIN_FACTOR+ 0.5f* (_ymin+ _ymax);
	const float POINT_RADIUS= 0.01f;
	const float DELTA_BUFFER= 0.05f;
	const float ARROW_SIZE= 0.1f;
	const float ARROW_SIZE_2= 0.03f;

	auto y_html= [VIEW_YMIN, VIEW_YMAX](float y) -> float {return VIEW_YMIN+ VIEW_YMAX- y;};

	std::ofstream f;
	f.open(html_path);
	f << "<!DOCTYPE html>\n<html>\n<head>\n";
	f << "<style>\n";
	f << ".point_class {fill: black;}\n";
	f << ".repere_point_class {fill: red;}\n";
	f << ".line_class {fill: transparent; stroke: black; stroke-width: 0.01; stroke-opacity: 0.3;}\n";
	f << ".repere_line_class {fill: transparent; stroke: red; stroke-width: 0.01; stroke-opacity: 0.3;}\n";
	f << ".arrow_line_class {fill: transparent; stroke: rgb(100,100,100); stroke-width: 0.01; stroke-opacity: 0.3;}\n";
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
		std::cout << "-----\n" << xg << " ; " << yg << "\n";
		
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
	for (auto face : d._faces) {
		os << *face << "\n";
	}
	return os;
}

