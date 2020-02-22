#ifndef LIGHT_H
#define LIGHT_H

#include <vector>
#include <sys/time.h>

#include <OpenGL/gl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "constantes.h"
#include "utile.h"


// sert au dessin de la lumière pour debug
class LightDraw {
public:
	LightDraw();
	LightDraw(GLuint prog_draw, float* position_world, float* spot_cone_direction_world, float* color);
	void draw(float * world2clip);
	
	
	GLuint _buffer;
	GLfloat _data[12];
	GLint _world2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _prog_draw;
};


// ---------------------------------------------------------------------------------------
class Light {
public:
	Light();
	Light(const LightParams &lp, GLuint prog_draw, float* position_world, float* spot_cone_direction_world);
	void anim(float * world2camera);
	void print();
	void move(glm::vec3 destination);
	
	float _position_camera[3];
	float _spot_cone_direction_camera[3];
	float _color[3];
	float _position_world[4];
	// de taille 3 (contrairement a position_world) car comme normal, il s'agit d'une direction
	float _spot_cone_direction_world[3]; 
	LightParams _lp;
	
	bool _is_active;
	LightDraw _light_draw;
};


// ---------------------------------------------------------------------------------------
class LightsUBO {
public:
	LightsUBO();
	void set_prog(GLuint prog);
	void init();
	void reset_buff();
	void update(std::vector<Light> lights);
	void release();
	void print();
	
	
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
