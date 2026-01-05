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
