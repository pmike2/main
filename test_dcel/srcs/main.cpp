#include <iostream>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "typedefs.h"
#include "dcel.h"


void test1() {
	DCEL * dcel= new DCEL();
	
	//dcel->import("0,0 -> 1,0 -> 0,1");

	//dcel->import("0,0 -> 1,0 -> 0,1 | 1,0 -> 1,3 -> 0,1 | 5,5 -> 6,5 -> 5,6");
	//dcel->add_bbox(pt_type(-1.0, -1.0), pt_type(7.0, 7.0));
	//dcel->add_bbox(pt_type(-1.0, -1.0), pt_type(2.0, 2.0));
	//dcel->add_bbox(pt_type(0.1, -1.0), pt_type(2.0, 2.0));

	//dcel->import("0,0 -> 1,0 -> 1,1 -> 0.4,0.4 -> 0.3,0.3 -> 0.2,0.2 -> 0.1,0.1 | 0,0 -> 0.1,0.1 -> 0.2,0.2 -> 0.3,0.3 -> 0.4,0.4 -> 1,1 -> 0,1");

	//dcel->import("0,0 -> 1,0 -> 0.5,0.5 | 1,0 -> 1,1 -> 0.5,0.5 | 0.5,0.5 -> 1,1 -> 0,1 | 0,1 -> 0,0 -> 0.5,0.5");

	//dcel->import("0,0 -> 4,0 -> 4,4 -> 0,4");
	//dcel->add_bbox(pt_type(-1.0, 0.5), pt_type(5.0, 3.5));

	//dcel->import("0,0 -> 4,0 -> 4,4 -> 0,4 | 1,1 -> 3,1 -> 3,3 -> 1,3");
	//dcel->add_bbox(pt_type(-1.0, 0.5), pt_type(5.0, 3.5));

	//dcel->import("0,0 -> 5,0 -> 5,5 -> 0,5 | 1,1 -> 2,1 -> 2,2 -> 1,2 | 3,3 -> 4,3 -> 4,4 -> 3,4");
	//dcel->add_bbox(pt_type(-1.0, 1.5), pt_type(6.0, 3.5));

	//dcel->import("1,-1 -> 1,1 -> 0.5, 0.5");
	//dcel->add_bbox(pt_type(0.0, 0.0), pt_type(0.7, 0.7));

	dcel->import("-5,-5 -> 0,0 -> -5,5 | -5,5 -> 0,0 -> 5,5 | 5,5 -> 0,0 -> 5,-5 | 5,-5 -> 0,0 -> -5,-5");
	dcel->add_bbox(pt_type(-1.0, -1.0), pt_type(1.0, 1.0));

	std::cout << *dcel;

	pt_type bbox_min, bbox_max;
	dcel->get_bbox(&bbox_min, &bbox_max);
	dcel->export_html("../data/test1.html", false, bbox_min- 0.1, bbox_max+ 0.1);
	dcel->export_html("../data/test1_simple.html", true, bbox_min- 0.1, bbox_max+ 0.1);
	
	delete dcel;
}



int main() {
	test1();
	
	return 0;
}
