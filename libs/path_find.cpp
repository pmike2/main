#include <queue>
#include <fstream>
#include <tuple>
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "json.hpp"

#include "path_find.h"
#include "utile.h"
#include "shapefile.h"

using json = nlohmann::json;


uint Map::_next_unit_id = 1;


std::string unit_status2str(UNIT_STATUS mode) {
	if (mode == WAITING) {
		return "WAITING";
	}
	else if (mode == MOVING) {
		return "MOVING";
	}
	else if (mode == COMPUTING_PATH) {
		return "COMPUTING_PATH";
	}
	else if (mode == COMPUTING_PATH_DONE) {
		return "COMPUTING_PATH_DONE";
	}
	else if (mode == COMPUTING_PATH_FAILED) {
		return "COMPUTING_PATH_FAILED";
	}

	std::cerr << mode << " : mode unit reconnu\n";
	return "UNKNOWN";
}


TERRAIN_TYPE str2terrain_type(std::string s) {
	if (s == "GROUND") {
		return GROUND;
	}
	else if (s == "OBSTACLE") {
		return OBSTACLE;
	}
	else if (s == "WATER") {
		return WATER;
	}
	std::cerr << s << " : type d'obstacle non reconnu\n";
	return UNKNOWN;
}


std::string terrain_type2str(TERRAIN_TYPE t) {
	if (t == GROUND) {
		return "GROUND";
	}
	else if (t == OBSTACLE) {
		return "OBSTACLE";
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
	_size = pt_3d(js["size"][0], js["size"][1], js["size"][2]);
	_max_velocity = js["max_velocity"];
	for (json::iterator it = js["weights"].begin(); it != js["weights"].end(); ++it) {
		TERRAIN_TYPE ot = str2terrain_type(it.key());
		if (ot == UNKNOWN) {
			continue;
		}
		_terrain_weights[ot] = it.value();
	}

	for (auto & ec : js["delta_elevation_coeffs"]) {
		number ec_min = ec["min"];
		number ec_max = ec["max"];
		number ec_coeff = ec["coeff"];
		UnitElevationCoeff coeff = {ec_min, ec_max, ec_coeff};
		_delta_elevation_coeffs.push_back(coeff);
	}
}


UnitType::~UnitType() {

}


number UnitType::elevation_coeff(number delta_elevation) {
	for (auto & ec : _delta_elevation_coeffs) {
		if (delta_elevation >= ec._elevation_min && delta_elevation <= ec._elevation_max) {
			return ec._coeff;
		}
	}
	std::cerr << "UnitType " << _name << " ::elevation_coeff : " << delta_elevation << " non gérée\n";
	return 0.0;
}


std::ostream & operator << (std::ostream & os, UnitType & ut) {
	os << "name = " << ut._name << " ; size = " << glm::to_string(ut._size) << " ; velocity = " << ut._max_velocity;
	os << "\nterrain_weights = ";
	for (auto & w : ut._terrain_weights) {
		os << terrain_type2str(w.first) << " -> " << w.second << " ; ";
	}
	os << "\nelevation_coeffs = ";
	for (auto & ec : ut._delta_elevation_coeffs) {
		os << "[elevation_min = " << ec._elevation_min  << " ; elevation_max = " << ec._elevation_max << " ; coeff = " << ec._coeff << "] ; ";
	}
	return os;
}


// -------------------------------------------------
Path::Path() : _idx_path(0) {

}


Path::~Path() {
	clear();
}


void Path::clear() {
	_idx_path = 0;
	_pts.clear();
	_nodes.clear();
	_weights.clear();
	for (auto & bbox : _bboxs) {
		delete bbox;
	}
	_bboxs.clear();
	_start = pt_3d(0.0);
	_goal = pt_3d(0.0);
}


bool Path::empty() {
	if (_pts.size() == 0) {
		return true;
	}
	return false;
}


std::ostream & operator << (std::ostream & os, Path & p) {
	os << "pts = ";
	for (auto pt : p._pts) {
		os << glm_to_string(pt, 1) << " ; ";
	}
	/*os << " | nodes = ";
	for (auto node : p._nodes) {
		os << node << " ; ";
	}*/
	os << " | weights = ";
	for (auto w : p._weights) {
		os << w << " ; ";
	}
	os << " | idx_path = " << p._idx_path;
	return os;
}


// -------------------------------------------------
Unit::Unit() {

}


Unit::Unit(UnitType * type, pt_3d pos, time_point t) :
	_type(type), _selected(false), _status(WAITING), _velocity(pt_3d(0.0)), _last_anim_t(t)
{
	//_aabb = new AABB(pos - 0.5 * _type->_size, pos + 0.5 * _type->_size);
	pt_3d vmin(pos.x - 0.5 * _type->_size.x, pos.y - 0.5 * _type->_size.y, pos.z);
	pt_3d vmax(pos.x + 0.5 * _type->_size.x, pos.y + 0.5 * _type->_size.y, pos.z + _type->_size.z);
	_aabb = new AABB(vmin, vmax);
	_path = new Path();
}


Unit::~Unit() {
	delete _aabb;
	delete _path;
}


void Unit::anim(time_point t) {
	if (_status == MOVING) {
		auto d= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_anim_t).count();
		_last_anim_t = t;

		//pt_3d v = number(d) * _velocity;
		pt_3d v = 15.0 * _velocity;
		//pt_3d v = 0.02 * glm::normalize(_velocity);

		number dist = glm::distance(_aabb->bottom_center(), _path->_pts[_path->_idx_path]);
		
		number speed = glm::length(v);
		if (speed < 0.001) {
			std::cerr << "speed = 0\n";
			return;
		}

		if (dist < speed) {
			v *= (dist / speed);
		}

		_aabb->translate(v);
	}
}


bool Unit::checkpoint_checked() {
	if (_path->_pts.size() == 0 || _path->_idx_path > _path->_pts.size() - 1) {
		std::cerr << "Unit::checkpoint_checked() : path vide ou dépassé\n";
		return true;
	}
	if (glm::distance(_aabb->bottom_center(), _path->_pts[_path->_idx_path]) < UNIT_DIST_PATH_EPS) {
		return true;
	}
	return false;
}


void Unit::goto_next_checkpoint(time_point t) {
	_status = MOVING;

	if (_path->empty()) {
		std::cerr << "Unit::follow_path : path empty\n";
		stop();
		return;
	}

	number velocity_amp = _type->_max_velocity * (1.0 - _path->_weights[_path->_idx_path] / MAX_UNIT_MOVING_WEIGHT);
	pt_3d direction = glm::normalize(_path->_pts[_path->_idx_path] - _aabb->bottom_center());
	//_velocity = velocity_amp * direction;
	_velocity = velocity_amp * pt_3d(direction.x, direction.y, 0.0);
	_last_anim_t = t;
}


void Unit::stop() {
	_status = WAITING;
	_path->clear();
}


std::ostream & operator << (std::ostream & os, Unit & unit) {
	os << "type = " << unit._type->_name;
	os << " ; mode = " << unit_status2str(unit._status);
	os << " ; aabb = " << *unit._aabb;
	os << " ; velocity = " << glm_to_string(unit._velocity);
	os << " ; path = " << *unit._path;
	return os;
}


// ----------------------------------------------------------------------------------------
/*Obstacle::Obstacle() {

}


Obstacle::Obstacle(OBSTACLE_TYPE type, const std::vector<pt_2d> & pts) : _type(type) {
	_polygon = new Polygon2D(pts);
	_polygon->update_all();
}


Obstacle::Obstacle(OBSTACLE_TYPE type, Polygon2D * polygon) : _type(type), _polygon(polygon) {
	_polygon->update_all();
}


Obstacle::~Obstacle() {
	delete _polygon;
}*/


// ----------------------------------------------------------------------------------------
Elevation::Elevation() {

}


Elevation::Elevation(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols) :
	_origin(origin), _size(size), _n_ligs(n_ligs), _n_cols(n_cols) 
{
	_resolution = pt_2d(_size.x / (number)(_n_cols- 1), _size.y / (number)(_n_ligs- 1));

	_altis = new number[_n_ligs * _n_cols];
	for (uint i=0; i<_n_ligs * _n_cols; ++i) {
		_altis[i] = 0.0;
	}
}


Elevation::~Elevation() {
	delete _altis;
}


bool Elevation::in_boundaries(int col, int lig) {
	if (col < 0 || lig < 0 || col >= _n_cols || lig >= _n_ligs) {
		return false;
	}
	return true;
}


bool Elevation::in_boundaries(pt_2d pt) {
	if (pt.x < _origin.x || pt.x > _origin.x + _size.x || pt.y < _origin.y || pt.y > _origin.y + _size.y) {
		return false;
	}
	return true;
}


std::pair<uint, uint> Elevation::id2col_lig(uint id) {
	return std::make_pair(id % _n_cols, id/ _n_cols);
}


uint Elevation::col_lig2id(uint col, uint lig) {
	return col+ _n_cols* lig;
}


pt_2d Elevation::col_lig2pt(uint col, uint lig) {
	return pt_2d(
		_origin.x+ (number)(col) * _resolution.x,
		_origin.y+ (number)(lig) * _resolution.y
	);
}


