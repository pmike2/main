#include <algorithm>
#include <iostream>
#include <fstream>

#include "dcel.h"


// ----------------------------------------------------------------------
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


// ----------------------------------------------------------------------
DCEL_Vertex * DCEL_HalfEdge::destination() {
	return _twin->_origin;
}


DCEL_Face * DCEL_HalfEdge::opposite_face() {
	return _twin->_incident_face;
}


// ----------------------------------------------------------------------
std::vector<DCEL_Vertex *> DCEL_Face::get_vertices() {
	std::vector<DCEL_Vertex *> result;
	DCEL_HalfEdge * first_edge= _outer_edge;
	DCEL_HalfEdge * current_edge= _outer_edge;
	do {
		result.push_back(current_edge->_origin);
		current_edge= current_edge->_next;
	} while(current_edge!= first_edge);

	return result;
}


std::vector<DCEL_HalfEdge *> DCEL_Face::get_edges() {
	std::vector<DCEL_HalfEdge *> result;
	DCEL_HalfEdge * first_edge= _outer_edge;
	DCEL_HalfEdge * current_edge= _outer_edge;
	do {
		result.push_back(current_edge);
		current_edge= current_edge->_next;
	} while(current_edge!= first_edge);

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
	} while(current_edge!= first_edge);

	return result;
}


// ----------------------------------------------------------------------
DCEL::DCEL() {

}


DCEL::~DCEL() {

}


void DCEL::export_html(std::string html_path) {
	unsigned int svg_width= 700;
	unsigned int svg_height= 700;

	std::ofstream f;
	f.open(html_path);
	f << "<!DOCTYPE html>\n<html>\n<head>\n";
	f << "<style>\n";
	f << ".point_class {fill: black;}\n";
	f << ".line_class {fill: transparent; stroke: black; stroke-width: 0.01; stroke-opacity: 0.3;}\n";
	f << "</style>\n</head>\n<body>\n";
	f << "<svg width=\"" << svg_width << "\" height=\"" << svg_height << "\" viewbox=\"" << -1.2 << " " << -0.2 << " " << 2.4 << " " << 2.4 << "\">\n";

	//f << "<text class=\"point_text_class\" x=\"" << pt_x+ 0.02f << "\" y=\"" << pt_y << "\" >" << f_str(node->_data) << "</text>\n";
	for (auto face : _faces) {
		std::vector<DCEL_HalfEdge *> edges= face->get_edges();
		for (auto edge : edges) {
			DCEL_Vertex * v1= edge->_origin;
			DCEL_Vertex * v2= edge->destination();
			f << "<circle class=\"point_class\" cx=\"" << v1->_x << "\" cy=\"" << v1->_y << "\" r=\"" << 0.01f << "\" />\n";
			f << "<circle class=\"point_class\" cx=\"" << v2->_x << "\" cy=\"" << v2->_y << "\" r=\"" << 0.01f << "\" />\n";
			f << "<line class=\"line_class\" x1=\"" << v1->_x << "\" y1=\"" << v1->_y << "\" x2=\"" << v2->_x << "\" y2=\"" << v2->_y << "\" />\n";
		}
	}

	f << "</svg>\n";
	f << "</body>\n</html>\n";
	f.close();

}

