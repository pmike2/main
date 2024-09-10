
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
	
	// jeu test où un site est situé pile en dessous d'un bkpt
	/*pts.push_back(glm::vec2(0.1, 0.7));
	pts.push_back(glm::vec2(0.2, 0.8));
	pts.push_back(glm::vec2(0.3, 0.3));
	pts.push_back(glm::vec2(0.5, 0.4));*/

	pts.push_back(glm::vec2(0.4, 0.6));
	pts.push_back(glm::vec2(0.1, 0.6));
	pts.push_back(glm::vec2(0.6, 0.6));
	pts.push_back(glm::vec2(0.3, 0.2));

	Voronoi * v= new Voronoi(pts, true, "../data/test1");
	v->_diagram->export_html("../data/test1/result.html", true, pts);
	
	delete v;
}


void test2() {
	int n_pts= 1000;
	for (unsigned int i=0; i<n_pts; ++i) {
		pts.push_back(glm::vec2(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f)));
		//std::cout << pts[i].x << ", " << pts[i].y << " | ";
	}

	std::sort(pts.begin(), pts.end(), [](const glm::vec2 &a, const glm::vec2 &b) { return a.y > b.y; });
	ofstream f;
	f.open("../data/test2/pts.txt");
	for (auto pt : pts) {
		f << pt.x << " " << pt.y << "\n";
	}
	f.close();

	auto t1= high_resolution_clock::now();
	Voronoi * v= new Voronoi(pts);
	//Voronoi * v= new Voronoi(pts, false, "../data/test2");
	auto t2= high_resolution_clock::now();
	auto ms= duration_cast<milliseconds>(t2- t1);
	cout << ms.count() << " ms\n";

	v->_diagram->export_html("../data/test2/result.html", true, -0.1f, -0.1f, 1.1f, 1.1f, pts);
	//v->_diagram->export_html("../data/test2/result.html", true, pts);

	delete v;
}


void test3() {
	/*std::signal(SIGTERM, [](int signal){
		end_loop= true;
	});

	std::signal(SIGABRT, [](int signal){
	});

	// Ctrl-C
	std::signal(SIGINT, [](int signal){
		end_loop= true;
	});*/

	int compt= 0;
	int n_pts= 10;
	while (true) {
		//std::cout << "-----------------------\n";
		std::cout << compt++ << "\n";
		pts.clear();
		for (unsigned int i=0; i<n_pts; ++i) {
			pts.push_back(glm::vec2(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f)));
			//std::cout << pts[i].x << ", " << pts[i].y << " | ";
		}
		//std::cout << "\n";

		ofstream f;
		f.open("../data/test3/pts.txt");
		for (auto pt : pts) {
			f << pt.x << " " << pt.y << "\n";
		}
		f.close();

		Voronoi * v= new Voronoi(pts);
		if (!v->_diagram->is_valid()) {
			break;
		}
		delete v;

		/*if (end_loop) {
			break;
		}*/
		if (compt> 100) {
			break;
		}
	}
}


void test4() {
	//ifstream f("../data/test2/pts.txt");
	ifstream f("../data/test3/pts.txt");
	while (f.good()) {
		string line;
		getline(f, line);
		istringstream iss(line);
		float x, y;
		iss >> x >> y;
		glm::vec2 pt(x, y);
		pts.push_back(pt);
	}

	Voronoi * v= new Voronoi(pts);
	//v->_diagram->export_html("../data/test2/result.html", true, -0.1f, -0.1f, 1.1f, 1.1f, pts);
	v->_diagram->export_html("../data/test3/result.html", true, -0.1f, -0.1f, 1.1f, 1.1f, pts);
	delete v;
}


// ------------------------------------------------------------------------
int main(int argc, char * argv[]) {
	srand(time(NULL));

	//test1();
	//test2();
	//test3();
	test4();

	return 0;
}
