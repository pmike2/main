#include <iostream>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "dcel.h"


void test1() {
	DCEL * dcel= new DCEL();
	
	//dcel->import("0,0 -> 1,0 -> 0,1");

	//dcel->import("0,0 -> 1,0 -> 0,1 | 1,0 -> 1,3 -> 0,1 | 5,5 -> 6,5 -> 5,6");
	//dcel->add_bbox(glm::vec2(-1.0, -1.0), glm::vec2(7.0, 7.0));
	//dcel->add_bbox(glm::vec2(-1.0, -1.0), glm::vec2(2.0, 2.0));
	//dcel->add_bbox(glm::vec2(0.1, -1.0), glm::vec2(2.0, 2.0));

	//dcel->import("0,0 -> 1,0 -> 1,1 -> 0.4,0.4 -> 0.3,0.3 -> 0.2,0.2 -> 0.1,0.1 | 0,0 -> 0.1,0.1 -> 0.2,0.2 -> 0.3,0.3 -> 0.4,0.4 -> 1,1 -> 0,1");

	//dcel->import("0,0 -> 1,0 -> 0.5,0.5 | 1,0 -> 1,1 -> 0.5,0.5 | 0.5,0.5 -> 1,1 -> 0,1 | 0,1 -> 0,0 -> 0.5,0.5");

	//dcel->import("0,0 -> 4,0 -> 4,4 -> 0,4");
	//dcel->add_bbox(glm::vec2(-1.0, 0.5), glm::vec2(5.0, 3.5));

	//dcel->import("0,0 -> 4,0 -> 4,4 -> 0,4 | 1,1 -> 3,1 -> 3,3 -> 1,3");
	//dcel->add_bbox(glm::vec2(-1.0, 0.5), glm::vec2(5.0, 3.5));

	dcel->import("0,0 -> 5,0 -> 5,5 -> 0,5 | 1,1 -> 2,1 -> 2,2 -> 1,2 | 3,3 -> 4,3 -> 4,4 -> 3,4");
	dcel->add_bbox(glm::vec2(-1.0, 1.5), glm::vec2(6.0, 3.5));

	//std::cout << *dcel;

	glm::vec2 bbox_min, bbox_max;
	dcel->get_bbox(&bbox_min, &bbox_max);
	dcel->export_html("../data/test1.html", false, bbox_min- 0.1f, bbox_max+ 0.1f);
	dcel->export_html("../data/test1_simple.html", true, bbox_min- 0.1f, bbox_max+ 0.1f);
	
	delete dcel;
}



int main() {
	test1();
	
	return 0;
}
