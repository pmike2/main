#ifndef PARTICLE_H
#define PARTICLE_H

#include <vector>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "bbox_2d.h"
#include "gl_utils.h"


class Particle {
public:
	Particle();
	Particle(glm::vec2 position, glm::vec2 size, glm::vec2 velocity, glm::vec4 color);
	~Particle();


	float _life;
	AABB_2D _aabb;
	glm::vec2 _velocity;
	glm::vec4 _color;
};


class ParticleSystem {
	ParticleSystem();
	ParticleSystem(GLuint prog, ScreenGL * screengl);
	~ParticleSystem();
	void fill_texture_array();
	void update();
	void draw();
	void anim();


	std::vector<Particle *> _particles;
	DrawContext * _context;
	GLuint _texture_id;
	glm::mat4 _camera2clip; // glm::ortho
};


#endif
