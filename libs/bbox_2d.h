#ifndef BBOX_2D_H
#define BBOX_2D_H

/*

https://www.youtube.com/watch?v=8JJ-4JgR7Dg
https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/OneLoneCoder_PGE_Rectangles.cpp

*/

#include <iostream>

#include <glm/glm.hpp>


class AABB_2D {
public:
	AABB_2D();
	AABB_2D(glm::vec2 pos, glm::vec2 size);
	AABB_2D(const AABB_2D & aabb);
	~AABB_2D();
	glm::vec2 center();
	AABB_2D * buffered(float size);
	friend std::ostream & operator << (std::ostream & os, const AABB_2D & aabb);


	glm::vec2 _pos;
	glm::vec2 _size;
};


bool point_in_aabb(const glm::vec2 & pt, const AABB_2D * aabb);
bool aabb_intersects_aabb(const AABB_2D * aabb_1, const AABB_2D * aabb_2);
bool ray_intersects_aabb(const glm::vec2 & ray_origin, const glm::vec2 & ray_dir, const AABB_2D * aabb, glm::vec2 & contact_pt, glm::vec2 & contact_normal, float & t_hit_near);


#endif
