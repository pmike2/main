
#ifndef CONSTANTES_H
#define CONSTANTES_H

#include <string>
#include <cmath>
#include <map>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


// en ms; temps entre 2 anims
const unsigned int DELTA_ANIM= 1;

// etat souris
enum MouseDownState {MOUSE_DOWN_NULL, MOUSE_DOWN_SPECTRUM, MOUSE_DOWN_WAVE, MOUSE_DOWN_ART};

// dimensions écran
const int MAIN_WIN_WIDTH= 1280;
const int MAIN_WIN_HEIGHT= 1024;
const float MAIN_BCK[]= {0.2f, 0.2f, 0.2f, 1.0f};

// dimensions OpenGL
const float GL_WIDTH= 20.0f;
//const float GL_HEIGHT= 20.0f;
const float GL_HEIGHT= GL_WIDTH* (float)(MAIN_WIN_HEIGHT)/ (float)(MAIN_WIN_WIDTH);


// fenetre spectrum
const int SPECTRUM_WIN_X= 5;
const int SPECTRUM_WIN_Y= 320;
const int SPECTRUM_WIN_WIDTH = 620;
const int SPECTRUM_WIN_HEIGHT= 620;
const float SPECTRUM_BCK[]= {0.5f, 0.5f, 0.5f, 1.0f};
const float SPECTRUM_Z_FACTOR= 0.3f;
const float SPECTRUM_UNTRIGGERED_COLOR[]= {0.0f, 1.0f, 0.0f};
const float SPECTRUM_TRIGGERED_COLOR[]= {1.0f, 0.0f, 0.0f};
const float SPECTRUM_SELECTED_STRENGTH= 1.0f;
const float SPECTRUM_UNSELECTED_STRENGTH= 0.5f;
const float SPECTRUM_WIDTH_STEP_INIT= 15.0f;
const float SPECTRUM_HEIGHT_STEP_INIT= 3.0f;
const float SPECTRUM_HEIGHT_STEP_MIN= 1.0f;
const float SPECTRUM_HEIGHT_STEP_MAX= 100.0f;
const float SPECTRUM_SHININESS= 10.0f;
const glm::vec3 SPECTRUM_AMBIENT_COLOR(0.7f, 0.7f, 0.7f);

// fenetre wave
const int WAVE_WIN_X= 5;
const int WAVE_WIN_Y= 5;
const int WAVE_WIN_WIDTH = 1270;
const int WAVE_WIN_HEIGHT= 300;
const float WAVE_BCK[]= {0.1f, 0.1f, 0.1f, 1.0f};
const float WAVE_WIDTH= 200.0f;
const float WAVE_HEIGHT= 200.0f;
const float WAVE_SELECTED_COLOR[]= {1.0f, 0.0f, 0.0f};
const float WAVE_UNSELECTED_COLOR[]= {0.0f, 1.0f, 1.0f};
const float WAVE_SELECTED_STRENGTH= 1.0f;
const float WAVE_UNSELECTED_STRENGTH= 0.5f;
const float WAVE_MULT_FACTOR= 30.0f;
const unsigned int WAVE_N_VERTICES= 100000;
const long WAVE_SAMPLE_WIDTH_INIT= 10000;
const long WAVE_SAMPLE_WIDTH_MAX= 200000;
const long WAVE_SAMPLE_WIDTH_MIN= 1000;

// fenetre simu
const int SIMU_WIN_X= 1000;
const int SIMU_WIN_Y= 980;
const int SIMU_WIN_WIDTH = 256;
const int SIMU_WIN_HEIGHT= 40;
const float SIMU_BCK[]= {0.5f, 0.5f, 0.5f, 1.0f};
const float SIMU_WIDTH= 200.0f;
const float SIMU_HEIGHT= 200.0f;
const float SIMU_WIDTH_MARGIN= 0.5f;
const float SIMU_HEIGHT_MARGIN= 5.0f;

// fenetre art
const int ART_WIN_X= 630;
const int ART_WIN_Y= 320;
const int ART_WIN_WIDTH = 620;
const int ART_WIN_HEIGHT= 620;
const float ART_BCK[]= {0.5f, 0.5f, 0.5f, 1.0f};


// audio -----------------------------------------------------------------------------------------
// nombre de samples dans 1s d'audio
const int SAMPLE_RATE= 44100;

// nombre de samples a traiter a chaque appel du callback portaudio
// 1024 rend l'affichage un peu cahotant en mode AUDIO_RECORD ; a retester
const int SAMPLES_PER_BUFFER= 512;

