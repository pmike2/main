#include "bbox_2d.h"

AABB_2D::AABB_2D() {

}


AABB_2D::AABB_2D(glm::vec2 pt_min, glm::vec2 pt_max) : _pt_min(pt_min), _pt_max(pt_max) {
    
}


AABB_2D::~AABB_2D() {
    
}