pt_2d Elevation::id2pt_2d(uint id) {
	std::pair<uint, uint> col_lig = id2col_lig(id);
	return col_lig2pt(col_lig.first, col_lig.second);
}


pt_3d Elevation::id2pt_3d(uint id) {
	pt_2d pt = id2pt_2d(id);
	return pt_3d(pt.x, pt.y, get_alti(id));
}


std::pair<uint, uint> Elevation::pt2col_lig(pt_2d pt) {
	if (!in_boundaries(pt)) {
		std::cerr << "Elevation::pt2col_lig : " << glm::to_string(pt) << " hors Elevation\n";
		return std::make_pair(0, 0);
	}
	int col= (int)((pt.x- _origin.x) / _resolution.x);
	int lig= (int)((pt.y- _origin.y) / _resolution.y);
	return std::make_pair(col, lig);
}


uint Elevation::pt2id(pt_2d pt) {
	std::pair<uint, uint> col_lig = pt2col_lig(pt);
	return col_lig2id(col_lig.first, col_lig.second);
}


number Elevation::get_alti(uint id) {
	return _altis[id];
}


number Elevation::get_alti(int col, int lig) {
	if (!in_boundaries(col, lig)) {
		std::cerr << "Elevation::get_alti : (" << col << " ; " << lig << ") hors Elevation (2!)\n";
		return 0.0;
	}
	return _altis[col_lig2id(col, lig)];
}


number Elevation::get_alti(pt_2d pt) {
	//std::pair<uint, uint> col_lig = pt2col_lig(pt);
	//return get_alti(col_lig.first, col_lig.second);
	
	if (!in_boundaries(pt)) {
		std::cerr << "Elevation::get_alti : " << glm::to_string(pt) << " hors Elevation\n";
		return 0.0;
	}
	int col_left= (int)((pt.x- _origin.x) / _resolution.x);
	int lig_bottom= (int)((pt.y- _origin.y) / _resolution.y);
	
	number col_mod = fmod(pt.x- _origin.x, _resolution.x) / _resolution.x;
	number lig_mod = fmod(pt.y- _origin.y, _resolution.y) / _resolution.y;
	number alti_left_bottom, alti_left_top, alti_right_bottom, alti_right_top, alti_left, alti_right;
	
	alti_left_bottom = get_alti(col_left, lig_bottom);
	
	if (lig_bottom == _n_ligs - 1) {
		alti_left_top = alti_left_bottom;
	}
	else {
		alti_left_top = get_alti(col_left, lig_bottom + 1);
	}
	if (col_left == _n_cols - 1) {
		alti_right_bottom = alti_left_bottom;
	}
	else {
		alti_right_bottom = get_alti(col_left + 1, lig_bottom);
	}
	if (lig_bottom == _n_ligs - 1) {
		alti_right_top = alti_right_bottom;
	}
	else if (col_left == _n_cols - 1) {
		alti_right_top = alti_left_top;
	}
	else {
		alti_right_top = get_alti(col_left + 1, lig_bottom + 1);
	}

	alti_left = alti_left_bottom * (1.0 - lig_mod) + alti_left_top * lig_mod;
	alti_right = alti_right_bottom * (1.0 - lig_mod) + alti_right_top * lig_mod;

	return alti_left * (1.0 - col_mod) + alti_right * col_mod;
	//return lig_mod;
}


number Elevation::get_alti_over_polygon(Polygon2D * polygon) {
	number result = 0.0;
	uint n_pts = 0;
	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			pt_2d pt = col_lig2pt(col, lig);
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
		std::cerr << "Elevation::get_alti_over_polygon pas de pt dans polygon\n";
		return 0.0;
	}
}


std::vector<uint> Elevation::get_ids_over_aabb(AABB_2D * aabb) {
	std::vector<uint> result;
	std::pair<uint, uint> col_lig_min = pt2col_lig(aabb->_pos);
	std::pair<uint, uint> col_lig_max = pt2col_lig(aabb->_pos + aabb->_size);
	for (uint col = col_lig_min.first; col<= col_lig_max.first; ++col) {
		for (uint lig = col_lig_min.second; lig<= col_lig_max.second; ++lig) {
			result.push_back(col_lig2id(col, lig));
		}
	}
	return result;
}


std::vector<uint> Elevation::get_neighbors(uint id) {
	std::vector<uint> result;
	std::pair<uint, uint> col_lig = id2col_lig(id);
	uint col = col_lig.first;
	uint lig = col_lig.second;
	if (col > 0) {
		result.push_back(col_lig2id(col - 1, lig));
		if (lig > 0) {
			result.push_back(col_lig2id(col - 1, lig - 1));
		}
		if (lig < _n_ligs - 1) {
			result.push_back(col_lig2id(col - 1, lig + 1));
		}
	}
	if (col < _n_cols - 1) {
		result.push_back(col_lig2id(col + 1, lig));
		if (lig > 0) {
			result.push_back(col_lig2id(col + 1, lig - 1));
		}
		if (lig < _n_ligs - 1) {
			result.push_back(col_lig2id(col + 1, lig + 1));
		}
	}
	if (lig > 0) {
		result.push_back(col_lig2id(col, lig - 1));
	}
	if (lig < _n_ligs - 1) {
		result.push_back(col_lig2id(col, lig + 1));
	}

	std::sort(result.begin(), result.end());

	return result;
}


pt_3d Elevation::get_normal(uint id) {
	std::pair<uint, uint> col_lig = id2col_lig(id);
	uint col = col_lig.first;
	uint lig = col_lig.second;
	std::vector<std::tuple<uint, uint, uint> > triangles;

	if (col > 0) {
		if (lig > 0) {
			triangles.push_back(std::make_tuple(col_lig2id(col - 1, lig), col_lig2id(col - 1, lig - 1), id));
			triangles.push_back(std::make_tuple(col_lig2id(col - 1, lig - 1), col_lig2id(col, lig - 1), id));
		}
		if (lig < _n_ligs - 1) {
			triangles.push_back(std::make_tuple(col_lig2id(col - 1, lig + 1), col_lig2id(col - 1, lig), id));
			triangles.push_back(std::make_tuple(col_lig2id(col, lig + 1), col_lig2id(col - 1, lig + 1), id));
		}
	}
	if (col < _n_cols - 1) {
		if (lig > 0) {
			triangles.push_back(std::make_tuple(col_lig2id(col, lig - 1), col_lig2id(col + 1, lig - 1), id));
			triangles.push_back(std::make_tuple(col_lig2id(col + 1, lig - 1), col_lig2id(col + 1, lig), id));
		}
		if (lig < _n_ligs - 1) {
			triangles.push_back(std::make_tuple(col_lig2id(col + 1, lig), col_lig2id(col + 1, lig + 1), id));
			triangles.push_back(std::make_tuple(col_lig2id(col + 1, lig + 1), col_lig2id(col, lig + 1), id));
		}
	}

	pt_3d result(0.0);
	for (auto & triangle : triangles) {
		pt_3d u = glm::normalize(id2pt_3d(std::get<1>(triangle)) - id2pt_3d(std::get<0>(triangle)));
		pt_3d v = glm::normalize(id2pt_3d(std::get<2>(triangle)) - id2pt_3d(std::get<0>(triangle)));
		result += glm::cross(u, v);
	}
	result = glm::normalize(result);
	
	return result;
}


std::vector<uint> Elevation::lowest_gradient(uint id_src) {
	std::vector<uint> result;
	uint id_current = id_src;
	
	while (true) {
		result.push_back(id_current);
		number alti_current = get_alti(id_current);
		std::vector<uint> neighbors = get_neighbors(id_current);
		number alti_min = 1e7;
		uint id_next = 0;
		for (auto & id : neighbors) {
			if (std::find(result.begin(), result.end(), id) != result.end()) {
				continue;
			}
			number alti_id = get_alti(id);
			if (alti_id < alti_min) {
				alti_min = alti_id;
				id_next = id;
			}
		}
		
		if (alti_min <= 0.0) {
			result.push_back(id_next);
			break;
		}

		if (alti_min > alti_current) {
			break;
		}

		//std::cout << id_current << " ; " << alti_current << " ; " << id_next << " ; " << alti_min << "\n";
		id_current = id_next;
	}

	return result;
}


void Elevation::set_alti(int col, int lig, number alti) {
	_altis[col_lig2id(col, lig)] = alti;
}


void Elevation::set_alti_over_polygon(Polygon2D * polygon, number alti) {
	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			pt_2d pt = col_lig2pt(col, lig);
			if (is_pt_inside_poly(pt, polygon)) {
				set_alti(col, lig, alti);
			}
		}
	}
}


void Elevation::set_alti_all(number alti) {
	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			set_alti(col, lig, alti);
		}
	}
}


void Elevation::set_negative_alti_2zero() {
	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			uint id = col_lig2id(col, lig);
			if (_altis[id]< 0.0) {
				_altis[id] = 0.0;
			}
		}
	}
}


