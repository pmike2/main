#include "utile.h"

#include "unit_path.h"


UnitPath::UnitPath() : _idx_path(0), _use_line_of_sight(true), _start(pt_3d(0.0)), _goal(pt_3d(0.0)) {

}


UnitPath::~UnitPath() {
	clear();
}


void UnitPath::clear() {
	_idx_path = 0;
	_nodes.clear();
	_pts.clear();
	_weights.clear();
	_pts_los.clear();
	_weights_los.clear();
	for (auto & bbox : _bboxs) {
		delete bbox;
	}
	_bboxs.clear();
	_start = pt_3d(0.0);
	_goal = pt_3d(0.0);
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
		if (_idx_path - 1 >= _weights_los.size()) {
			std::cerr << "Path::get_weight() : " << _idx_path  << ">=" << _weights_los.size() << "\n";
			return 0.0;
		}
		return _weights_los[_idx_path - 1];
	}
	else {
		if (_idx_path - 1 >= _weights.size()) {
			std::cerr << "Path::get_weight() : " << _idx_path  << ">=" << _weights.size() << "\n";
			return 0.0;
		}
		return _weights[_idx_path - 1];
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


void UnitPath::copy_path(UnitPath * path) {
	clear();

	_start = path->_start;
	_goal = path->_goal;
	for (auto & node : path->_nodes) {
		_nodes.push_back(node);
	}
	for (auto & pt : path->_pts) {
		_pts.push_back(pt);
	}
	for (auto & weight : path->_weights) {
		_weights.push_back(weight);
	}
	for (auto & pt : path->_pts_los) {
		_pts_los.push_back(pt);
	}
	for (auto & weight : path->_weights_los) {
		_weights_los.push_back(weight);
	}
	for (auto & bbox : path->_bboxs) {
		_bboxs.push_back(new BBox_2D(*bbox));
	}
	_idx_path = path->_idx_path;
	_use_line_of_sight = path->_use_line_of_sight;
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
	os << " | weights = ";
	for (auto w : p._weights) {
		os << w << " ; ";
	}
	os << " | idx_path = " << p._idx_path;
	return os;
}


