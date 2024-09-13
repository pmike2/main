#include <iostream>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "dcel.h"


void test1() {
	DCEL * dcel= new DCEL();
	
	DCEL_Vertex * v1= dcel->add_vertex(0.0f, 0.0f);
	DCEL_Vertex * v2= dcel->add_vertex(1.0f, 0.0f);
	DCEL_Vertex * v3= dcel->add_vertex(0.0f, 1.0f);
	DCEL_Vertex * v4= dcel->add_vertex(1.0f, 3.0f);
	
	DCEL_HalfEdge * e12= dcel->add_edge(v1, v2);
	DCEL_HalfEdge * e23= dcel->add_edge(v2, v3);
	DCEL_HalfEdge * e31= dcel->add_edge(v3, v1);
	DCEL_HalfEdge * e24= dcel->add_edge(v2, v4);
	DCEL_HalfEdge * e43= dcel->add_edge(v4, v3);
	e12->set_next(e23);
	e23->set_next(e31);
	e31->set_next(e12);
	e24->set_next(e43);
	e43->set_next(e23->_twin);
	e23->_twin->set_next(e24);

	dcel->create_faces_from_half_edges();

	dcel->add_unbounded_face();

	//std::cout << *dcel;
	dcel->export_html("../data/test1.html", false, -1.0f, -1.0f, 5.0f, 5.0f);
	
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
