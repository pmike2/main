#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <algorithm>
#include <chrono>

#include <glm/glm.hpp>

#include "utile.h"
#include "geom_2d.h"


using namespace std;
using namespace std::chrono;


// ------------------------------------------------------------------------
struct Edge {
	//unsigned int _start_node;
	//unsigned int _end_node;
	float _weight;
};


struct Vertex {
	float _weight;
	glm::vec2 _pos;
	unordered_map<unsigned int, Edge> _edges;
};


struct Graph {
	unordered_map<unsigned int, Vertex> _vertices;
	unordered_map<unsigned int, Vertex>::iterator _it_v;
	unordered_map<unsigned int, Edge>::iterator _it_e;


	Graph() {

	}


	~Graph() {
		clear();
	}


	void add_vertex(unsigned int i, float weight=0.0f, float x=0.0f, float y=0.0f) {
		if (!_vertices.count(i)) {
			Vertex v= {};
			v._weight= weight;
			v._pos.x= x;
			v._pos.y= y;
			_vertices[i]= v;
		}
	}


	void add_edge(unsigned int i, unsigned int j, float weight=0.0f, bool weight_is_dist=false) {
		if ((_vertices.count(i)) && (_vertices.count(j))) {
			Edge e= {};
			if (weight_is_dist) {
				e._weight= glm::distance(_vertices[i]._pos, _vertices[j]._pos);
			}
			else {
				e._weight= weight;
			}
			
			_vertices[i]._edges[j]= e;
		}
	}


	void remove_vertex(unsigned int i) {
		if (_vertices.count(i)) {
			_vertices.erase(i);

			// lent
			//for (pair<unsigned int, Vertex> v : _vertices) {
				//v.second._edges.erase(i);
			//}

			_it_v= _vertices.begin();
			while (_it_v!= _vertices.end()) {
				if (_it_v->second._edges.count(i)) {
					_it_v->second._edges.erase(i);
				}
				_it_v++;
			}
		}
	}


	void remove_edge(unsigned int i, unsigned int j) {
		if ((_vertices.count(i)) && (_vertices.count(j))) {
			_vertices[i]._edges.erase(j);
		}
	}


	vector<unsigned int> neighbors(unsigned int i) {
		vector<unsigned int> result;
		result.reserve(_vertices[i]._edges.size());
		_it_e= _vertices[i]._edges.begin();
		while (_it_e!= _vertices[i]._edges.end()) {
			result.push_back(_it_e->first);
			_it_e++;
		}
		return result;
	}


	float cost(unsigned int i, unsigned int j) {
		return _vertices[i]._edges[j]._weight;
	}


	void clear() {
		_vertices.clear();
	}


	void rand_1() {
		unsigned int n_vertices= 100;

		clear();

		for (unsigned int i=0; i<n_vertices; ++i) {
			float x= rand_float(-5.0f, 5.0f);
			float y= rand_float(-5.0f, 5.0f);
			add_vertex(i, 0.0f, x, y);
		}

		_it_v= _vertices.begin();
		while (_it_v!= _vertices.end()) {
			for (unsigned int i=0; i<100; ++i) {
				unsigned int a= rand_int(0, n_vertices- 1);
				float d= glm::distance(_it_v->second._pos, _vertices[a]._pos);
				if (d< 2.0f) {
					add_edge(_it_v->first, a, 0.0f, true);
				}
			}
			_it_v++;
		}
	}


	friend ostream & operator << (ostream & os, Graph & g) {
		os << "graph -----" << "\n";
		os << "n_vertices = " << g._vertices.size() << "\n";
		g._it_v= g._vertices.begin();
		while (g._it_v!= g._vertices.end()) {
			os << "vertex : id=" << g._it_v->first << "\n";
			vector<unsigned int> l= g.neighbors(g._it_v->first);
			for (auto neighbor : l) {
				cout << "\t-> " << neighbor << "\n";
			}
			g._it_v++;
		}
		os << "-----------\n";
		return os;
	}
};


struct Grid : public Graph {
	unsigned int _width;
	unsigned int _height;


	Grid() : Graph() {

	}


