
#include "const.h"


std::string unit_status2str(UNIT_STATUS mode) {
	if (mode == WAITING) {
		return "WAITING";
	}
	else if (mode == MOVING) {
		return "MOVING";
	}
	else if (mode == COMPUTING_PATH) {
		return "COMPUTING_PATH";
	}
	else if (mode == COMPUTING_PATH_DONE) {
		return "COMPUTING_PATH_DONE";
	}
	else if (mode == COMPUTING_PATH_FAILED) {
		return "COMPUTING_PATH_FAILED";
	}

	std::cerr << mode << " : mode unit reconnu\n";
	return "UNKNOWN";
}


TERRAIN_TYPE str2terrain_type(std::string s) {
	if (s == "GROUND") {
		return GROUND;
	}
	else if (s == "OBSTACLE") {
		return OBSTACLE;
	}
	else if (s == "SEA") {
		return SEA;
	}
	else if (s == "LAKE") {
		return LAKE;
	}
	else if (s == "RIVER") {
		return RIVER;
	}
	else if (s == "SEA_COAST") {
		return SEA_COAST;
	}
	else if (s == "LAKE_COAST") {
		return LAKE_COAST;
	}
	std::cerr << s << " : type d'obstacle non reconnu\n";
	return UNKNOWN;
}


std::string terrain_type2str(TERRAIN_TYPE t) {
	if (t == GROUND) {
		return "GROUND";
	}
	else if (t == OBSTACLE) {
		return "OBSTACLE";
	}
	else if (t == SEA) {
		return "SEA";
	}
	else if (t == LAKE) {
		return "LAKE";
	}
	else if (t == RIVER) {
		return "RIVER";
	}
	else if (t == SEA_COAST) {
		return "SEA_COAST";
	}
	else if (t == LAKE_COAST) {
		return "LAKE_COAST";
	}
	std::cerr << t << " : type d'obstacle non reconnu\n";
	return "UNKNOWN";
}

