
#ifndef CONSTANTES_H
#define CONSTANTES_H

#include <string>


// en ms; temps entre 2 anims
const unsigned int DELTA_ANIM= 1;

// dimensions écran
const int MAIN_WIN_WIDTH= 1280;
const int MAIN_WIN_HEIGHT= 1024;
const float MAIN_BCK[]= {0.2f, 0.2f, 0.2f, 1.0f};

// dimensions OpenGL
const float GL_WIDTH= 20.0f;
const float GL_HEIGHT= 20.0f;

// params moteur physique
const float DT_PHYSICS= 1.0f/ 60.0f; // en fraction de seconde
const unsigned int DT_PHYSICS_MS= (unsigned int)(DT_PHYSICS* 1000.0f); // en millisecondes



#endif
