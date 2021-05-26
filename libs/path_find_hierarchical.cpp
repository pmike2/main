#include "path_find_hierarchical.h"

using namespace std;



// ------------------------------------------------------------
HPAEdge::HPAEdge() {

}


HPAEdge::HPAEdge(float weight) : _weight(weight) {

}


HPAEdge::~HPAEdge() {

}


// ------------------------------------------------------------
HPAVertex::HPAVertex() {

}


HPAVertex::HPAVertex(glm::vec2 pos) : _pos(pos), _active(true), _child(NULL) {

}


HPAVertex::~HPAVertex() {

}


// ------------------------------------------------------------
HPACluster::HPACluster() {

}


HPACluster::HPACluster(glm::vec2 pos, glm::vec2 size) {
	_aabb= new AABB_2D(pos, size);
}


HPACluster::~HPACluster() {
	delete _aabb;
}


// ------------------------------------------------------------
HPAGraph::HPAGraph() {

}


HPAGraph::~HPAGraph() {
	
}


void HPAGraph::add_vertex(unsigned int i, float weight, glm::vec2 pos) {
	if (!_vertices.count(i)) {
		HPAVertex v(pos);
		_vertices[i]= v;
	}
}


void HPAGraph::add_edge(unsigned int i, unsigned int j, float weight) {
	if ((_vertices.count(i)) && (_vertices.count(j))) {
		HPAEdge e(weight);
		_vertices[i]._edges[j]= e;
	}
}


void HPAGraph::remove_vertex(unsigned int i) {
	if (_vertices.count(i)) {
		_vertices.erase(i);

		_it_v= _vertices.begin();
		while (_it_v!= _vertices.end()) {
			if (_it_v->second._edges.count(i)) {
				_it_v->second._edges.erase(i);
			}
			_it_v++;
		}
	}
}


void HPAGraph::remove_edge(unsigned int i, unsigned int j) {
	if ((_vertices.count(i)) && (_vertices.count(j))) {
		_vertices[i]._edges.erase(j);
	}
}


vector<unsigned int> HPAGraph::neighbors(unsigned int i) {
	vector<unsigned int> result;
	_it_e= _vertices[i]._edges.begin();
	while (_it_e!= _vertices[i]._edges.end()) {
		if (_vertices[_it_e->first]._active) {
			result.push_back(_it_e->first);
		}
		_it_e++;
	}
	return result;
}


// ------------------------------------------------------------
HPA::HPA() {

}


HPA::HPA(glm::vec2 step_size, glm::uvec2 level0_dimensions, std::vector<glm::uvec2> cluster_sizes) {
	HPAGraph * graph= new HPAGraph();

	for (unsigned int lig=0; lig<level0_dimensions.y; ++lig) {
		for (unsigned int col=0; col<level0_dimensions.x; ++col) {
			unsigned int id= col_lig2id(col, lig);
			//float weight= rand_float(1.0f, 10.0f);
			float weight= 1.0f;
			/*if (rand_bool()) {
				weight= 10.0f;
			}*/
			float x= _origin.x+ ((float)(col)/ (float)(_n_cols))* _size.x;
			float y= _origin.y+ ((float)(lig)/ (float)(_n_ligs))* _size.y;
			graph->add_vertex(id, weight, x, y);
		}
	}
	
	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		unsigned int col= id2col_lig(_it_v->first).first;
		unsigned int lig= id2col_lig(_it_v->first).second;
		if (col> 0) {
			add_edge(_it_v->first, _it_v->first- 1);
		}
		if (col< _n_cols- 1) {
			add_edge(_it_v->first, _it_v->first+ 1);
		}
		if (lig> 0) {
			add_edge(_it_v->first, _it_v->first- _n_cols);
		}
		if (lig< _n_ligs- 1) {
			add_edge(_it_v->first, _it_v->first+ _n_cols);
		}

		if (is8connex) {
			if ((col> 0) && (lig> 0)) {
				add_edge(_it_v->first, _it_v->first- 1- _n_cols);
			}
			if ((col< _n_cols- 1) && (lig> 0)) {
				add_edge(_it_v->first, _it_v->first+ 1- _n_cols);
			}
			if ((col> 0) && (lig< _n_ligs- 1)) {
				add_edge(_it_v->first, _it_v->first- 1+ _n_cols);
			}
			if ((col< _n_cols- 1) && (lig< _n_ligs- 1)) {
				add_edge(_it_v->first, _it_v->first+ 1+ _n_cols);
			}
		}

		_it_v++;
	}

	_graphs.push_back(graph);
}


HPA::~HPA() {

}

