
#include <iostream>

#include "utile.h"
#include "typedefs.h"

#include "geom.h"
#include "bbox.h"


void test_convex_hull() {
	ConvexHull * hull = new ConvexHull();
	
	//hull->randomize(10, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0);

	hull->add_pt(4.0, 10.0, 1.0);
	hull->add_pt(3.0, 8.0, 3.0);
	hull->add_pt(2.0, 1.0, 8.0);
	hull->add_pt(9.0, 7.0, 2.0);
	hull->add_pt(2.0, 1.7, 6.0);
	hull->add_pt(9.0, 4.0, 10.0);

	hull->compute();

	delete hull;
}


void test_bbox() {
	pt_3d origin(0.0, 0.0, 50.0);
	pt_3d direction(-14.0, -5.0, -50.0);
	AABB * aabb = new AABB(pt_3d(0.01, 0.01, 0.5), pt_3d(1.0, 1.0, 0.5));
	number t_hit = 0.0;

	bool b = ray_intersects_aabb(origin, direction, aabb, t_hit);
	std::cout << b << " ; " << t_hit << "\n";
	delete aabb;
}


int main() {
	//test_convex_hull();
	test_bbox();

	return 0;
}
