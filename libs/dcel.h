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
	bool _delete_mark;
};


struct DCEL_HalfEdge {
	DCEL_HalfEdge();
	//DCEL_HalfEdge(DCEL_Vertex * origin);
	~DCEL_HalfEdge();
	DCEL_Vertex * destination();
	DCEL_Face * opposite_face();
	void set_twin(DCEL_HalfEdge * hedge);
	void set_next(DCEL_HalfEdge * hedge);
	void set_previous(DCEL_HalfEdge * hedge);
	void set_tmp_data(glm::vec2 direction, glm::vec2 position);
	void set_tmp_data(glm::vec2 direction);
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
	bool _delete_mark;
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
	std::vector<DCEL_HalfEdge *> _inner_edges; // edges de trous, 1 par trou
	bool _unbounded; // est-ce la face infinie
	bool _delete_mark;
};


class DCEL {
public:
	DCEL();
	~DCEL();
	DCEL_Vertex * add_vertex(float x, float y);
	DCEL_HalfEdge * add_edge(DCEL_Vertex * v1, DCEL_Vertex * v2);
	DCEL_Face * add_face(DCEL_HalfEdge * outer_edge=NULL);
	void delete_vertex(DCEL_Vertex * v);
	void delete_edge(DCEL_HalfEdge * he);
	void delete_face(DCEL_Face * face);
	void clear();
	bool recreate_unbounded_face();
	bool create_faces_from_half_edges();
	void make_valid();
	bool is_empty();
	bool add_bbox(float xmin, float ymin, float xmax, float ymax);
	bool is_valid();
	void import(std::string s);
	DCEL_Vertex * get_vertex(float x, float y);
	DCEL_HalfEdge * get_edge(float x_ori, float y_ori, float x_dst, float y_dst);
	void bbox(float * xmin, float * ymin, float * xmax, float * ymax);
	void export_html(std::string html_path, bool simple, float xmin, float ymin, float xmax, float ymax, const std::vector<glm::vec2> & sites=std::vector<glm::vec2>());
	friend std::ostream & operator << (std::ostream & os, DCEL & d);
	
	
	std::vector<DCEL_Vertex *> _vertices;
	std::vector<DCEL_HalfEdge *> _half_edges;
	std::vector<DCEL_Face *> _faces;
	DCEL_Face * _unbounded_face;
};

#endif
