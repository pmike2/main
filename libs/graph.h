#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <queue>

#include <glm/glm.hpp>

#include "typedefs.h"
#include "bbox_2d.h"
#include "geom_2d.h"
#include "utile.h"


//const number DEFAULT_VERTEX_WEIGHT = 0.0;
const number DEFAULT_EDGE_WEIGHT = 0.0;


struct GraphVertex;


struct GraphEdge {
	//number _weight;
	void * _data = NULL;
	GraphVertex * _start; //inutilisé pour l'instant mais peut être utile
	GraphVertex * _end; //inutilisé pour l'instant mais peut être utile
};


struct GraphVertex {
	pt_3d _pos;
	//number _weight;
	std::unordered_map<uint, GraphEdge> _edges;
	void * _data = NULL;
};


// TODO : mettre en private et utiliser les getters dans les codes appelants
struct Graph {
	Graph();
	~Graph();
	GraphVertex get_vertex(uint i);
	GraphEdge get_edge(uint i, uint j);
	GraphEdge get_edge(uint_pair p);
	void add_vertex(uint i, pt_3d pos=pt_3d(0.0));
	void add_edge(uint i, uint j);
	void remove_vertex(uint i);
	void remove_edge(uint i, uint j);
	std::vector<uint> neighbors(uint i);
	void clear();
	//void reinit_weights();
	friend std::ostream & operator << (std::ostream & os, Graph & g);


	std::unordered_map<uint, GraphVertex> _vertices;
	std::unordered_map<uint, GraphVertex>::iterator _it_v;
	std::unordered_map<uint, GraphEdge>::iterator _it_e;
};


struct GraphGrid : public Graph {
	GraphGrid();
	GraphGrid(const pt_2d & origin, const pt_2d & size, uint n_ligs, uint n_cols, bool is8connex=true);
	GraphGrid(const GraphGrid & grid);
	~GraphGrid();

	std::map<std::string, uint> named_neighbors(uint id);
	
	bool in_boundaries(uint id);
	bool in_boundaries(int col, int lig);
	bool in_boundaries(pt_2d pt);
	bool in_boundaries(AABB_2D * aabb);

	bool id_in_ids(uint id, const std::vector<uint> & ids);

	int_pair id2col_lig(uint id);
	uint col_lig2id(int col, int lig);
	pt_2d col_lig2pt_2d(int col, int lig, bool use_vertices=true);
	pt_3d col_lig2pt_3d(int col, int lig);
	pt_2d id2pt_2d(uint id);
	pt_3d id2pt_3d(uint id);
	int_pair pt2col_lig(pt_2d pt);
	uint pt2id(pt_2d pt);
	uint pt2closest_id(pt_2d pt);
	uint_pair pt2closest_edge(pt_2d pt);
	std::pair<int_pair, int_pair> aabb2col_lig_min_max(AABB_2D * aabb);
	
	std::vector<uint_pair> edges_intersecting_segment(pt_2d pt1, pt_2d pt2);
	bool is_edge_intersecting_segment(pt_2d pt1, pt_2d pt2, uint_pair edge);
	std::vector<uint_pair> edges_intersecting_aabb(AABB_2D * aabb);
	std::vector<uint_pair> edges_intersecting_bbox(BBox_2D * bbox);
	std::vector<uint_pair> edges_intersecting_polygon(Polygon2D * polygon);
	std::vector<uint_pair> edges_in_cell_containing_pt(pt_2d pt, bool only_diagonals=false);
	
	std::vector<uint> vertices_in_aabb(AABB_2D * aabb);
	std::vector<uint> vertices_in_cell_containing_pt(pt_2d pt);
	
	int_pair next_direction(int_pair u);
	uint angle(int_pair u, int_pair v);
	std::vector<uint> prune(std::vector<uint> ids);
	Polygon2D * ids2polygon(std::vector<uint> ids);
	Polygon2D * pts2polygon(std::vector<pt_2d> pts);

	std::vector<std::tuple<uint, uint, uint> > ids2triangles(std::vector<uint> ids);
	std::vector<uint> triangles2ids(const std::vector<std::tuple<uint, uint, uint> > & triangles);

	std::vector<uint> neighbors_dist(uint id_root, uint distance);
	std::vector<uint> buffered_ids(const std::vector<uint> & ids, uint distance);

	std::string ids2wkt(std::vector<uint> ids);
	std::string triangles2wkt(std::vector<std::tuple<uint, uint, uint> > triangles);
	
	friend std::ostream & operator << (std::ostream & os, GraphGrid & g);


	uint _n_ligs;
	uint _n_cols;
	pt_2d _origin;
	pt_2d _size;
	pt_2d _resolution;
	AABB_2D * _aabb;
};

/*
struct Mesh {
	std::vector<int_pair> _edges;
	bool _debug= true;
};
*/

#endif
