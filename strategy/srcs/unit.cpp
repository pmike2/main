#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "utile.h"

#include "unit.h"


// -------------------------------------------------
Unit::Unit() {

}


Unit::Unit(UnitType * type, pt_3d pos, Elevation * elevation) : 
	InstancePosRot(pos, quat(1.0, 0.0, 0.0, 0.0), pt_3d(1.0), type->_obj_data->_aabb),
	_type(type), _status(WAITING), _velocity(pt_3d(0.0)), _paused(false), _elevation(elevation)
{
	_path = new UnitPath();
}


Unit::~Unit() {
	delete _path;
}


/*pt_3d Unit::pt2dto3d(pt_2d pt) {
	pt_3d result(pt.x, pt.y, 0.0);
	if (_type->_floats) {
		result.z = 0.0;
	}
	else {
		result.z = _elevation->get_alti(pt);
	}
	return result;
}*/


void Unit::anim() {
	if (_paused) {
		return;
	}

	if (_status == MOVING) {
		if (checkpoint_checked()) {
			if (_path->is_last_checkpoint()) {
				set_status(LAST_CHECKPOINT_CHECKED);
			}
			else {
				set_status(CHECKPOINT_CHECKED);
			}
			return;
		}
		//auto d= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_anim_t).count();
		//_last_anim_t = t;

		number velocity_amp = _type->_max_velocity * (1.0 - _path->get_current_interval()->_weight / MAX_UNIT_MOVING_WEIGHT);
		pt_2d direction = glm::normalize(pt_2d(_path->get_current_pt()) - pt_2d(_position));
		_velocity = velocity_amp * pt_3d(direction.x, direction.y, 0.0);

		pt_3d next_position = _position + _velocity;
		next_position.z = _elevation->get_alti(next_position);
		if (_type->_floats && next_position.z < 0.0) {
			next_position.z = 0.0;
		}
		
		number angle = atan2(_velocity.y, _velocity.x);
		// https://en.wikipedia.org/wiki/Slerp
		const number slerp_speed = 0.05;
		quat next_quat = glm::angleAxis(float(angle), glm::vec3(0.0f, 0.0f, 1.0f));
		quat interpolated_quat = _rotation * glm::pow(glm::inverse(_rotation) * next_quat, slerp_speed);

		set_pos_rot_scale(next_position, interpolated_quat, pt_3d(1.0));
	}
}


bool Unit::checkpoint_checked() {
	if (glm::distance(pt_2d(_position), pt_2d(_path->get_current_pt())) < UNIT_DIST_PATH_EPS) {
		return true;
	}
	return false;
}


void Unit::set_status(UNIT_STATUS status) {
	const bool verbose = true;

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
			set_status(WAITING);
		}
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


Team::Team(std::string name, Elevation * elevation) : _name(name), _elevation(elevation) {

}


Team::~Team() {
	for (auto & unit : _units) {
		delete unit;
	}
	_units.clear();
}


Unit * Team::add_unit(UnitType * type, uint id, pt_2d pos) {
	if (pos.x < _elevation->_origin.x || pos.y < _elevation->_origin.y || pos.x >= _elevation->_origin.x + _elevation->_size.x || pos.y >= _elevation->_origin.y + _elevation->_size.y) {
		std::cerr << "Team::add_unit hors Elevation\n";
		return NULL;
	}
	pt_3d pt3d(pos.x, pos.y, _elevation->get_alti(pos));
	if (type->_floats && pt3d.z < 0.0) {
		pt3d.z = 0.0;
	}
	Unit * unit = new Unit(type, pt3d, _elevation);
	unit->_id = id;
	_units.push_back(unit);
	return unit;
}


void Team::remove_unit(Unit * unit) {
	_units.erase(std::remove_if(_units.begin(), _units.end(), [unit](Unit * u) {
		return u == unit;
	}), _units.end());
	delete unit;
}


void Team::remove_units_in_aabb(AABB_2D * aabb) {
	std::vector<Unit *> units2remove;
	for (auto & unit : _units) {
		if (aabb2d_intersects_aabb2d(aabb, unit->_bbox->_aabb->aabb2d())) {
			units2remove.push_back(unit);
		}
	}
	for (auto & unit : units2remove) {
		remove_unit(unit);
	}
}


void Team::clear() {
	for (auto & unit : _units) {
		delete unit;
	}
	_units.clear();
}


std::ostream & operator << (std::ostream & os, Team & team) {
	os << "team name = " << team._name << " ; units =\n";
	for (auto & unit : team._units) {
		os << *unit << "\n";
	}
	return os;
}

