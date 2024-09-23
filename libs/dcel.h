#ifndef DCEL_H
#define DCEL_H

#include <iostream>
#include <string>
#include <vector>
#include <utility>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "geom_2d.h"


struct DCEL_HalfEdge;
struct DCEL_Face;


struct DCEL_Vertex {
	DCEL_Vertex();
	DCEL_Vertex(const glm::vec2 & coords);
	~DCEL_Vertex();
	std::vector<DCEL_HalfEdge *> get_incident_edges();
	friend std::ostream & operator << (std::ostream & os, DCEL_Vertex & v);

	glm::vec2 _coords;
	DCEL_HalfEdge * _incident_edge; // 1 des edges ayant ce vertex comme origine
	bool _delete_mark;
};


struct DCEL_HalfEdge {
	DCEL_HalfEdge();
	~DCEL_HalfEdge();
	DCEL_Vertex * destination();
	DCEL_Face * opposite_face();
	void set_twin(DCEL_HalfEdge * hedge);
	void set_next(DCEL_HalfEdge * hedge);
	void set_previous(DCEL_HalfEdge * hedge);
	void set_origin(DCEL_Vertex * v);
	void set_incident_face(DCEL_Face * f);
	void set_tmp_data(const glm::vec2 & direction, const glm::vec2 & position);
	void set_tmp_data(const glm::vec2 & direction);
	void set_tmp_data();
	friend std::ostream & operator << (std::ostream & os, DCEL_HalfEdge & e);

	DCEL_Vertex * _origin;
	DCEL_HalfEdge * _twin;
	DCEL_HalfEdge * _next;
	DCEL_HalfEdge * _previous;
	DCEL_Face * _incident_face;
	glm::vec2 _tmp_direction;
	glm::vec2 _tmp_position;
	bool _delete_mark;
};


struct DCEL_Face {
	DCEL_Face();
	~DCEL_Face();
	std::vector<DCEL_Vertex *> get_vertices();
	Polygon2D * get_polygon();
	std::vector<DCEL_HalfEdge *> get_outer_edges();
	std::vector<DCEL_HalfEdge *> get_inner_edges();
	std::vector<DCEL_Face *> get_adjacent_faces();
	glm::vec2 get_gravity_center();
	bool ccw();
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
	DCEL_Vertex * add_vertex(const glm::vec2 & coords);
	DCEL_HalfEdge * add_edge(DCEL_Vertex * v1, DCEL_Vertex * v2);
	DCEL_Face * add_face(DCEL_HalfEdge * outer_edge=NULL);
	void delete_vertex(DCEL_Vertex * v);
	void delete_edge(DCEL_HalfEdge * he);
	void delete_face(DCEL_Face * face);
	void delete_face_without_edges(DCEL_Face * face);
	void clear();
	//void clear_unbounded_face();
	void clear_next_equals_twin_edges();
	void clear_unconnected_vertices();
	void create_nexts_from_twins();
	void create_faces_from_half_edges();
	void check_ccw_faces();
	void delete_markeds();
	void make_valid();
	bool is_empty();
	void add_bbox(const glm::vec2 & bbox_min, const glm::vec2 & bbox_max);
	bool is_valid();
	void import(std::string s);
	DCEL_Vertex * get_vertex(const glm::vec2 & coords);
	DCEL_HalfEdge * get_edge(const glm::vec2 & ori, const glm::vec2 & dst);
	void bbox(glm::vec2 * bbox_min, glm::vec2 * bbox_max);
	void export_html(std::string html_path, bool simple, const glm::vec2 & bbox_min, const glm::vec2 & bbox_max, const std::vector<glm::vec2> & sites=std::vector<glm::vec2>());
	friend std::ostream & operator << (std::ostream & os, DCEL & d);
	
	
	std::vector<DCEL_Vertex *> _vertices;
	std::vector<DCEL_HalfEdge *> _half_edges;
	std::vector<DCEL_Face *> _faces;
	DCEL_Face * _unbounded_face;
};

#endif