void Elevation::randomize() {
	// https://www.redblobgames.com/maps/Elevation-from-noise/
	// https://gamedev.stackexchange.com/questions/116205/terracing-mountain-features/188513

	number alti_offset = -5.0;
	uint n_levels = 5;
	uint gradient_base_size = 10;
	number max_factor = 20.0;
	number redistribution_power = 0.8;
	number fudge_factor = 1.5;
	number mix_island = 0.4;
	number island_max_alti = 20.0;
	number terrace_factor = 0.5;
	number terrace_hmin = 1.0;
	number terrace_hmax = 4.0;
	number terrace_perlin_factor = 20.0;
	uint terrace_gradient_w= 4;
	uint terrace_gradient_h= 4;

	//std::vector<number> amplitudes {1.0, 0.5, 0.25, 0.12, 0.06};
	std::vector<number> amplitudes {1.0, 0.5, 0.33, 0.25, 0.2};
	number amp_sum = 0.0;
	for (auto & a : amplitudes) {
		amp_sum += a;
	}

	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			//_altis[col_lig2id(col, lig)] = rand_number(0.0, 1000.0);
			//pt_2d pt = col_lig2pt(col, lig);
			//_altis[col_lig2id(col, lig)] = rand_number(0.0, 100.0) * exp(-1.0 * pow(glm::distance(pt, _origin + 0.5 * _size) / (0.5 * _size.x), 2.0));
			//_altis[col_lig2id(col, lig)] = perlin(col, lig, gradient, gradient_w, gradient_h);
			//number p = perlin(col, lig, gradient, gradient_w, gradient_h);
			//std::cout << col << " ; " << lig << " ; " << p << "\n";

			_altis[col_lig2id(col, lig)] = alti_offset;
		}
	}

	for (uint level=0; level<n_levels; ++level) {
		srand(time(NULL));

		uint gradient_w= gradient_base_size* (level+ 1);
		uint gradient_h= gradient_base_size* (level+ 1);
		number * gradient = perlin_gradient(gradient_w, gradient_h);
		//number factor= max_factor* pow(2.0, -1.0 * number(level)) / amp_sum;
		number factor = max_factor* amplitudes[level] / amp_sum;
	
		for (uint col=0; col< _n_cols; ++col) {
			for (uint lig=0; lig< _n_ligs; ++lig) {
				number ii= number(col)* (gradient_w- 1)/ _n_cols;
				number jj= number(lig)* (gradient_h- 1)/ _n_ligs;
				_altis[col_lig2id(col, lig)] += factor* perlin(ii, jj, gradient, gradient_w, gradient_h);
			}
		}
	}

	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			uint id = col_lig2id(col, lig);
			if (_altis[id]> 0.0) {
				_altis[id] = pow(_altis[id] * fudge_factor, redistribution_power);
			}
		}
	}
	

	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			uint id = col_lig2id(col, lig);
			number nx = 2.0 * number(col) / _n_cols - 1.0;
			number ny = 2.0 * number(lig) / _n_ligs - 1.0;
			number d = 1.0 - (1.0 - nx * nx) * (1.0 - ny * ny);
			_altis[id] = (1.0 - mix_island) * _altis[id] + mix_island * (1.0 - d) * island_max_alti;
		}
	}

	number * gradient = perlin_gradient(terrace_gradient_w, terrace_gradient_h);
	//number damp = 0.9;
	for (uint col=0; col< _n_cols; ++col) {
		for (uint lig=0; lig< _n_ligs; ++lig) {
			uint id = col_lig2id(col, lig);
			number ii= number(col)* (terrace_gradient_w- 1)/ _n_cols;
			number jj= number(lig)* (terrace_gradient_h- 1)/ _n_ligs;
			number hm = terrace_perlin_factor * perlin(ii, jj, gradient, terrace_gradient_w, terrace_gradient_h);
			
			/*if (h1 + hm < _altis[id] && h2 + hm > _altis[id]) {
				_altis[id] *= damp;
			}
			else if (h2 + hm < _altis[id]) {
				_altis[id] -= (h2 - h1) * damp;
			}*/

			/*number k = floor(_altis[id] / terrace_factor);
			number f = (_altis[id] - k * terrace_factor) / terrace_factor;
			number s = std::min(2.0 * f, 1.0);
			if (terrace_hmin + hm < _altis[id] && terrace_hmax + hm > _altis[id]) {
				_altis[id] = (k + s) * terrace_factor;
			}*/
			if (terrace_hmin + hm < _altis[id] && terrace_hmax + hm > _altis[id]) {
				_altis[id] = round(_altis[id] * 2.0) / 2.0;
			}
		}
	}
	delete gradient;

	set_negative_alti_2zero();

	//alti2pbm("../data/test.pgm");
}


void Elevation::alti2pbm(std::string pbm_path) {
	FILE *f;
	f= fopen(pbm_path.c_str(), "wb");
	//fprintf(f, "P1\n%d %d\n", _n_cols, _n_ligs);
	fprintf(f, "P2\n%d %d\n1\n", _n_cols, _n_ligs);
	for (uint lig=0; lig<_n_ligs; ++lig) {
		for (uint col=0; col<_n_cols; ++col) {
			int v = 0;
			if (get_alti(col, lig) < 0.1) {
				v = 1;
			}
			fprintf(f, "%d ", v);
		}
		fprintf(f, "\n");
	}
	fclose(f);
}


// River ---------------------------------------------------------------------------
River::River() {

}


River::River(Elevation * elevation, pt_2d src) : _elevation(elevation) {

	std::cout << "river begin\n";
	uint id_src = _elevation->pt2id(src);
	_id_nodes = _elevation->lowest_gradient(id_src);
	
std::cout << "river triangles\n";
	for (uint i=0; i<_id_nodes.size() - 1; ++i) {
		pt_3d pt_begin = _elevation->id2pt_3d(_id_nodes[i]);
		pt_3d pt_end = _elevation->id2pt_3d(_id_nodes[i + 1]);
		number length = glm::length(pt_end - pt_begin);
		pt_3d u = (pt_end - pt_begin) / length;
		pt_3d normal = _elevation->get_normal(_id_nodes[i]);
		pt_3d v = glm::cross(normal, u);
		//number width = rand_number(0.1, 0.5);
		//number width = 0.5;
		number width = 0.1 + 0.4 * number(i) / number(_id_nodes.size() - 2);
		number offset = 0.3;
		pt_3d pt0 = pt_begin - 0.5 * width * v + offset * length * u;
		pt_3d pt1 = pt_begin - 0.5 * width * v + (1.0 - offset) * length * u;
		pt_3d pt2 = pt_begin + 0.5 * width * v + (1.0 - offset) * length * u;
		pt_3d pt3 = pt_begin + 0.5 * width * v + offset * length * u;

		_triangles.push_back(std::make_tuple(pt0, pt1, pt2, normal));
		_triangles.push_back(std::make_tuple(pt0, pt2, pt3, normal));
	}

std::cout << "river jointures\n";
	std::vector<std::tuple<pt_3d, pt_3d, pt_3d, pt_3d> > triangles_jointure;
	for (uint i=0; i<=(_triangles.size() - 4) / 2; ++i) {
		pt_3d pt0 = std::get<1>(_triangles[2 * i]);
		pt_3d pt1 = std::get<0>(_triangles[2 * i + 3]);
		pt_3d pt2 = std::get<2>(_triangles[2 * i + 3]);
		pt_3d pt3 = std::get<2>(_triangles[2 * i]);
		pt_3d normal = glm::normalize(std::get<3>(_triangles[2 * i]) + std::get<3>(_triangles[2 * i + 3]));

		triangles_jointure.push_back(std::make_tuple(pt0, pt1, pt2, normal));
		triangles_jointure.push_back(std::make_tuple(pt0, pt2, pt3, normal));
	}

	_triangles.insert(_triangles.end(), triangles_jointure.begin(), triangles_jointure.end());

	uint n_attrs_per_pts= 10;
	_n_pts = 3 * _triangles.size();
	_data = new float[_n_pts * n_attrs_per_pts];
	std::cout << "river end\n";
	update_data();
}


River::~River() {
	delete _data;
}


void River::update_data() {
	std::cout << "river update\n";
	const number RIVER_Z_OFFSET = 0.1;
	const glm::vec4 RIVER_COLOR(0.5, 0.7, 0.9, 1.0);
	const glm::vec4 RIVER_COLOR_DEBUG(0.9, 0.3, 0.3, 1.0);
	float * ptr = _data;
	uint debug = 0;
	for (auto & triangle : _triangles) {
		std::vector<pt_3d> pts = {std::get<0>(triangle), std::get<1>(triangle), std::get<2>(triangle)};
		pt_3d normal = std::get<3>(triangle);

		for (uint i=0; i<3; ++i) {
			ptr[0] = float(pts[i].x);
			ptr[1] = float(pts[i].y);
			ptr[2] = float(pts[i].z + RIVER_Z_OFFSET);
			if (debug < (2 * _id_nodes.size() - 1)) {
			ptr[3] = RIVER_COLOR.r;
			ptr[4] = RIVER_COLOR.g;
			ptr[5] = RIVER_COLOR.b;
			ptr[6] = RIVER_COLOR.a;
			}
			else {
			ptr[3] = RIVER_COLOR_DEBUG.r;
			ptr[4] = RIVER_COLOR_DEBUG.g;
			ptr[5] = RIVER_COLOR_DEBUG.b;
			ptr[6] = RIVER_COLOR_DEBUG.a;
			}
			ptr[7] = float(normal.x);
			ptr[8] = float(normal.y);
			ptr[9] = float(normal.z);
			ptr += 10;
		}
		debug++;
	}
	std::cout << "river update end\n";
}


