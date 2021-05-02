#include <queue>
#include <fstream>

#include "path_find.h"
#include "utile.h"


using namespace std;


// ----------------------------------------------------------------------------------------
Graph::Graph() {

}


Graph::~Graph() {
	
}

void Graph::add_vertex(unsigned int i, float weight, float x, float y) {
	if (!_vertices.count(i)) {
		Vertex v= {};
		v._weight= weight;
		v._pos.x= x;
		v._pos.y= y;
		v._visited= false;
		_vertices[i]= v;
	}
}


void Graph::add_edge(unsigned int i, unsigned int j, float weight, bool weight_is_dist) {
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


void Graph::remove_vertex(unsigned int i) {
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


void Graph::remove_edge(unsigned int i, unsigned int j) {
	if ((_vertices.count(i)) && (_vertices.count(j))) {
		_vertices[i]._edges.erase(j);
	}
}


vector<unsigned int> Graph::neighbors(unsigned int i) {
	vector<unsigned int> result;
	result.reserve(_vertices[i]._edges.size());
	_it_e= _vertices[i]._edges.begin();
	while (_it_e!= _vertices[i]._edges.end()) {
		result.push_back(_it_e->first);
		_it_e++;
	}
	return result;
}


float Graph::cost(unsigned int i, unsigned int j) {
	//return _vertices[i]._edges[j]._weight;
	// theta * empeche d'utiliser edge._weight car i et j ne sont pas forcement voisins
	//return glm::distance(_vertices[i]._pos, _vertices[j]._pos)+ _vertices[j]._weight;
	return _vertices[j]._weight;
}


float Graph::heuristic(unsigned int i, unsigned int j) {
	//return abs(_vertices[i]._pos.x- _vertices[j]._pos.x)+ abs(_vertices[i]._pos.y- _vertices[j]._pos.y);
	return glm::distance(_vertices[i]._pos, _vertices[j]._pos);
}


void Graph::clear() {
	_vertices.clear();
}


void Graph::rand() {
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


ostream & operator << (ostream & os, Graph & g) {
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


// ----------------------------------------------------------------------------------------
Grid::Grid() {

}


Grid::Grid(unsigned int width, unsigned int height) : Graph(), _width(width), _height(height) {
	for (unsigned int i=0; i<_width; ++i) {
		for (unsigned int j=0; j<_height; ++j) {
			unsigned int id= i+ _width* j;
			//float weight= rand_float(1.0f, 10.0f);
			float weight= 1.0f;
			/*if (rand_bool()) {
				weight= 10.0f;
			}*/
			float x= -5.0f+ ((float)(i)/ (float)(_width))* 10.0f;
			float y= -5.0f+ ((float)(j)/ (float)(_height))* 10.0f;
			add_vertex(id, weight, x, y);
		}
	}
	
	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		unsigned int i= _it_v->first% _width;
		unsigned int j= _it_v->first/ _width;
		if (i> 0) {
			add_edge(_it_v->first, _it_v->first- 1, 1.0f, false);
		}
		if (i< _width- 1) {
			add_edge(_it_v->first, _it_v->first+ 1, 1.0f, false);
		}
		if (j> 0) {
			add_edge(_it_v->first, _it_v->first- _width, 1.0f, false);
		}
		if (j< _height- 1) {
			add_edge(_it_v->first, _it_v->first+ _width, 1.0f, false);
		}

		// 8 connexity
		if ((i> 0) && (j> 0)) {
			add_edge(_it_v->first, _it_v->first- 1- _width, 1.0f, false);
		}
		if ((i< _width- 1) && (j> 0)) {
			add_edge(_it_v->first, _it_v->first+ 1- _width, 1.0f, false);
		}
		if ((i> 0) && (j< _height- 1)) {
			add_edge(_it_v->first, _it_v->first- 1+ _width, 1.0f, false);
		}
		if ((i< _width- 1) && (j< _height- 1)) {
			add_edge(_it_v->first, _it_v->first+ 1+ _width, 1.0f, false);
		}

		_it_v++;
	}
}


Grid::~Grid() {

}


ostream & operator << (ostream & os, Grid & g) {
	os << static_cast<Graph &>(g);
	return os;
}


// ----------------------------------------------------------------------------------------
bool frontier_cmp(pair<unsigned int, float> x, pair<unsigned int, float> y) {
	return x.second> y.second;
}


Level::Level() {

}


Level::Level(unsigned int width, unsigned int height) {
	_grid= new Grid(width, height);
	
	
	for (unsigned int i=0; i<10; ++i) {
		Polygon2D * poly= new Polygon2D();
		float x= rand_float(-5.0f, 5.0f);
		float y= rand_float(-5.0f, 5.0f);
		poly->randomize(10, 1.0f, glm::vec2(x, y));
		_polygons.push_back(poly);
	}
	

	/*
	Polygon2D * poly= new Polygon2D();
	unsigned int n_pts= 3;
	float pts[]= {-4.0f, -4.0f, -2.0f, -4.0f, -3.0f, 7.0f};
	poly->set_points(pts, n_pts);
	_polygons.push_back(poly);
	*/

	vector<unsigned int> vertices_to_erase;
	_grid->_it_v= _grid->_vertices.begin();
	while (_grid->_it_v!= _grid->_vertices.end()) {
		for (auto poly : _polygons) {
			//if (is_pt_inside_poly(_grid->_it_v->second._pos, poly)) {
			if (distance_poly_pt(poly, _grid->_it_v->second._pos, NULL)< 0.1f) {
				vertices_to_erase.push_back(_grid->_it_v->first);
				break;
			}
		}
		_grid->_it_v++;
	}
	for (auto it_erase : vertices_to_erase) {
		_grid->remove_vertex(it_erase);
	}

	vector<pair<unsigned int, unsigned int> > edges_to_erase;
	_grid->_it_v= _grid->_vertices.begin();
	while (_grid->_it_v!= _grid->_vertices.end()) {
		_grid->_it_e= _grid->_it_v->second._edges.begin();
		while (_grid->_it_e!= _grid->_it_v->second._edges.end()) {
			glm::vec2 pt_begin= _grid->_it_v->second._pos;
			glm::vec2 pt_end= _grid->_vertices[_grid->_it_e->first]._pos;
			for (auto poly : _polygons) {
				if (segment_intersects_poly(pt_begin, pt_end, poly, NULL)) {
					edges_to_erase.push_back(make_pair(_grid->_it_v->first, _grid->_it_e->first));
					break;
				}
			}
			_grid->_it_e++;
		}
		_grid->_it_v++;
	}
	for (auto it_erase : edges_to_erase) {
		_grid->remove_edge(it_erase.first, it_erase.second);
	}

}


Level::~Level() {
	delete _grid;
}


bool Level::line_of_sight(unsigned int i, unsigned int j) {
	glm::vec2 pt_begin= _grid->_vertices[i]._pos;
	glm::vec2 pt_end= _grid->_vertices[j]._pos;
	for (auto poly : _polygons) {
		//if (segment_intersects_poly(pt_begin, pt_end, poly, NULL)) {
		if (distance_poly_segment(poly, pt_begin, pt_end, NULL)< 0.1f) {
			return false;
		}
	}
	return true;
}


vector<unsigned int> Level::path_find(unsigned int start, unsigned int goal) {
	priority_queue< pair<unsigned int, float>, vector<pair<unsigned int, float> >, decltype(&frontier_cmp) > frontier(frontier_cmp);
	unordered_map<unsigned int, unsigned int> came_from;
	unordered_map<unsigned int, float> cost_so_far;

	frontier.emplace(start, 0.0f);
	came_from[start]= start;
	cost_so_far[start]= 0.0f;

	while (!frontier.empty()) {
		unsigned int current= frontier.top().first;
		frontier.pop();
		_grid->_vertices[current]._visited= true;

		if (current== goal) {
			break;
		}

		vector<unsigned int> nexts= _grid->neighbors(current);
		for (auto next : nexts) {
			unsigned int theta= current;
			/*if (l->line_of_sight(came_from[current], next)) {
				theta= came_from[current];
			}*/
			float new_cost= cost_so_far[theta]+ _grid->cost(theta, next);
			if ((!cost_so_far.count(next)) || (new_cost< cost_so_far[next])) {
				cost_so_far[next]= new_cost;
				came_from[next]= theta;
				//float priority= new_cost; // dijkstra
				//float priority= g->heuristic(next, goal); // greedy best first search
				float priority= new_cost+ _grid->heuristic(next, goal); // A *
				frontier.emplace(next, priority);
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


void Level::draw_svg(vector<unsigned int> path, string result) {
	ofstream f;
	f.open(result);
	f << "<!DOCTYPE html>\n<html>\n<body>\n";
	f << "<svg width=\"1000\" height=\"1000\" viewbox=\"-10.0 -10.0 20.0 20.0\">\n";

	if (path.size()) {
		for (unsigned int i=0; i<path.size()- 1; ++i) {
			float x1= _grid->_vertices[path[i]]._pos.x;
			float y1= _grid->_vertices[path[i]]._pos.y;
			float x2= _grid->_vertices[path[i+ 1]]._pos.x;
			float y2= _grid->_vertices[path[i+ 1]]._pos.y;
			f << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" stroke=\"red\" stroke-width=\"0.06\" />\n";
		}
	}

	for (auto poly : _polygons) {
		f << "<polygon points=\"";
		for (auto pt : poly->_pts) {
			f << pt.x << "," << pt.y << " ";
		}
		f << "\" fill=\"none\" stroke=\"purple\" stroke-width=\"0.02\" />\n";
	}

	_grid->_it_v= _grid->_vertices.begin();
	while (_grid->_it_v!= _grid->_vertices.end()) {
		float x1= _grid->_it_v->second._pos.x;
		float y1= _grid->_it_v->second._pos.y;
		string color= "black";
		if (_grid->_it_v->second._visited) {
			color= "cyan";
		}
		float radius= 0.01* _grid->_it_v->second._weight;
		f << "<circle cx=\"" << x1 << "\" cy=\"" << y1 << "\" r=\"" << radius << "\" fill=\"" << color << "\" />\n";
		//f << "<text x=\"" << x1+ 0.15f << "\" y=\"" << y1- 0.15f << "\" fill=\"black\" font-size=\"0.2px\">" << to_string(_grid->_it_v->first) << "</text>\n";

		_grid->_it_e= _grid->_it_v->second._edges.begin();
		while (_grid->_it_e!= _grid->_it_v->second._edges.end()) {
			float x2= _grid->_vertices[_grid->_it_e->first]._pos.x;
			float y2= _grid->_vertices[_grid->_it_e->first]._pos.y;
			f << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" stroke=\"black\" stroke-width=\"0.01\" />\n";
			_grid->_it_e++;
		}

		_grid->_it_v++;
	}
	
	f << "</svg>\n</body>\n</html>\n";
	f.close();
}
