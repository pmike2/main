#include <iostream>
#include <vector>

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
	DCEL_HalfEdge * e32= dcel->add_edge(v3, v2);

	DCEL_Face * f1= dcel->add_face(std::vector<DCEL_HalfEdge *>{e12, e23, e31});
	DCEL_Face * f2= dcel->add_face(std::vector<DCEL_HalfEdge *>{e24, e43, e32});

	std::cout << *dcel;
	dcel->export_html("../data/test1.html");
	
	delete dcel;
}


int main() {
	test1();
	
	return 0;
}