// Lake ----------------------------------------------------------------
Lake::Lake() {

}


Lake::Lake(Elevation * elevation, pt_2d src) : _elevation(elevation) {
	uint id_src = _elevation->pt2id(src);
	std::vector<uint> id_nodes_src = _elevation->lowest_gradient(id_src);
	uint id_lowest = id_nodes_src[id_nodes_src.size() - 1];
	if (_elevation->get_alti(id_lowest) <= 0.0) {
		std::cerr << "Lake impossible lowest pt alti <= 0.0\n";
		return;
	}

	// std::pair<uint, uint> col_lig = _elevation->id2col_lig(id_lowest);
	// int size = 1000;
	// std::vector<uint> id_nodes;
	// for (int col = int(col_lig.first) - size; col < int(col_lig.first) + size; ++col) {
	// 	for (int lig = int(col_lig.second) - size; lig < int(col_lig.second) + size; ++lig) {
	// 		if (!_elevation->in_boundaries(col, lig)) {
	// 			continue;
	// 		}

	// 		uint id = _elevation->col_lig2id(uint(col), uint(lig));
	// 		std::vector<uint> l_ids = _elevation->lowest_gradient(id);
	// 		if (l_ids[l_ids.size() - 1] == id_lowest) {
	// 			id_nodes.push_back(id);
	// 		}
	// 	}
	// }

	// for (auto & id : id_nodes) {
	// 	std::vector<uint> id_neighbors = _elevation->get_neighbors(id);
	// 	bool is_in_frontier = false;
	// 	for (auto & id_n : id_neighbors) {
	// 		if (std::find(id_nodes.begin(), id_nodes.end(), id_n) == id_nodes.end()) {
	// 			is_in_frontier = true;
	// 			break;
	// 		}
	// 	}
	// 	if (is_in_frontier) {
	// 		_id_nodes.push_back(id);
	// 	}
	// }

	std::queue<uint> frontier;
	std::vector<uint> checked;
	frontier.push(id_lowest);
	checked.push_back(id_lowest);
	//number current_alti = _elevation->get_alti(id_lowest);
	//number alti_max = 1e9;

	number alti_max = _elevation->get_alti(id_lowest) + 0.2;
	
	while (!frontier.empty()) {
		uint id = frontier.front();
		frontier.pop();
		number alti = _elevation->get_alti(id);

		if (alti < alti_max) {
			_id_nodes.push_back(id);
		}

		//std::cout << "alti = " << alti << "\n";

		/*if (alti < current_alti) {
			continue;
		}*/

		//_id_nodes.push_back(id);

		//std::cout << id << " ; " << frontier.size() << " ; " << checked.size() << " ; " << "\n";

		std::vector<uint> id_neighbors = _elevation->get_neighbors(id);

		for (auto & id_n : id_neighbors) {
			if (std::find(checked.begin(), checked.end(), id_n) != checked.end()) {
				continue;
			}
			checked.push_back(id_n);


			number alti_n = _elevation->get_alti(id_n);

			//std::cout << "\talti_n = " << alti_n << "\n";

			/*if (alti_n >= current_alti) {
				frontier.push(id_n);
			}*/

			if (alti_n < alti_max) {
				frontier.push(id_n);
			}
		}
			
	}

	/*std::cout << "alti_max = " << alti_max << " ; checked.size() = " << checked.size() << "\n";

	checked.clear();
	frontier.push(id_lowest);

	while (!frontier.empty()) {
		uint id = frontier.front();
		frontier.pop();
		number alti = _elevation->get_alti(id);

		std::cout << "alti = " << alti << "\n";

		if (alti > alti_max) {
			continue;
		}

		_id_nodes.push_back(id);

		std::vector<uint> id_neighbors = _elevation->get_neighbors(id);

		for (auto & id_n : id_neighbors) {
			if (std::find(checked.begin(), checked.end(), id_n) != checked.end()) {
				continue;
			}

			number alti_n = _elevation->get_alti(id_n);

			if (alti_n <= alti) {
			}
			else if (std::find(checked.begin(), checked.end(), id_n) == checked.end()) {
				frontier.push(id_n);
				checked.push_back(id_n);
			}
		}
	}*/

	std::cout << _id_nodes.size() << "\n";

	update_data();
}


Lake::~Lake() {
	delete _data;
}


void Lake::update_data() {

}


// ----------------------------------------------------------------------------------------
PathFinder::PathFinder() {
	
}

PathFinder::PathFinder(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols) : _use_line_of_sight(true), _verbose(false) {
	_elevation_grid = new GraphGrid(origin, size, n_ligs, n_cols);
	_units_position_grid = new GraphGrid(origin, size, n_ligs, n_cols);
	_terrain_grid = new GraphGrid(origin, size, n_ligs, n_cols);

	_elevation_grid->_it_v= _elevation_grid->_vertices.begin();
	while (_elevation_grid->_it_v!= _elevation_grid->_vertices.end()) {
		_elevation_grid->_it_e= _elevation_grid->_it_v->second._edges.begin();
		while (_elevation_grid->_it_e!= _elevation_grid->_it_v->second._edges.end()) {
			GraphEdge & edge = _elevation_grid->_vertices[_elevation_grid->_it_v->first]._edges[_elevation_grid->_it_e->first];
			edge._data = new ElevationEdgeData();
			_elevation_grid->_it_e++;
		}
		_elevation_grid->_it_v++;
	}

	_units_position_grid->_it_v= _units_position_grid->_vertices.begin();
	while (_units_position_grid->_it_v!= _units_position_grid->_vertices.end()) {
		_units_position_grid->_it_e= _units_position_grid->_it_v->second._edges.begin();
		while (_units_position_grid->_it_e!= _units_position_grid->_it_v->second._edges.end()) {
			GraphEdge & edge = _units_position_grid->_vertices[_units_position_grid->_it_v->first]._edges[_units_position_grid->_it_e->first];
			edge._data = new UnitsPositionEdgeData();
			_units_position_grid->_it_e++;
		}
		_units_position_grid->_it_v++;
	}

	_terrain_grid->_it_v= _terrain_grid->_vertices.begin();
	while (_terrain_grid->_it_v!= _terrain_grid->_vertices.end()) {
		_terrain_grid->_it_e= _terrain_grid->_it_v->second._edges.begin();
		while (_terrain_grid->_it_e!= _terrain_grid->_it_v->second._edges.end()) {
			GraphEdge & edge = _terrain_grid->_vertices[_terrain_grid->_it_v->first]._edges[_terrain_grid->_it_e->first];
			edge._data = new TerrainEdgeData();
			_terrain_grid->_it_e++;
		}
		_terrain_grid->_it_v++;
	}
}


PathFinder::~PathFinder() {
	// TODO : détruire les edge._data ici
	delete _elevation_grid;
	delete _units_position_grid;
	delete _terrain_grid;
}


number PathFinder::elevation_weight(uint i, uint j) {
	GraphEdge edge = _elevation_grid->_vertices[i]._edges[j];
	ElevationEdgeData * data = (ElevationEdgeData *)(edge._data);
	return data->_delta_elevation;
}


number PathFinder::units_position_weight(Unit * unit, uint i, uint j) {
	GraphEdge edge = _units_position_grid->_vertices[i]._edges[j];
	UnitsPositionEdgeData * data = (UnitsPositionEdgeData *)(edge._data);
	if (data->_ids.empty() || (data->_ids.size() == 1 && data->_ids[0] == unit->_id)) {
		return DEFAULT_EDGE_WEIGHT;
	}
	return MAX_UNIT_MOVING_WEIGHT;
}


number PathFinder::terrain_weight(Unit * unit, uint i, uint j) {
	GraphEdge edge = _terrain_grid->_vertices[i]._edges[j];
	TerrainEdgeData * data = (TerrainEdgeData *)(edge._data);
	if (data->_type == UNKNOWN) {
		std::cerr << "PathFinder::terrain_weight UNKNOWN\n";
		return 0.0;
	}
	return unit->_type->_terrain_weights[data->_type];
}


number PathFinder::cost(Unit * unit, uint i, uint j) {
	number result = 0.0;
	result += elevation_weight(i, j);
	result += units_position_weight(unit, i, j);
	result += terrain_weight(unit, i, j);
	return result;
}


number PathFinder::heuristic(uint i, uint j) {
	return glm::distance(_elevation_grid->_vertices[i]._pos, _elevation_grid->_vertices[j]._pos);
}


