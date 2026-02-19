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

#include "unit_type.h"
#include "unit_path.h"
#include "elevation.h"
#include "ammo.h"


using json = nlohmann::json;


struct Instruction {
	pt_3d _destination;
	time_point _t;
};


struct Team;


struct Unit : public InstancePosRot {
	Unit();
	Unit(Team * team, UnitType * type, pt_3d pos, Elevation * elevation, time_point t);
	~Unit();
	//quat orientation(pt_2d v);
	void anim(time_point t);
	bool checkpoint_checked();
	bool last_checkpoint_checked();
	void set_status(UNIT_STATUS status, time_point t);
	void set_hit_status(UNIT_HIT_STATUS hit_status, time_point t);
	void hit(Ammo * ammo, time_point t);
	void update_alti_path();
	json get_json();
	friend std::ostream & operator << (std::ostream & os, Unit & unit);
	
	
	uint _id;
	Team * _team;
	UnitType * _type;
	UNIT_STATUS _status;
	UNIT_HIT_STATUS _hit_status;
	UnitPath * _path;
	pt_3d _velocity;
	std::queue<Instruction> _instructions;
	bool _paused;
	Elevation * _elevation;
	time_point _last_moving_t;
	time_point _last_shooting_t;
	time_point _creation_t;
	bool _delete;
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
	Unit * add_unit(UnitType * type, uint id, pt_2d pos, time_point t);
	std::vector<Unit *> get_units_in_aabb(AABB_2D * aabb);
	std::vector<Unit *> get_selected_units();
	void remove_unit(Unit * unit);
	//void remove_units_in_aabb(AABB_2D * aabb);
	void clear2delete();
	void clear(bool reinit_fow);
	void clear_selection();
	void unit_goto(Unit * unit, pt_3d pt, time_point t);
	void selected_units_goto(pt_3d pt, time_point t);
	bool is_target_reachable(Unit * unit, Unit * target);
	void unit_attack(Unit * unit, Unit * target, time_point t);
	void selected_units_attack(Unit * target, time_point t);
	Unit * search_target(Unit * unit, Team * ennemy_team);
	void update_fow_unit(Unit * unit);
	void update_fow();
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
