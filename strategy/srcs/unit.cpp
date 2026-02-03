#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "utile.h"

#include "unit.h"


// -------------------------------------------------
Unit::Unit() {

}


Unit::Unit(Team * team, UnitType * type, pt_3d pos, Elevation * elevation) : 
	InstancePosRot(pos, quat(1.0, 0.0, 0.0, 0.0), pt_3d(1.0), type->_obj_data->_aabb),
	_team(team), _type(type), _status(WAITING), _velocity(pt_3d(0.0)), _paused(false), _elevation(elevation),
	_delete(false), _angle(0.0), _life(type->_life_init), _hit_status(NO_HIT)
{
	_path = new UnitPath();
}


Unit::~Unit() {
	delete _path;
}


void Unit::anim(time_point t) {
	auto d_moving = std::chrono::duration_cast<std::chrono::milliseconds>(t - _last_moving_t).count();
	_last_moving_t = t;

	if (_paused) {
		return;
	}

	if (_hit_status == HIT_ASCEND) {
		_hit += 0.4;
		if (_hit > 0.2 * _hit_ammo->_damage) {
			set_hit_status(HIT_DESCEND, t);
		}
	}
	else if (_hit_status == HIT_DESCEND) {
		_hit -= 0.3;
		if (_hit < 0.0) {
			set_hit_status(NO_HIT, t);
		}
	}
	else if (_hit_status == FINAL_HIT) {
		_hit += 0.5;
		if (_hit > 100.0) {
			set_status(DESTROYED, t);
		}
		return;
	}

	if (_status == MOVING) {
		number velocity_amp = _type->_max_velocity * (1.0 - _path->get_current_interval()->_weight / MAX_UNIT_MOVING_WEIGHT);
		//std::cout << d << "\n";
		velocity_amp *= number(d_moving) * 0.0625; // 60fps -> 1 frame == 1000 / 60 ~= 16 ms et 1 / 16 == 0.0625
		pt_2d direction = glm::normalize(pt_2d(_path->get_current_pt()) - pt_2d(_position));
		_velocity = velocity_amp * pt_3d(direction.x, direction.y, 0.0);

		pt_3d next_position = _position + _velocity;
		next_position.z = _elevation->get_alti(next_position);
		if (_type->_floats && next_position.z < 0.0) {
			next_position.z = 0.0;
		}
		
		number next_angle = atan2(_velocity.y, _velocity.x);
		// pour ne pas faire des 3/4 de tour quand les 2 angles sont de part et d'autre de l'axe x
		if (next_angle - _angle > M_PI) {
			next_angle -= 2.0 * M_PI;
		}
		_angle = next_angle;
		
		// https://en.wikipedia.org/wiki/Slerp
		const number slerp_speed = 0.05;
		quat next_quat = glm::angleAxis(float(_angle), glm::vec3(0.0f, 0.0f, 1.0f));
		quat interpolated_quat = _rotation * glm::pow(glm::inverse(_rotation) * next_quat, slerp_speed);

		set_pos_rot_scale(next_position, interpolated_quat, pt_3d(1.0));
	}
	else if (_status == ATTACKING) {
		if (_target == NULL)  {
			std::cerr << "Unit " << _id << " ATTACKING mais _target == NULL.\n";
			return;
		}

		number next_angle = atan2(_target->_position.y - _position.y, _target->_position.x - _position.x);
		// pour ne pas faire des 3/4 de tour quand les 2 angles sont de part et d'autre de l'axe x
		if (next_angle - _angle > M_PI) {
			next_angle -= 2.0 * M_PI;
		}
		_angle = next_angle;
		
		// https://en.wikipedia.org/wiki/Slerp
		const number slerp_speed = 0.05;
		quat next_quat = glm::angleAxis(float(_angle), glm::vec3(0.0f, 0.0f, 1.0f));
		quat interpolated_quat = _rotation * glm::pow(glm::inverse(_rotation) * next_quat, slerp_speed);

		set_pos_rot_scale(_position, interpolated_quat, pt_3d(1.0));

		auto d_shooting = std::chrono::duration_cast<std::chrono::milliseconds>(t - _last_shooting_t).count();
		//std::cout << number(d_shooting) << " ; " << _type->_shooting_rate * 1000.0 << "\n";
		if (number(d_shooting) > _type->_shooting_rate * 1000.0) {
			//std::cout << "Unit " << _id << " attacks Unit " << _target->_id << "\n";
			_last_shooting_t = t;
			
			set_status(SHOOTING, t);
		}
	}
}


bool Unit::checkpoint_checked() {
	if (glm::distance(pt_2d(_position), pt_2d(_path->get_current_pt())) < UNIT_DIST_PATH_EPS) {
		return true;
	}
	return false;
}


bool Unit::last_checkpoint_checked() {
	if (_path->is_last_checkpoint() && checkpoint_checked()) {
		return true;
	}
	return false;
}


void Unit::set_status(UNIT_STATUS status, time_point t) {
	const bool verbose = false;

	if (verbose) {
		std::cout << "Unit id = " << _id << " : status " << unit_status2str(_status) << " -> " << unit_status2str(status) << "\n";
	}
	
	_status = status;
	
	if (_status == WAITING) {
		_path->clear();
	}
	else if (_status == MOVING) {
		if (_path->empty()) {
			std::cerr << "Unit::follow_path : path empty\n";
			set_status(WAITING, t);
		}
		_last_moving_t = t;
	}
	else if (_status == ATTACKING) {
		_last_shooting_t = t;
	}
}


