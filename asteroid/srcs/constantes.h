#ifndef CONSTANTES_H
#define CONSTANTES_H

#include <string>

#include <glm/glm.hpp>


// couleurs
const glm::dvec4 BORDER_COLOR(0.3, 0.3, 0.2, 1.0);
const glm::dvec4 AABB_FRIENDLY_COLOR(0.0, 1.0, 0.0, 1.0);
const glm::dvec4 AABB_UNFRIENDLY_COLOR(1.0, 0.0, 0.0, 1.0);
const glm::dvec4 FOOTPRINT_FRIENDLY_COLOR(0.0, 1.0, 1.0, 1.0);
const glm::dvec4 FOOTPRINT_UNFRIENDLY_COLOR(1.0, 0.0, 1.0, 1.0);
// vitesse héro
const float HERO_VELOCITY= 0.1;
// plans z de contrainte d'affichage de glm::ortho
const float Z_NEAR= 0.0f;
const float Z_FAR= 1000.0f;
// taille textures
const unsigned int TEXTURE_SIZE= 1024;
// temps d'invulnérabilité après avoir été touché
const unsigned int HIT_UNTOUCHABLE_MS= 130;
// temps de rotation dans un sens lors d'un hit
const unsigned int HIT_ROTATION_MS= 60;
// incrément de rotation lors d'un hit
const float HIT_ROTATION_INC= 0.1;
// temps d'animation de la mort
const unsigned int DEATH_MS= 200;
// incrément de scale lors d'une mort
const float DEATH_SCALE_INC= 0.1;
// nom de l'action principale de chaque ship (voir jsons)
const std::string MAIN_ACTION_NAME= "main";
// temps de fade in / out de la musique
const unsigned int MUSIC_FADE_IN_MS= 1500;
const unsigned int MUSIC_FADE_OUT_MS= 1500;
// nombre d'étoiles
const unsigned int N_STARS= 1000;
// taille d'étoile min
const float MIN_STAR_SIZE= 0.05;
// taille d'étoile max
const float MAX_STAR_SIZE= 0.7;
// exposant visant à faire tendre une + grande proba de création pour les + petites étoiles
const float STAR_POW_EXP= 10.0;
// ratio liant la taille à la vitesse ; les + grosses étoiles doivent aller + vite
const float STAR_SIZE_VELOCITY_RATIO= 0.02;
// opacité max d'une étoile
const float STAR_MAX_ALPHA= 0.7;

// type de ship
enum ShipType {HERO, ENEMY, BULLET};
// mode jeu : PLAYING = jeu en cours ; INACTIVE = affichage meilleurs scores ; SET_SCORE_NAME = saisie nom meilleur score
enum GameMode {PLAYING, INACTIVE, SET_SCORE_NAME};
// type d'événement dans un niveau
enum EventType {NEW_ENEMY, LEVEL_END};


#endif
