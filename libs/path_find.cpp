#include <queue>
#include <fstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "path_find.h"
#include "utile.h"
#include "shapefile.h"



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

/*void GraphGrid::set_heavy_weight(AABB_2D * aabb) {
	int col_min= (int)((aabb->_pos.x- _origin.x)* (number)(_n_cols)/ _size.x);
	int col_max= (int)((aabb->_pos.x+ aabb->_size.x- _origin.x)* (number)(_n_cols)/ _size.x);
	int lig_min= (int)((aabb->_pos.y- _origin.y)* (number)(_n_ligs)/ _size.y);
	int lig_max= (int)((aabb->_pos.y+ aabb->_size.y- _origin.y)* (number)(_n_ligs)/ _size.y);

	for (int col=col_min; col<=col_max; ++col) {
		for (int lig=lig_min; lig<=lig_max; ++lig) {
			if ((col< 0) || (col>= _n_cols) || (lig< 0) || (lig>= _n_ligs)) {
				continue;
			}
			_vertices[col_lig2id(col, lig)]._weight= 10.0f;
			//_vertices[col_lig2id(col, lig)]._active= false;
		}
	}
}*/


std::ostream & operator << (std::ostream & os, GraphGrid & g) {
	os << static_cast<Graph &>(g);
	return os;
}


// ----------------------------------------------------------------------------------------
bool frontier_cmp(std::pair<uint, number> x, std::pair<uint, number> y) {
	return x.second> y.second;
}


PathFinder::PathFinder() {

}


PathFinder::PathFinder(uint n_ligs, uint n_cols, const pt_type & origin, const pt_type & size, bool is8connex) {
	_grid= new GraphGrid(n_ligs, n_cols, origin, size, is8connex);
}


PathFinder::~PathFinder() {
	clear();
	delete _grid;
}


void PathFinder::update_grid() {
	/*std::vector<uint> vertices_to_erase;
	_grid->_it_v= _grid->_vertices.begin();
	while (_grid->_it_v!= _grid->_vertices.end()) {
		for (auto poly : _polygons) {
			pt_type v(_grid->_it_v->second._pos);
			if (is_pt_inside_poly(v, poly)) {
			//if (distance_poly_pt(poly, _grid->_it_v->second._pos, NULL)< 0.1f) {
				vertices_to_erase.push_back(_grid->_it_v->first);
				break;
			}
		}
		_grid->_it_v++;
	}
	for (auto it_erase : vertices_to_erase) {
		_grid->remove_vertex(it_erase);
	}*/

	//std::vector<std::pair<uint, uint> > edges_to_erase;
	_grid->_it_v= _grid->_vertices.begin();
	while (_grid->_it_v!= _grid->_vertices.end()) {
		_grid->_it_e= _grid->_it_v->second._edges.begin();
		while (_grid->_it_e!= _grid->_it_v->second._edges.end()) {
			pt_type pt_begin= _grid->_it_v->second._pos;
			pt_type pt_end= _grid->_vertices[_grid->_it_e->first]._pos;
			for (auto poly : _polygons) {
				if (segment_intersects_poly(pt_begin, pt_end, poly, NULL)) {
					//edges_to_erase.push_back(std::make_pair(_grid->_it_v->first, _grid->_it_e->first));
					_grid->_vertices[_grid->_it_v->first]._edges[_grid->_it_e->first]._weight = 1000.0;
					break;
				}
			}
			_grid->_it_e++;
		}
		_grid->_it_v++;
	}
	/*for (auto it_erase : edges_to_erase) {
		_grid->remove_edge(it_erase.first, it_erase.second);
	}*/
}


void PathFinder::clear() {
	for (auto polygon : _polygons) {
		delete polygon;
	}
	_polygons.clear();
}


void PathFinder::read_shapefile(std::string shp_path, pt_type origin, pt_type size, bool reverse_y) {
	std::vector<Polygon2D *> polygons;
	read_shp(shp_path, polygons);
	for (auto poly : polygons) {
		Polygon2D * poly_reproj= new Polygon2D();
		std::vector<pt_type> pts;
		for (auto pt : poly->_pts) {
			number x= ((pt.x- origin.x)/ size.x)* _grid->_size.x+ _grid->_origin.x;
			number y;
			if (reverse_y) {
				y= ((origin.y- pt.y)/ size.y)* _grid->_size.y+ _grid->_origin.y;
			}
			else {
				y= ((pt.y- origin.y)/ size.y)* _grid->_size.y+ _grid->_origin.y;
			}
			pts.push_back(pt_type(x, y));
		}
		poly_reproj->set_points(pts);
		poly_reproj->update_all();
		_polygons.push_back(poly_reproj);
		delete poly;
	}
}


