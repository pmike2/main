#ifndef BBOX_2D_H
#define BBOX_2D_H

/*

https://www.youtube.com/watch?v=8JJ-4JgR7Dg
https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/OneLoneCoder_PGE_Rectangles.cpp

*/

#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include "typedefs.h"


enum BBOX_SIDE {LEFT_SIDE, RIGHT_SIDE, TOP_SIDE, BOTTOM_SIDE, NO_SIDE};
enum BBOX_CORNER {BOTTOMLEFT_CORNER, BOTTOMRIGHT_CORNER, TOPLEFT_CORNER, TOPRIGHT_CORNER, NO_CORNER};


class AABB_2D {
public:
	AABB_2D();
	AABB_2D(pt_2d pos, pt_2d size);
	AABB_2D(const AABB_2D & aabb);
	AABB_2D(const std::vector<pt_2d> & pts);
	~AABB_2D();
	pt_2d center();
	AABB_2D * buffered(number size);
	void buffer(number size);
	friend std::ostream & operator << (std::ostream & os, const AABB_2D & aabb);


	pt_2d _pos;
	pt_2d _size;
};


class BBox_2D {
public:
	BBox_2D();
	BBox_2D(pt_2d center, pt_2d half_size, number alpha=0.0);
	BBox_2D(number width, pt_2d pt1, pt_2d pt2);
	~BBox_2D();
	void set_aabb(AABB_2D & aabb);
	void update();
	BBox_2D * buffered(number size);
	friend std::ostream & operator << (std::ostream & os, const BBox_2D & bbox);


	pt_2d _center;
	pt_2d _half_size;
	number _alpha; // rotation
	pt_2d _pts[4];
	AABB_2D * _aabb;
};


#endif
