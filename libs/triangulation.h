#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "bbox_2d.h"



struct Triangle {
	Triangle();
	Triangle(int v0, int v1, int v2, Triangle * a0, Triangle * a1, Triangle * a2);
	~Triangle();


	int _vertices[3];
	Triangle * _adjacents[3];
};


struct PointBin {
	PointBin();
	PointBin(glm::vec2 pt, int idx_bin);
	~PointBin();


	glm::vec2 _pt;
	int _idx_bin;
};


bool sort_pts(PointBin * pt1, PointBin * pt2);


struct Triangulation {
	Triangulation();
	Triangulation(std::vector<glm::vec2> & pts, bool verbose=false);
	~Triangulation();
	int idx_triangle(Triangle * triangle);
	void print_triangle(Triangle * triangle, bool verbose=false, bool is_pt_init=true);
	void draw(std::string svg_path);
	glm::vec2 svg_coords(glm::vec2 & v);


	std::vector<glm::vec2> _pts_init;
	std::vector<PointBin *> _pts;
	std::vector<Triangle *> _triangles;
	AABB_2D * _aabb;
	std::vector<AABB_2D *> _bins;
	float _svg_margin= 0.1f;
};


struct Opposition {
	Opposition();
	Opposition(Triangle * new_triangle, Triangle * opposite_triangle);
	~Opposition();


	Triangle * _new_triangle;
	Triangle * _opposite_triangle;
	int _new_edge_idx;
	int _opposite_edge_idx;
};


#endif