void PathFinder::rand(uint n_polys, uint n_pts_per_poly, number poly_radius) {
	for (uint i=0; i<n_polys; ++i) {
		Polygon2D * poly= new Polygon2D();
		number x= rand_number(_grid->_origin.x, _grid->_origin.x+ _grid->_size.x);
		number y= rand_number(_grid->_origin.y, _grid->_origin.y+ _grid->_size.y);
		poly->randomize(n_pts_per_poly, poly_radius, pt_type(x, y));
		_polygons.push_back(poly);
	}

	update_grid();
}


number PathFinder::cost(uint i, uint j) {
	return _grid->_vertices[i]._edges[j]._weight;
}


number PathFinder::heuristic(uint i, uint j) {
	//return abs(_grid->_vertices[i]._pos.x- _grid->_vertices[j]._pos.x)+ abs(_grid->_vertices[i]._pos.y- _grid->_vertices[j]._pos.y);
	return glm::distance(_grid->_vertices[i]._pos, _grid->_vertices[j]._pos);
}


bool PathFinder::line_of_sight(uint i, uint j) {
	pt_type pt_begin= _grid->_vertices[i]._pos;
	pt_type pt_end= _grid->_vertices[j]._pos;
	for (auto poly : _polygons) {
		if (segment_intersects_poly(pt_begin, pt_end, poly, NULL)) {
		//if (distance_poly_segment(poly, pt_begin, pt_end, NULL)< 0.1) {
			return false;
		}
	}
	return true;
}


bool PathFinder::line_of_sight(pt_type pt1, pt_type pt2) {
	for (auto poly : _polygons) {
		if (segment_intersects_poly(pt1, pt2, poly, NULL)) {
		//if (distance_poly_segment(poly, pt_begin, pt_end, NULL)< 0.1) {
			return false;
		}
	}
	return true;
}


bool PathFinder::path_find_nodes(uint start, uint goal, std::vector<uint> & path) {
	std::priority_queue< std::pair<uint, number>, std::vector<std::pair<uint, number> >, decltype(&frontier_cmp) > frontier(frontier_cmp);
	std::unordered_map<uint, uint> came_from;
	std::unordered_map<uint, number> cost_so_far;

	frontier.emplace(start, 0.0f);
	came_from[start]= start;
	cost_so_far[start]= 0.0f;

	while (!frontier.empty()) {
		uint current= frontier.top().first;
		frontier.pop();
		
		if (current== goal) {
			break;
		}

		std::vector<uint> nexts= _grid->neighbors(current);
		for (auto next : nexts) {
			number new_cost= cost_so_far[current]+ cost(current, next);
			if ((!cost_so_far.count(next)) || (new_cost< cost_so_far[next])) {
				cost_so_far[next]= new_cost;
				came_from[next]= current;
				//number priority= new_cost; // dijkstra
				//number priority= heuristic(next, goal); // greedy best first search
				number priority= new_cost+ heuristic(next, goal); // A *
				frontier.emplace(next, priority);
			}
		}
	}

	if (!came_from.count(goal)) {
		//cout << "disconnected\n";
		return false;
	}

	uint current = goal;
	while (current != start) {
		path.push_back(current);
		current = came_from[current];
		/*uint previous_sight = came_from[current];
		while (previous_sight != start && line_of_sight(current, came_from[previous_sight])) {
			previous_sight = came_from[previous_sight];
		}
		current = previous_sight;*/
	}
	path.push_back(start);
	std::reverse(path.begin(), path.end());

	return true;
}


