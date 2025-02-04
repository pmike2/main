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
	Star(pt_type pos, pt_type size, glm::vec2 velocity, glm::vec4 color);
	~Star();
	void anim(std::chrono::system_clock::time_point t);


	AABB_2D _aabb;
	glm::vec2 _velocity;
	glm::vec4 _color;
	//bool _dead;
};


class StarSystem {
public:
	StarSystem();
	StarSystem(glm::vec2 pt_min, glm::vec2 pt_max);
	~StarSystem();
	void add_random_star();
	void anim(std::chrono::system_clock::time_point t);


	std::vector<Star *> _stars;
	glm::vec2 _pt_min, _pt_max;
};


#endif
