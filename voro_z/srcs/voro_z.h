#ifndef VORO_Z_H
#define VORO_Z_H

#include <iostream>
#include <vector>
#include <map>

#include <OpenGL/gl3.h>

#include "typedefs.h"
#include "input_state.h"
#include "dcel.h"


typedef enum {WATER, COAST, FOREST, MOUNTAIN} BiomeType;


class Biome {
public:
	Biome();
	Biome(BiomeType type, number zmin, number zmax, glm::vec4 color, std::string texture_path);
	~Biome();


	BiomeType _type;
	number _zmin;
	number _zmax;
	glm::vec4 _color;
	std::string _texture_path;
	unsigned int _idx_texture;
};


class DCEL_FaceData {
public:
	DCEL_FaceData();
	DCEL_FaceData(number z);
	~DCEL_FaceData();
	void set_biome(Biome * biome);


	number _z;
	Biome * _biome;
};


class DrawContext {
public:
	DrawContext();
	DrawContext(GLuint prog, GLuint buffer, std::vector<std::string> locs_attrib, std::vector<std::string> locs_uniform);
	~DrawContext();

	GLuint _prog;
	std::map<std::string, GLint> _locs;
	GLuint _buffer;
};


class Light {
public:
	Light();
	Light(glm::vec3 position_ini, glm::vec3 color);
	~Light();
	void anim();


	glm::vec3 _position_ini;
	glm::vec3 _position;
	glm::vec3 _color;
	bool _animated;
};


class VoroZ {
public:
	VoroZ();
	VoroZ(GLuint prog_draw_simple, GLuint prog_draw_texture, GLuint prog_draw_light);
	~VoroZ();

	void init_biome();
	void init_context(GLuint prog_draw_simple, GLuint prog_draw_texture, GLuint prog_draw_light);
	void init_texture();
	void init_light();
	void init_dcel();

	void draw_simple(const glm::mat4 & world2clip);
	void draw_texture(const glm::mat4 & world2clip);
	void draw_light(const glm::mat4 & world2clip, const glm::vec3 & camera_position);
	void draw(const glm::mat4 & world2clip, const glm::vec3 & camera_position);

	void update_simple();
	void update_texture();
	void update_light();
	void update();

	void anim();

	bool key_down(InputState * input_state, SDL_Keycode key);


	unsigned int _n_pts;
	DCEL * _dcel;
	std::map<BiomeType, Biome *> _biomes;
	
	DrawContext * _context_simple;
	DrawContext * _context_texture;
	DrawContext *  _context_light;
	GLuint _texture_id;
	Light * _light;
};


#endif