bool PathFinder::path_find(pt_type start, pt_type goal, std::vector<pt_type> & path) {
	if ((!point_in_aabb(start, _grid->_aabb)) || (!point_in_aabb(goal, _grid->_aabb))) {
		std::cerr << "PathFinder::path_find : point hors grille\n";
		return false;
	}

	uint start_id = _grid->pt2id(start);
	uint goal_id = _grid->pt2id(goal);
	std::vector<uint> nodes;
	bool is_path_ok= path_find_nodes(start_id, goal_id, nodes);
	if (!is_path_ok) {
		std::cerr << "PathFinder::path_find : pas de chemin trouvé\n";
		return false;
	}


	/*uint start_col_min= (uint)(((start.x- _grid->_origin.x)/ _grid->_size.x)* (number)(_grid->_n_cols- 1));
	uint start_col_max= start_col_min+ 1;
	uint start_lig_min= (uint)(((start.y- _grid->_origin.y)/ _grid->_size.y)* (number)(_grid->_n_ligs- 1));
	uint start_lig_max= start_lig_min+ 1;
	std::vector<uint> start_ids;
	start_ids.push_back(_grid->col_lig2id(start_col_min, start_lig_min));
	start_ids.push_back(_grid->col_lig2id(start_col_max, start_lig_min));
	start_ids.push_back(_grid->col_lig2id(start_col_max, start_lig_max));
	start_ids.push_back(_grid->col_lig2id(start_col_min, start_lig_max));

	uint goal_col_min= (uint)(((goal.x- _grid->_origin.x)/ _grid->_size.x)* (number)(_grid->_n_cols- 1));
	uint goal_col_max= goal_col_min+ 1;
	uint goal_lig_min= (uint)(((goal.y- _grid->_origin.y)/ _grid->_size.y)* (number)(_grid->_n_ligs- 1));
	uint goal_lig_max= goal_lig_min+ 1;
	std::vector<uint> goal_ids;
	goal_ids.push_back(_grid->col_lig2id(goal_col_min, goal_lig_min));
	goal_ids.push_back(_grid->col_lig2id(goal_col_max, goal_lig_min));
	goal_ids.push_back(_grid->col_lig2id(goal_col_max, goal_lig_max));
	goal_ids.push_back(_grid->col_lig2id(goal_col_min, goal_lig_max));

	std::vector<uint> nodes;
	bool is_path_ok= path_find_nodes(start_ids[0], goal_ids[0], nodes);
	if (!is_path_ok) {
		return false;
	}
	
	uint start_idx= 0;
	for (uint i=1; i<nodes.size(); ++i) {
		if (std::find(start_ids.begin(), start_ids.end(), nodes[i])== start_ids.end()) {
			start_idx= i- 1;
			break;
		}
	}
	uint goal_idx= nodes.size()- 1;
	for (uint i=nodes.size()-2; i>=0; --i) {
		if (std::find(goal_ids.begin(), goal_ids.end(), nodes[i])== goal_ids.end()) {
			goal_idx= i+ 1;
			break;
		}
	}*/

	std::vector<pt_type> raw_path;
	raw_path.push_back(start);
	for (uint i=0; i<nodes.size(); ++i) {
		raw_path.push_back(_grid->_vertices[nodes[i]]._pos);
	}
	raw_path.push_back(goal);

	path.clear();
	//path.push_back(start);
	uint idx = 1;
	uint last = 0;
	while (idx < raw_path.size()) {
		std::cout << "last = " << last << " ; idx = " << idx << "\n";
		path.push_back(raw_path[last]);
		while (idx < raw_path.size() && line_of_sight(raw_path[last], raw_path[idx])) {
			idx++;
		}
		last = idx - 1;
		/*if (idx != raw_path.size()) {
			path.push_back(raw_path[last]);
		}*/
	}
	path.push_back(goal);
	
	/*for (auto p : raw_path) {
		path.push_back(p);
	}*/

	return true;
}


void PathFinder::draw_svg(const std::vector<uint> & path, std::string svg_path) {
	std::ofstream f;
	f.open(svg_path);
	f << "<!DOCTYPE html>\n<html>\n<body>\n";
	f << "<svg width=\"1000\" height=\"1000\" viewbox=\"" << _grid->_origin.x << " " << _grid->_origin.y << " " << _grid->_size.x << " " << _grid->_size.y << "\">\n";

	if (path.size()) {
		for (uint i=0; i<path.size()- 1; ++i) {
			number x1= _grid->_vertices[path[i]]._pos.x;
			number y1= _grid->_vertices[path[i]]._pos.y;
			number x2= _grid->_vertices[path[i+ 1]]._pos.x;
			number y2= _grid->_vertices[path[i+ 1]]._pos.y;
			f << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" stroke=\"red\" stroke-width=\"0.06\" />\n";
		}

		number radius = 0.1;
		std::string color_start = "green";
		number x_start= _grid->_vertices[path[0]]._pos.x;
		number y_start= _grid->_vertices[path[0]]._pos.y;
		f << "<circle cx=\"" << x_start << "\" cy=\"" << y_start << "\" r=\"" << radius << "\" fill=\"" << color_start << "\" />\n";
		std::string color_end = "orange";
		number x_end= _grid->_vertices[path[path.size() - 1]]._pos.x;
		number y_end= _grid->_vertices[path[path.size() - 1]]._pos.y;
		f << "<circle cx=\"" << x_end << "\" cy=\"" << y_end << "\" r=\"" << radius << "\" fill=\"" << color_end << "\" />\n";
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
		number x1= _grid->_it_v->second._pos.x;
		number y1= _grid->_it_v->second._pos.y;
		std::string color= "black";
		number radius= 0.01* _grid->_it_v->second._weight;
		f << "<circle cx=\"" << x1 << "\" cy=\"" << y1 << "\" r=\"" << radius << "\" fill=\"" << color << "\" />\n";
		//f << "<text x=\"" << x1+ 0.15f << "\" y=\"" << y1- 0.15f << "\" fill=\"black\" font-size=\"0.2px\">" << to_string(_grid->_it_v->first) << "</text>\n";

		_grid->_it_e= _grid->_it_v->second._edges.begin();
		while (_grid->_it_e!= _grid->_it_v->second._edges.end()) {
			number x2= _grid->_vertices[_grid->_it_e->first]._pos.x;
			number y2= _grid->_vertices[_grid->_it_e->first]._pos.y;
			f << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2 << "\" stroke=\"black\" stroke-width=\"0.01\" />\n";
			_grid->_it_e++;
		}

		_grid->_it_v++;
	}
	
	f << "</svg>\n</body>\n</html>\n";
	f.close();
}


