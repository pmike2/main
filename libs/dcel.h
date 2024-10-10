#ifndef DCEL_H
#define DCEL_H

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <deque>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "typedefs.h"
#include "geom_2d.h"


typedef enum {VERTEX, HALF_EDGE, FACE} DCEL_Type;


struct DeleteEvent {
	DCEL_Type _type;
	void * _ptr;
	bool operator==(const DeleteEvent& rhs) { return rhs._type == _type && rhs._ptr== _ptr;}
};


struct DCEL_HalfEdge;
struct DCEL_Face;


struct DCEL_Vertex {
	DCEL_Vertex();
	DCEL_Vertex(const pt_type & coords);
	~DCEL_Vertex();
	std::vector<DCEL_HalfEdge *> get_incident_edges();
	friend std::ostream & operator << (std::ostream & os, DCEL_Vertex & v);

	pt_type _coords;
	DCEL_HalfEdge * _incident_edge; // 1 des edges ayant ce vertex comme origine
	void * _data; // on pourra utiliser _data pour ajouter des infos par sommet, edge ou face
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
	friend std::ostream & operator << (std::ostream & os, DCEL_HalfEdge & e);

	DCEL_Vertex * _origin;
	DCEL_HalfEdge * _twin;
	DCEL_HalfEdge * _next;
	DCEL_HalfEdge * _previous;
	DCEL_Face * _incident_face;
	void * _data;
};


struct DCEL_Face {
	DCEL_Face();
	~DCEL_Face();
	std::vector<DCEL_Vertex *> get_vertices();
	Polygon2D * get_polygon();
	std::vector<DCEL_HalfEdge *> get_outer_edges();
	std::vector<std::vector<DCEL_HalfEdge *> > get_inner_edges();
	std::vector<DCEL_Face *> get_adjacent_faces();
	pt_type get_gravity_center();
	bool ccw();
	friend std::ostream & operator << (std::ostream & os, DCEL_Face & f);

	DCEL_HalfEdge * _outer_edge; // 1 des edges délimitant la face; NULL pour la face infinie
	std::vector<DCEL_HalfEdge *> _inner_edges; // edges de trous, 1 par trou
	void * _data;
};


class DCEL {
public:
	DCEL();
	~DCEL();
	DCEL_Vertex * add_vertex(const pt_type & coords);
	DCEL_HalfEdge * add_edge(DCEL_Vertex * v1, DCEL_Vertex * v2);
	DCEL_HalfEdge * add_edge(const pt_type & ori, const pt_type & dst);
	DCEL_Face * add_face();
	/*void delete_vertex(DCEL_Vertex * v);
	void delete_edge(DCEL_HalfEdge * he);
	void delete_face(DCEL_Face * face);*/
	DCEL_HalfEdge * split_edge(DCEL_HalfEdge * he, const pt_type & coords);
	DCEL_HalfEdge * cut_face(DCEL_HalfEdge * he1, DCEL_HalfEdge * he2);
	void clear();
	void create_nexts_from_twins();
	void create_faces_from_half_edges();
	void check_ccw_faces();
	void check_integrity();
	void delete_loop_edge();
	void add2queue(DeleteEvent evt);
	void delete_queue();
	bool is_empty();
	void add_bbox(const pt_type & bbox_min, const pt_type & bbox_max);
	bool is_valid();
	void import(std::string s);
	DCEL_Vertex * get_vertex(const pt_type & coords);
	DCEL_HalfEdge * get_edge(const pt_type & ori, const pt_type & dst);
	void get_bbox(pt_type * bbox_min, pt_type * bbox_max);
	number smallest_edge();
	void export_html(std::string html_path, bool simple, const pt_type & bbox_min, const pt_type & bbox_max, const std::vector<pt_type> & sites=std::vector<pt_type>());
	friend std::ostream & operator << (std::ostream & os, DCEL & d);
	
	
	std::vector<DCEL_Vertex *> _vertices;
	std::vector<DCEL_HalfEdge *> _half_edges;
	std::vector<DCEL_Face *> _faces;
	std::deque<DeleteEvent> _delete_queue;
};

#endif
