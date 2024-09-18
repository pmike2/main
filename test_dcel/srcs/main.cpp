#include <iostream>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "dcel.h"


void test1() {
	DCEL * dcel= new DCEL();
	
	dcel->import("0,0 -> 1,0 -> 0,1 | 1,0 -> 1,3 -> 0,1 | 5,5 -> 6,5 -> 5,6");
	dcel->make_valid();
	dcel->add_bbox(0.1, -1.0, 2.0, 2.0);

	//std::vector<DCEL_HalfEdge *> edges= dcel->get_vertex(0, 0)->get_incident_edges();
	dcel->make_valid();
	//dcel->get_vertex(1,3)->get_incident_edges();
	//std::cout << *dcel;
	//return;

	/*dcel->import("0,0 -> 1,0 -> 1,1 -> 0.4,0.4 -> 0.3,0.3 -> 0.2,0.2 -> 0.1,0.1 | 0,0 -> 0.1,0.1 -> 0.2,0.2 -> 0.3,0.3 -> 0.4,0.4 -> 1,1 -> 0,1");
	dcel->make_valid();
	dcel->delete_edge(dcel->get_edge(0.2, 0.2, 0.3, 0.3));
	dcel->make_valid();*/

	/*dcel->import("0,0 -> 1,0 -> 0.5,0.5 | 1,0 -> 1,1 -> 0.5,0.5 | 0.5,0.5 -> 1,1 -> 0,1 | 0,1 -> 0,0 -> 0.5,0.5");
	dcel->make_valid();
	dcel->delete_edge(dcel->get_edge(0.5, 0.5, 1.0, 0.0));
	dcel->delete_vertex(dcel->get_vertex(0.5, 0.5));
	dcel->make_valid();*/

	//std::cout << *dcel;
	float xmin, ymin, xmax, ymax;
	dcel->bbox(&xmin, &ymin, &xmax, &ymax);
	//std::cout << xmin << " ; " << ymin << " ; " << xmax << " ; " << ymax << "\n";
	dcel->export_html("../data/test1.html", false, xmin- 0.1f, ymin- 0.1f, xmax+ 0.1f, ymax+ 0.1f);
	
	delete dcel;
}


void test2() {
	DCEL * dcel= new DCEL();

	DCEL_HalfEdge * he= dcel->add_edge(NULL, NULL);
	he->_dx= 1.0f;
	he->_dy= -2.0f;
	he->_tmp_x= 0.5f;
	he->_tmp_y= 0.5f;

	//dcel->add_bbox();
	dcel->create_faces_from_half_edges();
	//std::cout << *dcel;
	//dcel->export_html("../data/test2.html", false);
}


void test3() {
	DCEL * dcel= new DCEL();

	DCEL_Vertex * v1= dcel->add_vertex(0.5f, 0.5f);

	DCEL_HalfEdge * he1= dcel->add_edge(v1, NULL);
	he1->set_tmp_data(glm::vec2(1.0f, -2.0f));

	DCEL_HalfEdge * he2= dcel->add_edge(v1, NULL);
	he2->set_tmp_data(glm::vec2(1.0f, 2.0f));

	DCEL_HalfEdge * he3= dcel->add_edge(v1, NULL);
	he3->set_tmp_data(glm::vec2(-1.0f, 0.0f));

	he1->_twin->set_next(he3);
	he3->_twin->set_next(he2);
	he2->_twin->set_next(he1);

	//dcel->add_bbox();
	dcel->create_faces_from_half_edges();
	//std::cout << *dcel;
	//dcel->export_html("../data/test3.html" , false);
}


int main() {
	test1();
	//test2();
	//test3();
	
	return 0;
}
