#include <iostream>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "dcel.h"


void test1() {
	DCEL * dcel= new DCEL();
	
	/*DCEL_Vertex * v1= dcel->add_vertex(0.0f, 0.0f);
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

	DCEL_Vertex * v5= dcel->add_vertex(5.0f, 5.0f);
	DCEL_Vertex * v6= dcel->add_vertex(6.0f, 5.0f);
	DCEL_Vertex * v7= dcel->add_vertex(5.0f, 6.0f);
	DCEL_HalfEdge * e56= dcel->add_edge(v5, v6);
	DCEL_HalfEdge * e67= dcel->add_edge(v6, v7);
	DCEL_HalfEdge * e75= dcel->add_edge(v7, v5);
	e56->set_next(e67);
	e67->set_next(e75);
	e75->set_next(e56);*/

	/*DCEL_Vertex * v1= dcel->add_vertex(0.0f, 0.0f);
	DCEL_Vertex * v2= dcel->add_vertex(1.0f, 0.0f);
	DCEL_Vertex * v3= dcel->add_vertex(1.0f, 1.0f);
	DCEL_Vertex * v4= dcel->add_vertex(0.0f, 1.0f);
	DCEL_Vertex * v5= dcel->add_vertex(0.1f, 0.1f);
	DCEL_Vertex * v6= dcel->add_vertex(0.2f, 0.2f);
	DCEL_Vertex * v7= dcel->add_vertex(0.3f, 0.3f);
	DCEL_Vertex * v8= dcel->add_vertex(0.4f, 0.4f);
	DCEL_HalfEdge * e12= dcel->add_edge(v1, v2);
	DCEL_HalfEdge * e23= dcel->add_edge(v2, v3);
	DCEL_HalfEdge * e38= dcel->add_edge(v3, v8);
	DCEL_HalfEdge * e87= dcel->add_edge(v8, v7);
	DCEL_HalfEdge * e76= dcel->add_edge(v7, v6);
	DCEL_HalfEdge * e65= dcel->add_edge(v6, v5);
	DCEL_HalfEdge * e51= dcel->add_edge(v5, v1);
	DCEL_HalfEdge * e34= dcel->add_edge(v3, v4);
	DCEL_HalfEdge * e41= dcel->add_edge(v4, v1);
	e12->set_next(e23);
	e23->set_next(e38);
	e38->set_next(e87);
	e87->set_next(e76);
	e76->set_next(e65);
	e65->set_next(e51);
	e51->set_next(e12);
	e34->set_next(e41);
	e41->set_next(e51->_twin);
	e51->_twin->set_next(e65->_twin);
	e65->_twin->set_next(e76->_twin);
	e76->_twin->set_next(e87->_twin);
	e87->_twin->set_next(e38->_twin);
	e38->_twin->set_next(e34);*/

	DCEL_Vertex * v1= dcel->add_vertex(0.0f, 0.0f);
	DCEL_Vertex * v2= dcel->add_vertex(1.0f, 0.0f);
	DCEL_Vertex * v3= dcel->add_vertex(1.0f, 1.0f);
	DCEL_Vertex * v4= dcel->add_vertex(0.0f, 1.0f);
	DCEL_Vertex * v5= dcel->add_vertex(0.5f, 0.5f);
	DCEL_HalfEdge * e12= dcel->add_edge(v1, v2);
	DCEL_HalfEdge * e25= dcel->add_edge(v2, v5);
	DCEL_HalfEdge * e51= dcel->add_edge(v5, v1);
	DCEL_HalfEdge * e23= dcel->add_edge(v2, v3);
	DCEL_HalfEdge * e35= dcel->add_edge(v3, v5);
	DCEL_HalfEdge * e34= dcel->add_edge(v3, v4);
	DCEL_HalfEdge * e45= dcel->add_edge(v4, v5);
	DCEL_HalfEdge * e41= dcel->add_edge(v4, v1);
	e12->set_next(e25);
	e25->set_next(e51);
	e51->set_next(e12);
	e23->set_next(e35);
	e35->set_next(e25->_twin);
	e25->_twin->set_next(e23);
	e34->set_next(e45);
	e45->set_next(e35->_twin);
	e35->_twin->set_next(e34);
	e41->set_next(e51->_twin);
	e51->_twin->set_next(e45->_twin);
	e45->_twin->set_next(e41);


	std::cout << "create_faces_from_half_edges : " << dcel->create_faces_from_half_edges() << "\n";
	std::cout << "add_unbounded_face : " << dcel->add_unbounded_face() << "\n";

	//v5->get_incident_edges();
	//std::cout << *e23 << " ; " << *e23->_next << "\n";
	//return;

	//dcel->delete_edge(e23);
	//dcel->delete_edge(e56);
	//dcel->delete_edge(e76);
	dcel->delete_edge(e25);
	dcel->delete_edge(e35);
	//dcel->delete_vertex(v5);

	dcel->delete_disconnected_vertices();
	dcel->create_faces_from_half_edges();


	//std::cout << *dcel;
	std:: cout << "export HTML\n";
	dcel->export_html("../data/test1.html", false, -0.1f, -0.1f, 1.1f, 1.1f);
	
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
