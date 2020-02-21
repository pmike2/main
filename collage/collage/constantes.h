
#ifndef CONSTANTES_H
#define CONSTANTES_H

#include <string>


// en ms; temps entre 2 anims
const unsigned int DELTA_ANIM= 1;

// dimensions écran
const int MAIN_WIN_WIDTH= 1280;
const int MAIN_WIN_HEIGHT= 1024;
const float MAIN_BCK[]= {0.7f, 0.7f, 0.7f, 1.0f};

// dimensions OpenGL
const float GL_WIDTH= 20.0f;
const float GL_HEIGHT= 20.0f;

// nombre de samples dans 1s d'audio
const int SAMPLE_RATE_PLAYBACK= 44100;

// nombre de samples a traiter a chaque appel du callback portaudio
const int SAMPLES_PER_BUFFER_PLAYBACK= 1024;


#endif
