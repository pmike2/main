#ifndef DUNGEON_H
#define DUNGEON_H

#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include "bbox.h"
#include "graph.h"
#include "utile.h"


struct Dungeon {
	Dungeon();
	Dungeon(glm::vec3 vmin, glm::vec3 vmax, glm::vec3 step, GLuint prog_draw);
	~Dungeon();
	unsigned int pos2idx(glm::uvec3 pos);
	glm::uvec3 idx2pos(unsigned int idx);
	glm::vec3 pos2posf(glm::uvec3 pos);
	glm::uvec3 posf2pos(glm::vec3 posf);
	void randomize();
	void draw(const glm::mat4 & world2clip);
	void update();


	AABB * _aabb;
	glm::vec3 _step;
	glm::uvec3 _n;
	Graph * _graph;
	std::vector<Mesh *> _meshes;
	
	GLuint _prog_draw;
	GLint _world2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffer;
	unsigned int _n_pts;
};

#endif
