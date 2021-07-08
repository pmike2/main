#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "bbox_2d.h"


bool sort_pts(PointBin & pt1, PointBin & pt2);


struct Triangle {
	Triangle();
	Triangle(int v0, int v1, int v2, int a0, int a1, int a2);
	~Triangle();


	int _vertices[3];
	int _adjacents[3];
};


struct PointBin {
	PointBin();
	PointBin(glm::vec2 pt, int idx_bin);
	~PointBin();


	glm::vec2 _pt;
	int _idx_bin;
};


struct Triangulation {
	Triangulation();
	Triangulation(std::vector<glm::vec2> & pts);
	~Triangulation();
	void triangulate();


	std::vector<glm::vec2> _pts_init;
	std::vector<PointBin *> _pts;
	std::vector<Triangle *> _triangles;
	AABB_2D * _aabb;
	std::vector<AABB_2D *> _bins;
};




#endif
