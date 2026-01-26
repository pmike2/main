#include "utile.h"

#include "unit_path.h"


PathInterval::PathInterval() {

}


PathInterval::PathInterval(number weight) : _weight(weight) {
	_bbox = new BBox_2D();
}


PathInterval::PathInterval(const PathInterval & interval) : _weight(interval._weight) {
	_bbox = new BBox_2D(*interval._bbox);
}


PathInterval::~PathInterval() {
	clear();
	delete _bbox;
}


void PathInterval::clear() {
	_edges.clear();
}


// -------------------------
UnitPath::UnitPath() : _idx_path(0), _use_line_of_sight(true), _start(pt_3d(0.0)), _goal(pt_3d(0.0)) {

}


UnitPath::~UnitPath() {
	clear();
}


void UnitPath::clear() {
	_idx_path = 0;
	_nodes.clear();
	_start = pt_3d(0.0);
	_goal = pt_3d(0.0);

	_pts.clear();
	for (auto & interval : _intervals) {
		interval->clear();
	}
	_intervals.clear();

	_pts_los.clear();
	for (auto & interval : _intervals_los) {
		interval->clear();
	}
	_intervals_los.clear();
}


bool UnitPath::empty() {
	if (_use_line_of_sight) {
		if (_pts_los.size() == 0) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		if (_pts.size() == 0) {
			return true;
		}
		else {
			return false;
		}
	}
}


bool UnitPath::is_last_checkpoint() {
	if (_use_line_of_sight) {
		if (_idx_path == _pts_los.size() - 1) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		if (_idx_path == _pts.size() - 1) {
			return true;
		}
		else {
			return false;
		}
	}
}


number UnitPath::get_weight() {
	if (_idx_path == 0) {
		std::cerr << "Path::get_weight() : _idx_path == 0\n";
		return 0.0;
	}

	if (_use_line_of_sight) {
		if (_idx_path - 1 >= _intervals_los.size()) {
			std::cerr << "Path::get_weight() : " << _idx_path  << ">=" << _intervals_los.size() << "\n";
			return 0.0;
		}
		return _intervals_los[_idx_path - 1]->_weight;
	}
	else {
		if (_idx_path - 1 >= _intervals.size()) {
			std::cerr << "Path::get_weight() : " << _idx_path  << ">=" << _intervals.size() << "\n";
			return 0.0;
		}
		return _intervals[_idx_path - 1]->_weight;
	}
}


pt_3d UnitPath::get_pt() {
	if (_use_line_of_sight) {
		if (_idx_path >= _pts_los.size()) {
			std::cerr << "Path::get_pt() : " << _idx_path  << ">=" << _pts_los.size() << "\n";
			return pt_3d(0.0);
		}
		return _pts_los[_idx_path];
	}
	else {
		if (_idx_path >= _pts.size()) {
			std::cerr << "Path::get_pt() : " << _idx_path  << ">=" << _pts.size() << "\n";
			return pt_3d(0.0);
		}
		return _pts[_idx_path];
	}
}


std::vector<PathInterval *> UnitPath::get_intervals() {
	if (_use_line_of_sight) {
		return _intervals_los;
	}
	else {
		return _intervals;
	}
}


void UnitPath::copy_path(UnitPath * path) {
	clear();

	_start = path->_start;
	_goal = path->_goal;
	_idx_path = path->_idx_path;
	_use_line_of_sight = path->_use_line_of_sight;
	for (auto & node : path->_nodes) {
		_nodes.push_back(node);
	}

	for (auto & pt : path->_pts) {
		_pts.push_back(pt);
	}
	for (auto & interval : path->_intervals) {
		_intervals.push_back(new PathInterval(*interval));
	}

	for (auto & pt : path->_pts_los) {
		_pts_los.push_back(pt);
	}
	for (auto & interval : path->_intervals_los) {
		_intervals_los.push_back(new PathInterval(*interval));
	}
}


std::ostream & operator << (std::ostream & os, UnitPath & p) {
	os << "pts = ";
	for (auto pt : p._pts) {
		os << glm_to_string(pt, 1) << " ; ";
	}
	/*os << " | nodes = ";
	for (auto node : p._nodes) {
		os << node << " ; ";
	}*/
	
	/*os << " | weights = ";
	for (auto w : p._weights) {
		os << w << " ; ";
	}*/
	os << " | idx_path = " << p._idx_path;
	return os;
}


