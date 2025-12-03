#include <queue>
#include <fstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "json.hpp"

#include "path_find.h"
#include "utile.h"
#include "shapefile.h"

using json = nlohmann::json;


OBSTACLE_TYPE str2type(std::string s) {
	if (s == "GROUND") {
		return GROUND;
	}
	else if (s == "SOLID") {
		return SOLID;
	}
	else if (s == "WATER") {
		return WATER;
	}
	std::cerr << s << " : type d'obstacle non reconnu\n";
	return UNKNOWN;
}


// -------------------------------------------------
UnitType::UnitType() {

}


UnitType::UnitType(std::string json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	_name = js["name"];
	_size = pt_type(js["size"][0], js["size"][1]);
	_velocity = js["velocity"];
	for (json::iterator it = js["weights"].begin(); it != js["weights"].end(); ++it) {
		OBSTACLE_TYPE ot = str2type(it.key());
		if (ot == UNKNOWN) {
			continue;
		}
		_weights[ot] = it.value();
	}
}


UnitType::~UnitType() {

}


Unit::Unit() {

}


Unit::Unit(UnitType * type, pt_type pos, GraphGrid * grid) : _type(type), _selected(false), _idx_path(0), _mode(WAITING), _grid(grid) {
	_aabb = new AABB_2D(pos - 0.5 * _type->_size, _type->_size);
}


Unit::~Unit() {
	
}


void Unit::clear_path() {
	_path.clear();
	_idx_path = 0;
	_mode = WAITING;
}


// ----------------------------------------------------------------------------------------
Obstacle::Obstacle() {

}


Obstacle::Obstacle(OBSTACLE_TYPE type, const std::vector<pt_type> & pts) : _type(type) {
	_polygon = new Polygon2D(pts);
	_polygon->update_all();
}


Obstacle::~Obstacle() {
	delete _polygon;
}


// ----------------------------------------------------------------------------------------
bool frontier_cmp(std::pair<uint, number> x, std::pair<uint, number> y) {
	return x.second> y.second;
}


PathFinder::PathFinder() {

}


PathFinder::~PathFinder() {

}


number PathFinder::cost(uint i, uint j, GraphGrid * grid) {
	return grid->_vertices[i]._edges[j]._weight;
}


number PathFinder::heuristic(uint i, uint j, GraphGrid * grid) {
	return glm::distance(grid->_vertices[i]._pos, grid->_vertices[j]._pos);
}


bool PathFinder::line_of_sight(pt_type pt1, pt_type pt2, GraphGrid * grid) {
	grid->_it_v= grid->_vertices.begin();
	while (grid->_it_v!= grid->_vertices.end()) {
		grid->_it_e= grid->_it_v->second._edges.begin();
		while (grid->_it_e!= grid->_it_v->second._edges.end()) {
			pt_type pt_begin= grid->_it_v->second._pos;
			pt_type pt_end= grid->_vertices[grid->_it_e->first]._pos;
			if (segment_intersects_segment(pt1, pt2, pt_begin, pt_end, NULL, true, true)) {
				if (grid->_it_e->second._weight > 2.0) {
					return false;
				}
			}
			grid->_it_e++;
		}
		grid->_it_v++;
	}

	return true;
}


