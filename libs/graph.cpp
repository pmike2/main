#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "graph.h"


Graph::Graph() {

}


Graph::~Graph() {
	// le nettoyage de edge._data ne peut pas se faire ici car void * ici

	/*_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		_it_e= _it_v->second._edges.begin();
		while (_it_e!= _it_v->second._edges.end()) {
			if (_it_e->second._data != NULL) {
				delete _it_e->second._data;
			}
			_it_e++;
		}
		_it_v++;
	}*/
}


void Graph::add_vertex(uint i, pt_3d pos) {
	if (!_vertices.count(i)) {
		GraphVertex v= {};
		v._pos= pos;
		_vertices[i]= v;
	}
}


void Graph::add_edge(uint i, uint j) {
	if ((_vertices.count(i)) && (_vertices.count(j))) {
		GraphEdge e= {};
		_vertices[i]._edges[j]= e;
	}
}


void Graph::remove_vertex(uint i) {
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


/*void Graph::reinit_weights() {
	_it_v= _vertices.begin();
	while (_it_v!= _vertices.end()) {
		_it_v->second._weight= DEFAULT_EDGE_WEIGHT;
		_it_v++;
	}
}*/


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
		/*std::cout << " ; edge weights = ";
		for (auto edge : g._it_v->second._edges) {
			std::cout << edge.second._weight << " ; ";
		}*/
		std::cout << "\n";

		g._it_v++;
	}
	os << "-----------\n";
	return os;
}


// ----------------------------------------------------------------------------------------
GraphGrid::GraphGrid() {

}


