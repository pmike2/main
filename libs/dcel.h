#ifndef DCEL_H
#define DCEL_H

#include <string>
#include <vector>


struct DCEL_HalfEdge;
struct DCEL_Face;


struct DCEL_Vertex {
	std::vector<DCEL_HalfEdge *> get_incident_edges();

	float _x, _y;
	DCEL_HalfEdge * _incident_edge; // 1 des edges ayant ce vertex comme origine
};


struct DCEL_HalfEdge {
	DCEL_Vertex * destination();
	DCEL_Face * opposite_face();

	DCEL_Vertex * _origin;
	DCEL_HalfEdge * _twin;
	DCEL_HalfEdge * _next;
	DCEL_HalfEdge * _prev;
	DCEL_Face * _incident_face;
};


struct DCEL_Face {
	std::vector<DCEL_Vertex *> get_vertices();
	std::vector<DCEL_HalfEdge *> get_edges();
	std::vector<DCEL_Face *> get_adjacent_faces();

	DCEL_HalfEdge * _outer_edge; // 1 des edges délimitant la face; NULL pour la face infinie
	std::vector<DCEL_HalfEdge *> _inner_edges; // trous
};


class DCEL {
public:
	DCEL();
	~DCEL();
	void export_html(std::string html_path);
	
	
	std::vector<DCEL_Vertex *> _vertices;
	std::vector<DCEL_HalfEdge *> _half_edges;
	std::vector<DCEL_Face *> _faces;
};

#endif
