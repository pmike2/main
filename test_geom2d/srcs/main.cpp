#include <iostream>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "typedefs.h"
#include "geom_2d.h"
#include "utile.h"


void test1() {
	pt_type result(0.0);
	pt_type origin1(0.0, 0.0);
	pt_type direction1(1.0, 0.0);
	pt_type origin2(0.0, 1.0);
	pt_type direction2(0.707, -0.707);

	bool is_inter= ray_intersects_ray(origin1, direction1, origin2, direction2, &result);
	std::cout << "is_inter = " << is_inter << " ; inter = " << glm_to_string(result) << "\n";
}


void test2() {
	pt_type result(0.0);
	pt_type origin(0.0, 0.0);
	pt_type direction(1.0, 0.0);
	pt_type pt_begin(0.0, 1.0);
	pt_type pt_end(2.0, -1.0);

	bool is_inter= ray_intersects_segment(origin, direction, pt_begin, pt_end, &result);
	std::cout << "is_inter = " << is_inter << " ; inter = " << glm_to_string(result) << "\n";
}


void test3() {
	pt_type pt1_begin(-1, -1);
	pt_type pt1_end(1, 1);
	pt_type pt2_begin(-1, 1);
	pt_type pt2_end(1, 1);
	pt_type result;
	bool exclude_seg1_extremities=false;
	bool exclude_seg2_extremities=false;
	bool is_inter= segment_intersects_segment(pt1_begin, pt1_end, pt2_begin, pt2_end, &result, exclude_seg1_extremities, exclude_seg2_extremities);
	std::cout << "is_inter = " << is_inter << " ; inter = " << glm_to_string(result) << "\n";
}



int main() {
	//test1();
	//test2();
	test3();
	
	return 0;
}