number PathFinder::line_of_sight_max_weight(Unit * unit, pt_2d pt1, pt_2d pt2) {
	std::vector<std::pair<uint, uint> > edges = _elevation_grid->segment_intersection(pt_2d(pt1), pt_2d(pt2));
	number result = 0.0;
	for (auto edge : edges) {
		pt_2d v1 = pt_2d(_elevation_grid->_vertices[edge.first]._pos);
		pt_2d v2 = pt_2d(_elevation_grid->_vertices[edge.second]._pos);
		number weight = cost(unit, edge.first, edge.second);
		if (glm::dot(v2 - v1, pt_2d(pt2 - pt1)) >= 0.0 && weight > result) {
			result = weight;
		}
	}
	return result;
}


bool PathFinder::path_find_nodes(Unit * unit, uint start, uint goal) {
	auto frontier_cmp = [](std::pair<uint, number> x, std::pair<uint, number> y) { return x.second > y.second; };
	std::priority_queue< std::pair<uint, number>, std::vector<std::pair<uint, number> >, decltype(frontier_cmp) > frontier(frontier_cmp);
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

		std::vector<uint> nexts= _elevation_grid->neighbors(current);
		for (auto & next : nexts) {
			number new_cost= cost_so_far[current]+ cost(unit, current, next);
			if ((!cost_so_far.count(next)) || (new_cost< cost_so_far[next])) {
				cost_so_far[next]= new_cost;
				came_from[next]= current;
				//number priority= new_cost; // dijkstra
				//number priority= heuristic(next, goal, grid); // greedy best first search
				number priority= new_cost + heuristic(next, goal); // A *
				//std::cout << priority << "\n";
				frontier.emplace(next, priority);
			}
		}
	}

	if (!came_from.count(goal)) {
		//cout << "disconnected\n";
		return false;
	}

	//unit->_path->_nodes.clear();
	uint current = goal;
	while (current != start) {
		unit->_path->_nodes.push_back(current);
		current = came_from[current];
	}
	unit->_path->_nodes.push_back(start);
	std::reverse(unit->_path->_nodes.begin(), unit->_path->_nodes.end());

	return true;
}


bool PathFinder::path_find(Unit * unit, pt_2d goal) {
	std::mutex mtx;

	unit->_path->clear();
	unit->_path->_start = pt_2d(unit->_aabb->bottom_center());
	unit->_path->_goal = goal;

	if ((!point_in_aabb2d(unit->_path->_start, _elevation_grid->_aabb)) || (!point_in_aabb2d(unit->_path->_goal, _elevation_grid->_aabb))) {
		if (_verbose) {
			std::cerr << "unit id " << unit->_id << " : PathFinder::path_find : point hors grille\n";
		}
		mtx.lock();
		unit->_status = COMPUTING_PATH_FAILED;
		mtx.unlock();
		return false;
	}

	/*std::vector<std::pair<uint, uint> > start_edges = units_position_grid->edges_in_cell_containing_pt(start, true);
	std::vector<uint> start_vertices = units_position_grid->vertices_in_cell_containing_pt(start);
	int start_id = -1;
	for (auto & id_vertex : start_vertices) {
		bool vertex_ok = true;
		GraphVertex vertex = units_position_grid->_vertices[id_vertex];
		units_position_grid->_it_e = vertex._edges.begin();
		while (units_position_grid->_it_e != vertex._edges.end()) {
			GraphEdge edge = units_position_grid->_it_e->second;
			for (auto & id_edge : start_edges) {
				if (
				(id_edge.first == units_position_grid->_it_e->first || id_edge.second == units_position_grid->_it_e->first)
				&& units_position_weight(edge, unit) >= MAX_UNIT_MOVING_WEIGHT
				&& units_position_grid->segment_intersects_edge(start, pt_2d(vertex._pos), id_edge)) {
					vertex_ok = false;
					break;
				}
			}
			if (vertex_ok == false) {
				break;
			}
			units_position_grid->_it_e++;
		}
		if (vertex_ok) {
			start_id = id_vertex;
		}
	}

	if (start_id < 0) {
		std::cerr << "PathFinder::path_find : pas de noeud valide start trouvé\n";
		mtx.lock();
		unit->_status = COMPUTING_PATH_DONE;
		mtx.unlock();
		return false;
	}*/

	//uint start_id = static_grid->pt2id(start);
	uint start_id = _elevation_grid->pt2closest_id(unit->_path->_start);
	//uint goal_id = static_grid->pt2id(goal);
	uint goal_id = _elevation_grid->pt2closest_id(unit->_path->_goal);

	bool is_path_ok= path_find_nodes(unit, start_id, goal_id);
	if (!is_path_ok) {
		if (_verbose) {
			std::cerr << "unit id " << unit->_id << " PathFinder::path_find : pas de chemin trouvé\n";
		}
		mtx.lock();
		unit->_status = COMPUTING_PATH_FAILED;
		mtx.unlock();
		return false;
	}
	//std::cout << unit->_path->_nodes.size() << "\n";

	std::vector<pt_2d> raw_path;
	for (uint i=0; i<unit->_path->_nodes.size(); ++i) {
		raw_path.push_back(_elevation_grid->_vertices[unit->_path->_nodes[i]]._pos);
	}
	raw_path.push_back(goal);
	//std::cout << raw_path.size() << "\n";

	std::vector<number> weights;
	
	// start
	/*number weight_start = 0.0;
	
	std::vector<std::pair<uint, uint> > static_start_edges = _elevation_grid->edges_in_cell_containing_pt(unit->_path->_start);
	number w_static_start = 1e9;
	for (auto & edge : static_start_edges) {
		number w = elevation_weight(edge.first, edge.second);
		if (w < w_static_start) {
			w_static_start = w;
		}
	}
	weight_start += w_static_start;

	std::vector<std::pair<uint, uint> > units_position_start_edges = _units_position_grid->edges_in_cell_containing_pt(unit->_path->_start);
	number w_units_position_start = 1e9;
	for (auto & edge : units_position_start_edges) {
		number w = units_position_weight(unit, edge.first, edge.second);
		if (w < w_units_position_start) {
			w_units_position_start = w;
		}
	}
	weight_start += w_units_position_start;
	weights.push_back(weight_start);*/

	std::vector<std::pair<uint, uint> > start_edges = _elevation_grid->edges_in_cell_containing_pt(unit->_path->_start);
	number weight_start = 1e9;
	for (auto & edge : start_edges) {
		number w = cost(unit, edge.first, edge.second);
		if (w < weight_start) {
			weight_start = w;
		}
	}
	weights.push_back(weight_start);

	// path
	number n_nodes = unit->_path->_nodes.size();
	for (uint i=0; i < n_nodes - 1; ++i) {
		weights.push_back(cost(unit, unit->_path->_nodes[i], unit->_path->_nodes[i + 1]));
	}

	// goal
	/*number weight_goal = 0.0;
	
	std::vector<std::pair<uint, uint> > static_goal_edges = _elevation_grid->edges_in_cell_containing_pt(unit->_path->_goal);
	number w_static_goal = 1e9;
	for (auto & edge : static_goal_edges) {
		number w = elevation_weight(edge.first, edge.second);
		if (w < w_static_goal) {
			w_static_goal = w;
		}
	}
	weight_goal += w_static_goal;

	std::vector<std::pair<uint, uint> > units_position_goal_edges = _units_position_grid->edges_in_cell_containing_pt(unit->_path->_goal);
	number w_units_position_goal = -1e9;
	for (auto & edge : units_position_goal_edges) {
		number w = units_position_weight(unit, edge.first, edge.second);
		if (w > w_units_position_goal) {
			w_units_position_goal = w;
		}
	}
	weight_goal += w_units_position_goal;
	weights.push_back(weight_goal);*/

	std::vector<std::pair<uint, uint> > goal_edges = _elevation_grid->edges_in_cell_containing_pt(unit->_path->_goal);
	number weight_goal = 1e9;
	for (auto & edge : goal_edges) {
		number w = cost(unit, edge.first, edge.second);
		if (w < weight_goal) {
			weight_goal = w;
		}
	}
	weights.push_back(weight_goal);

	/*for (auto & w : weights) {
		std::cout << w << " ; ";
	}
	std::cout << "\n";*/
	//std::cout << "weight_goal = " << weight_goal << "\n";

	for (uint i=0; i<weights.size(); ++i) {
		if (weights[i] >= MAX_UNIT_MOVING_WEIGHT) {
			if (i == 0) {
				raw_path.clear();
				break;
			}
			else {
				raw_path.erase(raw_path.begin() + i - 1, raw_path.end());
				break;
			}
		}
	}
	if (raw_path.size() == 0) {
		if (_verbose) {
			std::cerr << "unit id " << unit->_id << " raw_path.size() == 0\n";
		}
		mtx.lock();
		unit->_status = COMPUTING_PATH_FAILED;
		mtx.unlock();
		return false;
	}

	if (_use_line_of_sight) {
		uint idx = 0;
		uint last = 0;
		//path->_pts.push_back(start);
		//path->_weights.push_back(0.0); // inutilisé mais nécessaire ?
		
		if (_verbose) {
			std::cout << "raw_path.size() = " << raw_path.size() << "\n\n";
		}

		while (idx < raw_path.size()) {
			number raw_max_weight = weights[last];
			number last_los_weight_ok = 0.0;

			if (_verbose) {
				std::cout << "BEGIN while\n";
				std::cout << "last = " << last << " ; idx = " << idx;
				std::cout << " ; raw_max_weight=" << raw_max_weight;
				//std::cout << " ; raw_path[last]=" << glm::to_string(raw_path[last]),
				//std::cout << " ; raw_path[idx]=" << glm::to_string(raw_path[idx]);
				std::cout << "\n";
			}

			while (true) {
				idx++;
				
				if (_verbose) {
					std::cout << "idx = " << idx << "\n";
				}

				if (idx >= raw_path.size()) {
					break;
				}
				number los_weight = line_of_sight_max_weight(unit, raw_path[last], raw_path[idx]);

				if (_verbose) {
					std::cout << "\tlos_weight = " << los_weight << " ; raw_max_weight = " << raw_max_weight << "\n";
				}

				if (los_weight> raw_max_weight) {
					break;
				}
				raw_max_weight = std::max(raw_max_weight, weights[idx]);
				last_los_weight_ok = los_weight;

				if (_verbose) {
					std::cout << "\tlast_los_weight_ok = " << last_los_weight_ok << " ; raw_max_weight = " << raw_max_weight << "\n";
				}
			}

			last = idx - 1;
			
			unit->_path->_pts.push_back(pt_3d(raw_path[last].x, raw_path[last].y, 0.0));
			unit->_path->_weights.push_back(last_los_weight_ok);

			if (_verbose) {
				std::cout << "END while\n";
				std::cout << "last = " << last << " ; idx = " << idx;
				//std::cout << " ; raw_path[last]=" << glm::to_string(raw_path[last]) << " ; raw_path[idx]=" << glm::to_string(raw_path[idx]) << "\n";
				std::cout << " ; raw_max_weight=" << raw_max_weight;
				std::cout << "\n\n";
			}
		}
	}
	else {
		for (uint i=0; i<raw_path.size(); ++i) {
			unit->_path->_pts.push_back(pt_3d(raw_path[i].x, raw_path[i].y, 0.0));
			unit->_path->_weights.push_back(weights[i]);
		}
	}

	//number r = unit->_aabb->_base_radius + 0.5 * std::max(unit_type->_size.x, unit_type->_size.y);
	number r = unit->_aabb->_base_radius;
	
	std::vector<pt_4d> segments;
	pt_2d p1 = pt_2d(unit->_aabb->bottom_center());

	pt_2d p2 = pt_2d(unit->_path->_pts[0]);
	segments.push_back(pt_4d(p1.x, p1.y, p2.x, p2.y));
	
	for (int i=0; i<unit->_path->_pts.size() - 1; ++i) {
		pt_2d p1 = pt_2d(unit->_path->_pts[i]);
		pt_2d p2 = pt_2d(unit->_path->_pts[i + 1]);
		segments.push_back(pt_4d(p1.x, p1.y, p2.x, p2.y));
	}

	for (auto & segment : segments) {
		pt_2d p1(segment.x, segment.y);
		pt_2d p2(segment.z, segment.w);
		//std::cout << glm_to_string(p1) << " ; "  << glm_to_string(p2) << "\n";
		if (norm2(p1 - p2) < 1e-8) {
			continue;
		}

		/*pt_2d v = glm::normalize(p2 - p1);
		pt_2d u(v.y, -v.x);
		std::vector<pt_2d> pts = {p1 - r * u - r * v, p1 + r * u - r * v, p2 + r * u + r * v, p2 - r * u + r * v};
		Polygon2D * polygon = new Polygon2D(pts);
		polygon->update_all();
		std::vector<std::pair<uint, uint> > path_edges = grid->polygon_intersection(polygon);
		delete polygon;*/
		
		pt_2d v = glm::normalize(p2 - p1);
		BBox_2D * bbox = new BBox_2D(2.0 * r, p1 - r * v, p2 + r * v);
		//std::cout << *bbox << "\n";
		unit->_path->_bboxs.push_back(bbox);
	}
	//std::cout << *path << "\n";

	mtx.lock();
	unit->_status = COMPUTING_PATH_DONE;
	mtx.unlock();

	return true;
}


