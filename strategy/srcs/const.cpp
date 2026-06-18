
#include "const.h"


std::string unit_status2str(UNIT_STATUS mode) {
	if (mode == WATCHING) {
		return "WATCHING";
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
	else if (mode == UNDER_CONSTRUCTION) {
		return "UNDER_CONSTRUCTION";
	}
	else if (mode == TAKEOFF) {
		return "TAKEOFF";
	}
	else if (mode == LANDING) {
		return "LANDING";
	}
	std::cerr << mode << " : mode unit reconnu\n";
	return "UNKNOWN";
}


UNIT_STATUS str2unit_status(std::string s) {
	if (s == "WATCHING") {
		return WATCHING;
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
	else if (s == "UNDER_CONSTRUCTION") {
		return UNDER_CONSTRUCTION;
	}
	else if (s == "TAKEOFF") {
		return TAKEOFF;
	}
	else if (s == "LANDING") {
		return LANDING;
	}
	std::cerr << s << " : status d'unité non reconnu\n";
	return WATCHING;
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
