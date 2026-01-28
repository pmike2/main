#ifndef CONST_H
#define CONST_H

#include <string>
#include <iostream>

#include "typedefs.h"
#include "utile.h"


const number MAX_UNIT_MOVING_WEIGHT = 100.0;


//enum UNIT_STATUS {WAITING, MOVING, CHECKPOINT_CHECKED, LAST_CHECKPOINT_CHECKED};
enum UNIT_STATUS {WAITING, MOVING};
std::string unit_status2str(UNIT_STATUS mode);

enum UNIT_PATH_STATUS {UNIT_PATH_COMPUTING_SUCCESS, UNIT_PATH_COMPUTING_FAILED};

enum TERRAIN_TYPE {TERRAIN_GROUND, TERRAIN_OBSTACLE, TERRAIN_SEA, TERRAIN_LAKE, TERRAIN_RIVER, TERRAIN_SEA_COAST, TERRAIN_LAKE_COAST, TERRAIN_UNKNOWN};
TERRAIN_TYPE str2terrain_type(std::string s);
std::string terrain_type2str(TERRAIN_TYPE t);

enum UNIT_TYPE {INFANTERY, TANK, HELICOPTER, BOAT, UNIT_UNKNOWN};
UNIT_TYPE str2unit_type(std::string s);
std::string unit_type2str(UNIT_TYPE u);

enum ELEMENT_TYPE {ELEMENT_TREE, ELEMENT_STONE, ELEMENT_RIVER, ELEMENT_LAKE, ELEMENT_UNKNOWN};
ELEMENT_TYPE str2element_type(std::string s);
std::string element_type2str(ELEMENT_TYPE e);

enum ELEVATION_MODE {ELEVATION_ZERO, ELEVATION_PLUS, ELEVATION_MINUS, ELEVATION_UNKNOWN};
ELEVATION_MODE str2elevation_mode(std::string s);
std::string elevation_mode2str(ELEVATION_MODE e);

enum VISIBLE_GRID_TYPE {ELEVATION, TERRAIN, UNITS_POSITION};
std::string visible_grid2str(VISIBLE_GRID_TYPE v);

#endif