// a revoir
/*void PathFinder::draw_svg(GraphGrid * grid, Path * path, std::string svg_path) {
	std::ofstream f;
	f.open(svg_path);
	f << "<!DOCTYPE html>\n<html>\n<body>\n";
	f << "<svg width=\"1000\" height=\"1000\" viewbox=\"" << grid->_origin.x << " " << grid->_origin.y << " " << grid->_size.x << " " << grid->_size.y << "\">\n";

	if (path->_pts.size()) {
		for (uint i=0; i<path->_pts.size()- 1; ++i) {
			number x1= grid->_vertices[path->_pts[i]]._pos.x;
			number y1= grid->_vertices[path[i]]._pos.y;
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
}*/


// ---------------------------------------------------------------------------------------------------
Map::Map() {

}


Map::Map(std::string unit_types_dir, std::string elements_dir, pt_2d origin, pt_2d size, pt_2d path_resolution, pt_2d elevation_resolution, time_point t) :
	_origin(origin), _size(size), _paused(false), _path_find_thr_active(false)
{
	uint n_ligs_path = uint(_size.y / path_resolution.y) + 1;
	uint n_cols_path = uint(_size.x / path_resolution.x) + 1;

	uint n_ligs_elevation = uint(_size.y / elevation_resolution.y) + 1;
	uint n_cols_elevation = uint(_size.x / elevation_resolution.x) + 1;

	_elevation = new Elevation(_origin, _size, n_ligs_elevation, n_cols_elevation);

	std::vector<std::string> jsons_paths = list_files(unit_types_dir, "json");
	for (auto & json_path : jsons_paths) {
		UnitType * unit_type = new UnitType(json_path);
		_unit_types[basename(json_path)] = unit_type;
		_path_finders[unit_type] = new PathFinder(_origin, _size, n_ligs_path, n_cols_path);
	}

	_elements = new Elements(elements_dir);

	sync2elevation();
}


Map::~Map() {
	clear();

	delete _elements;
	
	for (auto & pf : _path_finders) {
		delete pf.second;
	}
	_path_finders.clear();

	for (auto & ut : _unit_types) {
		delete ut.second;
	}
	_unit_types.clear();

	delete _elevation;
}


void Map::add_unit(std::string type_name, pt_2d pos, time_point t) {
	if (pos.x < _origin.x || pos.y < _origin.y || pos.x >= _origin.x + _size.x || pos.y >= _origin.y + _size.y) {
		std::cerr << "Map::add_unit hors Elevation\n";
		return;
	}

	pt_3d pt3d(pos.x, pos.y, _elevation->get_alti(pos));
	Unit * unit = new Unit(_unit_types[type_name], pt3d, t);
	unit->_id = _next_unit_id++;
	_units.push_back(unit);

	add_waiting_unit_to_position_grids(unit);
}


void Map::add_static_element(std::string element_name, pt_3d pos, pt_3d size) {
	AABB * element_aabb;
	if (element_name == "stone") {
		Stone * stone = _elements->add_stone(pos, size);
		element_aabb = stone->_aabb;
	}
	else {
		Tree * tree = _elements->add_tree(element_name, pos, size);
		element_aabb = tree->_aabb;
	}

	for (auto & path_finder : _path_finders) {
		UnitType * unit_type = path_finder.first;
		GraphGrid * grid = path_finder.second->_terrain_grid;
		
		AABB_2D * aabb = new AABB_2D(pt_2d(element_aabb->_vmin - 0.5 * unit_type->_size), pt_2d(element_aabb->size() + unit_type->_size));

		std::vector<std::pair<uint, uint> > edges = grid->aabb_intersection(aabb);
		for (auto & e : edges) {
			GraphEdge & edge = grid->_vertices[e.first]._edges[e.second];
			TerrainEdgeData * data = (TerrainEdgeData *)(edge._data);
			data->_type = OBSTACLE;
		}
	}
}


void Map::add_river(pt_2d src) {
	_rivers.push_back(new River(_elevation, src));
}


void Map::add_lake(pt_2d src) {
	_lakes.push_back(new Lake(_elevation, src));
}


// maj des altis des vertices de la grille
void Map::update_alti_grid(GraphGrid * grid) {
	grid->_it_v= grid->_vertices.begin();
	while (grid->_it_v!= grid->_vertices.end()) {
		pt_3d & pt= grid->_it_v->second._pos;
		pt.z = _elevation->get_alti(pt);
		grid->_it_v++;
	}
}


void Map::update_alti_path(Unit * unit) {
	for (auto & pt : unit->_path->_pts) {
		pt.z = _elevation->get_alti(pt);
	}
}