	Grid(unsigned int width, unsigned int height) : Graph(), _width(width), _height(height) {
		for (unsigned int i=0; i<_width; ++i) {
			for (unsigned int j=0; j<_height; ++j) {
				unsigned int id= i+ _width* j;
				float x= -5.0f+ ((float)(i)/ (float)(_width))* 10.0f;
				float y= -5.0f+ ((float)(j)/ (float)(_height))* 10.0f;
				add_vertex(id, 0.0f, x, y);
			}
		}
		
		_it_v= _vertices.begin();
		while (_it_v!= _vertices.end()) {
			unsigned int i= _it_v->first% _width;
			unsigned int j= _it_v->first/ _width;
			if (i> 0) {
				add_edge(_it_v->first, _it_v->first- 1, 0.0f, true);
			}
			if (i< _width- 1) {
				add_edge(_it_v->first, _it_v->first+ 1, 0.0f, true);
			}
			if (j> 0) {
				add_edge(_it_v->first, _it_v->first- _width, 0.0f, true);
			}
			if (j< _height- 1) {
				add_edge(_it_v->first, _it_v->first+ _width, 0.0f, true);
			}
			_it_v++;
		}
	}


	~Grid() {

	}
};


struct Level {
	Grid * _grid;
	vector<Polygon2D *> _polygons;


	Level() {
		_grid= new Grid(10, 8);
		for (unsigned int i=0; i<4; ++i) {
			Polygon2D * poly= new Polygon2D();
			float x= rand_float(-5.0f, 5.0f);
			float y= rand_float(-5.0f, 5.0f);
			cout << x << " ; " << y << "\n";
			poly->randomize(10, 1.0f, glm::vec2(x, y));
			_polygons.push_back(poly);
		}

		_grid->_it_v= _grid->_vertices.begin();
		while (_grid->_it_v!= _grid->_vertices.end()) {
			bool erased= false;
			for (auto poly : _polygons) {
				if (is_pt_inside_poly(_grid->_it_v->second._pos, poly)) {
					_grid->remove_vertex(_grid->_it_v->first);
					erased= true;
					cout << "erased\n";
					break;
				}
			}
			if (!erased) {
				_grid->_it_v++;
			}
		}
	}


	~Level() {
		delete _grid;
	}
};


// fonctions --------------------------------------------------------
bool frontier_cmp(pair<unsigned int, float> x, pair<unsigned int, float> y) {
	return x.second> y.second;
}


vector<unsigned int> search(Graph * g, unsigned int start, unsigned int goal) {
	priority_queue< pair<unsigned int, float>, vector<pair<unsigned int, float> >, decltype(&frontier_cmp) > frontier(frontier_cmp);
	unordered_map<unsigned int, unsigned int> came_from;
	unordered_map<unsigned int, float> cost_so_far;

	frontier.emplace(start, 0.0f);
	came_from[start]= start;
	cost_so_far[start]= 0.0f;

	while (!frontier.empty()) {
		unsigned int current= frontier.top().first;
		frontier.pop();

		if (current== goal) {
			break;
		}

		vector<unsigned int> nexts= g->neighbors(current);
		for (auto next : nexts) {
			float new_cost= cost_so_far[current]+ g->cost(current, next);
			if ((!cost_so_far.count(next)) || (new_cost< cost_so_far[next])) {
				cost_so_far[next]= new_cost;
				came_from[next]= current;
				frontier.emplace(next, new_cost);
			}
		}
	}

	vector<unsigned int> path;

	if (!came_from.count(goal)) {
		cout << "disconnected\n";
		return path;
	}

	unsigned int current= goal;
	while (current!= start) {
		path.push_back(current);
		current= came_from[current];
	}
	path.push_back(start);
	reverse(path.begin(), path.end());
	
	return path;
}


