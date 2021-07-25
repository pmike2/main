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
	bool is_valid();


	int _vertices[3];
	int _vertices_init[3];
	Triangle * _adjacents[3];
};


struct PointBin {
	PointBin();
	PointBin(glm::vec2 pt_init, glm::vec2 pt, int idx_init, int idx_bin);
	~PointBin();


	glm::vec2 _pt_init;
	glm::vec2 _pt;
	int _idx_init;
	int _idx_bin;
	std::vector<unsigned int> _idx_triangles;
};


bool sort_pts_by_idx_bin(PointBin * pt1, PointBin * pt2);
bool sort_pts_by_idx_init(PointBin * pt1, PointBin * pt2);


struct Opposition {
	Opposition();
	Opposition(Triangle * triangle_1, Triangle * triangle_2);
	~Opposition();


	Triangle * _triangle_1;
	Triangle * _triangle_2;
	int _edge_idx_1;
	int _edge_idx_2;
	bool _is_valid;
};


struct Triangulation {
	Triangulation();
	Triangulation(const std::vector<glm::vec2> & pts, const std::vector<std::pair<unsigned int, unsigned int> > & constrained_edges=std::vector<std::pair<unsigned int, unsigned int> >(), bool sort_by_bin=true, bool verbose=false);
	~Triangulation();
	// méthodes utiles
	void init_pts(const std::vector<glm::vec2> & pts);
	void add_large_triangle();
	Triangle * get_containing_triangle(unsigned int idx_pt);
	void delete_triangle(Triangle * triangle);
	void swap_triangle(Opposition * opposition, Triangle * new_triangle_1, Triangle * new_triangle_2);
	void add_pt(unsigned int idx_pt);
	void remove_large_triangle();
	void finish_pts();
	// méthodes de debug
	int idx_triangle(Triangle * triangle);
	void print_triangle(Triangle * triangle, bool verbose=false, bool is_pt_init=true);
	void draw(std::string svg_path, bool verbose=false);
	glm::vec2 svg_coords(glm::vec2 & v);


	std::vector<PointBin *> _pts;
	std::vector<Triangle *> _triangles;
	AABB_2D * _aabb;
	std::vector<AABB_2D *> _bins;
	float _svg_margin= 0.1f;
	bool _sort_by_bin, _verbose;
};


#endif