void Map::update_elevation_grids() {
	for (auto & path_finder : _path_finders) {
		UnitType * unit_type = path_finder.first;
		GraphGrid * grid = path_finder.second->_elevation_grid;

		//number max_weight = -1e-5;
		//number min_weight = 1e-5;

		grid->_it_v= grid->_vertices.begin();
		while (grid->_it_v!= grid->_vertices.end()) {
			grid->_it_e= grid->_it_v->second._edges.begin();
			while (grid->_it_e!= grid->_it_v->second._edges.end()) {
				GraphEdge & edge = grid->_vertices[grid->_it_v->first]._edges[grid->_it_e->first];
				pt_3d & pt_begin= grid->_it_v->second._pos;
				pt_3d & pt_end= grid->_vertices[grid->_it_e->first]._pos;
				ElevationEdgeData * data = (ElevationEdgeData *)(edge._data);
				data->_delta_elevation = unit_type->elevation_coeff(pt_end.z - pt_begin.z);

				/*if (pt_begin.z < 0.01 && pt_end.z < 0.01) {
					edge._weight += type_grid.first->_weights[WATER];
				}
				else {
					edge._weight += type_grid.first->_weights[GROUND];
				}*/
				
				/*for (auto & obstacle : _buffered_obstacles[type_grid.first]) {
					if (segment_intersects_poly(pt_begin, pt_end, obstacle->_polygon, NULL)) {
						edge._weight += type_grid.first->_weights[obstacle->_type];
						break;
					}
				}*/

				/*if (edge._weight > max_weight) {
					max_weight = edge._weight;
				}
				if (edge._weight < min_weight) {
					min_weight = edge._weight;
				}*/

				grid->_it_e++;
			}
			grid->_it_v++;
		}

		//std::cout << type_grid.first->_name << " ; min_w=" << min_weight << " ; max_w=" << max_weight << "\n";
	}
}


void Map::update_terrain_grids_with_elevation(BBox_2D * bbox) {
	for (auto & path_finder : _path_finders) {
		UnitType * unit_type = path_finder.first;
		GraphGrid * grid = path_finder.second->_terrain_grid;

		if (bbox == NULL) {
			grid->_it_v= grid->_vertices.begin();
			while (grid->_it_v!= grid->_vertices.end()) {
				grid->_it_e= grid->_it_v->second._edges.begin();
				while (grid->_it_e!= grid->_it_v->second._edges.end()) {
					GraphEdge & edge = grid->_vertices[grid->_it_v->first]._edges[grid->_it_e->first];
					pt_3d & pt_begin = grid->_it_v->second._pos;
					pt_3d & pt_end = grid->_vertices[grid->_it_e->first]._pos;
					TerrainEdgeData * data = (TerrainEdgeData *)(edge._data);
					
					if (pt_begin.z < 0.01 || pt_end.z < 0.01) {
						data->_type = WATER;
					}
					else {
						data->_type = GROUND;
					}

					grid->_it_e++;
				}
				grid->_it_v++;
			}
		}
		else {
			std::vector<std::pair<uint, uint> > edges = grid->bbox_intersection(bbox);
			for (auto & e : edges) {
				GraphEdge & edge = grid->_vertices[e.first]._edges[e.second];
				pt_3d & pt_begin = grid->_vertices[e.first]._pos;
				pt_3d & pt_end = grid->_vertices[e.second]._pos;
				TerrainEdgeData * data = (TerrainEdgeData *)(edge._data);

				if (pt_begin.z < 0.01 || pt_end.z < 0.01) {
					data->_type = WATER;
				}
				else {
					data->_type = GROUND;
				}
			}
		}
	}
}


void Map::sync2elevation() {
	for (auto & path_finder : _path_finders) {
		for (auto & grid : std::vector<GraphGrid *> {
			path_finder.second->_elevation_grid, path_finder.second->_units_position_grid, path_finder.second->_terrain_grid
		}) {
			update_alti_grid(grid);
		}
	}

	update_elevation_grids();
	update_terrain_grids_with_elevation();
}


void Map::clear_units_position_grids() {
	for (auto & path_finder : _path_finders) {
		UnitType * unit_type = path_finder.first;
		GraphGrid * grid = path_finder.second->_units_position_grid;
		grid->_it_v= grid->_vertices.begin();
		while (grid->_it_v!= grid->_vertices.end()) {
			grid->_it_e = grid->_vertices[grid->_it_v->first]._edges.begin();
			while (grid->_it_e != grid->_vertices[grid->_it_v->first]._edges.end()) {
				UnitsPositionEdgeData * data = (UnitsPositionEdgeData * )(grid->_vertices[grid->_it_v->first]._edges[grid->_it_e->first]._data);
				data->_ids.clear();
				grid->_it_e++;
			}
			grid->_it_v++;
		}
	}
}


std::vector<std::pair<uint, uint> > Map::waiting_unit_positions_edges(Unit * unit, UnitType * unit_type) {
	GraphGrid * grid = _path_finders[unit_type]->_units_position_grid;
	AABB_2D * aabb = new AABB_2D(pt_2d(unit->_aabb->_vmin - 0.5 * unit_type->_size), pt_2d(unit->_aabb->size() + unit_type->_size));
	std::vector<std::pair<uint, uint> > edges = grid->aabb_intersection(aabb);
	delete aabb;

	return edges;
}


std::vector<std::pair<uint, uint> > Map::moving_unit_positions_edges(Unit * unit, UnitType * unit_type, bool all) {
	GraphGrid * grid = _path_finders[unit_type]->_units_position_grid;
	// demi-taille de la diagonale du + petit carré contenant unit_type->_size (+ epsilon)
	number buffer_size = 0.5 * norm(pt_2d(std::max(unit_type->_size.x, unit_type->_size.y))) + 0.1;
	AABB_2D * aabb_unit = unit->_aabb->aabb2d();

	std::vector<std::pair<uint, uint> > edges;
	bool intersection_happened = false;

	AABB_2D * aabb_start = new AABB_2D(unit->_path->_start - 0.5 * pt_2d(unit_type->_size), pt_2d(unit->_aabb->size() + unit_type->_size));
	if (!all && !intersection_happened) {
		if (aabb2d_intersects_aabb2d(aabb_unit, aabb_start)) {
			intersection_happened = true;
		}
	}
	if (all || !intersection_happened) {
		std::vector<std::pair<uint, uint> > start_edges = grid->aabb_intersection(aabb_start);
		edges.insert(edges.end(), start_edges.begin(), start_edges.end());
	}
	delete aabb_start;
	
	for (uint i=0; i<unit->_path->_bboxs.size(); ++i) {
		BBox_2D * buffered_bbox = unit->_path->_bboxs[i]->buffered(buffer_size);
		if (!all && !intersection_happened) {
			if (aabb2d_intersects_aabb2d(aabb_unit, buffered_bbox->_aabb)) {
				intersection_happened = true;
			}
		}
		if (all || !intersection_happened) {
			std::vector<std::pair<uint, uint> > path_edges = grid->bbox_intersection(buffered_bbox);
			edges.insert(edges.end(), path_edges.begin(), path_edges.end());
		}
		delete buffered_bbox;
	}

	AABB_2D * aabb_goal = new AABB_2D(unit->_path->_goal - pt_2d(unit->_type->_size- 0.5 * unit_type->_size), pt_2d(unit->_type->_size + unit_type->_size));
	if (!all && !intersection_happened) {
		if (aabb2d_intersects_aabb2d(aabb_unit, aabb_goal)) {
			intersection_happened = true;
		}
	}
	if (all || !intersection_happened) {
		std::vector<std::pair<uint, uint> > goal_edges = grid->aabb_intersection(aabb_goal);
		edges.insert(edges.end(), goal_edges.begin(), goal_edges.end());
	}
	delete aabb_goal;

	delete aabb_unit;
	
	return edges;
}


void Map::add_waiting_unit_to_position_grids(Unit * unit) {
	for (auto & path_finder : _path_finders) {
		UnitType * unit_type = path_finder.first;
		GraphGrid * grid = path_finder.second->_units_position_grid;

		for (auto & e : waiting_unit_positions_edges(unit, unit_type)) {
			GraphEdge & edge = grid->_vertices[e.first]._edges[e.second];
			UnitsPositionEdgeData * data = (UnitsPositionEdgeData *)(edge._data);
			if (std::find(data->_ids.begin(), data->_ids.end(), unit->_id) == data->_ids.end()) {
				data->_ids.push_back(unit->_id);
			}
		}
	}
}


void Map::remove_waiting_unit_from_position_grids(Unit * unit) {
	for (auto & path_finder : _path_finders) {
		UnitType * unit_type = path_finder.first;
		GraphGrid * grid = path_finder.second->_units_position_grid;

		for (auto & e : waiting_unit_positions_edges(unit, unit_type)) {
			GraphEdge & edge = grid->_vertices[e.first]._edges[e.second];
			UnitsPositionEdgeData * data = (UnitsPositionEdgeData *)(edge._data);
			if (std::find(data->_ids.begin(), data->_ids.end(), unit->_id) != data->_ids.end()) {
				data->_ids.erase(std::find(data->_ids.begin(), data->_ids.end(), unit->_id));
			}
		}
	}
}


