
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <string>
#include <vector>
#include <chrono>
#include <csignal>


#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>


#include "utile.h"
#include "voronoi.h"


using namespace std;
using namespace std::chrono;

vector<glm::vec2> pts;
bool end_loop= false;


void test1() {
	pts.push_back(glm::vec2(0.1369, 0.162));
	pts.push_back(glm::vec2(0.9261, 0.9271));
	pts.push_back(glm::vec2(0.0051, 0.0552));
	pts.push_back(glm::vec2(0.0873, 0.8768));

	Voronoi * v= new Voronoi(pts, "../data/test1");
	v->_diagram->export_html("../data/test1/result.html", false);
	
	delete v;
}


void test2() {
	int n_pts= 100;
	for (unsigned int i=0; i<n_pts; ++i) {
		pts.push_back(glm::vec2(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f)));
	}

	auto t1= high_resolution_clock::now();
	Voronoi * v= new Voronoi(pts, "../data/test2");
	auto t2= high_resolution_clock::now();
	auto ms= duration_cast<milliseconds>(t2- t1);
	cout << ms.count() << " ms\n";

	v->_diagram->export_html("../data/test2/result.html", true, -0.1f, -0.1f, 1.1f, 1.1f);

	delete v;
}


void test3() {
	std::signal(SIGTERM, [](int signal){
		//std::cout << "signal term = " << signal << "\n";
		end_loop= true;
		/*for (auto & pt : pts) {
			std::cout << pt.x << ", " << pt.y << "\n";
		}*/
	});

	std::signal(SIGABRT, [](int signal){
		//std::cout << "signal abrt  = " << signal << "\n";
	});

	// Ctrl-C
	std::signal(SIGINT, [](int signal){
		//std::cout << "SIGINT\n";
		end_loop= true;
	});

	int compt= 0;
	int n_pts= 10;
	while (true) {
		std::cout << "-----------------------\n";
		std::cout << compt++ << "\n";
		pts.clear();
		for (unsigned int i=0; i<n_pts; ++i) {
			pts.push_back(glm::vec2(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f)));
			std::cout << pts[i].x << ", " << pts[i].y << " | ";
		}
		std::cout << "\n";
		Voronoi * v= new Voronoi(pts, "../data/test3");
		delete v;

		if (end_loop) {
			break;
		}
	}
	/*ofstream f;
	f.open("../data/debug.txt");
	for (auto pt : pts) {
		f << pt.x << " " << pt.y << "\n";
	}
	f.close();*/
}


void test4() {
	ifstream f("../data/pts.txt");
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

	//test1();
	test2();
	//test3();

	return 0;
}
