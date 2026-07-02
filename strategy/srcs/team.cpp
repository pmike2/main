#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "utile.h"

#include "team.h"


Team::Team() {

}


Team::Team(std::string name, glm::vec3 color, Elevation * elevation, PathFinder * path_finder, pt_2d fow_resolution) : 
	_name(name), _color(color), _elevation(elevation), _path_finder(path_finder), _ia(false)
{
	uint n_ligs = uint(_elevation->_size.y / fow_resolution.y) + 1;
	uint n_cols = uint(_elevation->_size.x / fow_resolution.x) + 1;
	_fow = new GraphGrid(_elevation->_origin, _elevation->_size, n_ligs, n_cols);

	_fow->_it_v= _fow->_vertices.begin();
	while (_fow->_it_v!= _fow->_vertices.end()) {
		FowVertexData * data = new FowVertexData();
		data->_status = UNDISCOVERED;
		data->_n_units = 0;
		data->_changed = false;
		_fow->_it_v->second._data = data;
		_fow->_it_v++;
	}

	_fow_data = new float[n_ligs * n_cols];
	for (uint i=0; i<n_ligs * n_cols; ++i) {
		_fow_data[i] = 0.0;
	}
}


Team::~Team() {
	for (auto & unit : _units) {
		delete unit;
	}
	_units.clear();
	delete _fow;
	delete _fow_data;
}


bool Team::fow_check(pt_2d pos) {
	if (!point_in_aabb2d(pos, _elevation->_aabb)) {
		return false;
	}

	std::vector<uint> vertices = _fow->vertices_in_cell_containing_pt(pos);
	for (auto & v : vertices) {
		GraphVertex vertex = _fow->get_vertex(v);
		FowVertexData * data = (FowVertexData *)(vertex._data);
		if (data->_status == UNDISCOVERED) {
			return false;
		}
	}
	return true;
}


bool Team::construction_check(UnitType * type) {
	if (get_unit_under_construction(type) != NULL) {
		return false;
	}
	return true;
}


bool Team::add_unit_check(UnitType * type, pt_2d pos, bool fow_active, bool construction_active) {
	if (_units.size() >= N_MAX_UNITS_PER_TEAM) {
		return false;
	}

	if (!point_in_aabb2d(pos, _elevation->_aabb)) {
		return false;
	}

	if (fow_active && !fow_check(pos)) {
		return false;
	}

	if (construction_active && !construction_check(type)) {
		return false;
	}

	pt_2d unit_size = type->get_max_square_size();
	AABB_2D * aabb = new AABB_2D(pos - 0.5 * unit_size, unit_size);
	// petit buffer
	aabb->buffer(1.5);
	std::vector<uint> vertices = _path_finder->vertices_in_aabb(aabb);
	delete aabb;
	for (auto & v : vertices) {
		if (_path_finder->is_vertex_obstacle(type->_name, v)) {
			return false;
		}
	}

	return true;
}


bool Team::move_unit_check(Unit * unit, pt_2d pos, bool fow_active) {
	if (fow_active && !fow_check(pos)) {
		return false;
	}

	pt_2d unit_size = unit->_type->get_max_square_size();
	AABB_2D * aabb = new AABB_2D(pos - 0.5 * unit_size, unit_size);
	std::vector<uint> vertices = _path_finder->vertices_in_aabb(aabb);
	delete aabb;
	for (auto & v : vertices) {
		if (_path_finder->is_vertex_obstacle(unit->_type->_name, v, unit)) {
			return false;
		}
	}
	return true;
}


bool Team::attack_unit_check(Unit * attacking_unit, Unit * attacked_unit, bool fow_active) {
	if (fow_active && !fow_check(pt_2d(attacked_unit->_position))) {
		return false;
	}
	return true;
}


bool Team::add_barrier_check(BarrierType * type, pt_2d pos, number orientation, bool fow_active) {
	if (fow_active && !fow_check(pos)) {
		return false;
	}
	return true;
}


Unit * Team::add_unit(UnitType * type, pt_2d pos, time_point t) {
	pt_3d pt3d(pos.x, pos.y, _elevation->get_alti(pos));
	if (type->_floats && pt3d.z < 0.0) {
		pt3d.z = 0.0;
	}

	Unit * new_unit = NULL;
	for (auto & unit : _units) {
		if (unit->_unit_status == UNIT_INACTIVE && unit->_type == type) {
			new_unit = unit;
			break;
		}
	}

	if (new_unit != NULL) {
		new_unit->reinit(pt3d, t);
	}
	else {
		new_unit = new Unit(this, type, pt3d, _elevation, t);
		_units.push_back(new_unit);
	}

	update_fow_unit(new_unit);

	_path_finder->init_gmo(new_unit);

	return new_unit;
}


