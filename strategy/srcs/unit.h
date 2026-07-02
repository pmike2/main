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
#include "team.h"


using json = nlohmann::json;


struct Team;


struct Unit : public InstancePosRot, public GridMovingObject {
	Unit();
	Unit(Team * team, UnitType * type, pt_3d pos, Elevation * elevation, time_point t);
	~Unit();
	void reinit(pt_3d pos, time_point t);
	quat quat_slerp(number angle_goal);
	quat quat_slerp(pt_2d direction_goal);
	number angle();
	void anim(time_point t);
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
	number _life;
	number _hit;
	Unit * _target;
	Ammo * _hit_ammo;
	std::vector<uint> _old_visible_tiles, _visible_tiles;
};


#endif