GraphGrid::GraphGrid(const pt_2d & origin, const pt_2d & size, uint n_ligs, uint n_cols, bool is8connex) :
	_origin(origin), _size(size), _n_ligs(n_ligs), _n_cols(n_cols)
{
	for (uint lig=0; lig<_n_ligs; ++lig) {
		for (uint col=0; col<_n_cols; ++col) {
			uint id= col_lig2id(col, lig);
			pt_2d pt = col_lig2pt(col, lig);
			add_vertex(id, pt_3d(pt.x, pt.y, 0.0));
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


GraphGrid::GraphGrid(const GraphGrid & grid) : 
	_origin(grid._origin), _size(grid._size), _n_ligs(grid._n_ligs), _n_cols(grid._n_cols)
{
	_aabb= new AABB_2D(*grid._aabb);
	for (auto v : grid._vertices) {
		_vertices[v.first] = v.second;
	}
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


pt_2d GraphGrid::col_lig2pt(uint col, uint lig) {
	return pt_2d(
		_origin.x+ ((number)(col)/ (number)(_n_cols - 1))* _size.x,
		_origin.y+ ((number)(lig)/ (number)(_n_ligs - 1))* _size.y
	);
}


std::pair<uint, uint> GraphGrid::pt2col_lig(pt_2d pt) {
	int col= (int)(((pt.x- _origin.x)/ _size.x)* (number)(_n_cols- 1));
	int lig= (int)(((pt.y- _origin.y)/ _size.y)* (number)(_n_ligs- 1));
	if (col < 0 || lig < 0 || col >= _n_cols || lig >= _n_ligs) {
		std::cerr << "GraphGrid::pt2col_lig : pt" << glm::to_string(pt) << " hors grille : col = " << col << " ; lig = " << lig << "\n";
		return std::make_pair(0, 0);
	}
	return std::make_pair(uint(col), uint(lig));
}


uint GraphGrid::pt2id(pt_2d pt) {
	std::pair<uint, uint> col_lig = pt2col_lig(pt);
	return col_lig2id(col_lig.first, col_lig.second);
}


uint GraphGrid::pt2closest_id(pt_2d pt) {
	std::pair<uint, uint> col_lig = pt2col_lig(pt);
	uint result = 0;
	number min_dist = 1e9;
	for (int col_offset=0; col_offset<2; ++col_offset) {
		for (int lig_offset=0; lig_offset<2; ++lig_offset) {
			pt_2d pt_vertex = col_lig2pt(col_lig.first + col_offset, col_lig.second + lig_offset);
			number dist = glm::distance(pt, pt_vertex);
			if (dist < min_dist) {
				min_dist = dist;
				result = col_lig2id(col_lig.first + col_offset, col_lig.second + lig_offset);
			}
		}
	}
	return result;
}


// améliorable avec un algo genre Bresenham au lieu de parcourir tout le AABB
std::vector<std::pair<uint, uint> > GraphGrid::segment_intersection(pt_2d pt1, pt_2d pt2) {
	std::vector<std::pair<uint, uint> > result;
	std::pair<uint, uint> col_lig_1 = pt2col_lig(pt1);
	std::pair<uint, uint> col_lig_2 = pt2col_lig(pt2);
	uint col_min = std::min(col_lig_1.first, col_lig_2.first);
	uint col_max = std::max(col_lig_1.first, col_lig_2.first);
	uint lig_min = std::min(col_lig_1.second, col_lig_2.second);
	uint lig_max = std::max(col_lig_1.second, col_lig_2.second);
	for (uint col=col_min; col<col_max; ++col) {
		for (uint lig=lig_min; lig<lig_max; ++lig) {
			pt_2d v1 = col_lig2pt(col, lig);
			pt_2d v2 = col_lig2pt(col + 1, lig);
			pt_2d v3 = col_lig2pt(col + 1, lig + 1);
			pt_2d v4 = col_lig2pt(col, lig + 1);
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


bool GraphGrid::segment_intersects_edge(pt_2d pt1, pt_2d pt2, std::pair<uint, uint> edge) {
	pt_2d pt1_edge = pt_2d(_vertices[edge.first]._pos);
	pt_2d pt2_edge = pt_2d(_vertices[edge.second]._pos);
	return segment_intersects_segment(pt1, pt2, pt1_edge, pt2_edge, NULL);
}


std::vector<std::pair<uint, uint> > GraphGrid::aabb_intersection(AABB_2D * aabb) {
	std::vector<std::pair<uint, uint> > result;
	std::pair<uint, uint> col_lig_min = pt2col_lig(aabb->_pos);
	std::pair<uint, uint> col_lig_max = pt2col_lig(aabb->_pos + aabb->_size);
	uint col_min = col_lig_min.first;
	uint lig_min = col_lig_min.second;
	uint col_max = col_lig_max.first + 1; // +1 car pt2col_lig renvoie le coin bas-gauche du carré contenant pt
	uint lig_max = col_lig_max.second + 1;
	for (uint col=col_min; col<col_max; ++col) {
		for (uint lig=lig_min; lig<lig_max; ++lig) {
			if (lig > lig_min) {
				result.push_back(std::make_pair(col_lig2id(col, lig), col_lig2id(col + 1, lig)));
				result.push_back(std::make_pair(col_lig2id(col + 1, lig), col_lig2id(col, lig)));
			}
			if (col > col_min) {
				result.push_back(std::make_pair(col_lig2id(col, lig), col_lig2id(col, lig + 1)));
				result.push_back(std::make_pair(col_lig2id(col, lig + 1), col_lig2id(col, lig)));
			}
			result.push_back(std::make_pair(col_lig2id(col, lig), col_lig2id(col + 1, lig + 1)));
			result.push_back(std::make_pair(col_lig2id(col + 1, lig + 1), col_lig2id(col, lig)));
			result.push_back(std::make_pair(col_lig2id(col + 1, lig), col_lig2id(col, lig + 1)));
			result.push_back(std::make_pair(col_lig2id(col, lig + 1), col_lig2id(col + 1, lig)));
		}
	}
	return result;
}


std::vector<std::pair<uint, uint> > GraphGrid::bbox_intersection(BBox_2D * bbox) {
	std::vector<std::pair<uint, uint> > result;
	std::pair<uint, uint> col_lig_min = pt2col_lig(bbox->_aabb->_pos);
	std::pair<uint, uint> col_lig_max = pt2col_lig(bbox->_aabb->_pos + bbox->_aabb->_size);
	uint col_min = col_lig_min.first;
	uint lig_min = col_lig_min.second;
	uint col_max = col_lig_max.first + 1; // +1 car pt2col_lig renvoie le coin bas-gauche du carré contenant pt
	uint lig_max = col_lig_max.second + 1;
	for (uint col=col_min; col<col_max; ++col) {
		for (uint lig=lig_min; lig<lig_max; ++lig) {
			pt_2d pt = col_lig2pt(col, lig);
			if (pt_in_bbox2d(pt, bbox)) {
				uint id = col_lig2id(col, lig);
				GraphVertex v = _vertices[id];
				_it_e= v._edges.begin();
				while (_it_e!= v._edges.end()) {
					result.push_back(std::make_pair(id, _it_e->first));
					result.push_back(std::make_pair(_it_e->first, id));
					_it_e++;
				}
			}
		}
	}
	return result;
}


std::vector<std::pair<uint, uint> > GraphGrid::polygon_intersection(Polygon2D * polygon) {
	std::vector<std::pair<uint, uint> > result;
	//std::cout << *polygon->_aabb << "\n";
	std::pair<uint, uint> col_lig_min = pt2col_lig(polygon->_aabb->_pos);
	std::pair<uint, uint> col_lig_max = pt2col_lig(polygon->_aabb->_pos + polygon->_aabb->_size);
	uint col_min = col_lig_min.first;
	uint lig_min = col_lig_min.second;
	uint col_max = col_lig_max.first + 1; // +1 car pt2col_lig renvoie le coin bas-gauche du carré contenant pt
	uint lig_max = col_lig_max.second + 1;
	for (uint col=col_min; col<col_max; ++col) {
		for (uint lig=lig_min; lig<lig_max; ++lig) {
			pt_2d pt = col_lig2pt(col, lig);
			//std::cout << glm_to_string(pt) << " ; " << col << " ; " << lig << "\n";
			if (is_pt_inside_poly(pt, polygon)) {
				/*result.push_back(std::make_pair(col_lig2id(col, lig), col_lig2id(col + 1, lig)));
				result.push_back(std::make_pair(col_lig2id(col + 1, lig), col_lig2id(col, lig)));
				result.push_back(std::make_pair(col_lig2id(col, lig), col_lig2id(col, lig + 1)));
				result.push_back(std::make_pair(col_lig2id(col, lig + 1), col_lig2id(col, lig)));
				result.push_back(std::make_pair(col_lig2id(col, lig), col_lig2id(col + 1, lig + 1)));
				result.push_back(std::make_pair(col_lig2id(col + 1, lig + 1), col_lig2id(col, lig)));
				result.push_back(std::make_pair(col_lig2id(col + 1, lig), col_lig2id(col, lig + 1)));
				result.push_back(std::make_pair(col_lig2id(col, lig + 1), col_lig2id(col + 1, lig)));*/
				uint id = col_lig2id(col, lig);
				GraphVertex v = _vertices[id];
				_it_e= v._edges.begin();
				while (_it_e!= v._edges.end()) {
					result.push_back(std::make_pair(id, _it_e->first));
					result.push_back(std::make_pair(_it_e->first, id));
					_it_e++;
				}
			}
		}
	}
	return result;
}


/*std::vector<number> GraphGrid::weights_in_cell_containing_pt(pt_2d pt) {
	std::pair<uint, uint> col_lig = pt2col_lig(pt);
	uint id_left_bottom = col_lig2id(col_lig.first, col_lig.second);
	uint id_right_bottom = col_lig2id(col_lig.first + 1, col_lig.second);
	uint id_left_top = col_lig2id(col_lig.first, col_lig.second + 1);
	uint id_right_top = col_lig2id(col_lig.first + 1, col_lig.second + 1);
	std::vector<number> result;
	result.push_back(_vertices[id_left_bottom]._edges[id_right_top]._weight);
	result.push_back(_vertices[id_right_top]._edges[id_left_bottom]._weight);
	result.push_back(_vertices[id_right_bottom]._edges[id_left_top]._weight);
	result.push_back(_vertices[id_left_top]._edges[id_right_bottom]._weight);
	return result;
}*/


std::vector<std::pair<uint, uint> > GraphGrid::edges_in_cell_containing_pt(pt_2d pt, bool only_diagonals) {
	std::pair<uint, uint> col_lig = pt2col_lig(pt);
	std::vector<glm::uvec4> offsets = {
		glm::uvec4(0, 0, 1, 1), glm::uvec4(1, 1, 0, 0), glm::uvec4(0, 1, 1, 0), glm::uvec4(1, 0, 0, 1)
	};
	if (!only_diagonals) {
		offsets.insert(offsets.end(), {
			glm::uvec4(0, 0, 1, 0), glm::uvec4(1, 0, 0, 0),
			glm::uvec4(1, 0, 1, 1), glm::uvec4(1, 1, 1, 0),
			glm::uvec4(1, 1, 0, 1), glm::uvec4(0, 1, 1, 1),
			glm::uvec4(0, 1, 0, 0), glm::uvec4(0, 0, 0, 1)
		});
	}

	std::vector<std::pair<uint, uint> > result;
	for (auto & offset : offsets) {
		uint id1 = col_lig2id(col_lig.first + offset[0], col_lig.second + offset[1]);
		uint id2 = col_lig2id(col_lig.first + offset[2], col_lig.second + offset[3]);
		//result.push_back(_vertices[id1]._edges[id2]);
		result.push_back(std::make_pair(id1, id2));
	}

	/*uint id_left_bottom = col_lig2id(col_lig.first, col_lig.second);
	uint id_right_bottom = col_lig2id(col_lig.first + 1, col_lig.second);
	uint id_left_top = col_lig2id(col_lig.first, col_lig.second + 1);
	uint id_right_top = col_lig2id(col_lig.first + 1, col_lig.second + 1);
	std::vector<GraphEdge> result;
	result.push_back(_vertices[id_left_bottom]._edges[id_right_top]);
	result.push_back(_vertices[id_right_top]._edges[id_left_bottom]);
	result.push_back(_vertices[id_right_bottom]._edges[id_left_top]);
	result.push_back(_vertices[id_left_top]._edges[id_right_bottom]);
	if (!only_diagonals) {
		result.push_back(_vertices[id_left_bottom]._edges[id_right_bottom]);
		result.push_back(_vertices[id_right_bottom]._edges[id_left_bottom]);

		result.push_back(_vertices[id_left_bottom]._edges[id_left_top]);
		result.push_back(_vertices[id_left_bottom]._edges[id_right_bottom]);
	}*/
	return result;
}


std::vector<uint> GraphGrid::vertices_in_cell_containing_pt(pt_2d pt) {
	std::pair<uint, uint> col_lig = pt2col_lig(pt);
	uint id_left_bottom = col_lig2id(col_lig.first, col_lig.second);
	uint id_right_bottom = col_lig2id(col_lig.first + 1, col_lig.second);
	uint id_left_top = col_lig2id(col_lig.first, col_lig.second + 1);
	uint id_right_top = col_lig2id(col_lig.first + 1, col_lig.second + 1);	
	std::vector<uint> result = {id_left_bottom, id_right_bottom, id_left_top, id_right_top};
	return result;
}


std::ostream & operator << (std::ostream & os, GraphGrid & g) {
	os << static_cast<Graph &>(g);
	return os;
}


