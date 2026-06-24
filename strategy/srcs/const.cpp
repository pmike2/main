
#include "const.h"


std::string unit_status2str(UNIT_STATUS mode) {
	if (mode == UNIT_WATCHING) {
		return "UNIT_WATCHING";
	}
	else if (mode == UNIT_ATTACKING) {
		return "UNIT_ATTACKING";
	}
	else if (mode == UNIT_SHOOTING) {
		return "UNIT_SHOOTING";
	}
	else if (mode == UNIT_DESTROYED) {
		return "UNIT_DESTROYED";
	}
	else if (mode == UNIT_UNDER_CONSTRUCTION) {
		return "UNIT_UNDER_CONSTRUCTION";
	}
	else if (mode == UNIT_TAKEOFF) {
		return "UNIT_TAKEOFF";
	}
	else if (mode == UNIT_LANDING) {
		return "UNIT_LANDING";
	}
	else if (mode == UNIT_INACTIVE) {
		return "UNIT_INACTIVE";
	}
	std::cerr << mode << " : mode unit reconnu\n";
	return "UNKNOWN";
}


UNIT_STATUS str2unit_status(std::string s) {
	if (s == "UNIT_WATCHING") {
		return UNIT_WATCHING;
	}
	else if (s == "UNIT_ATTACKING") {
		return UNIT_ATTACKING;
	}
	else if (s == "UNIT_SHOOTING") {
		return UNIT_SHOOTING;
	}
	else if (s == "UNIT_DESTROYED") {
		return UNIT_DESTROYED;
	}
	else if (s == "UNIT_UNDER_CONSTRUCTION") {
		return UNIT_UNDER_CONSTRUCTION;
	}
	else if (s == "UNIT_TAKEOFF") {
		return UNIT_TAKEOFF;
	}
	else if (s == "UNIT_LANDING") {
		return UNIT_LANDING;
	}
	else if (s == "UNIT_INACTIVE") {
		return UNIT_INACTIVE;
	}
	std::cerr << s << " : status d'unité non reconnu\n";
	return UNIT_WATCHING;
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
