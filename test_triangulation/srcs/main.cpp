
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <string>
#include <vector>
#include <chrono>


#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>


#include "utile.h"
#include "triangulation.h"
#include "geom_2d.h"


using namespace std;
using namespace std::chrono;

vector<glm::vec2> pts;


void test1() {
	pts.push_back(glm::vec2(0.0f, 0.0f));
	pts.push_back(glm::vec2(1.0f, 0.0f));
	pts.push_back(glm::vec2(0.0f, 1.0f));
	pts.push_back(glm::vec2(0.0f, -1.0f));
	pts.push_back(glm::vec2(0.5f, 0.0f));

	pts.push_back(glm::vec2(0.0f, 0.0f));
	pts.push_back(glm::vec2(1.0f, 0.0f));
	pts.push_back(glm::vec2(0.0f, 1.0f));
	pts.push_back(glm::vec2(0.0f, -1.0f));
	pts.push_back(glm::vec2(0.5f, 0.0f));
}


void test2() {
	pts.push_back(glm::vec2(0.000000, 0.428553));
	pts.push_back(glm::vec2(0.460314, 1.000000));
	pts.push_back(glm::vec2(0.465795, 0.350450));
	pts.push_back(glm::vec2(0.237588, 0.183974));
	pts.push_back(glm::vec2(0.445873, 0.366553));
	pts.push_back(glm::vec2(0.318963, 0.000000));
	pts.push_back(glm::vec2(0.183725, 0.641182));
	pts.push_back(glm::vec2(0.933593, 0.126030));
	pts.push_back(glm::vec2(0.787499, 0.076075));
	pts.push_back(glm::vec2(0.834194, 0.282110));
	pts.push_back(glm::vec2(0.924107, 0.295930));
	pts.push_back(glm::vec2(1.000000, 0.083048));
	pts.push_back(glm::vec2(0.709813, 0.630278));
	pts.push_back(glm::vec2(0.827448, 0.942183));
	pts.push_back(glm::vec2(0.804891, 0.872195));
	pts.push_back(glm::vec2(0.951196, 0.957018));
}


void test3() {
	srand(time(NULL));
	for (unsigned int i=0; i<10000; ++i) {
		pts.push_back(glm::vec2(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f)));
	}

	ofstream f;
	f.open("../data/debug.txt");
	for (auto pt : pts) {
		f << pt.x << " " << pt.y << "\n";
	}
	f.close();
}


void test4() {
	ifstream f("../data/debug.txt");
	while (f.good()) {
		string line;
		getline(f, line);
		istringstream iss(line);
		float x, y;
		iss >> x >> y;
		glm::vec2 pt(x, y);
		pts.push_back(pt);
	}
}


// ------------------------------------------------------------------------
int main(int argc, char * argv[]) {
	auto t1= high_resolution_clock::now();

	//test3();
	test4();
	Triangulation * tgl= new Triangulation(pts, false, false);

	auto t2= high_resolution_clock::now();
	auto ms= duration_cast<milliseconds>(t2- t1);
	cout << ms.count() << " ms\n";

	//tgl->draw("../data/result.html", true);
	delete tgl;


/*
	glm::vec2 bary(0.246591, 0.163983);
	glm::vec2 pt(0.276628, 0.189919);
	glm::vec2 edge1(0.248825, 0.160316);
	glm::vec2 edge2(0.246925, 0.169817);
	glm::vec2 result(0.0f);
	bool x= segment_intersects_segment(bary, pt, edge1, edge2, &result, true);
	cout << x << " ; " << glm::to_string(result) << "\n";
*/

/*	glm::vec2 pt1(0.460314f, 1.000000f);
	glm::vec2 pt2(0.000000f, 0.428553f);
	glm::vec2 pt3(0.465795f, 0.350450f);
	glm::vec2 pt(0.237588f, 0.183974f);
	bool x= point_in_circumcircle(pt1, pt2, pt3, pt);
	cout << x << "\n";
*/
	return 0;
}
