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
	Dungeon(glm::vec3 vmin, glm::vec3 vmax, glm::vec3 step);
	~Dungeon();
	void randomize();
	unsigned int pos2idx(glm::uvec3 pos);
	glm::uvec3 idx2pos(unsigned int idx);
	glm::vec3 pos2posf(glm::uvec3 pos);
	glm::uvec3 posf2pos(glm::vec3 posf);


	AABB * _aabb;
	glm::vec3 _step;
	glm::uvec3 _n;
	Graph * _graph;
	std::vector<Mesh *> _meshes;
};

#endif

