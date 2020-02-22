
#ifndef CONSTANTES_H
#define CONSTANTES_H

#include <string>
#include <cmath>


const float PI_DIV_180= M_PI/ 180.0;

const float FRUSTUM_NEAR= 1.0;
const float FRUSTUM_FAR= 50.0;
const float FRUSTUM_SCALE= 1.0;
const int SCREEN_WIDTH= 1024;
const int SCREEN_HEIGHT= 1024;

const char * const CH_LOG= "sdl.log";

const unsigned int DELTA_ANIM= 10; // en ms; temps entre 2 anims

// ATTENTION il faut que n_lights dans le shader soit = à N_MAX_LIGHTS !!!
const unsigned int N_MAX_LIGHTS= 8;
const unsigned int N_MAX_OBJS= 200;
const unsigned int N_MAX_VMATS= 32;

const char * const LIGHT_NAMES[]=
{
 "lights[0].light_color",
 "lights[0].light_position",
 "lights[0].spot_cone_direction",
 "lights[0].strength",
 "lights[0].constant_attenuation",
 "lights[0].linear_attenuation",
 "lights[0].quadratic_attenuation",
 "lights[0].spot_cos_cutoff",
 "lights[0].spot_exponent",
 "lights[0].is_active",
 "lights[0].is_spotlight"
};

// pour mémoire, pour windows je crois qu'il faut mettre ça :
/*const char * const LIGHT_NAMES[]=
{
	"lights_uni.lights[0].light_color",
	"lights_uni.lights[0].light_position",
	"lights_uni.lights[0].spot_cone_direction",
	"lights_uni.lights[0].strength",
	"lights_uni.lights[0].constant_attenuation",
	"lights_uni.lights[0].linear_attenuation",
	"lights_uni.lights[0].quadratic_attenuation",
	"lights_uni.lights[0].spot_cos_cutoff",
	"lights_uni.lights[0].spot_exponent",
	"lights_uni.lights[0].is_active",
	"lights_uni.lights[0].is_spotlight"
};
*/

// ces valeurs doivent correspondre à celles dans pa_server/main.c
const int PORT_NUMBER= 50000; // les ports utilises vont de PORT_NUMBER a PORT_NUMBER+ N_MAX_CHANNELS- 1
// en millisecondes; quand un channel est trigge on le rend untriggable pendant MAX_TRIGGER_DURATION pour eviter les 'faux multitrigs'
const float MAX_TRIGGER_DURATION= 200; 
const unsigned int SOCKET_BUFFER_SIZE= 8; // nombre de charactères lus sur le socket a la fois
const unsigned int N_MAX_CHANNELS= 32; // nombre max de channels sur la carte son

const int PORT_NUMBER_FLTK= 49999; // sur 49999 on recupere les données de fltk (IHM)
const unsigned int SOCKET_BUFFER_SIZE_FLTK= 100000; // doit etre > taille de n'importe quel fichier de config a2v

// type de channel
enum channel_type {LOAD_OBJ, LOAD_LIGHT, LOAD_VMAT};

const char DELIM_ACTION= ';';
const char DELIM_CHANNEL= ':';
const char DELIM_A2V= '|';

const std::string ROOT_MODELES= "../modeles";
const std::string ROOT_MATS= "../modeles"; // a faire évoluer ?


#endif
