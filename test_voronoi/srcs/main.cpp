
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <string>
#include <vector>
#include <chrono>
#include <csignal>
#include <map>


#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>


#include "utile.h"
#include "voronoi.h"


using namespace std;
using namespace std::chrono;

vector<glm::vec2> pts;
bool end_loop= false;


void test1() {
	std::map<std::string, std::vector<glm::vec2> > m;
	
	// jeu test de base, sans ambiguité
	m["simple"]= std::vector<glm::vec2> {glm::vec2(0.1, 0.7), glm::vec2(0.2, 0.8), glm::vec2(0.4, 0.1), glm::vec2(0.5, 0.4)};
	// jeu test où un site est situé pile en dessous d'un bkpt
	m["site_bkpt_x_align"]= std::vector<glm::vec2> {glm::vec2(0.1, 0.7), glm::vec2(0.2, 0.8), glm::vec2(0.3, 0.3), glm::vec2(0.5, 0.4)};
	// jeu test où les sites les + hauts sont alignés
	m["higher_y_align"]= std::vector<glm::vec2> {glm::vec2(0.4, 0.6), glm::vec2(0.1, 0.6), glm::vec2(0.6, 0.6), glm::vec2(0.3, 0.2)};
	// jeu test où des sites sont alignés mais pas tout en haut
	m["y_align"]= std::vector<glm::vec2> {glm::vec2(0.4, 0.8), glm::vec2(0.1, 0.6), glm::vec2(0.6, 0.6), glm::vec2(0.3, 0.6)};
	// jeu test avec 2 fois le même site
	m["site_doublon"]= std::vector<glm::vec2> {glm::vec2(0.4, 0.8), glm::vec2(0.1, 0.6), glm::vec2(0.1, 0.6), glm::vec2(0.3, 0.2)};
	// jeu test carré
	m["square"]= std::vector<glm::vec2> {glm::vec2(0.1, 0.1), glm::vec2(0.3, 0.1), glm::vec2(0.1, 0.3), glm::vec2(0.3, 0.3)};
	// racine niemes de l'unité
	int n1= 13;
	m["unit_root"]= std::vector<glm::vec2> {};
	for (int i=0; i<n1; ++i) {
		m["unit_root"].push_back(glm::vec2(cos(2.0* M_PI* (double)(i)/ (double)(n1)), sin(2.0* M_PI* (double)(i)/ (double)(n1))));
	}
	// racine niemes de l'unité avec centre
	int n2= 5;
	m["unit_root_center"]= std::vector<glm::vec2> {};
	for (int i=0; i<n2; ++i) {
		m["unit_root_center"].push_back(glm::vec2(cos(2.0* M_PI* (double)(i)/ (double)(n2)), sin(2.0* M_PI* (double)(i)/ (double)(n2))));
	}
	m["unit_root_center"].push_back(glm::vec2(0.0, 0.0));

	for (auto const & x : m) {
		if (x.first!= "simple") {continue;}
		Voronoi * v= new Voronoi(x.second, true, "../data/test1/"+ x.first);
		//Voronoi * v= new Voronoi(pts);

		glm::vec2 bbox_min, bbox_max;
		v->_diagram->get_bbox(&bbox_min, &bbox_max);
		v->_diagram->export_html("../data/test1/"+ x.first+ "/result.html", true, bbox_min- glm::vec2(-0.1f, -0.1f), bbox_max+ glm::vec2(0.1f, 0.1f), x.second);
		delete v;
	}
}


void test2() {
	int n_pts= 3000;
	float xmin= 0.0f;
	float xmax= 1.0f;
	float ymin= 0.0f;
	float ymax= 1.0f;
	for (unsigned int i=0; i<n_pts; ++i) {
		pts.push_back(glm::vec2(rand_float(xmin, xmax), rand_float(ymin, ymax)));
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
	//Voronoi * v= new Voronoi(pts, true, "../data/test2");
	auto t2= high_resolution_clock::now();
	auto ms= duration_cast<milliseconds>(t2- t1);
	cout << ms.count() << " ms\n";

	glm::vec2 bbox_min, bbox_max;
	v->_diagram->get_bbox(&bbox_min, &bbox_max);
	v->_diagram->export_html("../data/test2/result.html", true, bbox_min- glm::vec2(-0.1f, -0.1f), bbox_max+ glm::vec2(0.1f, 0.1f), pts);

	delete v;
}


void test3() {
	int n_pts= 100;
	float xmin= 0.0f;
	float xmax= 0.5f;
	float ymin= 0.0f;
	float ymax= 0.5f;
	int compt= 0;
	while (true) {
		std::cout << compt++ << "\n";
		pts.clear();
		for (unsigned int i=0; i<n_pts; ++i) {
			pts.push_back(glm::vec2(rand_float(xmin, xmax), rand_float(ymin, ymax)));
		}

		ofstream f;
		f.open("../data/test3/pts.txt");
		for (auto pt : pts) {
			f << pt.x << " " << pt.y << "\n";
		}
		f.close();

		Voronoi * v= new Voronoi(pts);
		
		glm::vec2 bbox_min, bbox_max;
		v->_diagram->get_bbox(&bbox_min, &bbox_max);
		v->_diagram->export_html("../data/test3/result.html", true, bbox_min- glm::vec2(-0.1f, -0.1f), bbox_max+ glm::vec2(0.1f, 0.1f), pts);
		if (!v->_diagram->is_valid()) {
			break;
		}
		delete v;
	}
}


void test4() {
	ifstream f("../data/test2/pts.txt");
	//ifstream f("../data/test3/pts.txt");
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

	glm::vec2 bbox_min, bbox_max;
	v->_diagram->get_bbox(&bbox_min, &bbox_max);
	v->_diagram->export_html("../data/test4/result.html", true, bbox_min- glm::vec2(-0.1f, -0.1f), bbox_max+ glm::vec2(0.1f, 0.1f), pts);
	delete v;
}


// ------------------------------------------------------------------------
int main(int argc, char * argv[]) {
	srand(time(NULL));

	test1();
	//test2();
	//test3();
	//test4();

	return 0;
}
