#ifndef LIGHT_H
#define LIGHT_H

#include <vector>
#include <sys/time.h>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "utile.h"


// constantes --------------------------------------------------------------------------------
// ATTENTION il faut que n_lights dans le shader soit = à N_MAX_LIGHTS !!!
const unsigned int N_MAX_LIGHTS= 8;

const char * const LIGHT_NAMES[]= {
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
/*const char * const LIGHT_NAMES[]= {
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

struct LightParams {
	bool _is_spotlight;
	float _strength;
	float _constant_attenuation;
	float _linear_attenuation;
	float _quadratic_attenuation;
	float _spot_cos_cutoff; // + la valeur est proche de 1.0, plus le spot est concentre
	float _spot_exponent; // une grande valeur adoucie les bords du spot
};

// _linear_attenuation et _quadratic_attenuation font rapidement tout devenir sombre !!!!
const struct LightParams LIGHT_PARAMS_1= {false, 1.0f, 1.0f, 0.0f, 0.0f, 0.90f, 200.0f};
const struct LightParams LIGHT_PARAMS_2= {true, 1.0f, 1.0f, 0.0f, 0.0f, 0.20f, 200.0f};

const float LIGHT_DRAW_MULT_FACTOR= 100.0f;


// ---------------------------------------------------------------------------------------
// sert au dessin de la lumière pour debug
class LightDraw {
public:
	LightDraw();
	LightDraw(GLuint prog_draw, glm::vec3 position_world, glm::vec3 spot_cone_direction_world, glm::vec3 color);
	void draw(glm::mat4 & world2clip);
	
	
	GLuint _buffer;
	GLfloat _data[12];
	GLint _world2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _prog_draw;
};


// ---------------------------------------------------------------------------------------
class Light {
public:
	Light();
	Light(const LightParams & lp, GLuint prog_draw, glm::vec3 position_world, glm::vec3 spot_cone_direction_world);
	void anim(glm::mat4 & world2camera);
	void print();
	void move(glm::vec3 destination);
	

	glm::vec3 _position_camera;
	glm::vec3 _spot_cone_direction_camera;
	glm::vec3 _color;
	glm::vec3 _position_world;
	glm::vec3 _spot_cone_direction_world;
	LightParams _lp;
	
	bool _is_active;
	LightDraw _light_draw;
};


// ---------------------------------------------------------------------------------------
class LightsUBO {
public:
	LightsUBO();
	LightsUBO(GLuint prog);
	~LightsUBO();
	void reset_buff();
	void update();
	void add_light(const LightParams & lp, GLuint prog_draw, glm::vec3 position_world, glm::vec3 spot_cone_direction_world);
	void anim(glm::mat4 & world2camera);
	void draw(glm::mat4 & world2clip);
	void print();
	
	
	std::vector<Light *> _lights;
	GLuint _lights_loc;
	GLuint _lights_buff_idx;
	GLint  _ubo_size;
	GLuint _light_binding_point;
	char * _lights_buff;
	unsigned int _n_attrs;
	GLuint * _indices;
	GLint * _offsets;
	GLint _light_size;
	GLuint _prog;
};

#endif
