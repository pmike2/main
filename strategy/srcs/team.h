#ifndef TEAM_H
#define TEAM_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <queue>
#include <unordered_set>

#include "json.hpp"

#include "typedefs.h"
#include "bbox_2d.h"
#include "bbox.h"
#include "graph.h"
#include "path_find.h"

#include "unit_type.h"
#include "unit.h"
#include "elevation.h"
#include "ammo.h"
#include "barrier_type.h"


using json = nlohmann::json;


struct Unit;


struct FowVertexData {
	FOW_STATUS _status;
	uint _n_units;
	bool _changed;
};


struct Team {
	Team();
	Team(std::string name, glm::vec3 color, Elevation * elevation, PathFinder * path_finder, pt_2d fow_resolution);
	~Team();
	
	bool fow_check(pt_2d pos);
	bool construction_check(UnitType * type);
	bool add_unit_check(UnitType * type, pt_2d pos, bool fow_active, bool construction_active);
	bool move_unit_check(Unit * unit, pt_2d pos, bool fow_active);
	bool attack_unit_check(Unit * attacking_unit, Unit * attacked_unit, bool fow_active);
	bool add_barrier_check(BarrierType * type, pt_2d pos, number orientation, bool fow_active);

	Unit * add_unit(UnitType * type, pt_2d pos, time_point t);
	std::vector<Unit *> get_units_in_aabb(AABB_2D * aabb);
	std::vector<Unit *> get_selected_units();
	Unit * get_first_active_unit();
	void remove_unit(Unit * unit);
	void clear();
	void clear_selection();
	bool empty();
	void selected_units_goto(pt_3d pt);
	void unit_attack(Unit * unit, Unit * target, time_point t, bool fow_active);
	void selected_units_attack(Unit * target, time_point t, bool fow_active);
	Unit * search_target(Unit * unit, Team * ennemy_team, bool fow_active);
	void anim_units(time_point t);
	void update_fow_unit(Unit * unit);
	void update_fow();
	void clear_fow();
	json get_json();
	Unit * get_unit_under_construction(UnitType * unit_type);
	number get_construction_progress(UnitType * unit_type, time_point t);
	friend std::ostream & operator << (std::ostream & os, Team & team);


	Elevation * _elevation;
	PathFinder * _path_finder;
	std::string _name;
	std::vector<Unit *> _units;
	glm::vec3 _color;
	bool _ia;
	GraphGrid * _fow;
	float * _fow_data; 
};


#endif
