#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "typedefs.h"
#include "bbox_2d.h"
#include "geom_2d.h"
#include "utile.h"


const number DEFAULT_VERTEX_WEIGHT = 0.0;
const number DEFAULT_EDGE_WEIGHT = 0.0;


struct GraphEdge {
	number _weight;
	void * _data;
};


struct GraphVertex {
	pt_3d _pos;
	number _weight;
	std::unordered_map<uint, GraphEdge> _edges;
};


struct Graph {
	Graph();
	~Graph();
	void add_vertex(uint i, pt_3d pos=pt_3d(0.0), number weight=DEFAULT_VERTEX_WEIGHT);
	void add_edge(uint i, uint j, number weight=DEFAULT_EDGE_WEIGHT, bool weight_is_dist=false);
	void remove_vertex(uint i);
	void remove_edge(uint i, uint j);
	std::vector<uint> neighbors(uint i);
	void clear();
	void reinit_weights();
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
	std::pair<uint, uint> id2col_lig(uint id);
	uint col_lig2id(uint col, uint lig);
	pt_2d col_lig2pt(uint col, uint lig);
	std::pair<uint, uint> pt2col_lig(pt_2d pt);
	uint pt2id(pt_2d pt);
	std::vector<std::pair<uint, uint> > segment_intersection(pt_2d pt1, pt_2d pt2);
	std::vector<std::pair<uint, uint> > aabb_intersection(AABB_2D * aabb);
	std::vector<std::pair<uint, uint> > bbox_intersection(BBox_2D * bbox);
	std::vector<std::pair<uint, uint> > polygon_intersection(Polygon2D * polygon);
	//std::vector<number> weights_in_cell_containing_pt(pt_2d pt);
	std::vector<GraphEdge> edges_in_cell_containing_pt(pt_2d pt);
	friend std::ostream & operator << (std::ostream & os, GraphGrid & g);


	uint _n_ligs;
	uint _n_cols;
	pt_2d _origin;
	pt_2d _size;
	AABB_2D * _aabb;
};

/*
struct Mesh {
	std::vector<std::pair<uint, uint> > _edges;
	bool _debug= true;
};
*/

#endif
