#ifndef DCEL_H
#define DCEL_H

#include <iostream>
#include <string>
#include <vector>
#include <utility>


struct DCEL_HalfEdge;
struct DCEL_Face;


struct DCEL_Vertex {
	DCEL_Vertex();
	DCEL_Vertex(float x, float y);
	~DCEL_Vertex();
	std::vector<DCEL_HalfEdge *> get_incident_edges();
	friend std::ostream & operator << (std::ostream & os, DCEL_Vertex & v);

	float _x, _y;
	DCEL_HalfEdge * _incident_edge; // 1 des edges ayant ce vertex comme origine
};


struct DCEL_HalfEdge {
	DCEL_HalfEdge();
	DCEL_HalfEdge(DCEL_Vertex * origin);
	~DCEL_HalfEdge();
	DCEL_Vertex * destination();
	DCEL_Face * opposite_face();
	void set_twin(DCEL_HalfEdge * hedge);
	void set_next(DCEL_HalfEdge * hedge);
	void set_tmp_data(glm::vec2 direction, glm::vec2 position=glm::vec2(0.0f));
	friend std::ostream & operator << (std::ostream & os, DCEL_HalfEdge & e);

	DCEL_Vertex * _origin;
	DCEL_HalfEdge * _twin;
	DCEL_HalfEdge * _next;
	DCEL_HalfEdge * _previous;
	DCEL_Face * _incident_face;
	float _dx;
	float _dy;
	float _tmp_x;
	float _tmp_y;
};


struct DCEL_Face {
	DCEL_Face();
	~DCEL_Face();
	std::vector<DCEL_Vertex *> get_vertices();
	std::vector<DCEL_HalfEdge *> get_edges();
	std::vector<DCEL_Face *> get_adjacent_faces();
	std::pair<float , float> get_gravity_center();
	friend std::ostream & operator << (std::ostream & os, DCEL_Face & f);

	DCEL_HalfEdge * _outer_edge; // 1 des edges délimitant la face; NULL pour la face infinie
	std::vector<DCEL_HalfEdge *> _inner_edges; // trous, pas utilisé pour l'instant
};


class DCEL {
public:
	DCEL();
	~DCEL();
	DCEL_Vertex * add_vertex(float x, float y);
	DCEL_HalfEdge * add_edge(DCEL_Vertex * v1, DCEL_Vertex * v2);
	DCEL_Face * add_face();
	bool is_empty();
	void compute_bbox();
	bool add_bbox(float bbox_expand=0.5f);
	bool create_faces_from_half_edges();
	bool is_valid();
	void export_html(std::string html_path, bool simple, std::vector<glm::vec2> sites=std::vector<glm::vec2>());
	void export_html(std::string html_path, bool simple, float xmin, float ymin, float xmax, float ymax, std::vector<glm::vec2> sites);
	friend std::ostream & operator << (std::ostream & os, DCEL & d);
	
	
	std::vector<DCEL_Vertex *> _vertices;
	std::vector<DCEL_HalfEdge *> _half_edges;
	std::vector<DCEL_Face *> _faces;
	float _xmin, _xmax, _ymin, _ymax;
};

#endif
