#include <queue>
#include <fstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "json.hpp"

#include "path_find.h"
#include "utile.h"
#include "shapefile.h"

using json = nlohmann::json;


std::string mode2str(UNIT_MODE mode) {
	if (mode == WAITING) {
		return "WAITING";
	}
	else if (mode == MOVING) {
		return "MOVING";
	}
	std::cerr << mode << " : mode unit reconnu\n";
	return "UNKNOWN";
}


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


std::string type2str(OBSTACLE_TYPE t) {
	if (t == GROUND) {
		return "GROUND";
	}
	else if (t == SOLID) {
		return "SOLID";
	}
	else if (t == WATER) {
		return "WATER";
	}
	std::cerr << t << " : type d'obstacle non reconnu\n";
	return "UNKNOWN";
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

	for (auto & ec : js["elevation_coeffs"]) {
		number ec_min = ec["min"];
		number ec_max = ec["max"];
		number ec_coeff = ec["coeff"];
		UnitElevationCoeff coeff = {ec_min, ec_max, ec_coeff};
		_elevation_coeffs.push_back(coeff);
	}
}


UnitType::~UnitType() {

}


number UnitType::elevation_coeff(number elevation) {
	for (auto & ec : _elevation_coeffs) {
		if (elevation >= ec._elevation_min && elevation <= ec._elevation_max) {
			return ec._coeff;
		}
	}
	std::cerr << "UnitType " << _name << " ::elevation_coeff : " << elevation << " non gérée\n";
	return 0.0;
}


std::ostream & operator << (std::ostream & os, UnitType & ut) {
	os << "name = " << ut._name << " ; size = " << glm::to_string(ut._size) << " ; velocity = " << ut._velocity;
	os << "\nweights = ";
	for (auto & w : ut._weights) {
		os << type2str(w.first) << " -> " << w.second << " ; ";
	}
	os << "\nelevation_coeffs = ";
	for (auto & ec : ut._elevation_coeffs) {
		os << "[elevation_min = " << ec._elevation_min  << " ; elevation_max = " << ec._elevation_max << " ; coeff = " << ec._coeff << "] ; ";
	}
	return os;
}


// -------------------------------------------------
Unit::Unit() {

}


Unit::Unit(UnitType * type, pt_type pos, GraphGrid * grid) : _type(type), _selected(false), _idx_path(0), _mode(WAITING), _grid(grid) {
	_aabb = new AABB_2D(pos - 0.5 * _type->_size, _type->_size);
}


Unit::~Unit() {
	delete _aabb;
}


void Unit::clear_path() {
	_path.clear();
	_idx_path = 0;
	_mode = WAITING;
}


std::ostream & operator << (std::ostream & os, Unit & unit) {
	os << "type = " << unit._type->_name << " ; selected = " << unit._selected << " ; aabb = " << *unit._aabb << " ; mode = " << mode2str(unit._mode);
	return os;
}


// ----------------------------------------------------------------------------------------
Obstacle::Obstacle() {

}


Obstacle::Obstacle(OBSTACLE_TYPE type, const std::vector<pt_type> & pts) : _type(type) {
	_polygon = new Polygon2D(pts);
	_polygon->update_all();
}


Obstacle::Obstacle(OBSTACLE_TYPE type, Polygon2D * polygon) : _type(type), _polygon(polygon) {
	_polygon->update_all();
}


Obstacle::~Obstacle() {
	delete _polygon;
}


// ----------------------------------------------------------------------------------------
Terrain::Terrain() {

}


Terrain::Terrain(pt_type origin, pt_type size, uint n_ligs, uint n_cols) : _origin(origin), _size(size), _n_ligs(n_ligs), _n_cols(n_cols) {
	_altis = new number[_n_ligs * _n_cols];
	for (uint i=0; i<_n_ligs * _n_cols; ++i) {
		_altis[i] = 0.0;
	}
}


Terrain::~Terrain() {
	delete _altis;
}


std::pair<uint, uint> Terrain::id2col_lig(uint id) {
	return std::make_pair(id % _n_cols, id/ _n_cols);
}


uint Terrain::col_lig2id(uint col, uint lig) {
	return col+ _n_cols* lig;
}


pt_type Terrain::col_lig2pt(uint col, uint lig) {
	return pt_type(
		_origin.x+ ((number)(col)/ (number)(_n_cols - 1))* _size.x,
		_origin.y+ ((number)(lig)/ (number)(_n_ligs - 1))* _size.y
	);
}


number Terrain::get_alti(int col, int lig) {
	if (col < 0 || lig < 0 || col >= _n_cols || lig >= _n_ligs) {
		std::cerr << "Terrain::get_alti : (" << col << " ; " << lig << ") hors terrain (2!)\n";
		return 0.0;
	}
	return _altis[col_lig2id(col, lig)];
}


