#include <iostream>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "geom_2d.h"
#include "utile.h"


void test1() {
	glm::vec2 result(0.0);
	glm::vec2 origin1(0.0, 0.0);
	glm::vec2 direction1(1.0, 0.0);
	glm::vec2 origin2(0.0, 1.0);
	glm::vec2 direction2(0.707, -0.707);

	bool is_inter= ray_intersects_ray(origin1, direction1, origin2, direction2, &result);
	std::cout << "is_inter = " << is_inter << " ; inter = " << glm_to_string(result) << "\n";
}


void test2() {
	glm::vec2 result(0.0);
	glm::vec2 origin(0.0, 0.0);
	glm::vec2 direction(1.0, 0.0);
	glm::vec2 pt_begin(0.0, 1.0);
	glm::vec2 pt_end(2.0, -1.0);

	bool is_inter= ray_intersects_segment(origin, direction, pt_begin, pt_end, &result);
	std::cout << "is_inter = " << is_inter << " ; inter = " << glm_to_string(result) << "\n";
}



int main() {
	test1();
	//test2();
	
	return 0;
}
