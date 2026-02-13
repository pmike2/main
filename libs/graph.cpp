#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "graph.h"


Graph::Graph() {

}


Graph::~Graph() {
	// le nettoyage de edge._data ne peut pas se faire ici car void * ici
}


GraphVertex Graph::get_vertex(uint i) {
	return _vertices[i];
}


GraphEdge Graph::get_edge(uint i, uint j) {
	return _vertices[i]._edges[j];
}


GraphEdge Graph::get_edge(uint_pair p) {
	return _vertices[p.first]._edges[p.second];
}


GraphEdge Graph::opposite_edge(GraphEdge e) {
	for (auto & edge : e._end->_edges) {
		if (edge.second._end == e._start) {
			return edge.second;
		}
	}
	std::cerr << "Graph::opposite_edge : non trouvé.\n";
	return GraphEdge{};
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
		GraphEdge e;
		e._start = &_vertices[i];
		e._end = &_vertices[j];
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
	_resolution = pt_2d(_size.x / (number)(_n_cols- 1), _size.y / (number)(_n_ligs- 1));
	
	for (uint lig=0; lig<_n_ligs; ++lig) {
		for (uint col=0; col<_n_cols; ++col) {
			uint id= col_lig2id(col, lig);
			// ici on fait use_vertices = false car le vertex n'existe pas encore
			pt_2d pt = col_lig2pt_2d(col, lig, false);
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


std::map<std::string, uint> GraphGrid::named_neighbors(uint id) {
	std::map<std::string, uint> result;
	int_pair col_lig = id2col_lig(id);
	result["right"] = col_lig2id(col_lig.first + 1, col_lig.second);
	result["left"] = col_lig2id(col_lig.first - 1, col_lig.second);
	result["top"] = col_lig2id(col_lig.first, col_lig.second + 1);
	result["bottom"] = col_lig2id(col_lig.first, col_lig.second - 1);
	result["top_right"] = col_lig2id(col_lig.first + 1, col_lig.second + 1);
	result["bottom_right"] = col_lig2id(col_lig.first + 1, col_lig.second - 1);
	result["top_left"] = col_lig2id(col_lig.first - 1, col_lig.second + 1);
	result["bottom_left"] = col_lig2id(col_lig.first - 1, col_lig.second - 1);
	return result;
}


bool GraphGrid::in_boundaries(uint id) {
	if (id >= _n_ligs * _n_cols) {
		return false;
	}
	return true;
}


bool GraphGrid::in_boundaries(int col, int lig) {
	if (col < 0 || lig < 0 || col >= _n_cols || lig >= _n_ligs) {
		return false;
	}
	return true;
}


bool GraphGrid::in_boundaries(pt_2d pt) {
	if (pt.x < _origin.x || pt.x > _origin.x + _size.x || pt.y < _origin.y || pt.y > _origin.y + _size.y) {
		return false;
	}
	return true;
}


bool GraphGrid::in_boundaries(AABB_2D * aabb) {
	if (in_boundaries(aabb->_pos) && in_boundaries(aabb->_pos + aabb->_size)) {
		return true;
	}
	return false;
}


bool GraphGrid::id_in_ids(uint id, const std::vector<uint> & ids) {
	if (!in_boundaries(id)) {
		return false;
	}
	if (std::find(ids.begin(), ids.end(), id) == ids.end()) {
		return false;
	}
	return true;
}


int_pair GraphGrid::id2col_lig(uint id) {
	if (!in_boundaries(id)) {
		std::cerr << "GraphGrid::id2col_lig : " << id << " hors grille\n";
		return std::make_pair(0, 0);
	}
	return std::make_pair(int(id % _n_cols), int(id/ _n_cols));
}


uint GraphGrid::col_lig2id(int col, int lig) {
	if (!in_boundaries(col, lig)) {
		std::cerr << "GraphGrid::col_lig2id : " << col << " ; " << lig << "hors grille\n";
		return 0;
	}
	return uint(col)+ _n_cols * uint(lig);
}


pt_2d GraphGrid::col_lig2pt_2d(int col, int lig, bool use_vertices) {
	if (!in_boundaries(col, lig)) {
		std::cerr << "GraphGrid::col_lig2pt_2d : " << col << " ; " << lig << " hors grille\n";
		return pt_2d(0.0);
	}
	
	if (use_vertices) {
		return id2pt_2d(col_lig2id(col, lig));
	}
	
	return pt_2d(
		_origin.x+ ((number)(col)/ (number)(_n_cols - 1))* _size.x,
		_origin.y+ ((number)(lig)/ (number)(_n_ligs - 1))* _size.y
	);
}


pt_3d GraphGrid::col_lig2pt_3d(int col, int lig) {
	if (!in_boundaries(col, lig)) {
		std::cerr << "GraphGrid::col_lig2pt_3d : " << col << " ; " << lig << " hors grille\n";
		return pt_3d(0.0);
	}
	return id2pt_3d(col_lig2id(col, lig));
}


pt_2d GraphGrid::id2pt_2d(uint id) {
	if (!in_boundaries(id)) {
		std::cerr << "GraphGrid::id2pt_2d : " << id << " hors grille\n";
		return pt_2d(0.0);
	}
	return pt_2d(_vertices[id]._pos);
}


pt_3d GraphGrid::id2pt_3d(uint id) {
	if (!in_boundaries(id)) {
		std::cerr << "GraphGrid::id2pt_3d : " << id << " hors grille\n";
		return pt_3d(0.0);
	}
	return _vertices[id]._pos;
}


int_pair GraphGrid::pt2col_lig(pt_2d pt) {
	if (!in_boundaries(pt)) {
		std::cerr << "GraphGrid::pt2col_lig : " << glm::to_string(pt) << " hors grille\n";
		return std::make_pair(0, 0);
	}
	int col= (int)(((pt.x- _origin.x)/ _size.x)* (number)(_n_cols- 1));
	int lig= (int)(((pt.y- _origin.y)/ _size.y)* (number)(_n_ligs- 1));
	return std::make_pair(uint(col), uint(lig));
}


uint GraphGrid::pt2id(pt_2d pt) {
	if (!in_boundaries(pt)) {
		std::cerr << "GraphGrid::pt2id : " << glm::to_string(pt) << " hors grille\n";
		return 0;
	}
	int_pair col_lig = pt2col_lig(pt);
	return col_lig2id(col_lig.first, col_lig.second);
}


uint GraphGrid::pt2closest_id(pt_2d pt) {
	if (!in_boundaries(pt)) {
		std::cerr << "GraphGrid::pt2closest_id : " << glm::to_string(pt) << " hors grille\n";
		return 0;
	}
	int_pair col_lig = pt2col_lig(pt);
	uint result = 0;
	number min_dist = 1e9;
	for (int col_offset=0; col_offset<2; ++col_offset) {
		for (int lig_offset=0; lig_offset<2; ++lig_offset) {
			pt_2d pt_vertex = col_lig2pt_2d(col_lig.first + col_offset, col_lig.second + lig_offset);
			number dist = glm::distance(pt, pt_vertex);
			if (dist < min_dist) {
				min_dist = dist;
				result = col_lig2id(col_lig.first + col_offset, col_lig.second + lig_offset);
			}
		}
	}
	return result;
}


uint_pair GraphGrid::pt2closest_edge(pt_2d pt) {
	if (!in_boundaries(pt)) {
		std::cerr << "GraphGrid::pt2closest_edge : " << glm_to_string(pt) << " hors grille.\n";
		return uint_pair{0, 0};
	}

	std::vector<uint_pair> edges = edges_in_cell_containing_pt(pt);
	uint_pair result(0, 0);
	number min_dist = 1e9;
	for (auto & edge : edges) {
		number dist = 0.0;
		distance_segment_pt(pt_2d(_vertices[edge.first]._pos), pt_2d(_vertices[edge.second]._pos), pt, &dist, NULL);
		if (dist < min_dist) {
			min_dist = dist;
			result = edge;
		}
	}
	return result;
}


// ------------------------------------------------------------------------
// TODO : cette méthode ENGLOBE le AABB ; faire une option qui empeche ça ?
// -> serait utile à vertices_in_aabb
// ------------------------------------------------------------------------
std::pair<int_pair, int_pair> GraphGrid::aabb2col_lig_min_max(AABB_2D * aabb) {
	if (!aabb2d_intersects_aabb2d(aabb, _aabb)) {
		std::cerr << "GraphGrid::aabb2col_lig_min_max : pas d'intersection entre " << *_aabb << " et " << *aabb << "\n";
		return std::make_pair(std::make_pair(0, 0), std::make_pair(0, 0));
	}

	pt_2d pt_min = aabb->_pos;
	pt_2d pt_max = aabb->_pos + aabb->_size;
	if (pt_min.x < _aabb->_pos.x) {
		pt_min.x = _aabb->_pos.x;
	}
	if (pt_min.y < _aabb->_pos.y) {
		pt_min.y = _aabb->_pos.y;
	}
	if (pt_max.x > _aabb->_pos.x + _aabb->_size.x) {
		pt_max.x = _aabb->_pos.x + _aabb->_size.x;
	}
	if (pt_max.y > _aabb->_pos.y + _aabb->_size.y) {
		pt_max.y = _aabb->_pos.y + _aabb->_size.y;
	}

	int_pair col_lig_min = pt2col_lig(pt_min);
	int_pair col_lig_max = pt2col_lig(pt_max);

	// car pt2col_lig renvoie le coin bas-gauche de la cellule contenant pt
	if (col_lig_max.first < _n_cols - 1) {
		col_lig_max.first++;
	}
	if (col_lig_max.second < _n_ligs - 1) {
		col_lig_max.second++;
	}
	return std::make_pair(col_lig_min, col_lig_max);
}


// améliorable avec un algo genre Bresenham au lieu de parcourir tout le AABB
std::vector<uint_pair> GraphGrid::edges_intersecting_segment(pt_2d pt1, pt_2d pt2) {
	if (!in_boundaries(pt1)) {
		std::cerr << "GraphGrid::pt2closest_id : " << glm::to_string(pt1) << " hors grille\n";
		return std::vector<uint_pair>{};
	}
	if (!in_boundaries(pt2)) {
		std::cerr << "GraphGrid::pt2closest_id : " << glm::to_string(pt2) << " hors grille\n";
		return std::vector<uint_pair>{};
	}
	std::vector<uint_pair> result;
	int_pair col_lig_1 = pt2col_lig(pt1);
	int_pair col_lig_2 = pt2col_lig(pt2);
	uint col_min = std::min(col_lig_1.first, col_lig_2.first);
	uint col_max = std::max(col_lig_1.first, col_lig_2.first);
	uint lig_min = std::min(col_lig_1.second, col_lig_2.second);
	uint lig_max = std::max(col_lig_1.second, col_lig_2.second);
	for (uint col=col_min; col<col_max; ++col) {
		for (uint lig=lig_min; lig<lig_max; ++lig) {
			pt_2d v1 = col_lig2pt_2d(col, lig);
			pt_2d v2 = col_lig2pt_2d(col + 1, lig);
			pt_2d v3 = col_lig2pt_2d(col + 1, lig + 1);
			pt_2d v4 = col_lig2pt_2d(col, lig + 1);
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


bool GraphGrid::is_edge_intersecting_segment(pt_2d pt1, pt_2d pt2, uint_pair edge) {
	if (!in_boundaries(edge.first)) {
		std::cerr << "GraphGrid::segment_intersects_edge : " << edge.first << " hors grille\n";
		return false;
	}
	if (!in_boundaries(edge.second)) {
		std::cerr << "GraphGrid::segment_intersects_edge : " << edge.second << " hors grille\n";
		return false;
	}
	pt_2d pt1_edge = pt_2d(_vertices[edge.first]._pos);
	pt_2d pt2_edge = pt_2d(_vertices[edge.second]._pos);
	return segment_intersects_segment(pt1, pt2, pt1_edge, pt2_edge, NULL);
}


std::vector<uint_pair> GraphGrid::edges_intersecting_aabb(AABB_2D * aabb) {
	std::vector<uint_pair> result;

	std::pair<int_pair, int_pair> col_lig_min_max = aabb2col_lig_min_max(aabb);

	uint col_min = col_lig_min_max.first.first;
	uint lig_min = col_lig_min_max.first.second;
	uint col_max = col_lig_min_max.second.first;
	uint lig_max = col_lig_min_max.second.second;
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


std::vector<uint_pair> GraphGrid::edges_intersecting_bbox(BBox_2D * bbox) {
	std::vector<uint_pair> result;
	int_pair col_lig_min = pt2col_lig(bbox->_aabb->_pos);
	int_pair col_lig_max = pt2col_lig(bbox->_aabb->_pos + bbox->_aabb->_size);
	uint col_min = col_lig_min.first;
	uint lig_min = col_lig_min.second;
	uint col_max = col_lig_max.first + 1; // +1 car pt2col_lig renvoie le coin bas-gauche du carré contenant pt
	uint lig_max = col_lig_max.second + 1;
	for (uint col=col_min; col<col_max; ++col) {
		for (uint lig=lig_min; lig<lig_max; ++lig) {
			pt_2d pt = col_lig2pt_2d(col, lig);
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


std::vector<uint_pair> GraphGrid::edges_intersecting_polygon(Polygon2D * polygon) {
	std::vector<uint_pair> result;
	std::vector<uint_pair> edges_in_aabb = edges_intersecting_aabb(polygon->_aabb);
	for (auto & e : edges_in_aabb) {
		if (is_pt_inside_poly(pt_2d(_vertices[e.first]._pos), polygon) ||
			is_pt_inside_poly(pt_2d(_vertices[e.second]._pos), polygon) ||
			segment_intersects_poly(pt_2d(_vertices[e.first]._pos), pt_2d(_vertices[e.second]._pos), polygon, NULL)) {
			result.push_back(e);
		}
	}
	return result;
}


std::vector<uint_pair> GraphGrid::edges_in_cell_containing_pt(pt_2d pt, bool only_diagonals) {
	if (!in_boundaries(pt)) {
		std::cerr << "GraphGrid::edges_in_cell_containing_pt : " << glm_to_string(pt) << " hors grille.\n";
		return std::vector<uint_pair>{};
	}
	int_pair col_lig = pt2col_lig(pt);
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

	std::vector<uint_pair> result;
	for (auto & offset : offsets) {
		uint id1 = col_lig2id(col_lig.first + offset[0], col_lig.second + offset[1]);
		uint id2 = col_lig2id(col_lig.first + offset[2], col_lig.second + offset[3]);
		result.push_back(std::make_pair(id1, id2));
	}

	return result;
}


std::vector<uint> GraphGrid::vertices_in_cell_containing_pt(pt_2d pt) {
	int_pair col_lig = pt2col_lig(pt);
	uint id_left_bottom = col_lig2id(col_lig.first, col_lig.second);
	uint id_right_bottom = col_lig2id(col_lig.first + 1, col_lig.second);
	uint id_left_top = col_lig2id(col_lig.first, col_lig.second + 1);
	uint id_right_top = col_lig2id(col_lig.first + 1, col_lig.second + 1);	
	std::vector<uint> result = {id_left_bottom, id_right_bottom, id_left_top, id_right_top};
	return result;
}


std::vector<uint> GraphGrid::vertices_in_aabb(AABB_2D * aabb) {
	std::vector<uint> result;
	std::pair<int_pair, int_pair> col_lig_min_max = aabb2col_lig_min_max(aabb);
	uint col_min = col_lig_min_max.first.first;
	uint lig_min = col_lig_min_max.first.second;
	uint col_max = col_lig_min_max.second.first;
	uint lig_max = col_lig_min_max.second.second;

	for (uint col = col_min; col<=col_max; ++col) {
		for (uint lig = lig_min; lig<=lig_max; ++lig) {
			result.push_back(col_lig2id(col, lig));
		}
	}
	return result;
}


std::vector<uint> GraphGrid::vertices_in_circle(pt_2d center, number radius) {
	AABB_2D * aabb = new AABB_2D(center - pt_2d(radius), pt_2d(2.0 * radius));
	std::vector<uint> l = vertices_in_aabb(aabb);
	delete aabb;
	std::vector<uint> result;
	for (auto v : l) {
		pt_2d v_pos = pt_2d(get_vertex(v)._pos);
		if (glm::length(v_pos - center) < radius) {
			result.push_back(v);
		}
	}
	return result;
}


std::vector<uint> GraphGrid::vertices_in_circle_section(pt_2d center, number radius, number direction_angle, number max_angle) {
	std::vector<uint> result;
	std::vector<uint> l = vertices_in_circle(center, radius);
	for (auto v : l) {
		pt_2d v_pos = pt_2d(get_vertex(v)._pos);
		if (glm::dot(glm::normalize(v_pos - center), pt_2d(cos(direction_angle), sin(direction_angle))) > max_angle) {
			result.push_back(v);
		}
	}
	return result;
}


int_pair GraphGrid::next_direction(int_pair u) {
	if (u.first == 1 && u.second == 0) {
		return std::make_pair(1, 1);
	}
	else if (u.first == 1 && u.second == 1) {
		return std::make_pair(0, 1);
	}
	else if (u.first == 0 && u.second == 1) {
		return std::make_pair(-1, 1);
	}
	else if (u.first == -1 && u.second == 1) {
		return std::make_pair(-1, 0);
	}
	else if (u.first == -1 && u.second == 0) {
		return std::make_pair(-1, -1);
	}
	else if (u.first == -1 && u.second == -1) {
		return std::make_pair(0, -1);
	}
	else if (u.first == 0 && u.second == -1) {
		return std::make_pair(1, -1);
	}
	else if (u.first == 1 && u.second == -1) {
		return std::make_pair(1, 0);
	}
	
	std::cerr << "GraphGrid::next_direction error : u = (" << u.first << " ; " << u.second << ")\n";
	return std::make_pair(0, 0);
}


uint GraphGrid::angle(int_pair u, int_pair v) {
	uint result = 0;
	int_pair d = u;
	while (d.first != v.first || d.second != v.second) {
		result++;
		d = next_direction(d);
	}
	return result;
}


std::vector<uint> GraphGrid::prune(std::vector<uint> ids) {
	std::vector<uint> result;
	for (auto & id : ids) {
		uint n_neighbors = 0;
		for (auto & n : neighbors(id)) {
			if (id_in_ids(n, ids)) {
				n_neighbors++;
			}
		}
		if (n_neighbors > 1) {
			result.push_back(id);
		}
	}
	return result;
}


Polygon2D * GraphGrid::ids2polygon(std::vector<uint> ids) {
	Polygon2D * result = new Polygon2D();
	std::vector<pt_2d> pts;

	std::vector<uint> pruned_ids = prune(ids);

	uint id_max_col = 0;
	number max_col = -1e9;
	for (auto & id : pruned_ids) {
		int_pair col_lig = id2col_lig(id);
		if (col_lig.first > max_col) {
			max_col = col_lig.first;
			id_max_col = id;
		}
	}
	
	uint current_id = id_max_col;
	uint last_id = 1e9;
	int_pair current_direction = std::make_pair(1, 0);
	const uint max_iter = 10000;
	uint iter = 0;
	
	while (true) {
		iter++;
		if (iter > max_iter) {
			std::cerr << "GraphGrid::ids2polygon infinite loop\n";
			std::cerr << ids2wkt(pruned_ids) << "\n";
			delete result;
			return NULL;
		}

		//std::cout << "current_id = " << current_id << "\n";

		pts.push_back(id2pt_2d(current_id));
		std::vector<uint> ids_neighbors = neighbors(current_id);
		int_pair current_col_lig = id2col_lig(current_id);
		uint next_id = 0;
		int_pair next_direction;
		uint min_angle = 1e9;

		for (auto & id_neighbor : ids_neighbors) {
			if (id_neighbor == last_id) {
				continue;
			}
			//if (std::find(ids.begin(), ids.end(), id_neighbor)!= ids.end()) {
			if (id_in_ids(id_neighbor, pruned_ids)) {
				int_pair neighbour_col_lig = id2col_lig(id_neighbor);
				int_pair direction = std::make_pair(neighbour_col_lig.first - current_col_lig.first, neighbour_col_lig.second - current_col_lig.second);
				uint a = angle(current_direction, direction);
				if (a < min_angle) {
					min_angle = a;
					next_id = id_neighbor;
					next_direction.first = - direction.first;
					next_direction.second = - direction.second;
				}
			}
		}

		last_id = current_id;
		current_id = next_id;
		if (current_id == id_max_col) {
			break;
		}
		current_direction = next_direction;
	}

	result->set_points(pts);
	result->update_all();
	return result;
}


Polygon2D * GraphGrid::pts2polygon(std::vector<pt_2d> pts) {
	std::vector<uint> ids;
	for (auto & pt : pts) {
		uint id = pt2id(pt);
		if (std::find(ids.begin(), ids.end(), id) == ids.end()) {
			ids.push_back(id);
		}
	}
	/*for (auto & id : ids) {
		std::cout << id << " ; ";
	}
	std::cout << "\n";*/
	return ids2polygon(ids);
}


std::vector<std::tuple<uint, uint, uint> > GraphGrid::ids2triangles(std::vector<uint> ids) {
	std::vector<std::tuple<uint, uint, uint> > triangles;
	
	for (auto & id : ids) {
		std::map<std::string, uint> ns = named_neighbors(id);
		
		if (id_in_ids(ns["right"], ids) && id_in_ids(ns["top_right"], ids) && id_in_ids(ns["top"], ids)) {
			triangles.push_back(std::make_tuple(id, ns["right"], ns["top_right"]));
			triangles.push_back(std::make_tuple(id, ns["top_right"], ns["top"]));
		}
		else if (id_in_ids(ns["right"], ids) && id_in_ids(ns["top_right"], ids) && !id_in_ids(ns["top"], ids)) {
			triangles.push_back(std::make_tuple(id, ns["right"], ns["top_right"]));
		}
		else if (id_in_ids(ns["right"], ids) && !id_in_ids(ns["top_right"], ids) && id_in_ids(ns["top"], ids)) {
			triangles.push_back(std::make_tuple(id, ns["right"], ns["top"]));
		}
		else if (!id_in_ids(ns["right"], ids) && id_in_ids(ns["top_right"], ids) && id_in_ids(ns["top"], ids)) {
			triangles.push_back(std::make_tuple(id, ns["top_right"], ns["top"]));
		}

		if (id_in_ids(ns["bottom_right"], ids) && id_in_ids(ns["right"], ids) && !id_in_ids(ns["bottom"], ids)) {
			triangles.push_back(std::make_tuple(id, ns["bottom_right"], ns["right"]));
		}
	}
	
	return triangles;
}


std::vector<uint> GraphGrid::triangles2ids(const std::vector<std::tuple<uint, uint, uint> > & triangles) {
	std::vector<uint> result;
	for (auto & triangle : triangles) {
		std::vector<uint> ids = {std::get<0>(triangle), std::get<1>(triangle), std::get<2>(triangle)};
		for (auto & id : ids) {
			if (std::find(result.begin(), result.end(), id) == result.end()) {
				result.push_back(id);
			}
		}
	}
	return result;
}


std::vector<uint> GraphGrid::neighbors_dist(uint id_root, uint distance) {
	std::vector<uint> result;

	std::queue<uint> frontier;
	std::vector<uint> checked;
	std::map<uint, uint> distances;
	
	frontier.push(id_root);
	checked.push_back(id_root);
	distances[id_root] = 0;
	
	while (!frontier.empty()) {
		uint id = frontier.front();
		frontier.pop();

		result.push_back(id);

		std::vector<uint> id_neighbors = neighbors(id);

		for (auto & id_n : id_neighbors) {
			if (std::find(checked.begin(), checked.end(), id_n) != checked.end()) {
				continue;
			}
			checked.push_back(id_n);
			distances[id_n] = distances[id] + 1;

			if (distances[id_n] <= distance) {
				frontier.push(id_n);
			}
		}
	}

	return result;
}


std::vector<uint> GraphGrid::buffered_ids(const std::vector<uint> & ids, uint distance) {
	std::vector<uint> result;
	for (auto & id : ids) {
		std::vector<uint> ns = neighbors_dist(id, distance);
		for (auto & n : ns) {
			if (std::find(result.begin(), result.end(), n) == result.end()) {
				result.push_back(n);
			}
		}
	}
	return result;
}


std::string GraphGrid::ids2wkt(std::vector<uint> ids) {
	std::string result = "MULTIPOINT(";
	for (auto & id : ids) {
		pt_2d pt = id2pt_2d(id);
		result += "(" + std::to_string(pt.x) + " " + std::to_string(pt.y) + ")";
		if (id != ids[ids.size() - 1]) {
			result += ", ";
		}
	}
	result += ")";
	return result;
}


std::string GraphGrid::triangles2wkt(std::vector<std::tuple<uint, uint, uint> > triangles) {
	std::string result = "MULTIPOLYGON(";
	for (auto & triangle : triangles) {
		std::vector<uint> ids = {std::get<0>(triangle), std::get<1>(triangle), std::get<2>(triangle)};
		result += "((";
		for (auto & id : ids) {
			pt_2d pt = id2pt_2d(id);
			result += std::to_string(pt.x) + " " + std::to_string(pt.y);
			if (id != ids[ids.size() - 1]) {
				result += ", ";
			}
		}
		result += "))";
		if (triangle != triangles[triangles.size() - 1]) {
			result += ", ";
		}
	}
	result += ")";
	return result;
}


std::ostream & operator << (std::ostream & os, GraphGrid & g) {
	os << static_cast<Graph &>(g);
	return os;
}


