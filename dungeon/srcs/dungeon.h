#ifndef DUNGEON_H
#define DUNGEON_H

#include <iostream>
#include <vector>

#include <OpenGL/gl3.h>

#include <glm/glm.hpp>

#include <SDL2/SDL_keycode.h>

#include "bbox.h"
#include "graph.h"
#include "utile.h"
#include "input_state.h"


struct Dungeon {
	Dungeon();
	Dungeon(glm::vec3 vmin, glm::vec3 vmax, glm::vec3 step, GLuint prog_draw_border, GLuint prog_draw_fill);
	~Dungeon();
	unsigned int pos2idx(glm::uvec3 pos);
	glm::uvec3 idx2pos(unsigned int idx);
	glm::vec3 pos2posf(glm::uvec3 pos);
	glm::uvec3 posf2pos(glm::vec3 posf);
	void randomize();
	void draw(const glm::mat4 & world2clip);
	void update();
	bool key_down(InputState * input_state, SDL_Keycode key);


	AABB * _aabb;
	glm::vec3 _step;
	glm::uvec3 _n;
	Graph * _graph;
	std::vector<Mesh *> _meshes;
	std::vector<AABB *> _aabbs_rooms;
	std::vector<BBox *> _bboxs_hallways;
	
	GLuint _prog_draw_border, _prog_draw_fill;
	GLint _world2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffers[4];
	unsigned int _n_pts;
	bool _draw[4];
};

#endif

