#ifndef GEOM_H
#define GEOM_H

#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "typedefs.h"
#include "bbox.h"


const bool VERBOSE = false;


bool triangle_intersects_triangle(pt_3d v[3], pt_3d w[3]);
bool aabb_intersects_aabb(AABB * aabb_1, AABB * aabb_2);
bool aabb_intersects_bbox(AABB * aabb, BBox * bbox);
bool bbox_intersects_bbox(BBox * bbox_1, BBox * bbox_2);
number aabb_distance_pt_2(AABB * aabb, const pt_3d & pt);
number aabb_distance_pt(AABB * aabb, const pt_3d & pt);
bool ray_intersects_aabb(pt_3d origin, pt_3d direction, AABB * aabb, number & t_hit);
bool segment_intersects_aabb(const pt_3d & pt1, const pt_3d & pt2, AABB * aabb);


#endif
