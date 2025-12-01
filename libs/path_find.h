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
	GraphGrid(unsigned int n_ligs, unsigned int n_cols, const pt_type & origin, const pt_type & size, bool is8connex=true);
	~GraphGrid();
	std::pair<unsigned int, unsigned int> id2col_lig(unsigned int id);
	unsigned int col_lig2id(unsigned int col, unsigned int lig);
	//void set_heavy_weight(AABB_2D * aabb);
	friend std::ostream & operator << (std::ostream & os, GraphGrid & g);


	unsigned int _n_ligs;
	unsigned int _n_cols;
	pt_type _origin;
	pt_type _size;
	AABB_2D * _aabb;
};


bool frontier_cmp(std::pair<unsigned int, number> x, std::pair<unsigned int, number> y);


struct PathFinder {
	PathFinder();
	PathFinder(unsigned int n_ligs, unsigned int n_cols, const pt_type & origin, const pt_type & size, bool is8connex=true);
	~PathFinder();
	void update_grid();
	void read_shapefile(std::string shp_path, pt_type origin, pt_type size, bool reverse_y=false);
	void rand(unsigned int n_polys, unsigned int n_pts_per_poly, number poly_radius);
	number cost(unsigned int i, unsigned int j);
	number heuristic(unsigned int i, unsigned int j);
	bool line_of_sight(unsigned int i, unsigned int j);
	bool path_find_nodes(unsigned int start, unsigned int goal, std::vector<unsigned int> & path);
	bool path_find(pt_type start, pt_type goal, std::vector<pt_type> & path);
	void draw_svg(const std::vector<unsigned int> & path, std::string svg_path);


	GraphGrid * _grid;
	std::vector<Polygon2D *> _polygons;
};


#endif
