
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
#include "voronoi.h"


using namespace std;
using namespace std::chrono;

vector<glm::vec2> pts;


void test1() {
	pts.push_back(glm::vec2(0.0f, 0.0f));
	pts.push_back(glm::vec2(1.0f, 0.0f));
	pts.push_back(glm::vec2(1.0f, 1.0f));
	pts.push_back(glm::vec2(0.0f, 1.0f));

	pts.push_back(glm::vec2(0.3f, 0.3f));
	pts.push_back(glm::vec2(0.6f, 0.3f));
	pts.push_back(glm::vec2(0.6f, 0.6f));
	pts.push_back(glm::vec2(0.3f, 0.6f));
}


void test2() {
	for (unsigned int i=0; i<100; ++i) {
		pts.push_back(glm::vec2(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f)));
	}

	ofstream f;
	f.open("../data/debug.txt");
	for (auto pt : pts) {
		f << pt.x << " " << pt.y << "\n";
	}
	f.close();
}


void test3() {
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

	test1();
	Voronoi * v= new Voronoi(pts);

	auto t2= high_resolution_clock::now();
	auto ms= duration_cast<milliseconds>(t2- t1);
	cout << ms.count() << " ms\n";

	//v->draw("../data/result.html", true);
	delete v;

	return 0;
}