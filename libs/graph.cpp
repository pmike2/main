#include <glm/gtx/string_cast.hpp>

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
		os << "vertex : id=" << g._it_v->first << " ; pos = " << glm::to_string(g._it_v->second._pos);
		/*std::vector<uint> l= g.neighbors(g._it_v->first);
		for (auto neighbor : l) {
			std::cout << "\t-> " << neighbor;
		}*/
		std::cout << " ; edge weights = ";
		for (auto edge : g._it_v->second._edges) {
			std::cout << edge.second._weight << " ; ";
		}
		std::cout << "\n";

		g._it_v++;
	}
	os << "-----------\n";
	return os;
}


// ----------------------------------------------------------------------------------------
GraphGrid::GraphGrid() {

}


GraphGrid::GraphGrid(const pt_type & origin, const pt_type & size, uint n_ligs, uint n_cols, bool is8connex) :
	_origin(origin), _size(size), _n_ligs(n_ligs), _n_cols(n_cols)
{
	for (uint lig=0; lig<_n_ligs; ++lig) {
		for (uint col=0; col<_n_cols; ++col) {
			uint id= col_lig2id(col, lig);
			pt_type pt = col_lig2pt(col, lig);
			number weight= 1.0;
			add_vertex(id, pt_type_3d(pt.x, pt.y, 0.0), weight);
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


pt_type GraphGrid::col_lig2pt(uint col, uint lig) {
	return pt_type(
		_origin.x+ ((number)(col)/ (number)(_n_cols - 1))* _size.x,
		_origin.y+ ((number)(lig)/ (number)(_n_ligs - 1))* _size.y
	);
}


std::pair<uint, uint> GraphGrid::pt2col_lig(pt_type pt) {
	int col= (int)(((pt.x- _origin.x)/ _size.x)* (number)(_n_cols- 1));
	int lig= (int)(((pt.y- _origin.y)/ _size.y)* (number)(_n_ligs- 1));
	if (col < 0 || lig < 0 || col >= _n_cols || lig >= _n_ligs) {
		std::cerr << "GraphGrid::pt2col_lig : pt" << glm::to_string(pt) << "hors jeu\n";
		return std::make_pair(0, 0);
	}
	return std::make_pair(uint(col), uint(lig));
}


uint GraphGrid::pt2id(pt_type pt) {
	std::pair<uint, uint> col_lig = pt2col_lig(pt);
	return col_lig2id(col_lig.first, col_lig.second);
}


std::vector<std::pair<uint, uint> > GraphGrid::segment_intersection(pt_type pt1, pt_type pt2) {
	std::vector<std::pair<uint, uint> > result;
	std::pair<uint, uint> col_lig_1 = pt2col_lig(pt1);
	std::pair<uint, uint> col_lig_2 = pt2col_lig(pt2);
	for (uint col=col_lig_1.first; col<col_lig_2.first; ++col) {
		for (uint lig=col_lig_1.second; lig<col_lig_2.second; ++lig) {
			pt_type v1 = col_lig2pt(col, lig);
			pt_type v2 = col_lig2pt(col + 1, lig);
			pt_type v3 = col_lig2pt(col + 1, lig + 1);
			pt_type v4 = col_lig2pt(col, lig + 1);
			if (segment_intersects_segment(pt1, pt2, v1, v2, NULL)) {
				result.push_back(std::make_pair(col_lig2id(col, lig), col_lig2id(col + 1, lig)));
				result.push_back(std::make_pair(col_lig2id(col + 1, lig), col_lig2id(col, lig)));
			}
			if (segment_intersects_segment(pt1, pt2, v1, v3, NULL)) {
				result.push_back(std::make_pair(col_lig2id(col, lig), col_lig2id(col + 1, lig + 1)));
				result.push_back(std::make_pair(col_lig2id(col + 1, lig + 1), col_lig2id(col, lig)));
			}
			if (segment_intersects_segment(pt1, pt2, v1, v4, NULL)) {
				result.push_back(std::make_pair(col_lig2id(col, lig), col_lig2id(col, lig + 1)));
				result.push_back(std::make_pair(col_lig2id(col, lig + 1), col_lig2id(col, lig)));
			}
			if (segment_intersects_segment(pt1, pt2, v2, v4, NULL)) {
				result.push_back(std::make_pair(col_lig2id(col + 1, lig), col_lig2id(col, lig + 1)));
				result.push_back(std::make_pair(col_lig2id(col, lig + 1), col_lig2id(col + 1, lig)));
			}
		}
	}
	return result;
}


std::ostream & operator << (std::ostream & os, GraphGrid & g) {
	os << static_cast<Graph &>(g);
	return os;
}


