#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <chrono>

#include <glm/glm.hpp>

using number = double;

using pt_2d = glm::dvec2;
using pt_3d = glm::dvec3;
using pt_4d = glm::dvec4;

using mat_2d = glm::dmat2;
using mat_3d = glm::dmat3;
using mat_4d = glm::dmat4;

using quat = glm::dquat;

using time_point = std::chrono::system_clock::time_point;

using uint = unsigned int;


#endif
