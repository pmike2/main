#include "graph.h"

using namespace std;


Graph::Graph() {

}


Graph::~Graph() {
	
}

void Graph::add_vertex(unsigned int i, pt_type_3d pos, number weight) {
	if (!_vertices.count(i)) {
		GraphVertex v= {};
		v._weight= weight;
		v._pos= pos;
		//v._active= true;
		_vertices[i]= v;
	}
}


void Graph::add_edge(unsigned int i, unsigned int j, number weight, bool weight_is_dist) {
	if ((_vertices.count(i)) && (_vertices.count(j))) {
		GraphEdge e= {};
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
	//result.reserve(_vertices[i]._edges.size());
	_it_e= _vertices[i]._edges.begin();
	while (_it_e!= _vertices[i]._edges.end()) {
		//if (_vertices[_it_e->first]._active) {
			result.push_back(_it_e->first);
		//}
		_it_e++;
	}
	return result;
}


void Graph::clear() {
	_vertices.clear();
}


/*void Graph::rand() {
	unsigned int n_vertices= 100;

	clear();

	for (unsigned int i=0; i<n_vertices; ++i) {
		number x= rand_number(-5.0f, 5.0f);
		number y= rand_number(-5.0f, 5.0f);
		add_vertex(i, 0.0f, x, y);
	}

	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		for (unsigned int i=0; i<100; ++i) {
			unsigned int a= rand_int(0, n_vertices- 1);
			number d= glm::distance(_it_v->second._pos, _vertices[a]._pos);
			if (d< 2.0f) {
				add_edge(_it_v->first, a, 0.0f, true);
			}
		}
		_it_v++;
	}
}*/


void Graph::reinit_weights() {
	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		_it_v->second._weight= 1.0f;
		//_it_v->second._active= true;
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
