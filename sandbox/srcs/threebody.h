
#ifndef THREE_BODY_H
#define THREE_BODY_H

#include <string>
#include <vector>

#include <OpenGL/gl3.h>

#include <glm/glm.hpp>


class Body {
public:
	Body();
	Body(glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass);
	~Body();
	void set_position(glm::vec3 position);
	void set_speed(glm::vec3 speed);
	void set_acceleration(glm::vec3 acceleration);
	void set_mass(float mass);
	void print();
	void randomize();


	glm::vec3 _position;
	glm::vec3 _speed;
	glm::vec3 _acceleration;
	float _mass;
};


class ThreeBody {
public:
	ThreeBody();
	ThreeBody(GLuint prog_draw);
	~ThreeBody();
	void add_body(glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass);
	void anim(float delta_t);
	void print();
	void randomize();
	void draw();


	std::vector<Body> _bodies;
	GLuint _prog_draw;
	GLint _world2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffers[1];
};

#endif
