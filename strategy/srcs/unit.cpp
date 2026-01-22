#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "utile.h"

#include "unit.h"


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


Unit::Unit(UnitType * type, pt_3d pos) : InstancePosRot(pos, quat(1.0, 0.0, 0.0, 0.0), pt_3d(1.0), type->_obj_data->_aabb),
	_type(type), _status(WAITING), _velocity(pt_3d(0.0)), _paused(false)
{
	_path = new Path();
}


Unit::~Unit() {
	delete _path;
}


void Unit::anim(Elevation * elevation) {
	if (_paused) {
		return;
	}

	if (_status == MOVING) {
		if (checkpoint_checked()) {
			if (_path->_idx_path == _path->_pts.size() - 1) {
				set_status(LAST_CHECKPOINT_CHECKED);
			}
			else {
				set_status(CHECKPOINT_CHECKED);
			}
			return;
		}
		//auto d= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_anim_t).count();
		//_last_anim_t = t;

		number velocity_amp = _type->_max_velocity * (1.0 - _path->_weights[_path->_idx_path] / MAX_UNIT_MOVING_WEIGHT);
		pt_2d direction = glm::normalize(pt_2d(_path->_pts[_path->_idx_path]) - pt_2d(_position));
		_velocity = velocity_amp * pt_3d(direction.x, direction.y, 0.0);

		pt_3d next_position = _position + _velocity;
		next_position.z = elevation->get_alti(next_position);
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
	if (_path->_pts.size() == 0 || _path->_idx_path > _path->_pts.size() - 1) {
		std::cerr << "Unit::checkpoint_checked() : path vide ou dépassé\n";
		return true;
	}
	if (glm::distance(pt_2d(_position), pt_2d(_path->_pts[_path->_idx_path])) < UNIT_DIST_PATH_EPS) {
		return true;
	}
	return false;
}


void Unit::set_status(UNIT_STATUS status) {
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
	Unit * unit = new Unit(type, pt3d);
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

