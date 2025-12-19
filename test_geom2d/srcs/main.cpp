#include <iostream>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "typedefs.h"
#include "geom_2d.h"
#include "utile.h"


void test1() {
	pt_2d result(0.0);
	pt_2d origin1(0.0, 0.0);
	pt_2d direction1(1.0, 0.0);
	pt_2d origin2(0.0, 1.0);
	pt_2d direction2(0.707, -0.707);

	bool is_inter= ray_intersects_ray(origin1, direction1, origin2, direction2, &result);
	std::cout << "is_inter = " << is_inter << " ; inter = " << glm_to_string(result) << "\n";
}


void test2() {
	pt_2d result(0.0);
	pt_2d origin(0.0, 0.0);
	pt_2d direction(1.0, 0.0);
	pt_2d pt_begin(0.0, 1.0);
	pt_2d pt_end(2.0, -1.0);

	bool is_inter= ray_intersects_segment(origin, direction, pt_begin, pt_end, &result);
	std::cout << "is_inter = " << is_inter << " ; inter = " << glm_to_string(result) << "\n";
}


void test3() {
	pt_2d pt1_begin(-1, -1);
	pt_2d pt1_end(1, 1);
	pt_2d pt2_begin(-1, 1);
	pt_2d pt2_end(1, 1);
	pt_2d result;
	bool exclude_seg1_extremities=false;
	bool exclude_seg2_extremities=false;
	bool is_inter= segment_intersects_segment(pt1_begin, pt1_end, pt2_begin, pt2_end, &result, exclude_seg1_extremities, exclude_seg2_extremities);
	std::cout << "is_inter = " << is_inter << " ; inter = " << glm_to_string(result) << "\n";
}


void test4() {
	Polygon2D * poly1= new Polygon2D();
	Polygon2D * poly2= new Polygon2D();

/*poly1=(-1.093 , -3.1544) , (0.074 , 3.5704) , (-4.57 , -0.7552) , 
poly2=(-0.596 , -2.9872) , (-1.755 , -1.4152) , (-3.853 , -0.7144) */

	poly1->set_points(std::vector<pt_2d>{pt_2d(-1.093 , -3.1544), pt_2d(0.074 , 3.5704), pt_2d(-4.57 , -0.7552)});
	poly2->set_points(std::vector<pt_2d>{pt_2d(-0.596 , -2.9872), pt_2d(-1.755 , -1.4152), pt_2d(-3.853 , -0.7144)});

	pt_2d axis(0.0, 0.0);
	number overlap= 0.0;
	unsigned int idx_pt= 0;
	bool is_pt_in_poly1= false;
	bool inter= poly_intersects_poly(poly1, poly2, &axis, &overlap, &idx_pt, &is_pt_in_poly1);
	std::cout << "inter=" << inter << " ; axis=(" << axis.x << " , " << axis.y << ") ; overlap=" << overlap;
	std::cout << " ; idx_pt=" << idx_pt << " ; is_pt_in_poly1=" << is_pt_in_poly1 << "\n";
}


void test5() {
	Polygon2D * poly= new Polygon2D();
	poly->set_points(std::vector<pt_2d>{pt_2d(0.0 , 0.0), pt_2d(1.0 , 0.0), pt_2d(1.0 , 1.0), pt_2d(0.0 , 1.0)});
	poly->triangulate();
	for (auto tri : poly->_triangles_idx) {
		std::cout << tri[0] << " ; " << tri[1] << " ; " << tri[2] << "\n";
	}
}


void test6() {
	Polygon2D * poly= new Polygon2D();
	poly->set_points(std::vector<pt_2d>{pt_2d(0.0 , 0.0), pt_2d(1.0 , 0.0), pt_2d(1.0 , 1.0), pt_2d(0.0 , 1.0)});
	poly->update_all();
	Polygon2D * buffered = poly->buffered(0.5);
	buffered->update_all();
	//std::cout << *buffered << "\n";
	for (auto pt : buffered->_pts) {
		std::cout << glm::to_string(pt) << "\n";
	}
}


int main() {
	//test1();
	//test2();
	//test3();
	//test4();
	//test5();
	test6();
	
	return 0;
}
