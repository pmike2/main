
#ifndef CONSTANTES_H
#define CONSTANTES_H


// en ms; temps entre 2 anims
//const unsigned int DELTA_ANIM= 1;

// dimensions écran
const int MAIN_WIN_WIDTH= 1024;
const int MAIN_WIN_HEIGHT= 1024;
const float MAIN_BCK[]= {0.2f, 0.2f, 0.2f, 1.0f};

// dimensions OpenGL
const float GL_WIDTH= 20.0f;
//const float GL_HEIGHT= 20.0f;
const float GL_HEIGHT= GL_WIDTH* (float)(MAIN_WIN_HEIGHT)/ (float)(MAIN_WIN_WIDTH);



#endif
