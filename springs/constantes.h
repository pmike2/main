
#ifndef CONSTANTES_H
#define CONSTANTES_H

#include <string>
#include <cmath>


const int SCREEN_WIDTH= 800;
const int SCREEN_HEIGHT= 700;

const char * const CH_LOG= "sdl.log";

const unsigned int DELTA_ANIM= 1;

const unsigned int MAX_VERTICES= 500;
const unsigned int MAX_SPRINGS= 5000;
const unsigned int MAX_DATA= 12* MAX_SPRINGS;
const unsigned int MAX_FORCES_PER_VERTEX= 32;
const unsigned int MAX_DATA_FORCES= 12* MAX_VERTICES* MAX_FORCES_PER_VERTEX;
const unsigned int MAX_DATA_ACCEL_SPEED= 24* MAX_VERTICES;

const float GRAVITY= 20.0f;
const float FRICTION= 7.0f;
const float GROUND_TRESHOLD= 1e-5;
const float SPRING_DAMP_TRESHOLD= 0.001f;
const float SPRING_CONTRACT_TRESHOLD= 0.05f;
const float SPRING_CONTRACT_MIN_AMOUNT= 20.0f;
const float SPRING_CONTRACT_MAX_AMOUNT= 70.0f;
const unsigned int SPRING_CONTRACT_CYCLE_TIME= 2000;

const float ANIM_STEP= 0.008f;

const unsigned int N_CUBES= 12;
const unsigned int N_ANIM_PER_GEN= 10000;
const unsigned int N_POPULATION= 40;
const unsigned int N_GENERATIONS= 100;

const float VERTEX_MASS= 0.07f;
const float VERTEX_AIR_RESIST= 0.05f;
const float SPRING_SIZE= 0.5f;
const float SPRING_STIFFNESS= 170.0f;
const float SPRING_DAMPING= 0.1f;

const float SAME_VERTICES_TRESHOLD= 1e-3;


#endif
