#ifndef BBOX_2D_H
#define BBOX_2D_H

/*

https://www.youtube.com/watch?v=8JJ-4JgR7Dg
https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/OneLoneCoder_PGE_Rectangles.cpp

*/

#include <iostream>

#include <glm/glm.hpp>

#include "typedefs.h"


class AABB_2D {
public:
	AABB_2D();
	AABB_2D(pt_type pos, pt_type size);
	AABB_2D(const AABB_2D & aabb);
	~AABB_2D();
	pt_type center();
	AABB_2D * buffered(number size);
	void buffer(number size);
	friend std::ostream & operator << (std::ostream & os, const AABB_2D & aabb);


	pt_type _pos;
	pt_type _size;
};


bool point_in_aabb(const pt_type & pt, const AABB_2D * aabb);
bool aabb_intersects_aabb(const AABB_2D * aabb_1, const AABB_2D * aabb_2);
bool aabb_contains_aabb(const AABB_2D * big_aabb, const AABB_2D * small_aabb);
bool ray_intersects_aabb(const pt_type & ray_origin, const pt_type & ray_dir, const AABB_2D * aabb, pt_type & contact_pt, pt_type & contact_normal, number & t_hit_near);


class BBox_2D {
public:
	BBox_2D();
	BBox_2D(pt_type center, pt_type size, float alpha=0.0);
	~BBox_2D();
	void update();
	friend std::ostream & operator << (std::ostream & os, const BBox_2D & bbox);


	pt_type _center;
	pt_type _size;
	float _alpha; // rotation
	pt_type _pts[4];
	AABB_2D * _aabb;
};

#endif
