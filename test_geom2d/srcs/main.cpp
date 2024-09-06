#include <iostream>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "geom_2d.h"
#include "utile.h"


void test1() {
	glm::vec2 result(0.0f);
	glm::vec2 origin1(1.0f, 1.0f);
	glm::vec2 direction1(-1.0f, -1.0f);
	glm::vec2 origin2(3.0f, 0.0f);
	glm::vec2 direction2(-2.0f, -1.0f);

	bool is_inter= ray_intersects_ray(origin1, direction1, origin2, direction2, &result);
	std::cout << "is_inter = " << is_inter << " ; inter = " << glm_to_string(result) << "\n";
}



int main() {
	test1();
	
	return 0;
}
