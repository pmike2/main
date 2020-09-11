#ifndef BBOX_2D_H
#define BBOX_2D_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>


class AABB_2D {
public:
    AABB_2D();
    AABB_2D(glm::vec2 pt_min, glm::vec2 pt_max);
    ~AABB_2D();


    glm::vec2 _pt_min;
    glm::vec2 _pt_max;
};

#endif
