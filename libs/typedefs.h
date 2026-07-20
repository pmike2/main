#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <utility>
#include <chrono>
#include <unordered_map>
#include <filesystem>

#include <glm/glm.hpp>

#include <tsl/robin_map.h>
//#include <sparsehash/sparse_hash_map>
//#include <sparsehash/dense_hash_map>


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

using int_pair = std::pair<int, int>;
using uint_pair = std::pair<uint, uint>;

using fs = std::filesystem::path;

// + rapide que std::unordered_map ; cf https://github.com/Tessil/robin-map/tree/master
//template<class X, class Y>
//using map = std::unordered_map<X, Y>;

template<class X, class Y>
using map = tsl::robin_map<X, Y>;

//template<class X, class Y>
//using map = google::sparse_hash_map<X, Y>;

//template<class X, class Y>
//using map = google::dense_hash_map<X, Y>;


#endif