// TODO : interpolation linéaire ?
number Terrain::get_alti(pt_type pt) {
	if (pt.x < _origin.x || pt.x > _origin.x + _size.x || pt.y < _origin.y || pt.y > _origin.y + _size.y) {
		std::cerr << "Terrain::get_alti : " << glm::to_string(pt) << " hors terrain\n";
		return 0.0;
	}
	int col= (int)(((pt.x- _origin.x)/ _size.x)* (number)(_n_cols- 1));
	int lig= (int)(((pt.y- _origin.y)/ _size.y)* (number)(_n_ligs- 1));
	return get_alti(col, lig);
}


number Terrain::get_alti_over_polygon(Polygon2D * polygon) {
	number result = 0.0;
	uint n_pts = 0;
	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			pt_type pt = col_lig2pt(col, lig);
			if (is_pt_inside_poly(pt, polygon)) {
				n_pts++;
				result += get_alti(col, lig);
			}
		}
	}
	if (n_pts > 0) {
		result /= number(n_pts);
		return result;
	}
	else {
		std::cerr << "Terrain::get_alti_over_polygon pas de pt dans polygon\n";
		return 0.0;
	}
}


void Terrain::set_alti(int col, int lig, number alti) {
	_altis[col_lig2id(col, lig)] = alti;
}


void Terrain::set_alti_over_polygon(Polygon2D * polygon, number alti) {
	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			pt_type pt = col_lig2pt(col, lig);
			if (is_pt_inside_poly(pt, polygon)) {
				set_alti(col, lig, alti);
			}
		}
	}
}


void Terrain::set_alti_all(number alti) {
	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			set_alti(col, lig, alti);
		}
	}
}


void Terrain::randomize() {
	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			//_altis[col_lig2id(col, lig)] = rand_number(0.0, 1000.0);
			pt_type pt = col_lig2pt(col, lig);
			_altis[col_lig2id(col, lig)] = rand_number(300.0, 1000.0) * exp(-1.0 * pow(glm::distance(pt, _origin + 0.5 * _size) / (0.5 * _size.x), 2.0));
		}
	}
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


number PathFinder::line_of_sight_max_weight(pt_type pt1, pt_type pt2, GraphGrid * grid) {
	number result = -1000.0;
	grid->_it_v= grid->_vertices.begin();
	while (grid->_it_v!= grid->_vertices.end()) {
		grid->_it_e= grid->_it_v->second._edges.begin();
		while (grid->_it_e!= grid->_it_v->second._edges.end()) {
			pt_type pt_begin= grid->_it_v->second._pos;
			pt_type pt_end= grid->_vertices[grid->_it_e->first]._pos;
			if (segment_intersects_segment(pt1, pt2, pt_begin, pt_end, NULL, true, true)) {
				if (grid->_it_e->second._weight > result) {
					result = grid->_it_e->second._weight;
				}
			}
			grid->_it_e++;
		}
		grid->_it_v++;
	}

	return result;
}


