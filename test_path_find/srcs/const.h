#ifndef CONST_H
#define CONST_H

#include <string>

#include "typedefs.h"
#include "utile.h"


const int MAIN_WIN_WIDTH= 1280;
const int MAIN_WIN_HEIGHT= 1024;
const float GL_WIDTH= 10.0f;
const float GL_HEIGHT= GL_WIDTH* (float)(MAIN_WIN_HEIGHT)/ (float)(MAIN_WIN_WIDTH);

const pt_2d GRID_ORIGIN(-50.0, -50.0);
const pt_2d GRID_SIZE(100.0, 100.0);
const pt_2d GRID_RESOLUTION(1.0);

const number GRID_CENTER_SIZE_RATIO = 0.1;

const float Z_EDGE = 0.0;
const float Z_CENTER = 0.01;
const float Z_GMO = 0.02;
const float Z_GMO_PATH = 0.03;
const float Z_FONT = 0.04;

const glm::vec4 SELECT_COLOR(1.0f, 1.0f, 0.0f, 0.5f);

#endif
