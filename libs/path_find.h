#ifndef PATH_FIND
#define PATH_FIND

#include <utility>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <OpenGL/gl3.h>

#include <glm/glm.hpp>

#include "geom_2d.h"
#include "bbox_2d.h"
#include "graph.h"



struct GraphGrid : public Graph {
	GraphGrid();
	GraphGrid(unsigned int n_ligs, unsigned int n_cols, const glm::vec2 & origin, const glm::vec2 & size, bool is8connex=true);
	~GraphGrid();
	std::pair<unsigned int, unsigned int> id2col_lig(unsigned int id);
	unsigned int col_lig2id(unsigned int col, unsigned int lig);
	void set_heavy_weight(AABB_2D * aabb);
	friend std::ostream & operator << (std::ostream & os, GraphGrid & g);


	unsigned int _n_ligs;
	unsigned int _n_cols;
	glm::vec2 _origin;
	glm::vec2 _size;
	AABB_2D * _aabb;
};


bool frontier_cmp(std::pair<unsigned int, float> x, std::pair<unsigned int, float> y);


struct PathFinder {
	PathFinder();
	PathFinder(unsigned int n_ligs, unsigned int n_cols, const glm::vec2 & origin, const glm::vec2 & size, bool is8connex=true);
	~PathFinder();
	void update_grid();
	void read_shapefile(std::string shp_path, glm::vec2 origin, glm::vec2 size, bool reverse_y=false);
	void rand(unsigned int n_polys, unsigned int n_pts_per_poly, float poly_radius);
	float cost(unsigned int i, unsigned int j);
	float heuristic(unsigned int i, unsigned int j);
	bool line_of_sight(unsigned int i, unsigned int j);
	bool path_find_nodes(unsigned int start, unsigned int goal, std::vector<unsigned int> & path, std::vector<unsigned int> & visited);
	bool path_find(glm::vec2 start, glm::vec2 goal, std::vector<glm::vec2> & path, std::vector<unsigned int> & visited);
	void draw_svg(const std::vector<unsigned int> & path, const std::vector<unsigned int> & visited, std::string svg_path);


	GraphGrid * _grid;
	std::vector<Polygon2D *> _polygons;
};


struct PathFinderDebug {
	PathFinderDebug();
	PathFinderDebug(GLuint prog_draw);
	~PathFinderDebug();
	void draw();
	void anim(const glm::mat4 & world2clip);
	void update(const PathFinder & path_finder, const std::vector<glm::vec2> & path, const std::vector<unsigned int> & visited);


	GLuint _prog_draw;
	GLint _world2clip_loc, _position_loc, _color_loc;
	GLuint _buffers[3];
	glm::mat4 _world2clip;
	unsigned int _n_pts_grid, _n_pts_obstacle, _n_pts_path;
};



#endif
