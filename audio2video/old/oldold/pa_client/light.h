#ifndef LIGHT_H
#define LIGHT_H

#include <vector>
#include <sys/time.h>

#include <OpenGL/gl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "constantes.h"
#include "utile.h"
#include "config.h"



class Light
{
public:
	Light();
	Light(bool is_spotlight_, GLuint prog_draw_, struct timeval t_);
	void draw(float * world2clip);
	void anim(float * world2camera);
	void set_config(LightConfig * config_);
	void print();
	
	
	float position_camera[3];
	float spot_cone_direction_camera[3];
	float strength;
	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
	float spot_cos_cutoff;
	float spot_exponent;
	bool is_spotlight;
	
	// sert au dessin de la lumière
	GLuint draw_light_buffer;
	GLfloat draw_light_data[12];
	GLint world2clip_loc, position_loc, diffuse_color_loc;
	GLuint prog_draw;

	LightMorph current_values;
	LightConfig * config;
	bool is_active;
	struct timeval t;

	int idx_channel, idx_action; // sert si retrig de la config associée
};


// ---------------------------------------------------------------------------------------------------


class LightsUBO{
public:
	LightsUBO();
	void set_prog(GLuint prog_);
	void init();
	void reset_buff();
	void update(std::vector<Light> lights);
	void release();
	void print();

	GLuint lights_loc;
	GLuint lights_buff_idx;
	GLint  ubo_size;
	GLuint light_binding_point;
	char * lights_buff;
	unsigned int n_attrs;
	GLuint * indices;
	GLint * offsets;
	GLint light_size;
	GLuint prog;
};

#endif