std::vector<Unit *> Team::get_units_in_aabb(AABB_2D * aabb) {
	std::vector<Unit *> result;
	for (auto & unit : _units) {
		if (aabb2d_intersects_aabb2d(aabb, unit->_bbox->_aabb->aabb2d())) {
			result.push_back(unit);
		}
	}
	return result;
}


std::vector<Unit *> Team::get_selected_units() {
	std::vector<Unit *> result;
	for (auto & unit : _units) {
		if (unit->_selected) {
			result.push_back(unit);
		}
	}
	return result;
}


Unit * Team::get_first_active_unit() {
	if (empty()) {
		std::cerr << "Team::get_first_active_unit() : empty\n";
		return NULL;
	}
	for (auto & unit : _units) {
		if (unit->_unit_status != UNIT_INACTIVE) {
			return unit;
		}
	}
	return NULL;
}


void Team::remove_unit(Unit * unit) {
	_units.erase(std::remove_if(_units.begin(), _units.end(), [unit](Unit * u) {
		return u == unit;
	}), _units.end());
	delete unit;
}


void Team::clear() {
	for (auto & unit : _units) {
		unit->_unit_status = UNIT_DESTROYED;
	}
}


void Team::clear_selection() {
	for (auto & unit : _units) {
		unit->_selected = false;
	}
}


bool Team::empty() {
	uint n_units = std::count_if(_units.begin(), _units.end(), [](Unit * unit) { return unit->_unit_status != UNIT_INACTIVE; });
	if (n_units == 0) {
		return true;
	}
	return false;
}


void Team::selected_units_goto(pt_3d pt) {
	for (auto & unit : _units) {
		if (unit->_selected) {
			_path_finder->goto_gmo(unit, pt_2d(pt), false);
		}
	}
}


bool Team::is_target_reachable(Unit * unit, Unit * target) {
	const number offset_z = 0.5;
	number dist = glm::length(unit->_position - target->_position);
	if (dist > unit->_type->_ammo_type->_max_distance) {
		return false;
	}

	if (!unit->_type->_ammo_type->_ballistic) {
		number max_elevation_alti = _elevation->get_max_alti_along_segment(unit->_position, target->_position);
		if (max_elevation_alti > std::max(unit->_position.z, target->_position.z) + offset_z) {
			return false;
		}
	}

	return true;
}


void Team::unit_attack(Unit * unit, Unit * target, time_point t) {
	if (is_target_reachable(unit, target)) {
		unit->_target = target;
		unit->_unit_status = UNIT_ATTACKING;
	}
}


void Team::selected_units_attack(Unit * target, time_point t) {
	for (auto & unit : _units) {
		if (unit->_selected) {
			unit_attack(unit, target, t);
		}
	}
}


Unit * Team::search_target(Unit * unit, Team * ennemy_team) {
	if (ennemy_team == this) {
		return NULL;
	}

	for (auto & ennemy_unit : ennemy_team->_units) {
		if (is_target_reachable(unit, ennemy_unit)) {
			return ennemy_unit;
		}
	}

	return NULL;
}


void Team::anim_units(time_point t) {
	bool verbose = false;

	for (auto & unit : _units) {
		if (unit->_unit_status == UNIT_INACTIVE) {
			return;
		}

		unit->anim(t);

		if (unit->_unit_status == UNIT_DESTROYED) {
			unit->_unit_status = UNIT_INACTIVE;
			unit->_gmo_status = GMO_IDLE;
			unit->_selected = false;

			for (auto & id_tile : unit->_visible_tiles) {
				GraphVertex vertex = _fow->get_vertex(id_tile);
				FowVertexData * data = (FowVertexData *)(vertex._data);
				data->_changed = true;
				data->_n_units--;
			}

			_path_finder->remove_gmo(unit);
			
			return;
		}

		if (unit->_gmo_status == GMO_IDLE) {
			if (unit->_unit_status == UNIT_ATTACKING) {
				if (unit->_target->_unit_status == UNIT_DESTROYED || unit->_target->_hit_status == FINAL_HIT) {
					unit->_unit_status = UNIT_WATCHING;
				}
			}
		}
		else if (unit->_gmo_status == GMO_MOVING || unit->_gmo_status == GMO_WAITING) {

		}
	}
}