bool PathFinder::path_find_nodes(uint start, uint goal, GraphGrid * grid, std::vector<uint> & path) {
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

		std::vector<uint> nexts= grid->neighbors(current);
		for (auto next : nexts) {
			number new_cost= cost_so_far[current]+ cost(current, next, grid);
			if ((!cost_so_far.count(next)) || (new_cost< cost_so_far[next])) {
				cost_so_far[next]= new_cost;
				came_from[next]= current;
				//number priority= new_cost; // dijkstra
				//number priority= heuristic(next, goal); // greedy best first search
				number priority= new_cost+ heuristic(next, goal, grid); // A *
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
	}
	path.push_back(start);
	std::reverse(path.begin(), path.end());

	return true;
}


bool PathFinder::path_find(pt_type start, pt_type goal, GraphGrid * grid, std::vector<pt_type> & path) {
	if ((!point_in_aabb(start, grid->_aabb)) || (!point_in_aabb(goal, grid->_aabb))) {
		std::cerr << "PathFinder::path_find : point hors grille\n";
		return false;
	}

	uint start_id = grid->pt2id(start);
	uint goal_id = grid->pt2id(goal);
	std::vector<uint> nodes;
	bool is_path_ok= path_find_nodes(start_id, goal_id, grid, nodes);
	if (!is_path_ok) {
		std::cerr << "PathFinder::path_find : pas de chemin trouvé\n";
		return false;
	}

	std::vector<pt_type> raw_path;
	raw_path.push_back(start);
	for (uint i=0; i<nodes.size(); ++i) {
		raw_path.push_back(grid->_vertices[nodes[i]]._pos);
	}
	raw_path.push_back(goal);

	path.clear();
	
	bool use_line_of_sight = false;
	if (use_line_of_sight) {
		uint idx = 1;
		uint last = 0;
		//std::cout << "raw_path size=" << raw_path.size() << "\n";
		while (idx < raw_path.size()) {
			//std::cout << "last = " << last << " ; idx = " << idx << "\n";
			path.push_back(raw_path[last]);
			
			while (idx < raw_path.size() && line_of_sight(raw_path[last], raw_path[idx], grid)) {
				//std::cout << "line of sight ok entre " << last << " et " << idx << "\n";
				idx++;
			}

			if (idx == last + 1) {
				//std::cout << "bug idx = " << idx << " last = " << last << "\n";
				idx++;
			}
			
			//idx++;
			
			last = idx - 1;
			/*if (idx != raw_path.size()) {
				path.push_back(raw_path[last]);
			}*/
		}
		path.push_back(goal);
	}
	else {
		for (auto p : raw_path) {
			path.push_back(p);
		}
	}

	return true;
}


// ---------------------------------------------------------------------------------------------------
Map::Map() {

}


Map::Map(std::string unit_types_dir, pt_type origin, pt_type size, pt_type path_resolution) : _origin(origin), _size(size) {
	uint n_ligs = uint(_size.y / path_resolution.y);
	uint n_cols = uint(_size.x / path_resolution.x);

	std::vector<std::string> jsons_paths = list_files(unit_types_dir, "json");
	for (auto json_path : jsons_paths) {
		_unit_types[basename(json_path)] = new UnitType(json_path);
		_grids[_unit_types[basename(json_path)]] = new GraphGrid(n_ligs, n_cols, _origin, _size);
	}
	_path_finder = new PathFinder();
}


Map::~Map() {

}


void Map::add_unit(std::string type_name, pt_type pos) {
	if (pos.x < _origin.x || pos.y < _origin.y || pos.x >= _origin.x + _size.x || pos.y >= _origin.y + _size.y) {
		std::cerr << "Map::add_unit hors terrain\n";
		return;
	}
	GraphGrid * grid = _grids[_unit_types[type_name]];
	_units.push_back(new Unit(_unit_types[type_name], pos, grid));
}


void Map::add_obstacle(OBSTACLE_TYPE type, const std::vector<pt_type> & pts) {
	for (auto pt : pts) {
		if (pt.x < _origin.x || pt.y < _origin.y || pt.x >= _origin.x + _size.x || pt.y >= _origin.y + _size.y) {
			std::cerr << "Map::add_obstacle hors terrain\n";
			return;
		}
	}
	_obstacles.push_back(new Obstacle(type, pts));
}


void Map::update_grids() {
	const number EPS = 0.1;
	for (auto type_grid : _grids) {
		//std::cout << type_grid.first->_name << "\n";
		GraphGrid * grid = type_grid.second;

		grid->_it_v= grid->_vertices.begin();
		while (grid->_it_v!= grid->_vertices.end()) {
			grid->_it_e= grid->_it_v->second._edges.begin();
			while (grid->_it_e!= grid->_it_v->second._edges.end()) {
				pt_type pt_begin= grid->_it_v->second._pos;
				pt_type pt_end= grid->_vertices[grid->_it_e->first]._pos;
				for (auto obstacle : _obstacles) {
					//if (segment_intersects_poly(pt_begin, pt_end, obstacle->_polygon, NULL)) {
					if (distance_poly_segment(obstacle->_polygon, pt_begin, pt_end, NULL) < 0.5 * std::max(type_grid.first->_size.x, type_grid.first->_size.y) + EPS) {
						grid->_vertices[grid->_it_v->first]._edges[grid->_it_e->first]._weight = type_grid.first->_weights[obstacle->_type];
						break;
					}
				}
				grid->_it_e++;
			}
			grid->_it_v++;
		}
	}
}


void Map::clear() {
	for (auto obstacle : _obstacles) {
		delete obstacle;
	}
	_obstacles.clear();
	for (auto unit : _units) {
		delete unit;
	}
	_units.clear();
}


void Map::read_shapefile(std::string shp_path, pt_type origin, pt_type size, bool reverse_y) {
	std::vector<ShpEntry *> entries;
	read_shp(shp_path, entries);
	for (auto entry : entries) {
		std::vector<pt_type> pts;
		for (auto pt : entry->_polygon->_pts) {
			number x= ((pt.x- origin.x)/ size.x)* _size.x+ _origin.x;
			number y;
			if (reverse_y) {
				y= ((origin.y- pt.y)/ size.y)* _size.y+ _origin.y;
			}
			else {
				y= ((pt.y- origin.y)/ size.y)* _size.y+ _origin.y;
			}
			pts.push_back(pt_type(x, y));
		}
		OBSTACLE_TYPE obst_type = str2type(entry->_fields["type"]);
		if (obst_type != UNKNOWN) {
			add_obstacle(obst_type, pts);
		}
		delete entry;
	}
}


void Map::anim() {
	for (auto unit : _units) {
		if (unit->_mode == MOVING) {
			if (glm::distance(unit->_aabb->center(), unit->_path[unit->_idx_path]) < 0.1) {
				unit->_idx_path++;
				if (unit->_idx_path == unit->_path.size()) {
					unit->_mode = WAITING;
					unit->clear_path();
					continue;
				}
			}

			unit->_aabb->_pos += unit->_type->_velocity * glm::normalize(unit->_path[unit->_idx_path] - unit->_aabb->_size * 0.5 - unit->_aabb->_pos);
		}
	}
}


void Map::selected_units_goto(pt_type pt) {
	for (auto unit : _units) {
		if (unit->_selected) {
			unit->clear_path();
			_path_finder->path_find(unit->_aabb->center(), pt, _grids[unit->_type], unit->_path);
			unit->_mode = MOVING;
		}
	}
}


/*void Map::rand(uint n_polys, uint n_pts_per_poly, number poly_radius) {
	for (uint i=0; i<n_polys; ++i) {
		Polygon2D * poly= new Polygon2D();
		number x= rand_number(_grid->_origin.x, _grid->_origin.x+ _grid->_size.x);
		number y= rand_number(_grid->_origin.y, _grid->_origin.y+ _grid->_size.y);
		poly->randomize(n_pts_per_poly, poly_radius, pt_type(x, y));
		_polygons.push_back(poly);
	}

	update_grid();
}
*/


/*void Map::draw_svg(const std::vector<uint> & path, std::string svg_path) {
	std::ofstream f;
	f.open(svg_path);
	f << "<!DOCTYPE html>\n<html>\n<body>\n";
	f << "<svg width=\"1000\" height=\"1000\" viewbox=\"" << _origin.x << " " << _origin.y << " " << _size.x << " " << _size.y << "\">\n";

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
*/