bool PathFinder::path_find_nodes(uint start, uint goal, GraphGrid * grid, std::vector<uint> & path) {
	std::priority_queue< std::pair<uint, number>, std::vector<std::pair<uint, number> >, decltype(&frontier_cmp) > frontier(frontier_cmp);
	std::unordered_map<uint, uint> came_from;
	std::unordered_map<uint, number> cost_so_far;

	frontier.emplace(start, 0.0);
	came_from[start]= start;
	cost_so_far[start]= 0.0;

	while (!frontier.empty()) {
		uint current= frontier.top().first;
		frontier.pop();
		
		if (current== goal) {
			break;
		}

		std::vector<uint> nexts= grid->neighbors(current);
		for (auto & next : nexts) {
			number new_cost= cost_so_far[current]+ cost(current, next, grid);
			if ((!cost_so_far.count(next)) || (new_cost< cost_so_far[next])) {
				cost_so_far[next]= new_cost;
				came_from[next]= current;
				number priority= new_cost; // dijkstra
				//number priority= heuristic(next, goal, grid); // greedy best first search
				//number priority= new_cost+ heuristic(next, goal, grid); // A *
				//std::cout << priority << "\n";
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
			
			while (idx < raw_path.size() && line_of_sight_max_weight(raw_path[last], raw_path[idx], grid) < 100.0) {
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
		for (auto & p : raw_path) {
			path.push_back(p);
		}
	}

	return true;
}


// ---------------------------------------------------------------------------------------------------
Map::Map() {

}


Map::Map(std::string unit_types_dir, pt_type origin, pt_type size, pt_type path_resolution, pt_type terrain_resolution) : _origin(origin), _size(size) {
	uint n_ligs_path = uint(_size.y / path_resolution.y);
	uint n_cols_path = uint(_size.x / path_resolution.x);

	uint n_ligs_terrain = uint(_size.y / terrain_resolution.y);
	uint n_cols_terrain = uint(_size.x / terrain_resolution.x);

	std::vector<std::string> jsons_paths = list_files(unit_types_dir, "json");
	for (auto & json_path : jsons_paths) {
		_unit_types[basename(json_path)] = new UnitType(json_path);
		_grids[_unit_types[basename(json_path)]] = new GraphGrid(_origin, _size, n_ligs_path, n_cols_path);
	}
	_path_finder = new PathFinder();
	_terrain = new Terrain(_origin, _size, n_ligs_terrain, n_cols_terrain);
	//_terrain->randomize();

	update_grids();
}


Map::~Map() {
	clear();
	for (auto & ut : _unit_types) {
		delete ut.second;
	}
	_unit_types.clear();
	for (auto & grid : _grids) {
		delete grid.second;
	}
	_grids.clear();
	delete _path_finder;
	delete _terrain;
}


void Map::add_unit(std::string type_name, pt_type pos) {
	if (pos.x < _origin.x || pos.y < _origin.y || pos.x >= _origin.x + _size.x || pos.y >= _origin.y + _size.y) {
		std::cerr << "Map::add_unit hors terrain\n";
		return;
	}
	GraphGrid * grid = _grids[_unit_types[type_name]];
	_units.push_back(new Unit(_unit_types[type_name], pos, grid));
}


Obstacle * Map::add_obstacle(OBSTACLE_TYPE type, const std::vector<pt_type> & pts) {
	const number EPS = 0.1;

	for (auto & pt : pts) {
		if (pt.x < _origin.x || pt.y < _origin.y || pt.x >= _origin.x + _size.x || pt.y >= _origin.y + _size.y) {
			std::cerr << "Map::add_obstacle hors terrain\n";
			return NULL;
		}
	}
	Obstacle * obstacle = new Obstacle(type, pts);
	_obstacles.push_back(obstacle);

	for (auto & unit_type : _unit_types) {
		Polygon2D * buffered_polygon = obstacle->_polygon->buffered(std::max(unit_type.second->_size.x, unit_type.second->_size.y) + EPS);
		_buffered_obstacles[unit_type.second].push_back(new Obstacle(type, buffered_polygon));
	}

	return obstacle;
}


void Map::update_grids() {
	//const number EPS = 0.1;
	for (auto & type_grid : _grids) {
		//std::cout << type_grid.first->_name << "\n";
		GraphGrid * grid = type_grid.second;

		grid->_it_v= grid->_vertices.begin();
		while (grid->_it_v!= grid->_vertices.end()) {
			grid->_it_e= grid->_it_v->second._edges.begin();
			while (grid->_it_e!= grid->_it_v->second._edges.end()) {
				pt_type pt_begin= grid->_it_v->second._pos;
				pt_type pt_end= grid->_vertices[grid->_it_e->first]._pos;
				number alti_begin = _terrain->get_alti(pt_begin);
				number alti_end = _terrain->get_alti(pt_end);
				grid->_vertices[grid->_it_v->first]._edges[grid->_it_e->first]._weight = type_grid.first->elevation_coeff(alti_end - alti_begin);
				//std::cout << alti_end << " ; " << alti_begin << " ; " << type_grid.first->elevation_coeff(alti_end - alti_begin) << "\n";
				for (auto & obstacle : _buffered_obstacles[type_grid.first]) {
					if (segment_intersects_poly(pt_begin, pt_end, obstacle->_polygon, NULL)) {
					//if (distance_poly_segment(obstacle->_polygon, pt_begin, pt_end, NULL) < 0.5 * std::max(type_grid.first->_size.x, type_grid.first->_size.y) + EPS) {
						grid->_vertices[grid->_it_v->first]._edges[grid->_it_e->first]._weight += type_grid.first->_weights[obstacle->_type];
						/*if (type_grid.first->_name == "tank") {
							std::cout << type_grid.first->_weights[obstacle->_type] << "\n";
						}*/
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
	for (auto & obstacle : _obstacles) {
		delete obstacle;
	}
	_obstacles.clear();
	for (auto & buffered_obstacle : _buffered_obstacles) {
		for (auto & obst : buffered_obstacle.second) {
			delete obst;
		}
		buffered_obstacle.second.clear();
	}
	for (auto & unit : _units) {
		delete unit;
	}
	_units.clear();
	_terrain->set_alti_all(0.0);
	update_grids();
}


void Map::read_shapefile(std::string shp_path, pt_type origin, pt_type size, bool reverse_y) {
	std::vector<ShpEntry *> entries;
	read_shp(shp_path, entries);
	for (auto & entry : entries) {
		std::vector<pt_type> pts;
		for (auto & pt : entry->_polygon->_pts) {
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
	for (auto & unit : _units) {
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
	for (auto & unit : _units) {
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


std::ostream & operator << (std::ostream & os, Map & map) {
	os << "unit_types = ";
	for (auto & ut : map._unit_types) {
		os << *ut.second << "\n";
	}
	os << "units = ";
	for (auto & unit : map._units) {
		os << *unit << "\n";
	}
	return os;
}