void Map::add_moving_unit_to_position_grids(Unit * unit) {
	for (auto & path_finder : _path_finders) {
		UnitType * unit_type = path_finder.first;
		GraphGrid * grid = path_finder.second->_units_position_grid;

		for (auto & e : moving_unit_positions_edges(unit, unit_type, true)) {
			GraphEdge & edge = grid->_vertices[e.first]._edges[e.second];
			UnitsPositionEdgeData * data = (UnitsPositionEdgeData *)(edge._data);
			if (std::find(data->_ids.begin(), data->_ids.end(), unit->_id) == data->_ids.end()) {
				data->_ids.push_back(unit->_id);
			}
		}
	}
}


void Map::remove_moving_unit_from_position_grids(Unit * unit, bool all) {
	for (auto & path_finder : _path_finders) {
		UnitType * unit_type = path_finder.first;
		GraphGrid * grid = path_finder.second->_units_position_grid;
		
		for (auto & e : moving_unit_positions_edges(unit, unit_type, all)) {
			GraphEdge & edge = grid->_vertices[e.first]._edges[e.second];
			UnitsPositionEdgeData * data = (UnitsPositionEdgeData *)(edge._data);
			if (std::find(data->_ids.begin(), data->_ids.end(), unit->_id) != data->_ids.end()) {
				data->_ids.erase(std::find(data->_ids.begin(), data->_ids.end(), unit->_id));
			}
		}
	}
}


void Map::path_find(Unit * unit, pt_2d goal) {
	_path_finders[unit->_type]->path_find(unit, goal);
}


void Map::clear() {
	/*for (auto & obstacle : _obstacles) {
		delete obstacle;
	}
	_obstacles.clear();
	for (auto & buffered_obstacle : _buffered_obstacles) {
		for (auto & obst : buffered_obstacle.second) {
			delete obst;
		}
		buffered_obstacle.second.clear();
	}*/

	clear_units_position_grids();
	
	for (auto & unit : _units) {
		delete unit;
	}
	_units.clear();

	_elements->clear();
	
	_elevation->set_alti_all(0.0);
	
	sync2elevation();
}


/*void Map::read_shapefile(std::string shp_path, pt_2d origin, pt_2d size, bool reverse_y) {
	std::vector<ShpEntry *> entries;
	read_shp(shp_path, entries);
	for (auto & entry : entries) {
		std::vector<pt_2d> pts;
		for (auto & pt : entry->_polygon->_pts) {
			number x= ((pt.x- origin.x)/ size.x)* _size.x+ _origin.x;
			number y;
			if (reverse_y) {
				y= ((origin.y- pt.y)/ size.y)* _size.y+ _origin.y;
			}
			else {
				y= ((pt.y- origin.y)/ size.y)* _size.y+ _origin.y;
			}
			pts.push_back(pt_2d(x, y));
		}
		OBSTACLE_TYPE obst_type = str2type(entry->_fields["type"]);
		if (obst_type != UNKNOWN) {
			add_obstacle(obst_type, pts);
		}
		delete entry;
	}
}*/


void Map::anim(time_point t) {
	if (_paused) {
		return;
	}

	for (auto & unit : _units) {

		std::mutex mtx;
		mtx.lock();
		if (unit->_status == COMPUTING_PATH) {
			continue;
		}
		mtx.unlock();

		if (unit->_status == COMPUTING_PATH_DONE) {
			_path_find_thr_active = false;
			//unit->_thr.join();
			_path_find_thr.join();
			update_alti_path(unit);
			unit->goto_next_checkpoint(t);
			remove_waiting_unit_from_position_grids(unit);
			add_moving_unit_to_position_grids(unit);
		}
		else if (unit->_status == COMPUTING_PATH_FAILED) {
			_path_find_thr_active = false;
			//unit->_thr.join();
			_path_find_thr.join();
			unit->_instructions.push({unit->_path->_goal, t + std::chrono::milliseconds(1000)});
			unit->stop();
		}
		else if (unit->_status == MOVING) {
			/*AABB * aabb_next = new AABB(*unit->_aabb);
			aabb_next->translate(unit->_velocity);
			for (auto & unit2 : _units) {
				if (unit2 == unit) {
					continue;
				}

				AABB * aabb2_buffered = new AABB(*unit2->_aabb);
				//aabb2_buffered->scale(1.0);
				if (aabb_intersects_aabb(aabb_next, aabb2_buffered)) {
					std::cout << "unit " << unit->_id << " : collision stop\n";
					unit->_instructions.push({unit->_path->destination(), t + std::chrono::milliseconds(2000 + rand_int(0, 2000))});
					unit->stop();
					//add_unit_to_position_grids(unit);
					break;
				}
			}*/

			unit->anim(t);

			if (unit->checkpoint_checked()) {
				if (unit->_path->_idx_path + 1 >= unit->_path->_pts.size()) {
					remove_moving_unit_from_position_grids(unit, true);
					add_waiting_unit_to_position_grids(unit);
					unit->stop();
				}
				else {
					remove_moving_unit_from_position_grids(unit, false);
					unit->_path->_idx_path++;
					unit->goto_next_checkpoint(t);
					/*if (unit->_path->_weights[unit->_path->_idx_path] >= MAX_UNIT_MOVING_WEIGHT) {
						std::cout << "unit " << unit->_id << "stops : path weight\n";
						unit->stop();
					}*/
				}
			}
			
			for (auto & unit2 : _units) {
				if (unit2 == unit) {
					continue;
				}

				if (aabb_intersects_aabb(unit->_aabb, unit2->_aabb)) {
					std::cerr << "intersection unit " << unit->_id << " et " << unit2->_id << "\n";
				}
			}

			pt_3d unit_center = unit->_aabb->bottom_center();
			number alti = _elevation->get_alti(unit_center);
			unit->_aabb->set_z(alti);

			/*if (unit->_status == WAITING) {
				std::cout << "unit " << unit->_id << " waiting\n";
				remove_unit_from_position_grids(unit);
				unit->_path->clear();
				add_unit_to_position_grids(unit);
			}*/
		}

		if (!_path_find_thr_active && !unit->_instructions.empty()) {
			Instruction i = unit->_instructions.front();
			if (i._t <= t) {
				unit->_instructions.pop();

				pt_2d pt = i._destination;
				//std::cout << "unit " << unit->_id << " : goto " << glm_to_string(pt) << "\n";

				unit->stop();
				unit->_status = COMPUTING_PATH;
				//unit->_thr= std::thread(&Map::path_find, this, unit, pt);
				_path_find_thr= std::thread(&Map::path_find, this, unit, pt);
				_path_find_thr_active = true;
			}
		}
	}
}


void Map::selected_units_goto(pt_2d pt, time_point t) {
	uint compt = 0;
	for (auto & unit : _units) {
		if (unit->_selected) {
			//unit->_instructions.push({pt, t + std::chrono::milliseconds(500 * compt)});
			unit->_instructions.push({pt, t});
			compt++;
		}
	}
}


void Map::randomize() {
	std::cout << "random begin\n";
	clear();
	
	_elevation->randomize();
	std::cout << "random 1\n";
	
	sync2elevation();

	std::cout << "random 2\n";

	for (uint i=0; i<1000; ++i) {
		std::string element_name = std::vector<std::string>{"tree_test", "stone"}[rand_int(0, 1)];
		pt_3d size;
		if (element_name == "stone") {
			size = rand_pt_3d(0.2, 1.0, 0.2, 1.0, 0.1, 0.5);
		}
		else {
			size = rand_pt_3d(0.2, 1.0, 0.2, 1.0, 1.0, 2.0);
		}
		pt_2d pt = rand_pt(_origin + 0.5 * pt_2d(size.x, size.y)+ 0.1, _origin + _size - 0.5 * pt_2d(size.x, size.y)- 0.1);
		number alti = _elevation->get_alti(pt);
		if (alti > 0.01) {
			//add_static_element(element_name, pt_3d(pt.x, pt.y, alti), size);
		}
	}
	std::cout << "random end\n";
}


void Map::save(std::string json_path) {
	json js;

	js["Elevation"] = json::array();
	for (uint lig = 0; lig<_elevation->_n_ligs; ++lig) {
		for (uint col = 0; col<_elevation->_n_cols; ++col) {
			js["Elevation"].push_back(_elevation->_altis[_elevation->col_lig2id(col, lig)]);
		}
	}

	js["units"] = json::array();
	for (auto unit : _units) {
		json entry;
		entry["type"] = unit->_type->_name;
		json position = json::array();
		position.push_back(unit->_aabb->center().x);
		position.push_back(unit->_aabb->center().y);
		//position.push_back(unit->_aabb->center().z);
		entry["position"]= position;
		js["units"].push_back(entry);
	}

	std::ofstream ofs(json_path);
	ofs << js << "\n";
}


void Map::load(std::string json_path, time_point t) {
	clear();

	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	uint idx = 0;
	for (auto & alti : js["Elevation"]) {
		_elevation->_altis[idx] = alti;
		idx++;
	}

	for (auto & unit : js["units"]) {
		add_unit(unit["type"], pt_2d(unit["position"][0], unit["position"][1]), t);
	}

	sync2elevation();
}


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