// cf http://www.fftw.org/fftw3_doc/One_002dDimensional-DFTs-of-Real-Data.html
const int FFT_INPUT_SIZE= SAMPLES_PER_BUFFER;
const int FFT_OUTPUT_SIZE= SAMPLES_PER_BUFFER/ 2+ 1;

// nombre de blocks regroupant de facon non uniforme les fréquences parmi SAMPLES_PER_BUFFER
const int N_BLOCKS_FFT= 32;

// nombrez de fréquences dans le 1er bloc des N_BLOCKS_FFT
const int N_COEFFS_IN_FIRST_BLOCK= 1;

// temps en ms de l'historique d'énergie
const int HISTORY_TIME_MS= 150;
// nombre de valeurs énergie a stocker dans l'historique
const int HISTORY_SIZE= (int)(HISTORY_TIME_MS* SAMPLE_RATE/ (SAMPLES_PER_BUFFER* 1000));

// facteur multiplicatif pour test déclenchement événement ; a adapter a chaque bloc ?
const float CMP_RATIO= 3.0f;

// seuil variance pour test déclenchement événement ; a adapter a chaque bloc ?
const float VARIANCE_TRESH= 100.0f;

// temps d'enregistrement en secondes
const float RECORD_TIME= 5.0f;
// nombre de samples d'enregistrement ; on tronque à un multiple de SAMPLES_PER_BUFFER
const long N_SAMPLES_RECORD= ((SAMPLE_RATE* (unsigned int)(RECORD_TIME))/ SAMPLES_PER_BUFFER)* SAMPLES_PER_BUFFER;

// mode audio
enum AudioMode {AUDIO_STOP, AUDIO_PLAYBACK, AUDIO_RECORD};

// amplitude playback initiale
const float INIT_PLAYBACK_AMPLITUDE= 0.5f;

// perte en alpha de simu avec le temps
const float VISU_SIMU_DECREASING_AMOUNT= 0.05f;

// temps en secondes qui caractérise un son triggé
const float SIGNATURE_TIME= 0.2f;

// temps en secondes min entre 2 trigs
const float TRIG_MIN_DIST= 0.1f;

// temps en secondes de durée d'un son triggé ; TRIG_MIN_LENGTH doit etre < TRIG_MAX_LENGTH
const float TRIG_MIN_LENGTH= 0.5f;
const float TRIG_MAX_LENGTH= 2.0f;

// seuil d'énergie en dessous duquel on considère qu'un son triggé est terminé : règle le release des enveloppes
const float MEAN_ENERGY_TRESHOLD= 40.0f;

struct RandomFloatMinMax {
	float _min;
	float _max;
};

struct RandomIntMinMax {
	int _min;
	int _max;
};

// valeurs min-max des randoms ; en secondes pour attack, release ; release n'aura d'impact qu'en mode AUDIO_RECORD (static_release)
const std::map<std::string, RandomFloatMinMax> MINMAXS_ENV {
	{"scale_base", RandomFloatMinMax{0.5f, 2.0f}},        {"scale_diff", RandomFloatMinMax{-0.5f, 0.5f}}, 
	{"translate_base", RandomFloatMinMax{-50.0f, 50.0f}},   {"translate_diff", RandomFloatMinMax{-10.0f, 10.0f}}, 
	{"rotate_base", RandomFloatMinMax{0.0f, M_PI* 2.0f}}, {"rotate_diff", RandomFloatMinMax{-M_PI* 2.0f* 0.02f, M_PI* 2.0f* 0.02f}},
	{"alpha_base", RandomFloatMinMax{0.1f, 1.0f}},        {"alpha_diff", RandomFloatMinMax{-0.5f, 0.5f}},
	{"shininess_base", RandomFloatMinMax{0.0f, 1.0f}},    {"shininess_diff", RandomFloatMinMax{-0.5f, 0.5f}},
	{"ambient_base", RandomFloatMinMax{0.0f, 1.0f}},      {"ambient_diff", RandomFloatMinMax{-0.5f, 0.5f}},
	{"attack", RandomFloatMinMax{0.0f, 0.1f}},            {"release", RandomFloatMinMax{0.0f, 2.0f}}, 
	{"attack_power", RandomFloatMinMax{0.5f, 2.0f}},      {"release_power", RandomFloatMinMax{0.5f, 2.0f}}
};

// min-max randoms nombre d'objets, morphs et connexions
const struct RandomIntMinMax MINMAX_N_OBJS= {8, 16};
const struct RandomIntMinMax MINMAX_N_MORPHS= {4, 6};
const struct RandomIntMinMax MINMAX_N_CONNS= {3, 8};
// proba qu'un morph affecte un obj
const unsigned int CONN_CHANCE= 5;

#endif
