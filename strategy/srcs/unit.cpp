#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "utile.h"

#include "unit.h"


Unit::Unit() {

}


Unit::Unit(Team * team, UnitType * type, pt_3d pos, Elevation * elevation, time_point t) : 
	InstancePosRot(pos, quat(1.0, 0.0, 0.0, 0.0), pt_3d(1.0), type->_obj_data->_aabb),
	GridMovingObject(
		type, 
		pt_2d(pos),
		type->get_max_square_size()
	),
	_team(team), _type(type), _unit_status(UNIT_UNDER_CONSTRUCTION), _paused(false), _elevation(elevation),
	_life(type->_life_init), _hit_status(NO_HIT), _hit(0.0), _target(NULL), _hit_ammo(NULL),
	_creation_t(t)
{

}


Unit::~Unit() {
	
}


void Unit::reinit(pt_3d pos, time_point t) {
	set_pos(pos);
	_aabb->set_center(pt_2d(pos));
	_unit_status = UNIT_UNDER_CONSTRUCTION;
	_life = _type->_life_init; 
	_hit_status = NO_HIT;
	_hit = 0.0;
	_target = NULL;
	_hit_ammo = NULL;
	_creation_t = t;
	_old_visible_tiles.clear();
	_visible_tiles.clear();
}


quat Unit::quat_slerp(number angle_goal) {
	quat next_quat = glm::angleAxis(angle_goal, pt_3d(0.0, 0.0, 1.0));

	// nécessaire sinon on a des retournements brusques
	// cf https://gabormakesgames.com/blog_quats_interpolate.html
	if (glm::dot(_rotation, next_quat) < 0.0) {
		next_quat = -1.0 * next_quat;
	}

	// https://en.wikipedia.org/wiki/Slerp
	// glm::mix fait le slerp
	//return _rotation * glm::pow(glm::inverse(_rotation) * next_quat, slerp_speed);
	return glm::mix(_rotation, next_quat, _type->_slerp_speed);
}


quat Unit::quat_slerp(pt_2d direction_goal) {
	number angle_goal = atan2(direction_goal.y, direction_goal.x);
	return quat_slerp(angle_goal);
}


number Unit::angle() {
	pt_3d euler = glm::eulerAngles(_rotation);
	return euler.z;
}


void Unit::anim(time_point t) {
	//auto d_moving = std::chrono::duration_cast<std::chrono::milliseconds>(t - _last_moving_t).count();
	//_last_moving_t = t;

	if (_paused) {
		return;
	}

	if (_unit_status == UNIT_INACTIVE) {
		return;
	}

	if (_hit_status == HIT_ASCEND) {
		_hit += 0.4;
		if (_hit > 0.2 * _hit_ammo->_damage) {
			_hit_status = HIT_DESCEND;

		}
	}
	else if (_hit_status == HIT_DESCEND) {
		_hit -= 0.3;
		if (_hit < 0.0) {
			_hit_status = NO_HIT;
			_hit = 0.0;
			_hit_ammo = NULL;
		}
	}
	else if (_hit_status == FINAL_HIT) {
		_hit += 0.5;
		if (_hit > 10.0) {
			_unit_status = UNIT_DESTROYED;
		}
		return;
	}
	
	if (_gmo_status == GMO_IDLE) {
		if (_type->_flies) {
			number elevation_z = _elevation->get_alti(_position);
			if (_position.z > elevation_z + LANDING_SPEED) {
				set_pos(_position - pt_3d(0.0, 0.0, LANDING_SPEED));
			}
		}

		if (_unit_status == UNIT_UNDER_CONSTRUCTION) {
			auto d_creation = std::chrono::duration_cast<std::chrono::milliseconds>(t - _creation_t).count();
			if (d_creation > _type->_creation_duration) {
				_gmo_status = GMO_IDLE;
				_unit_status = UNIT_WATCHING;
			}

		}
		/*else if (_unit_status == WAITING) {
			_life += _type->_regen;
			if (_life > _type->_life_init) {
				_life = _type->_life_init;
			}
		}*/
		else if (_unit_status == UNIT_WATCHING) {
			number current_angle = angle();
			number next_angle = current_angle + 0.1;
			quat next_quat = quat_slerp(next_angle);
			set_pos_rot(_position, next_quat);
		}
		else if (_unit_status == UNIT_ATTACKING) {
			if (_target == NULL)  {
				std::cerr << "Unit " << _id << " ATTACKING mais _target == NULL.\n";
				return;
			}

			quat next_quat = quat_slerp(_target->_position - _position);
			set_pos_rot(_position, next_quat);

			number current_angle = angle();
			number aligned = abs(cross2d(pt_2d(_target->_position - _position) / norm(_target->_position - _position), pt_2d(cos(current_angle), sin(current_angle))));
			auto d_shooting = std::chrono::duration_cast<std::chrono::milliseconds>(t - _last_shooting_t).count();
			if (aligned < 0.01 && number(d_shooting) > _type->_ammo_type->_rate * 1000.0) {
				_last_shooting_t = t;
				_unit_status = UNIT_SHOOTING;
			}
		}
	}
	else if (_gmo_status == GMO_MOVING) {
		set_speed(_type->_max_velocity * (1.0 - _path_cost[_idx_path] / PATH_FIND_OBSTACLE_THRESH));

		pt_3d next_position;
		next_position.x = _aabb->center().x;
		next_position.y = _aabb->center().y;
		next_position.z = _position.z;

		number elevation_z = _elevation->get_alti(next_position);
		
		if (_type->_flies) {
			if (_idx_path < _path.size() / 2) {
				next_position.z += TAKEOFF_SPEED;
				if (next_position.z > FLY_ALTI) {
					next_position.z = FLY_ALTI;
				}
			}
			else {
				next_position.z -= LANDING_SPEED;
				if (next_position.z < elevation_z) {
					next_position.z = elevation_z;
				}
			}
		}
		else {
			next_position.z = elevation_z;
		}
		
		if (_type->_floats && next_position.z < 0.0) {
			next_position.z = 0.0;
		}

		quat next_quat = quat_slerp(_direction);
		set_pos_rot(next_position, next_quat);
	}
}


/*void Unit::set_hit_status(UNIT_HIT_STATUS hit_status, time_point t) {
	_hit_status = hit_status;
	if (_hit_status == NO_HIT) {
		_hit = 0.0;
		_hit_ammo = NULL;
	}
	else if (_hit_status == HIT_ASCEND) {
	}
	else if (_hit_status == HIT_DESCEND) {
	}
	else if (_hit_status == FINAL_HIT) {
	}
}*/


void Unit::hit(Ammo * ammo, time_point t) {
	_hit_ammo = ammo;
	_life -= ammo->_damage;
	if (_life <= 0.0) {
		_life = 0.0;
		_hit_status = FINAL_HIT;
	}
	else {
		_hit_status = HIT_ASCEND;
	}
}


json Unit::get_json() {
	json result;
	result["type"] = _type->_name;
	result["id"] = _id;
	result["position"] = json::array();
	result["position"].push_back(_position.x);
	result["position"].push_back(_position.y);
	result["status"] = unit_status2str(_unit_status);
	result["life"] = _life;
	return result;
}


std::ostream & operator << (std::ostream & os, Unit & unit) {
	os << "id = " << unit._id;
	os << " ; type = " << unit._type->_name;
	os << " ; status = " << unit_status2str(unit._unit_status);
	os << " ; position = " << glm_to_string(unit._position);
	return os;
}