void Team::update_fow_unit(Unit * unit) {
	// vertices_in_circle_section et vertices_in_circle sont trop lents...

	//std::vector<uint> vertices_in_front_of_unit = _fow->vertices_in_circle_section(pt_2d(unit->_position), unit->_type->_vision_distance, unit->_angle, unit->_type->_vision_angle);
	
	//std::vector<uint> vertices_in_front_of_unit = _fow->vertices_in_circle(pt_2d(unit->_position), unit->_type->_vision_distance);
	
	AABB_2D * aabb = new AABB_2D(pt_2d(unit->_position) - pt_2d(unit->_type->_vision_distance), pt_2d(2.0 *  unit->_type->_vision_distance));
	std::vector<uint> vertices_in_front_of_unit = _fow->vertices_in_aabb(aabb);
	delete aabb;

	unit->_visible_tiles.clear();
	unit->_visible_tiles.insert(unit->_visible_tiles.begin(), vertices_in_front_of_unit.begin(), vertices_in_front_of_unit.end());
	//std::vector<uint> vertices_covering_unit = _fow->vertices_in_aabb(unit->_bbox->_aabb->aabb2d());
	//unit->_visible_tiles.insert(vertices_covering_unit.begin(), vertices_covering_unit.end());

	std::vector<uint> old_minus_new, new_minus_old;
	std::sort(unit->_visible_tiles.begin(), unit->_visible_tiles.end());
	std::sort(unit->_old_visible_tiles.begin(), unit->_old_visible_tiles.end());
	std::set_difference(unit->_old_visible_tiles.begin(), unit->_old_visible_tiles.end(), unit->_visible_tiles.begin(), unit->_visible_tiles.end(), std::inserter(old_minus_new, old_minus_new.begin()));
	std::set_difference(unit->_visible_tiles.begin(), unit->_visible_tiles.end(), unit->_old_visible_tiles.begin(), unit->_old_visible_tiles.end(), std::inserter(new_minus_old, new_minus_old.begin()));
	
	for (auto & id_tile : old_minus_new) {
		GraphVertex vertex = _fow->get_vertex(id_tile);
		FowVertexData * data = (FowVertexData *)(vertex._data);
		data->_changed = true;
		data->_n_units--;
	}

	for (auto & id_tile : new_minus_old) {
		GraphVertex vertex = _fow->get_vertex(id_tile);
		FowVertexData * data = (FowVertexData *)(vertex._data);
		data->_changed = true;
		data->_n_units++;
	}

	unit->_old_visible_tiles.clear();
	unit->_old_visible_tiles.insert(unit->_old_visible_tiles.begin(), unit->_visible_tiles.begin(), unit->_visible_tiles.end());
}


void Team::update_fow() {
	for (auto & unit : _units) {
		if (unit->_gmo_status == GMO_MOVING || unit->_unit_status == UNIT_WATCHING) {
			update_fow_unit(unit);
		}
	}

	_fow->_it_v= _fow->_vertices.begin();
	while (_fow->_it_v!= _fow->_vertices.end()) {
		FowVertexData * data = (FowVertexData *)(_fow->_it_v->second._data);
		if (data->_changed) {
			if (data->_n_units == 0) {
				data->_status = DISCOVERED;
				_fow_data[_fow->_it_v->first] = 0.5;
			}
			else {
				data->_status = WATCHED;
				_fow_data[_fow->_it_v->first] = 1.0;
			}

			data->_changed = false;
		}
		_fow->_it_v++;
	}
}


void Team::clear_fow() {
	_fow->_it_v= _fow->_vertices.begin();
	while (_fow->_it_v!= _fow->_vertices.end()) {
		FowVertexData * data = (FowVertexData *)(_fow->_it_v->second._data);
		data->_n_units = 0;
		data->_changed = false;
		_fow->_it_v++;
	}

	for (uint i=0; i<_fow->_n_ligs * _fow->_n_cols; ++i) {
		_fow_data[i] = 0.0;
	}
}


json Team::get_json() {
	json result;
	result["name"] = _name;
	result["color"] = json::array();
	result["color"].push_back(_color.r);
	result["color"].push_back(_color.g);
	result["color"].push_back(_color.b);
	result["units"] = json::array();
	for (auto & unit : _units) {
		if (unit->_unit_status != UNIT_INACTIVE) {
			result["units"].push_back(unit->get_json());
		}
	}
	return result;
}


Unit * Team::get_unit_under_construction(UnitType * unit_type) {
	for (auto & unit : _units) {
		if (unit->_type == unit_type && unit->_unit_status == UNIT_UNDER_CONSTRUCTION) {
			return unit;
		}
	}
	return NULL;
}


number Team::get_construction_progress(UnitType * unit_type, time_point t) {
	Unit * unit = get_unit_under_construction(unit_type);
	if (unit != NULL) {
		auto d_creation = std::chrono::duration_cast<std::chrono::milliseconds>(t - unit->_creation_t).count();
		return number(d_creation) / number(unit_type->_creation_duration);
	}
	return 1.0;
}


std::ostream & operator << (std::ostream & os, Team & team) {
	os << "team name = " << team._name << " ; units =\n";
	for (auto & unit : team._units) {
		os << *unit << "\n";
	}
	return os;
}

