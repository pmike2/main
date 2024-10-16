
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
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>


#include "typedefs.h"
#include "utile.h"
#include "voronoi.h"


using namespace std;
using namespace std::chrono;

vector<pt_type> pts;
bool end_loop= false;


void test1() {
	std::map<std::string, std::vector<pt_type> > m;
	
	// jeu test de base, sans ambiguité
	m["simple"]= std::vector<pt_type> {pt_type(0.1, 0.7), pt_type(0.2, 0.8), pt_type(0.4, 0.1), pt_type(0.5, 0.4)};
	// jeu test où un site est situé pile en dessous d'un bkpt
	m["site_bkpt_x_align"]= std::vector<pt_type> {pt_type(0.1, 0.7), pt_type(0.2, 0.8), pt_type(0.3, 0.3), pt_type(0.5, 0.4)};
	// jeu test où les sites les + hauts sont alignés
	m["higher_y_align"]= std::vector<pt_type> {pt_type(0.4, 0.6), pt_type(0.1, 0.6), pt_type(0.6, 0.6), pt_type(0.3, 0.2)};
	// jeu test où des sites sont alignés mais pas tout en haut
	m["y_align"]= std::vector<pt_type> {pt_type(0.4, 0.8), pt_type(0.1, 0.6), pt_type(0.6, 0.6), pt_type(0.3, 0.6)};
	// jeu test avec 2 fois le même site
	m["site_doublon"]= std::vector<pt_type> {pt_type(0.4, 0.8), pt_type(0.1, 0.6), pt_type(0.1, 0.6), pt_type(0.3, 0.2)};
	// racine niemes de l'unité
	int n1= 5;
	m["unit_root"]= std::vector<pt_type> {};
	for (int i=0; i<n1; ++i) {
		m["unit_root"].push_back(pt_type(cos(2.0* M_PI* (double)(i)/ (double)(n1)), sin(2.0* M_PI* (double)(i)/ (double)(n1))));
	}
	// racine niemes de l'unité avec centre
	int n2= 7;
	m["unit_root_center"]= std::vector<pt_type> {};
	for (int i=0; i<n2; ++i) {
		m["unit_root_center"].push_back(pt_type(cos(2.0* M_PI* (double)(i)/ (double)(n2)), sin(2.0* M_PI* (double)(i)/ (double)(n2))));
	}
	m["unit_root_center"].push_back(pt_type(0.0, 0.0));
	// grille
	int n3= 3;
	m["grid"]= std::vector<pt_type> {};
	for (int i=0; i<n3; ++i) {
		for (int j=0; j<n3; ++j) {
			m["grid"].push_back(pt_type(number(i)/ number(n3), number(j)/ number(n3)));
		}
	}

	for (auto const & x : m) {
		//if (x.first!= "higher_y_align") {continue;}

		std::cout << "TEST1 " << x.first << " ----------------------\n";
		Voronoi * v= new Voronoi(x.second, true, true, true, true, "../data/test1/"+ x.first);

		pt_type bbox_min, bbox_max;
		v->_diagram->get_bbox(&bbox_min, &bbox_max);
		v->_diagram->export_html("../data/test1/"+ x.first+ "/result.html", false, bbox_min- pt_type(0.2f), bbox_max+ pt_type(0.2f), x.second);
		delete v;
	}
}


void test2() {
	int n_pts= 2000;
	number xmin= 0.0;
	number xmax= 100.0;
	number ymin= 0.0;
	number ymax= 100.0;
	number min_dist= 0.3;
	
	for (unsigned int i=0; i<n_pts; ++i) {
		pt_type pt(rand_number(xmin, xmax), rand_number(ymin, ymax));
		bool ok= true;
		/*for (auto pt2 : pts) {
			if (glm::distance2(pt, pt2)< min_dist* min_dist) {
				ok= false;
				break;
			}
		}*/
		if (ok) {
			pts.push_back(pt);
		}
	}
	
	/*for (unsigned int i=0; i<n_pts; ++i) {
		pt_type pt(rand_number(xmin, xmax), number(i)* 0.1);
		pts.push_back(pt);
	}*/

	std::sort(pts.begin(), pts.end(), [](const pt_type &a, const pt_type &b) { return a.y > b.y; });
	ofstream f;
	f.open("../data/test4/pts.txt");
	for (auto pt : pts) {
		f << pt.x << " " << pt.y << "\n";
	}
	f.close();

	Voronoi * v= new Voronoi(pts, false, false, false, true, "../data/test2");

	pt_type bbox_min, bbox_max;
	v->_diagram->get_bbox(&bbox_min, &bbox_max);
	v->_diagram->export_html("../data/test2/result.html", true, bbox_min- pt_type(0.2), bbox_max+ pt_type(0.2), pts);

	//v->_beachline->draw("../data/test2/beachline.pbm");

	//std::cout << "smallest edge = " << v->_diagram->smallest_edge() << "\n";

	delete v;
}


void test3() {
	int n_pts= 100;
	number xmin= 0.0;
	number xmax= 0.5;
	number ymin= 0.0;
	number ymax= 0.5;
	int compt= 0;
	while (true) {
		std::cout << compt++ << "\n";
		pts.clear();
		for (unsigned int i=0; i<n_pts; ++i) {
			pts.push_back(pt_type(rand_number(xmin, xmax), rand_number(ymin, ymax)));
		}

		ofstream f;
		f.open("../data/test3/pts.txt");
		for (auto pt : pts) {
			f << pt.x << " " << pt.y << "\n";
		}
		f.close();

		Voronoi * v= new Voronoi(pts);
		
		pt_type bbox_min, bbox_max;
		v->_diagram->get_bbox(&bbox_min, &bbox_max);
		v->_diagram->export_html("../data/test3/result.html", true, bbox_min- pt_type(0.5), bbox_max+ pt_type(0.5), pts);
		if (!v->_diagram->is_valid()) {
			break;
		}
		delete v;
	}
}


void test4() {
	ifstream f("../data/test4/pts.txt");
	while (f.good()) {
		string line;
		getline(f, line);
		istringstream iss(line);
		number x, y;
		iss >> x >> y;
		pt_type pt(x, y);
		pts.push_back(pt);
	}

	Voronoi * v= new Voronoi(pts);

	pt_type bbox_min, bbox_max;
	v->_diagram->get_bbox(&bbox_min, &bbox_max);
	v->_diagram->export_html("../data/test4/result.html", true, bbox_min- pt_type(0.5), bbox_max+ pt_type(0.5), pts);
	delete v;
}


void test5() {
	pt_type site_left(3, 9.98);
	pt_type site_right(2, 9.996);
	number yline= 9.975;
	number x= parabolas_intersection(site_left, site_right, yline);
	std::cout << x << "\n";
}


// ------------------------------------------------------------------------
int main(int argc, char * argv[]) {
	srand(time(NULL));

	//test1();
	test2();
	//test3();
	//test4();
	//test5();

	return 0;
}
