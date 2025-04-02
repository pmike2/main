#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <chrono>

#include <glm/glm.hpp>

/*
using number = float;
using pt_type = glm::vec2;
using mat = glm::mat2;
*/

using number = double;
using pt_type = glm::dvec2;
using mat = glm::dmat2;

using time_point = std::chrono::system_clock::time_point;


#endif
