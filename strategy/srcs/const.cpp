
#include "const.h"


std::string unit_status2str(UNIT_STATUS mode) {
	if (mode == WAITING) {
		return "WAITING";
	}
	else if (mode == WATCHING) {
		return "WATCHING";
	}
	else if (mode == MOVING) {
		return "MOVING";
	}
	else if (mode == ATTACKING) {
		return "ATTACKING";
	}
	else if (mode == SHOOTING) {
		return "SHOOTING";
	}
	else if (mode == DESTROYED) {
		return "DESTROYED";
	}
	std::cerr << mode << " : mode unit reconnu\n";
	return "UNKNOWN";
}


UNIT_STATUS str2unit_status(std::string s) {
	if (s == "WAITING") {
		return WAITING;
	}
	else if (s == "WATCHING") {
		return WATCHING;
	}
	else if (s == "MOVING") {
		return MOVING;
	}
	else if (s == "ATTACKING") {
		return ATTACKING;
	}
	else if (s == "SHOOTING") {
		return SHOOTING;
	}
	else if (s == "DESTROYED") {
		return DESTROYED;
	}
	std::cerr << s << " : status d'unité non reconnu\n";
	return WAITING;
}


TERRAIN_TYPE str2terrain_type(std::string s) {
	if (s == "GROUND") {
		return TERRAIN_GROUND;
	}
	else if (s == "OBSTACLE") {
		return TERRAIN_OBSTACLE;
	}
	else if (s == "SEA") {
		return TERRAIN_SEA;
	}
	else if (s == "LAKE") {
		return TERRAIN_LAKE;
	}
	else if (s == "RIVER") {
		return TERRAIN_RIVER;
	}
	else if (s == "SEA_COAST") {
		return TERRAIN_SEA_COAST;
	}
	else if (s == "LAKE_COAST") {
		return TERRAIN_LAKE_COAST;
	}
	std::cerr << s << " : type d'obstacle non reconnu\n";
	return TERRAIN_UNKNOWN;
}


std::string terrain_type2str(TERRAIN_TYPE t) {
	if (t == TERRAIN_GROUND) {
		return "GROUND";
	}
	else if (t == TERRAIN_OBSTACLE) {
		return "OBSTACLE";
	}
	else if (t == TERRAIN_SEA) {
		return "SEA";
	}
	else if (t == TERRAIN_LAKE) {
		return "LAKE";
	}
	else if (t == TERRAIN_RIVER) {
		return "RIVER";
	}
	else if (t == TERRAIN_SEA_COAST) {
		return "SEA_COAST";
	}
	else if (t == TERRAIN_LAKE_COAST) {
		return "LAKE_COAST";
	}
	std::cerr << t << " : type d'obstacle non reconnu\n";
	return "UNKNOWN";
}


UNIT_TYPE str2unit_type(std::string s) {
	if (s == "INFANTERY") {
		return INFANTERY;
	}
	else if (s == "TANK") {
		return TANK;
	}
	else if (s == "HELICOPTER") {
		return HELICOPTER;
	}
	else if (s == "BOAT") {
		return BOAT;
	}
	std::cerr << s << " : type d'unité non reconnu\n";
	return UNIT_UNKNOWN;
}


std::string unit_type2str(UNIT_TYPE u) {
	if (u == INFANTERY) {
		return "INFANTERY";
	}
	else if (u == TANK) {
		return "TANK";
	}
	else if (u == BOAT) {
		return "BOAT";
	}
	else if (u == HELICOPTER) {
		return "HELICOPTER";
	}
	std::cerr << u << " : type d'unité non reconnu\n";
	return "UNKNOWN";
}


ELEMENT_TYPE str2element_type(std::string s) {
	if (s == "TREE") {
		return ELEMENT_TREE;
	}
	else if (s == "STONE") {
		return ELEMENT_STONE;
	}
	else if (s == "LAKE") {
		return ELEMENT_LAKE;
	}
	else if (s == "RIVER") {
		return ELEMENT_RIVER;
	}
	std::cerr << s << " : type d'élément non reconnu\n";
	return ELEMENT_UNKNOWN;
}


std::string element_type2str(ELEMENT_TYPE e) {
	if (e == ELEMENT_TREE) {
		return "TREE";
	}
	else if (e == ELEMENT_STONE) {
		return "STONE";
	}
	else if (e == ELEMENT_LAKE) {
		return "LAKE";
	}
	else if (e == ELEMENT_RIVER) {
		return "RIVER";
	}
	std::cerr << e << " : type d'élément non reconnu\n";
	return "UNKNOWN";
}


ELEVATION_MODE str2elevation_mode(std::string s) {
	if (s == "ELEVATION_ZERO") {
		return ELEVATION_ZERO;
	}
	else if (s == "ELEVATION_PLUS") {
		return ELEVATION_PLUS;
	}
	else if (s == "ELEVATION_MINUS") {
		return ELEVATION_MINUS;
	}
	std::cerr << s << " : type d'élévation non reconnu\n";
	return ELEVATION_UNKNOWN;
}


std::string elevation_mode2str(ELEVATION_MODE e) {
	if (e == ELEVATION_ZERO) {
		return "ELEVATION_ZERO";
	}
	else if (e == ELEVATION_PLUS) {
		return "ELEVATION_PLUS";
	}
	else if (e == ELEVATION_MINUS) {
		return "ELEVATION_MINUS";
	}
	std::cerr << e << " : mode d'élévation non reconnu\n";
	return "UNKNOWN";
}


std::string visible_grid2str(VISIBLE_GRID_TYPE v) {
	if (v == ELEVATION) {
		return "ELEVATION";
	}
	else if (v == TERRAIN) {
		return "TERRAIN";
	}
	else if (v == UNITS_POSITION) {
		return "UNITS_POSITION";
	}
	std::cerr << v << " : visible grid non reconnu\n";
	return "UNKNOWN";
}
