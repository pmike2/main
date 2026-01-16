#ifndef CONST_H
#define CONST_H

#include <string>
#include <iostream>

#include "typedefs.h"


const number MAX_UNIT_MOVING_WEIGHT = 100.0;
const number UNIT_DIST_PATH_EPS = 0.5;
const number EPS_UNIT_TYPE_BUFFER_SIZE = 0.1;


enum UNIT_STATUS {WAITING, MOVING, COMPUTING_PATH, COMPUTING_PATH_DONE, COMPUTING_PATH_FAILED, CHECKPOINT_CHECKED, LAST_CHECKPOINT_CHECKED};
std::string unit_status2str(UNIT_STATUS mode);

enum TERRAIN_TYPE {GROUND, OBSTACLE, SEA, LAKE, RIVER, SEA_COAST, LAKE_COAST, UNKNOWN};
TERRAIN_TYPE str2terrain_type(std::string s);
std::string terrain_type2str(TERRAIN_TYPE t);

#endif
