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


struct DrawContext {
	GLuint _prog;
	std::map<std::string, GLint> _locs;
	GLuint _buffer;
};


class VoroZ {
public:
	VoroZ();
	VoroZ(GLuint prog_draw_simple, GLuint prog_draw_texture);
	~VoroZ();

	void draw_top_simple(const glm::mat4 & world2clip);
	void draw_top_texture(const glm::mat4 & world2clip);
	void draw_side_simple(const glm::mat4 & world2clip);
	void draw_side_texture(const glm::mat4 & world2clip);
	void draw(const glm::mat4 & world2clip);

	void update_top_simple();
	void update_top_texture();
	void update_side_simple();
	void update_side_texture();
	void update();

	bool key_down(InputState * input_state, SDL_Keycode key);


	DrawContext _context_top_simple, _context_side_simple, _context_top_texture, _context_side_texture;
	unsigned int _n_pts_tops, _n_pts_sides;
	DCEL * _dcel;
	std::map<BiomeType, Biome *> _biomes;
	GLuint _texture_id;
};


#endif
