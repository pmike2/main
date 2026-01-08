#ifndef VORO_Z_H
#define VORO_Z_H

#include <iostream>
#include <vector>
#include <map>

#include <OpenGL/gl3.h>

#include "typedefs.h"
#include "input_state.h"
#include "dcel.h"
#include "gl_utils.h"


const int TEX_SIZE= 1024;

typedef enum {WATER, COAST, FOREST, MOUNTAIN, DIRT} BiomeType;
typedef enum {SIMPLE, TEXTURE, LIGHT, NORMAL, PARALLAX} DrawMode;


glm::vec3 normal(const glm::vec3 & p1, const glm::vec3 & p2, const glm::vec3 & p3);
glm::vec3 tangent(const glm::vec3 & p1, const glm::vec3 & p2, const glm::vec3 & p3, const glm::vec2 & uv1, const glm::vec2 & uv2, const glm::vec2 & uv3);
//glm::vec3 triangle2euler(const glm::vec3 & p1, const glm::vec3 & p2, const glm::vec3 & p3);
//glm::mat3 triangle2mat(const glm::vec3 & p1, const glm::vec3 & p2, const glm::vec3 & p3);


class Biome {
public:
	Biome();
	Biome(BiomeType type, number zmin, number zmax, glm::vec4 color, float uv_factor,
		std::string diffuse_texture_path, std::string normal_texture_path, std::string parallax_texture_path, float anim_speed);
	~Biome();


	BiomeType _type;
	number _zmin;
	number _zmax;
	glm::vec4 _color;
	float _uv_factor;
	std::string _diffuse_texture_path, _normal_texture_path, _parallax_texture_path;
	uint _diffuse_texture_idx_start, _diffuse_texture_idx_end;
	float _diffuse_texture_idx_current;
	uint _normal_texture_idx_start, _normal_texture_idx_end;
	float _normal_texture_idx_current;
	uint  _parallax_texture_idx;
	std::vector<std::string> _diffuse_pngs;
	std::vector<std::string> _normal_pngs;
	float _anim_speed;
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
	TriangleData(
		const glm::vec3 & pt1, const glm::vec3 & pt2, const glm::vec3 & pt3, 
		const glm::vec2 & uv1_diffuse, const glm::vec2 & uv2_diffuse, const glm::vec2 & uv3_diffuse,
		const glm::vec2 & uv1_normal, const glm::vec2 & uv2_normal, const glm::vec2 & uv3_normal,
		const glm::vec2 & uv1_parallax, const glm::vec2 & uv2_parallax, const glm::vec2 & uv3_parallax,
		Biome * biome);
	~TriangleData();


	glm::vec3 _pts[3];
	glm::vec2 _uvs_diffuse[3];
	glm::vec2 _uvs_normal[3];
	glm::vec2 _uvs_parallax[3];
	//glm::vec4 _color;
	Biome * _biome;
	glm::vec3 _normal;
	glm::vec3 _tangent;
	//glm::vec3 _bitangent;
};


class VoroZ {
public:
	VoroZ();
	VoroZ(std::map<std::string, GLuint> progs);
	~VoroZ();

	void init_biome();
	void init_context(std::map<std::string, GLuint> progs);
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


	uint _n_pts;
	DCEL * _dcel;
	std::map<BiomeType, Biome *> _biomes;
	std::map<std::string, DrawContext *> _contexts;
	GLuint _texture_id_diffuse, _texture_id_normal, _texture_id_parallax;
	Light * _light;
	std::vector<TriangleData *> _triangle_data;
	DrawMode _draw_mode;
};


#endif
