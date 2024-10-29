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


glm::vec3 normal(const glm::vec3 & p1, const glm::vec3 & p2, const glm::vec3 & p3);
glm::vec3 tangent(const glm::vec3 & p1, const glm::vec3 & p2, const glm::vec3 & p3, const glm::vec2 & uv1, const glm::vec2 & uv2, const glm::vec2 & uv3);
glm::vec3 triangle2euler(const glm::vec3 & p1, const glm::vec3 & p2, const glm::vec3 & p3);
glm::mat3 triangle2mat(const glm::vec3 & p1, const glm::vec3 & p2, const glm::vec3 & p3);


class Biome {
public:
	Biome();
	Biome(BiomeType type, number zmin, number zmax, glm::vec4 color, std::string diffuse_texture_path, std::string normal_texture_path, std::string parallax_texture_path);
	~Biome();


	BiomeType _type;
	number _zmin;
	number _zmax;
	glm::vec4 _color;
	std::string _diffuse_texture_path, _normal_texture_path, _parallax_texture_path;
	unsigned int _diffuse_texture_idx, _normal_texture_idx, _parallax_texture_idx;
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


class TriangleData {
public:
	TriangleData();
	TriangleData(const glm::vec3 & pt1, const glm::vec3 & pt2, const glm::vec3 & pt3, const glm::vec2 & uv1, const glm::vec2 & uv2, const glm::vec2 & uv3, const glm::vec4 & color, Biome * biome);
	~TriangleData();


	glm::vec3 _pts[3];
	glm::vec2 _uvs[3];
	glm::vec4 _color;
	Biome * _biome;
	glm::vec3 _normal;
	glm::vec3 _tangent;
	//glm::vec3 _bitangent;
};


class VoroZ {
public:
	VoroZ();
	VoroZ(GLuint prog_draw_simple, GLuint prog_draw_texture, GLuint prog_draw_light, GLuint prog_draw_normal, GLuint prog_draw_parallax);
	~VoroZ();

	void init_biome();
	void init_context(GLuint prog_draw_simple, GLuint prog_draw_texture, GLuint prog_draw_light, GLuint prog_draw_normal, GLuint prog_draw_parallax);
	void init_texture_diffuse();
	void init_texture_normal();
	void init_texture_parallax();
	void init_light();
	void init_dcel();

	void draw_simple(const glm::mat4 & world2clip);
	void draw_texture(const glm::mat4 & world2clip);
	void draw_light(const glm::mat4 & world2clip, const glm::vec3 & camera_position);
	void draw_normal(const glm::mat4 & world2clip, const glm::vec3 & camera_position);
	void draw_parallax(const glm::mat4 & world2clip, const glm::vec3 & camera_position);
	void draw(const glm::mat4 & world2clip, const glm::vec3 & camera_position);

	void update_triangle_data();
	void update_simple();
	void update_texture();
	void update_light();
	void update_normal();
	void update_parallax();
	void update();

	void anim();

	bool key_down(InputState * input_state, SDL_Keycode key);


	unsigned int _n_pts;
	DCEL * _dcel;
	std::map<BiomeType, Biome *> _biomes;
	std::map<std::string, DrawContext *> _contexts;
	
	GLuint _texture_id_diffuse, _texture_id_normal, _texture_id_parallax;
	Light * _light;
	std::vector<TriangleData *> _triangle_data;
};


#endif
