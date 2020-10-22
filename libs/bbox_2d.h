#ifndef BBOX_2D_H
#define BBOX_2D_H

/*

https://www.youtube.com/watch?v=8JJ-4JgR7Dg
https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/OneLoneCoder_PGE_Rectangles.cpp

*/

#include <string>
#include <iostream>
#include <algorithm>


#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>


class AABB_2D {
public:
	AABB_2D();
	AABB_2D(glm::vec2 pos, glm::vec2 size);
	~AABB_2D();


	glm::vec2 _pos;
	glm::vec2 _size;
};


bool point_in_aabb(const glm::vec2 & pt, const AABB_2D * aabb);
bool aabb_intersects_aabb(const AABB_2D * aabb_1, const AABB_2D * aabb_2);
bool ray_intersects_aabb(const glm::vec2 & ray_origin, const glm::vec2 & ray_dir, const AABB_2D * aabb, glm::vec2 & contact_pt, glm::vec2 & contact_normal, float & t_hit_near);
//bool dynamic_aabb_intersects_aabb(const AABB_2D * dynamic_aabb, const AABB_2D * static_aabb, const float time_step, glm::vec2 & contact_pt, glm::vec2 & contact_normal, float & contact_time);
//bool resolve_dynamic_aabb_intersects_aabb(AABB_2D * dynamic_aabb, const AABB_2D * static_aabb, const float time_step);


#endif
