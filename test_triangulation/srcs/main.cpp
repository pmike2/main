
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
vector<pair<unsigned int, unsigned int> > constrained_edges;


void test1() {
	pts.push_back(glm::vec2(0.0f, 0.0f));
	pts.push_back(glm::vec2(1.0f, 0.0f));
	pts.push_back(glm::vec2(1.0f, 1.0f));
	pts.push_back(glm::vec2(0.0f, 1.0f));

	pts.push_back(glm::vec2(0.3f, 0.3f));
	pts.push_back(glm::vec2(0.6f, 0.3f));
	pts.push_back(glm::vec2(0.6f, 0.6f));
	pts.push_back(glm::vec2(0.3f, 0.6f));

	constrained_edges.push_back(make_pair(4, 5));
	constrained_edges.push_back(make_pair(5, 6));
	constrained_edges.push_back(make_pair(6, 7));
	constrained_edges.push_back(make_pair(7, 4));
}


void test2() {
	pts.push_back(glm::vec2(0.0f, 0.0f));
	pts.push_back(glm::vec2(1.0f, 0.0f));
	pts.push_back(glm::vec2(1.0f, 1.0f));
	pts.push_back(glm::vec2(0.0f, 1.0f));

	unsigned int n_obstacles= 8;
	for (unsigned int i=0; i<n_obstacles; ++i) {
		glm::vec2 vmin= glm::vec2(rand_float(0.1f, 0.8f), rand_float(0.1f, 0.8f));
		glm::vec2 vmax= vmin+ glm::vec2(rand_float(0.05f, 0.1f), rand_float(0.05f, 0.1f));
		
		pts.push_back(glm::vec2(vmin.x, vmin.y));
		pts.push_back(glm::vec2(vmax.x, vmin.y));
		pts.push_back(glm::vec2(vmax.x, vmax.y));
		pts.push_back(glm::vec2(vmin.x, vmax.y));
		
		constrained_edges.push_back(make_pair(4+ i* 4+ 0, 4+ i* 4+ 1));
		constrained_edges.push_back(make_pair(4+ i* 4+ 1, 4+ i* 4+ 2));
		constrained_edges.push_back(make_pair(4+ i* 4+ 2, 4+ i* 4+ 3));
		constrained_edges.push_back(make_pair(4+ i* 4+ 3, 4+ i* 4+ 0));
	}
}


void test3() {
	for (unsigned int i=0; i<100; ++i) {
		pts.push_back(glm::vec2(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f)));
	}
	for (unsigned int i=0; i<10; ++i) {
		constrained_edges.push_back(make_pair(rand_int(0, pts.size()- 1), rand_int(0, pts.size()- 1)));
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
	srand(time(NULL));
	auto t1= high_resolution_clock::now();

	test2();
	Triangulation * tgl= new Triangulation(pts, constrained_edges, true, true, true);

	auto t2= high_resolution_clock::now();
	auto ms= duration_cast<milliseconds>(t2- t1);
	cout << ms.count() << " ms\n";

	tgl->draw("../data/result.html", true);
	delete tgl;


/*
	glm::vec2 pts[4]= {glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(0.1f, 0.1f), glm::vec2(0.0f, 1.0f)};
	bool x= is_quad_convex(pts);
	cout << x << "\n";
*/

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
