#ifndef PATH_FIND
#define PATH_FIND

#include <utility>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "geom_2d.h"


struct GraphEdge {
	//unsigned int _start_node;
	//unsigned int _end_node;
	float _weight;
};


struct GraphVertex {
	float _weight;
	glm::vec2 _pos;
	bool _visited;
	std::unordered_map<unsigned int, GraphEdge> _edges;
};


struct Graph {
	std::unordered_map<unsigned int, GraphVertex> _vertices;
	std::unordered_map<unsigned int, GraphVertex>::iterator _it_v;
	std::unordered_map<unsigned int, GraphEdge>::iterator _it_e;


	Graph();
	~Graph();
	void add_vertex(unsigned int i, float weight=0.0f, float x=0.0f, float y=0.0f);
	void add_edge(unsigned int i, unsigned int j, float weight=0.0f, bool weight_is_dist=false);
	void remove_vertex(unsigned int i);
	void remove_edge(unsigned int i, unsigned int j);
	std::vector<unsigned int> neighbors(unsigned int i);
	void clear();
	void rand();
	friend std::ostream & operator << (std::ostream & os, Graph & g);
};


struct GraphGrid : public Graph {
	unsigned int _n_ligs;
	unsigned int _n_cols;
	glm::vec2 _origin;
	glm::vec2 _size;


	GraphGrid();
	GraphGrid(unsigned int n_ligs, unsigned int n_cols, glm::vec2 & origin, glm::vec2 & size, bool is8connex=true);
	~GraphGrid();
	friend std::ostream & operator << (std::ostream & os, GraphGrid & g);
};


bool frontier_cmp(std::pair<unsigned int, float> x, std::pair<unsigned int, float> y);


struct PathFinder {
	GraphGrid * _grid;
	std::vector<Polygon2D *> _polygons;


	PathFinder();
	PathFinder(unsigned int n_ligs, unsigned int n_cols, glm::vec2 & origin, glm::vec2 & size, bool is8connex=true);
	~PathFinder();
	void update_grid();
	void read_shapefile(std::string shp_path, glm::vec2 origin, glm::vec2 size, bool reverse_y=false);
	void rand(unsigned int n_polys, unsigned int n_pts_per_poly, float poly_radius);
	float cost(unsigned int i, unsigned int j);
	float heuristic(unsigned int i, unsigned int j);
	bool line_of_sight(unsigned int i, unsigned int j);
	std::vector<unsigned int> path_find(unsigned int start, unsigned int goal);
	void draw_svg(std::vector<unsigned int> path, std::string result);
};


#endif