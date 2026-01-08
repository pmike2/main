#ifndef STAR_H
#define STAR_H

#include <string>
#include <vector>
#include <chrono>

#include <glm/glm.hpp>

#include "bbox_2d.h"

#include "constantes.h"


class Star {
public:
	Star();
	Star(pt_2d pos, pt_2d size, float z, glm::vec2 velocity, glm::vec4 color, uint idx_texture);
	~Star();
	void anim(std::chrono::system_clock::time_point t);


	AABB_2D _aabb;
	float _z;
	glm::vec2 _velocity;
	glm::vec4 _color;
	uint _idx_texture;
};


class StarSystem {
public:
	StarSystem();
	StarSystem(glm::vec2 pt_min, glm::vec2 pt_max, std::string pngs_dir);
	~StarSystem();
	void add_random_star();
	void sort_by_z();
	void anim(std::chrono::system_clock::time_point t);


	std::vector<Star *> _stars;
	glm::vec2 _pt_min, _pt_max;
	std::vector<std::string> _pngs;
};


#endif
