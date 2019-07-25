
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

const int PORT_NUMBER= 12000; // OSC port number
const unsigned int N_MAX_CHANNELS= 32; // nombre max de channels sur la carte son

// type d'action
//enum action_type {LOAD_OBJ, LOAD_LIGHT, LOAD_VMAT};

//const std::string ROOT_MODELES= "../modeles";
//const std::string ROOT_MATS= "../modeles";


#endif
