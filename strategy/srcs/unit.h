#ifndef UNIT_H
#define UNIT_H

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
#include "elevation.h"
#include "ammo.h"


using json = nlohmann::json;


struct Team;


struct Unit : public InstancePosRot, public GridMovingObject {
	Unit();
	Unit(Team * team, UnitType * type, pt_3d pos, Elevation * elevation, time_point t);
	~Unit();
	void reinit(pt_3d pos, time_point t);
	void anim(time_point t);
	void set_hit_status(UNIT_HIT_STATUS hit_status, time_point t);
	void hit(Ammo * ammo, time_point t);
	json get_json();
	friend std::ostream & operator << (std::ostream & os, Unit & unit);
	
	
	Team * _team;
	UnitType * _type;
	UNIT_STATUS _unit_status;
	UNIT_HIT_STATUS _hit_status;
	bool _paused;
	Elevation * _elevation;
	time_point _last_shooting_t;
	time_point _creation_t;
	number _angle;
	number _life;
	number _hit;
	Unit * _target;
	Ammo * _hit_ammo;
	std::vector<uint> _old_visible_tiles, _visible_tiles;
};


struct FowVertexData {
	FOW_STATUS _status;
	uint _n_units;
	bool _changed;
};


struct Team {
	Team();
	Team(std::string name, glm::vec3 color, Elevation * elevation, pt_2d fow_resolution);
	~Team();
	Unit * add_unit(UnitType * type, pt_2d pos, time_point t);
	std::vector<Unit *> get_units_in_aabb(AABB_2D * aabb);
	std::vector<Unit *> get_selected_units();
	Unit * get_first_active_unit();
	void remove_unit(Unit * unit);
	void clear();
	void clear_selection();
	bool empty();
	bool is_target_reachable(Unit * unit, Unit * target);
	void unit_attack(Unit * unit, Unit * target, time_point t);
	void selected_units_attack(Unit * target, time_point t);
	Unit * search_target(Unit * unit, Team * ennemy_team);
	void update_fow_unit(Unit * unit);
	void update_fow();
	void clear_fow();
	json get_json();
	Unit * get_unit_under_construction(UnitType * unit_type);
	number get_construction_progress(UnitType * unit_type, time_point t);
	friend std::ostream & operator << (std::ostream & os, Team & team);


	Elevation * _elevation;
	std::string _name;
	std::vector<Unit *> _units;
	glm::vec3 _color;
	bool _ia;
	GraphGrid * _fow;
	float * _fow_data; 
};

#endif
