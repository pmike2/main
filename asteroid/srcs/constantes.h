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
const float Z_NEAR= -10.0f;
const float Z_FAR= 10.0f;
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


// type de ship
enum ShipType {HERO, ENEMY, BULLET};
// mode jeu : PLAYING = jeu en cours ; INACTIVE = affichage meilleurs scores ; SET_SCORE_NAME = saisie nom meilleur score
enum GameMode {PLAYING, INACTIVE, SET_SCORE_NAME};
// type d'événement dans un niveau
enum EventType {NEW_ENEMY, LEVEL_END};


#endif
