#include "utile.h"

#include "unit_path.h"


PathInterval::PathInterval() {

}


PathInterval::PathInterval(number weight) : _weight(weight), _active(false) {
	_bbox = new BBox_2D();
}


PathInterval::PathInterval(const PathInterval & interval) : _weight(interval._weight), _active(false) {
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
UnitPath::UnitPath() : _idx_path(0), _use_line_of_sight(false), _start(pt_3d(0.0)), _goal(pt_3d(0.0)) {

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


PathInterval * UnitPath::get_current_interval() {
	if (_idx_path == 0) {
		std::cerr << "Path::get_current_interval() : _idx_path == 0\n";
		return NULL;
	}

	if (_use_line_of_sight) {
		if (_idx_path - 1 >= _intervals_los.size()) {
			std::cerr << "Path::get_current_interval() : " << _idx_path  << ">=" << _intervals_los.size() << "\n";
			return NULL;
		}
		return _intervals_los[_idx_path - 1];
	}
	else {
		if (_idx_path - 1 >= _intervals.size()) {
			std::cerr << "Path::get_current_interval() : " << _idx_path  << ">=" << _intervals.size() << "\n";
			return NULL;
		}
		return _intervals[_idx_path - 1];
	}
}


PathInterval * UnitPath::get_last_active_interval() {
	if (_use_line_of_sight) {
		for (int i=_intervals_los.size() - 1; i>=0; --i) {
			if (_intervals_los[i]->_active) {
				return _intervals_los[i];
			}
		}
	}
	else {
		for (int i=_intervals.size() - 1; i>=0; --i) {
			if (_intervals[i]->_active) {
				return _intervals[i];
			}
		}
	}

	std::cerr << "UnitPath::get_last_active_interval() : aucun intervalle actif.\n";
	return NULL;
}


pt_3d UnitPath::get_current_pt() {
	if (_use_line_of_sight) {
		if (_idx_path >= _pts_los.size()) {
			std::cerr << "Path::get_current_pt() : " << _idx_path  << ">=" << _pts_los.size() << "\n";
			return pt_3d(0.0);
		}
		return _pts_los[_idx_path];
	}
	else {
		if (_idx_path >= _pts.size()) {
			std::cerr << "Path::get_current_pt() : " << _idx_path  << ">=" << _pts.size() << "\n";
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

	update_active_intervals();
}


void UnitPath::next_checkpoint() {
	if (is_last_checkpoint()) {
		std::cerr << "UnitPath::next_checkpoint : is_last_checkpoint = true.\n";
		return;
	}
	_idx_path++;
	update_active_intervals();
}


void UnitPath::update_active_intervals() {
	if (_use_line_of_sight) {
		for (uint idx=0; idx<_intervals_los.size(); ++idx) {
			if (idx >= _idx_path - 1 && idx < _idx_path - 1 + N_ACTIVE_INTERVALS) {
				_intervals_los[idx]->_active = true;
			}
			else {
				_intervals_los[idx]->_active = false;
			}
		}
	}
	else {
		for (uint idx=0; idx<_intervals.size(); ++idx) {
			if (idx >= _idx_path - 1 && idx < _idx_path - 1 + N_ACTIVE_INTERVALS) {
				_intervals[idx]->_active = true;
			}
			else {
				_intervals[idx]->_active = false;
			}
		}
	}
}


json UnitPath::get_json() {
	json result;

	//result["start_id"] = _start_id;
	//result["goal_id"] = _goal_id;
	result["start"] = json::array();
	result["start"].push_back(_start.x);
	result["start"].push_back(_start.y);
	result["start"].push_back(_start.z);
	result["goal"].push_back(_goal.x);
	result["goal"].push_back(_goal.y);
	result["goal"].push_back(_goal.z);
	result["nodes"] = json::array();
	for (auto & node : _nodes) {
		result["nodes"].push_back(node);
	}
	result["idx_path"] = _idx_path;
	result["use_line_of_sight"] = _use_line_of_sight;
	
	result["pts"] = json::array();
	for (auto & pt : _pts) {
		json js_pt = json::array();
		js_pt.push_back(pt.x);
		js_pt.push_back(pt.y);
		js_pt.push_back(pt.z);
		result["pts"].push_back(js_pt);
	}
	
	result["pts_los"] = json::array();
	for (auto & pt : _pts_los) {
		json js_pt = json::array();
		js_pt.push_back(pt.x);
		js_pt.push_back(pt.y);
		js_pt.push_back(pt.z);
		result["pts_los"].push_back(js_pt);
	}
	
	result["intervals"] = json::array();
	for (auto & interval : _intervals) {
		json js_interval;
		js_interval["weight"] = interval->_weight;
		js_interval["bbox"] = json::object();
		js_interval["bbox"]["center"] = json::array({interval->_bbox->_center.x, interval->_bbox->_center.y});
		js_interval["bbox"]["half_size"] = json::array({interval->_bbox->_half_size.x, interval->_bbox->_half_size.y});
		js_interval["bbox"]["alpha"] = interval->_bbox->_alpha;
		result["intervals"].push_back(js_interval);
	}

	result["intervals_los"] = json::array();
	for (auto & interval : _intervals_los) {
		json js_interval;
		js_interval["weight"] = interval->_weight;
		js_interval["bbox"] = json::object();
		js_interval["bbox"]["center"] = json::array({interval->_bbox->_center.x, interval->_bbox->_center.y});
		js_interval["bbox"]["half_size"] = json::array({interval->_bbox->_half_size.x, interval->_bbox->_half_size.y});
		js_interval["bbox"]["alpha"] = interval->_bbox->_alpha;
		result["intervals_los"].push_back(js_interval);
	}

	return result;
}


std::ostream & operator << (std::ostream & os, UnitPath & p) {
	os << "unit_id = " << p._unit_id;
	os << " ; unit_type = " << unit_type2str(p._unit_type->_type);
	os << " ; start_id = " << p._start_id << " ; start = " << glm_to_string(p._start);
	os << " ; goal_id = " << p._goal_id << " ; goal = " << glm_to_string(p._goal);
	os << " ; idx_path = " << p._idx_path;
	os << " ; use_line_of_sight = " << p._use_line_of_sight;
	os << " ; status = " << p._status;

	os << " ; n pts = " << p._pts.size();
	os << " ; n intervals = " << p._intervals.size();
	os << " ; n pts_los = " << p._pts_los.size();
	os << " ; n intervals_los = " << p._intervals_los.size();

	return os;
}


