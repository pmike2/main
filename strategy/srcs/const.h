#ifndef CONST_H
#define CONST_H

#include <string>
#include <iostream>

#include "typedefs.h"
#include "utile.h"


const glm::vec4 SELECTED_UNIT_COLOR(1.0f, 1.0f, 0.0f, 0.7f);
const glm::vec4 EDIT_MAP_COLOR(0.0f, 1.0f, 1.0f, 1.0f);
const glm::vec4 SELECT_COLOR(1.0f, 1.0f, 0.0f, 0.5f);

const number Z_OFFSET_EDGE = 0.05;
const number Z_OFFSET_UNIT = 0.05;
const number Z_OFFSET_PATH = 0.5;
const number Z_OFFSET_EDIT_MAP = 0.5;
const number Z_OFFSET_UNIT_PATH_BBOX = 0.5;
const number Z_OFFSET_SELECTION = 0.2;
const number Z_OFFSET_ATTACK_UNIT = 0.2;
const number Z_OFFSET_MOVE_UNIT = 0.2;
const number Z_FOW = 1.0;

const glm::vec3 LIGHT_POSITION(0.0f, 0.0f, 50.0f);
const glm::vec3 LIGHT_COLOR(1.0f);

const uint EDIT_MAP_N_VERTICES_PER_CIRCLE = 32;
const uint SELECTION_N_VERTICES_PER_CIRCLE = 8;
const uint ATTACK_UNIT_N_VERTICES_PER_CIRCLE = 8;

const number DEFAULT_ATTACK_UNIT_CIRCLE_RADIUS = 1.0;
const number MOVE_UNIT_SEGMENT_SIZE = 1.5;

const pt_2d MAP_ORIGIN(-100.0, -100.0);
const pt_2d MAP_SIZE(200.0, 200.0);
const pt_2d PATH_RESOLUTION(2.0);
const pt_2d ELEVATION_RESOLUTION(1.0);
const pt_2d FOW_RESOLUTION(2.0);

const number RIVER_ANGLE_SPEED = 0.1;

const number LAKE_WAVE_AMPLITUDE = 0.2;
const number LAKE_WAVE_FREQ = 2.0;
const number LAKE_ANGLE_SPEED = 0.1;

const pt_2d SEA_ORIGIN(-200.0, -200.0);
const pt_2d SEA_SIZE(400.0, 400.0);
const number SEA_WAVE_AMPLITUDE = 0.4;
const number SEA_WAVE_FREQ = 0.05;
const number SEA_LEVEL = 0.0;
const glm::vec4 SEA_COLOR(0.4, 0.8, 0.9, 0.4);
const number SEA_SHININESS = 10.0;
const number SEA_ANGLE_SPEED = 0.02;

const uint STONE_N_POINTS_HULL = 30;

const uint BRANCH_N_POINTS_PER_CIRCLE = 6;
const uint N_PTS_PER_BRANCH_SIDE= BRANCH_N_POINTS_PER_CIRCLE * 6;
const uint N_PTS_PER_BRANCH_BOTTOM = BRANCH_N_POINTS_PER_CIRCLE * 3;
const uint N_PTS_PER_BRANCH_TOP = BRANCH_N_POINTS_PER_CIRCLE * 3;

const uint N_ACTIVE_INTERVALS = 3;

const number EPS_UNIT_TYPE_BUFFER_SIZE = 0.1;

const number UNIT_DIST_PATH_EPS = 0.5;
const number MAX_UNIT_MOVING_WEIGHT = 100.0;

// pas 0.0 pour qu'il n'y ait pas de z-fight avec SEA_LEVEL = 0.0
const number DEFAULT_ELEVATION = -0.01;

const pt_2d DEFAULT_OVERVIEW_POSITION(4.0, 2.8);
const pt_2d DEFAULT_OVERVIEW_SIZE(2.0, 2.0);
const number Z_OVERVIEW = -10.0;
const number DEFAULT_OVERVIEW_ALPHA = 0.7;


enum PLAY_MODE {SELECT_UNIT, ADD_UNIT, ACTION_UNIT};
enum EDIT_MODE {ADD_ELEMENT, EDIT_ELEVATION, ERASE};
enum UNIT_ACTION_MODE {WAIT, WATCH, MOVE, ATTACK};

enum UNIT_STATUS {WAITING, WATCHING, MOVING, ATTACKING, SHOOTING, DESTROYED, COMPUTING_PATH};
std::string unit_status2str(UNIT_STATUS mode);
UNIT_STATUS str2unit_status(std::string s);

enum UNIT_PATH_STATUS {UNIT_PATH_COMPUTING_SUCCESS, UNIT_PATH_COMPUTING_FAILED};

enum UNIT_HIT_STATUS {NO_HIT, HIT_ASCEND, HIT_DESCEND, FINAL_HIT};

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

enum FOW_STATUS {UNDISCOVERED, DISCOVERED, WATCHED};

#endif
