
#ifndef THREE_BODY_H
#define THREE_BODY_H

#include <string>
#include <vector>

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


	glm::vec3 _position;
	glm::vec3 _speed;
	glm::vec3 _acceleration;
	float _mass;
};


class ThreeBody {
public:
	ThreeBody();
	~ThreeBody();
	void add_body(glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass);
	void anim(float delta_t);
	void print();


	std::vector<Body> _bodies;
};

#endif
