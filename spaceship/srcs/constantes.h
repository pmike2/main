
#ifndef CONSTANTES_H
#define CONSTANTES_H

#include <string>
#include <cmath>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


// général -------------------------------------------------------------------------------
// en ms; temps entre 2 anims
const uint DELTA_ANIM= 1;

// dimensions écran
const int SCREEN_WIDTH= 1024;
const int SCREEN_HEIGHT= 1024;

// visu des forces -----------------------------------------------------------------------
// nombre max de forces à dessiner
const int NMAX_FORCE_DRAW= 128;

// facteur multiplicatif pour mieux visualiser les forces
const float FORCE_DRAW_MULT_FACTOR= 30.0f;

// ship ----------------------------------------------------------------------------------
// distance de suivi caméra <-> ship
const float FOLLOW_CAMERA_DISTANCE= 10.0f;

// entre 0.0 (caméra immobile) et 1.0 (caméra suit parfaitement ship)
const float FOLLOW_LINEAR_FACTOR= 0.07f;
const float FOLLOW_ANGULAR_FACTOR= 0.09f;

// incrément lors de l'anim de ship
const float ANIM_STEP= 0.05f;

// freins linéaire et angulaire
const float LINEAR_DRAG_COEFF= 1.0f;
const float ANGULAR_DRAG_COEFF= 1.0f;
// seuils à partir duquel les drags sont appliqués
const float LINEAR_DRAG_TRESH= 10.0f;
const float ANGULAR_DRAG_TRESH= 0.003f;

// scaling initial
const float SHIP_SIZE_FACTOR= 0.4f;

// vitesse nominale
const float NOMINAL_SPEED= 40.0f;

// densité uniforme du ship
const float SHIP_DENSITY= 1.0f;

// mot-clefs des différentes forces applicables au ship
const std::string LEFT_ENGINE_UP= "LEFT_ENGINE_UP";
const std::string LEFT_ENGINE_DOWN= "LEFT_ENGINE_DOWN";
const std::string LEFT_ENGINE_FWD= "LEFT_ENGINE_FWD";
const std::string LEFT_ENGINE_BWD= "LEFT_ENGINE_BWD";
const std::string RIGHT_ENGINE_UP= "RIGHT_ENGINE_UP";
const std::string RIGHT_ENGINE_DOWN= "RIGHT_ENGINE_DOWN";
const std::string RIGHT_ENGINE_FWD= "RIGHT_ENGINE_FWD";
const std::string RIGHT_ENGINE_BWD= "RIGHT_ENGINE_BWD";
const std::string BACK_UP= "BACK_UP";
const std::string BACK_DOWN= "BACK_DOWN";

// mots-clefs des touches
const std::string KEY_UP= "KEY_UP";
const std::string KEY_DOWN= "KEY_DOWN";
const std::string KEY_LEFT= "KEY_LEFT";
const std::string KEY_RIGHT= "KEY_RIGHT";
const std::string KEY_FWD= "KEY_FWD";
const std::string KEY_BWD= "KEY_BWD";

// intensités des forces appliquées ; à ajuster en fonction de SHIP_SIZE_FACTOR
const float LEFT_RIGHT_INTENSITY= 0.007f;
const float UP_DOWN_INTENSITY= 0.014f;
const float FWD_BWD_INTENSITY= 20.0f;

// distance ship - light qui le suit
const float DIST_SHIP_LIGHT= 200.0f;

// fog -----------------------------------------------------------------------------------
// brouillard ; démarre à distance FOG_START ; à FOG_END on ne voit que du fog
const float FOG_START= 400.0f;
const float FOG_END= 1000.0f;
const float FOG_COLOR[3]= {0.3f, 0.6f, 0.9f};

// bullet --------------------------------------------------------------------------------
// vitesse balles
const float BULLET_SPEED= 20.0f;
// temps (en ms) entre 2 tirs
const uint BULLET_FREQUENCY= 120;
// scaling initial
const float BULLET_SIZE_FACTOR= 3.5f;
// nombre max de bullets actives pour un ship
const uint MAX_BULLETS= 100;

// explosion -----------------------------------------------------------------------------
struct ExplosionParams {
	float _size_factor;
	uint _n_particles;
	float _translation;
	float _angle;
	float _scale;
};

const struct ExplosionParams LITTLE_EXPLOSION_PARAMS= {5.0f, 100, 1.2f, 0.03f, 0.03f};
const struct ExplosionParams BIG_EXPLOSION_PARAMS= {12.0f, 100, 1.2f, 0.1f, 0.004f};
const uint N_MAX_LITTLE_EXPLOSIONS= 2000;
const uint N_MAX_BIG_EXPLOSIONS= 100;

// IA ------------------------------------------------------------------------------------
const uint N_ENEMIES= 9;

// considère que sa direction courante est ok si courant - target est < cette valeur
const float IA_ORIENTATION_TOLERANCE= 0.1f;
// angle de vue pour le repérage des autres
const float IA_VISU_ANGLE= cos(glm::pi<float>()/ 0.3f);
// profondeur de vue le repérage des autres
const float IA_VISU_DEPTH= 400.0f;
// en ms; temps entre 2 recalculs
const uint IA_THINKING_TIME= 500;
// distance test pour crash level / box
const float IA_DISTANCE_TEST= 200.0f;

// map -----------------------------------------------------------------------------------
// dimensions en pixels de la carte
const uint MAP_WIDTH = 256;
const uint MAP_HEIGHT= 256;
// taille icone ship
const float MAP_SHIP_SIZE= 0.05f;

// ranking -------------------------------------------------------------------------------
const float RANKING_OFFX= 30.0f;
const float RANKING_OFFY= 30.0f;
const float RANKING_SIZEY= 30.0f;
const float RANKING_SCALE= 0.4f;
const float RANKING_ALPHA= 0.8f;

// cloud ---------------------------------------------------------------------------------
const uint NCLOUDS= 300;
const float CLOUD_MIN_SIZE= 40.0f;
const float CLOUD_MAX_SIZE= 400.0f;
const float CLOUD_MIN_SPEED= 0.3f;
const float CLOUD_MAX_SPEED= 1.0f;
const glm::vec3 CLOUD_COLOR= glm::vec3(0.8f, 0.8f, 0.9f);
const float CLOUD_ALPHA= 0.6f;
const float CLOUD_MIN_ALTI= 700.0f;
const float CLOUD_MAX_ALTI= 2000.0f;

// global msg ----------------------------------------------------------------------------
const float GLOBAL_MSG_OFFX= 300.0f;
const float GLOBAL_MSG_OFFY= 100.0f;
const float GLOBAL_MSG_SCALE= 0.8f;
const float GLOBAL_MSG_ALPHA_ANIM= 0.006f;

// divers --------------------------------------------------------------------------------
const glm::vec3 HEROS_COLOR= glm::vec3(1.0f, 0.0f, 0.0f);


#endif
