#include "graph.h"


Graph::Graph() {

}


Graph::~Graph() {
	
}

void Graph::add_vertex(uint i, pt_type_3d pos, number weight) {
	if (!_vertices.count(i)) {
		GraphVertex v= {};
		v._weight= weight;
		v._pos= pos;
		_vertices[i]= v;
	}
}


void Graph::add_edge(uint i, uint j, number weight, bool weight_is_dist) {
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


void Graph::remove_vertex(uint i) {
	if (_vertices.count(i)) {
		_vertices.erase(i);

		// lent
		//for (pair<uint, Vertex> v : _vertices) {
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


void Graph::remove_edge(uint i, uint j) {
	if ((_vertices.count(i)) && (_vertices.count(j))) {
		_vertices[i]._edges.erase(j);
	}
}


std::vector<uint> Graph::neighbors(uint i) {
	std::vector<uint> result;
	_it_e= _vertices[i]._edges.begin();
	while (_it_e!= _vertices[i]._edges.end()) {
		result.push_back(_it_e->first);
		_it_e++;
	}
	return result;
}


void Graph::clear() {
	_vertices.clear();
}


void Graph::reinit_weights() {
	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		_it_v->second._weight= 1.0;
		_it_v++;
	}
}


std::ostream & operator << (std::ostream & os, Graph & g) {
	os << "graph -----" << "\n";
	os << "n_vertices = " << g._vertices.size() << "\n";
	g._it_v= g._vertices.begin();
	while (g._it_v!= g._vertices.end()) {
		os << "vertex : id=" << g._it_v->first << "\n";
		std::vector<uint> l= g.neighbors(g._it_v->first);
		for (auto neighbor : l) {
			std::cout << "\t-> " << neighbor << "\n";
		}
		g._it_v++;
	}
	os << "-----------\n";
	return os;
}


// ----------------------------------------------------------------------------------------
GraphGrid::GraphGrid() {

}


GraphGrid::GraphGrid(uint n_ligs, uint n_cols, const pt_type & origin, const pt_type & size, bool is8connex) :
	_n_ligs(n_ligs), _n_cols(n_cols), _origin(origin), _size(size) 
{
	for (uint lig=0; lig<_n_ligs; ++lig) {
		for (uint col=0; col<_n_cols; ++col) {
			uint id= col_lig2id(col, lig);
			number weight= 1.0;
			number x= _origin.x+ ((number)(col)/ (number)(_n_cols))* _size.x;
			number y= _origin.y+ ((number)(lig)/ (number)(_n_ligs))* _size.y;
			add_vertex(id, pt_type_3d(x, y, 0.0), weight);
		}
	}
	
	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		uint col= id2col_lig(_it_v->first).first;
		uint lig= id2col_lig(_it_v->first).second;
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

	_aabb= new AABB_2D(_origin, _size);
}


GraphGrid::~GraphGrid() {
	delete _aabb;
}


std::pair<uint, uint> GraphGrid::id2col_lig(uint id) {
	return std::make_pair(id % _n_cols, id/ _n_cols);
}


uint GraphGrid::col_lig2id(uint col, uint lig) {
	return col+ _n_cols* lig;
}


uint GraphGrid::pt2id(pt_type pt) {
	uint col_min= (uint)(((pt.x- _origin.x)/ _size.x)* (number)(_n_cols- 1));
	uint lig_min= (uint)(((pt.y- _origin.y)/ _size.y)* (number)(_n_ligs- 1));
	return col_lig2id(col_min, lig_min);
}


std::ostream & operator << (std::ostream & os, GraphGrid & g) {
	os << static_cast<Graph &>(g);
	return os;
}


