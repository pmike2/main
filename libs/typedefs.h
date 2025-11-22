#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <chrono>

#include <glm/glm.hpp>

using number = double;

using pt_type = glm::dvec2;
using mat = glm::dmat2;

using pt_type_3d = glm::dvec3;
using mat_3d = glm::dmat3;

using time_point = std::chrono::system_clock::time_point;

using uint = unsigned int;


#endif