void draw_svg(Level * l, vector<unsigned int> path) {
	ofstream f;
	f.open("../data/graph.html");
	f << "<!DOCTYPE html>\n<html>\n<body>\n";
	f << "<svg width=\"1000\" height=\"1000\" viewbox=\"-10.0 -10.0 20.0 20.0\">\n";

	Grid * g= l->_grid;

	g->_it_v= g->_vertices.begin();
	while (g->_it_v!= g->_vertices.end()) {
		float x1= g->_it_v->second._pos.x;
		float y1= g->_it_v->second._pos.y;
		f << "<circle cx=\"" << x1 << "\" cy=\"" << y1 << "\" r=\"0.02\" fill=\"black\" />\n";
		//f << "<text x=\"" << x1+ 0.15f << "\" y=\"" << y1- 0.15f << "\" fill=\"black\" font-size=\"0.2px\">" << to_string(g->_it_v->first) << "</text>\n";

		g->_it_e= g->_it_v->second._edges.begin();
		while (g->_it_e!= g->_it_v->second._edges.end()) {
			float x2= g->_vertices[g->_it_e->first]._pos.x;
			float y2= g->_vertices[g->_it_e->first]._pos.y;
			f << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" stroke=\"black\" stroke-width=\"0.01\" />\n";
			g->_it_e++;
		}

		g->_it_v++;
	}
	
	if (path.size()) {
		for (unsigned int i=0; i<path.size()- 1; ++i) {
			float x1= g->_vertices[path[i]]._pos.x;
			float y1= g->_vertices[path[i]]._pos.y;
			float x2= g->_vertices[path[i+ 1]]._pos.x;
			float y2= g->_vertices[path[i+ 1]]._pos.y;
			f << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" stroke=\"red\" stroke-width=\"0.02\" />\n";
		}
	}

	for (auto poly : l->_polygons) {
		f << "<polygon points=\"";
		for (auto pt : poly->_pts) {
			f << pt.x << "," << pt.y << " ";
		}
		f << "\" fill=\"none\" stroke=\"purple\" stroke-width=\"0.05\" />\n";
	}

	f << "</svg>\n</body>\n</html>\n";
	f.close();
}


// tests --------------------------------------------------------------------
void test1() {
	Graph * g= new Graph();
	g->add_vertex(0);
	g->add_vertex(1);
	g->add_vertex(2);
	g->add_edge(0, 1);
	g->add_edge(0, 2);
	cout << *g << "\n";
	/*g->remove_edge(0, 1);
	cout << *g << "\n";
	g->remove_vertex(0);
	cout << *g << "\n";*/
}


void test2() {
	Graph * g= new Graph();
	unsigned int n_vertices= 10000;
	unsigned int n_edges= 100000;
	for (unsigned int i=0; i<n_vertices; ++i) {
		g->add_vertex(i);
	}
	for (unsigned int i=0; i<n_edges; ++i) {
		unsigned int a= (unsigned int)(rand() % n_vertices);
		unsigned int b= (unsigned int)(rand() % n_vertices);
		//cout << a << " ; " << b << "\n";
		g->add_edge(a, b);
		//g->remove_edge(a, b);
	}

	for (unsigned int i=0; i<n_vertices; ++i) {
		//g->remove_vertex(i);
	}
	cout << *g << "\n";
}


void test3() {
	Graph * g= new Graph();
	
	g->add_vertex(0, 0.0f, -5.0f, 0.0f);
	g->add_vertex(1, 0.0f, 0.0f, 3.0f);
	g->add_vertex(2, 0.0f, 0.0f, -3.0f);
	g->add_vertex(3, 0.0f, 5.0f, 0.0f);

	g->add_edge(0, 1, 1.0f);
	g->add_edge(0, 2, 2.0f);
	g->add_edge(1, 3, 1.0f);
	g->add_edge(2, 3, 1.0f);
	
	vector<unsigned int> path= search(g, 0, 3);
	for (auto it : path) {
		cout << it << " ; ";
	}
	cout << "\n";
	//draw_svg(g, path);
}


void test4() {
	Graph * g= new Graph();
	g->rand_1();
	vector<unsigned int> path= search(g, 0, 1);
	//draw_svg(g, path);
	delete g;
}


void test5() {
	Grid * g= new Grid(30, 40);
	vector<unsigned int> path= search(g, 0, 1000);
	//draw_svg(g, path);
	delete g;
}


void test6() {
	Level * l= new Level();
	vector<unsigned int> path= search(l->_grid, 0, 30);
	draw_svg(l, path);
	delete l;
}


// main ----------------------------------------------------------------
int main(int argc, char *argv[]) {
	srand(time(NULL));
	auto t1= high_resolution_clock::now();

	//test1();
	//test2();
	//test3();
	//test4();
	//test5();
	test6();
	
	auto t2= high_resolution_clock::now();
	auto ms= duration_cast<milliseconds>(t2- t1);
	cout << ms.count() << " ms\n";

	return 0;
}
