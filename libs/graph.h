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


struct GraphEdge {
	//number _weight;
	void * _data = NULL;
};


struct GraphVertex {
	pt_3d _pos;
	//number _weight;
	std::unordered_map<uint, GraphEdge> _edges;
};


struct Graph {
	Graph();
	~Graph();
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
	
	/*uint right(uint id);
	uint left(uint id);
	uint top(uint id);
	uint bottom(uint id);
	uint top_right(uint id);
	uint bottom_right(uint id);
	uint top_left(uint id);
	uint bottom_left(uint id);*/

	bool in_boundaries(uint id);
	bool in_boundaries(int col, int lig);
	bool in_boundaries(pt_2d pt);

	bool id_in_ids(uint id, const std::vector<uint> & ids);

	std::pair<uint, uint> id2col_lig(uint id);
	uint col_lig2id(uint col, uint lig);
	pt_2d col_lig2pt_2d(uint col, uint lig, bool use_vertices=true);
	pt_3d col_lig2pt_3d(uint col, uint lig);
	pt_2d id2pt_2d(uint id);
	pt_3d id2pt_3d(uint id);
	std::pair<uint, uint> pt2col_lig(pt_2d pt);
	uint pt2id(pt_2d pt);
	uint pt2closest_id(pt_2d pt);
	std::vector<std::pair<uint, uint> > segment_intersection(pt_2d pt1, pt_2d pt2);
	bool segment_intersects_edge(pt_2d pt1, pt_2d pt2, std::pair<uint, uint> edge);
	std::vector<std::pair<uint, uint> > aabb_intersection(AABB_2D * aabb);
	std::vector<std::pair<uint, uint> > bbox_intersection(BBox_2D * bbox);
	std::vector<std::pair<uint, uint> > polygon_intersection(Polygon2D * polygon);
	std::vector<std::pair<uint, uint> > edges_in_cell_containing_pt(pt_2d pt, bool only_diagonals=false);
	std::vector<uint> vertices_in_aabb(AABB_2D * aabb);
	std::vector<uint> vertices_in_cell_containing_pt(pt_2d pt);
	
	std::pair<int, int> next_direction(std::pair<int, int> u);
	uint angle(std::pair<int, int> u, std::pair<int, int> v);
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
	std::vector<std::pair<uint, uint> > _edges;
	bool _debug= true;
};
*/

#endif
