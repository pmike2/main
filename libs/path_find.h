#ifndef PATH_FIND
#define PATH_FIND

#include <utility>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <OpenGL/gl3.h>

#include <glm/glm.hpp>

#include "typedefs.h"
#include "geom_2d.h"
#include "bbox_2d.h"
#include "graph.h"



struct GraphGrid : public Graph {
	GraphGrid();
	GraphGrid(uint n_ligs, uint n_cols, const pt_type & origin, const pt_type & size, bool is8connex=true);
	~GraphGrid();
	std::pair<uint, uint> id2col_lig(uint id);
	uint col_lig2id(uint col, uint lig);
	uint pt2id(pt_type pt);
	//void set_heavy_weight(AABB_2D * aabb);
	friend std::ostream & operator << (std::ostream & os, GraphGrid & g);


	uint _n_ligs;
	uint _n_cols;
	pt_type _origin;
	pt_type _size;
	AABB_2D * _aabb;
};


bool frontier_cmp(std::pair<uint, number> x, std::pair<uint, number> y);


struct PathFinder {
	PathFinder();
	PathFinder(uint n_ligs, uint n_cols, const pt_type & origin, const pt_type & size, bool is8connex=true);
	~PathFinder();
	void update_grid();
	void clear();
	void read_shapefile(std::string shp_path, pt_type origin, pt_type size, bool reverse_y=false);
	void rand(uint n_polys, uint n_pts_per_poly, number poly_radius);
	number cost(uint i, uint j);
	number heuristic(uint i, uint j);
	bool line_of_sight(uint i, uint j);
	bool line_of_sight(pt_type pt1, pt_type pt2);
	bool path_find_nodes(uint start, uint goal, std::vector<uint> & path);
	bool path_find(pt_type start, pt_type goal, std::vector<pt_type> & path);
	void draw_svg(const std::vector<uint> & path, std::string svg_path);


	GraphGrid * _grid;
	std::vector<Polygon2D *> _polygons;
};


#endif
