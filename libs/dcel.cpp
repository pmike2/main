#include <algorithm>
#include <fstream>

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

}


DCEL_HalfEdge::~DCEL_HalfEdge() {

}


DCEL_Vertex * DCEL_HalfEdge::destination() {
	return _twin->_origin;
}


DCEL_Face * DCEL_HalfEdge::opposite_face() {
	return _twin->_incident_face;
}


std::ostream & operator << (std::ostream & os, DCEL_HalfEdge & e) {
	os << *e._origin;
	os << " -> ";
	os << *e.destination();
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


void DCEL::export_html(std::string html_path) {
	unsigned int svg_width= 700;
	unsigned int svg_height= 700;

	float margin_factor= 1.5f;
	float view_xmin= (_xmin- 0.5f* (_xmin+ _xmax))* margin_factor+ 0.5f* (_xmin+ _xmax);
	float view_ymin= (_ymin- 0.5f* (_ymin+ _ymax))* margin_factor+ 0.5f* (_ymin+ _ymax);
	float view_xmax= (_xmax- 0.5f* (_xmin+ _xmax))* margin_factor+ 0.5f* (_xmin+ _xmax);
	float view_ymax= (_ymax- 0.5f* (_ymin+ _ymax))* margin_factor+ 0.5f* (_ymin+ _ymax);

	std::ofstream f;
	f.open(html_path);
	f << "<!DOCTYPE html>\n<html>\n<head>\n";
	f << "<style>\n";
	f << ".point_class {fill: black;}\n";
	f << ".line_class {fill: transparent; stroke: black; stroke-width: 0.01; stroke-opacity: 0.3;}\n";
	f << "</style>\n</head>\n<body>\n";
	f << "<svg width=\"" << svg_width << "\" height=\"" << svg_height << "\" ";
	// viewbox = xmin, ymin, width, height
	f << "viewbox=\"" << view_xmin << " " << view_ymin << " " << view_xmax- view_xmin << " " << view_ymax- view_ymin << "\" ";
	f << "style=\"background-color:rgb(200,240,220)\"";
	f << "\">\n";

	for (auto face : _faces) {
		std::vector<DCEL_HalfEdge *> edges= face->get_edges();
		for (auto edge : edges) {
			DCEL_Vertex * v1= edge->_origin;
			DCEL_Vertex * v2= edge->destination();
			
			float x1= v1->_x;
			float y1= view_ymin+ view_ymax- v1->_y;
			float x2= v2->_x;
			float y2= view_ymin+ view_ymax- v2->_y;
			float radius= 0.01f;
			f << "<circle class=\"point_class\" cx=\"" << x1 << "\" cy=\"" << y1 << "\" r=\"" << radius << "\" />\n";
			f << "<circle class=\"point_class\" cx=\"" << x2 << "\" cy=\"" << y2 << "\" r=\"" << radius << "\" />\n";
			f << "<line class=\"line_class\" x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" />\n";
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