void Unit::set_hit_status(UNIT_HIT_STATUS hit_status, time_point t) {
	_hit_status = hit_status;
	if (_hit_status == NO_HIT) {
		_hit = 0.0;
		_hit_ammo = NULL;
	}
	else if (_hit_status == HIT_ASCEND) {
		//_last_hit_t = t;
	}
	else if (_hit_status == HIT_DESCEND) {

	}
	else if (_hit_status == FINAL_HIT) {
		//_last_hit_t = t;
	}
}


void Unit::hit(Ammo * ammo, time_point t) {
	_hit_ammo = ammo;
	_life -= ammo->_damage;
	if (_life <= 0.0) {
		_life = 0.0;
		//std::cout << "Unit " << _id << " destroyed\n";
		set_hit_status(FINAL_HIT, t);
	}
	else {
		set_hit_status(HIT_ASCEND, t);
	}
}


void Unit::update_alti_path() {
	for (auto & pt : _path->_pts) {
		if (_type->_floats) {
			pt.z = 0.0;
		}
		else {
			pt.z = _elevation->get_alti(pt_2d(pt.x, pt.y));
		}
	}
	for (auto & pt : _path->_pts_los) {
		if (_type->_floats) {
			pt.z = 0.0;
		}
		else {
			pt.z = _elevation->get_alti(pt_2d(pt.x, pt.y));
		}
	}
}


std::ostream & operator << (std::ostream & os, Unit & unit) {
	os << "type = " << unit_type2str(unit._type->_type);
	os << " ; mode = " << unit_status2str(unit._status);
	os << " ; velocity = " << glm_to_string(unit._velocity);
	os << " ; path = " << *unit._path;
	return os;
}


// Team ------------------------------------------
Team::Team() {

}


Team::Team(std::string name, glm::vec3 color, Elevation * elevation) : 
	_name(name), _color(color), _elevation(elevation), _ia(false)
{

}


Team::~Team() {
	for (auto & unit : _units) {
		delete unit;
	}
	_units.clear();
}


Unit * Team::add_unit(UnitType * type, uint id, pt_2d pos) {
	pt_3d pt3d(pos.x, pos.y, _elevation->get_alti(pos));
	if (type->_floats && pt3d.z < 0.0) {
		pt3d.z = 0.0;
	}
	Unit * unit = new Unit(this, type, pt3d, _elevation);
	unit->_id = id;
	_units.push_back(unit);
	return unit;
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


void Team::remove_unit(Unit * unit) {
	_units.erase(std::remove_if(_units.begin(), _units.end(), [unit](Unit * u) {
		return u == unit;
	}), _units.end());
	delete unit;
}


/*void Team::remove_units_in_aabb(AABB_2D * aabb) {
	std::vector<Unit *> units2remove;
	for (auto & unit : _units) {
		if (aabb2d_intersects_aabb2d(aabb, unit->_bbox->_aabb->aabb2d())) {
			units2remove.push_back(unit);
		}
	}
	for (auto & unit : units2remove) {
		remove_unit(unit);
	}
}*/


void Team::clear2delete() {
	// TODO : faire mieux ?

	std::vector<Unit * > tmp;
	for (auto & unit : _units) {
		if (unit->_delete) {
			tmp.push_back(unit);
		}
	}

	_units.erase(std::remove_if(_units.begin(), _units.end(), [](Unit * u) {
		return u->_delete;
	}), _units.end());

	for (auto & unit : tmp) {
		delete unit;
	}
}


void Team::clear() {
	for (auto & unit : _units) {
		delete unit;
	}
	_units.clear();
}


void Team::clear_selection() {
	for (auto & unit : _units) {
		unit->_selected = false;
	}
}


void Team::unit_goto(Unit * unit, pt_3d pt, time_point t) {
	unit->_instructions.push({pt, t});
}


void Team::selected_units_goto(pt_3d pt, time_point t) {
	//uint compt = 0;
	for (auto & unit : _units) {
		if (unit->_selected) {
			//unit->_instructions.push({pt, t + std::chrono::milliseconds(500 * compt)});
			unit_goto(unit, pt, t);
			//compt++;
		}
	}
}


bool Team::is_target_reachable(Unit * unit, Unit * target) {
	const number offset_z = 0.5;
	number dist = glm::length(unit->_position - target->_position);
	if (dist > unit->_type->_ammo_type->_max_distance) {
		return false;
	}
	number max_elevation_alti = _elevation->get_max_alti_along_segment(unit->_position, target->_position);
	if (max_elevation_alti > std::max(unit->_position.z, target->_position.z) + offset_z) {
		return false;
	}
	return true;
}


void Team::unit_attack(Unit * unit, Unit * target, time_point t) {
	if (is_target_reachable(unit, target)) {
		unit->_target = target;
		unit->set_status(ATTACKING, t);
	}
}


void Team::selected_units_attack(Unit * target, time_point t) {
	for (auto & unit : _units) {
		if (unit->_selected) {
			unit_attack(unit, target, t);
		}
	}
}


std::ostream & operator << (std::ostream & os, Team & team) {
	os << "team name = " << team._name << " ; units =\n";
	for (auto & unit : team._units) {
		os << *unit << "\n";
	}
	return os;
}

