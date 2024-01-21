#include <iostream>

#include "utile.h"
#include "threebody.h"


Body::Body() : _position(glm::vec3(0.0f)), _speed(glm::vec3(0.0f)), _acceleration(glm::vec3(0.0f)) {

}


Body::~Body() {

}


Body::Body(glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass) :
 	_position(position), _speed(speed), _acceleration(acceleration), _mass(mass) {

}


void Body::set_position(glm::vec3 position) {
	_position= position;
}


void Body::set_speed(glm::vec3 speed) {
	_speed= speed;
}


void Body::set_acceleration(glm::vec3 acceleration) {
	_acceleration= acceleration;
}


void Body::set_mass(float mass) {
	_mass= mass;
}


void Body::print() {
	std::cout << "posit= (" << _position.x << " ; " << _position.y << " ; " << _position.z << ")\n";
	std::cout << "speed= (" << _speed.x << " ; " << _speed.y << " ; " << _speed.z << ")\n";
	std::cout << "accel= (" << _acceleration.x << " ; " << _acceleration.y << " ; " << _acceleration.z << ")\n";
}


void Body::randomize() {
	float n= 10.0f;
	_position.x= rand_float(-1.0f* n, n);
	_position.y= rand_float(-1.0f* n, n);
	_position.z= rand_float(-1.0f* n, n);
	_speed.x= rand_float(-1.0f* n, n);
	_speed.y= rand_float(-1.0f* n, n);
	_speed.z= rand_float(-1.0f* n, n);
	_acceleration.x= 0.0f;
	_acceleration.y= 0.0f;
	_acceleration.z= 0.0f;
}


// --------------------------------------------------
ThreeBody::ThreeBody() {

}


ThreeBody::ThreeBody(GLuint prog_draw) {
	glGenBuffers(4, _buffers);
		
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_world2clip_loc= glGetUniformLocation(_prog_draw, "world2clip_matrix");
}


ThreeBody::~ThreeBody() {

}


void ThreeBody::add_body(glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass) {
	_bodies.push_back(Body(position, speed, acceleration, mass));
}


void ThreeBody::anim(float delta_t) {
	for (unsigned int idx_body=0; idx_body<_bodies.size(); ++idx_body) {
		glm::vec3 force(0.0f);
		for (unsigned int idx_body_2=0; idx_body_2<_bodies.size(); ++idx_body_2) {
			if (idx_body== idx_body_2) {
				continue;
			}
			glm::vec3 body1tobody2= _bodies[idx_body_2]._position- _bodies[idx_body]._position;
			float squared_dist= body1tobody2.x* body1tobody2.x+ body1tobody2.y* body1tobody2.y+ body1tobody2.z* body1tobody2.z;
			if (squared_dist<1e-8) {
				std::cout << "collision\n";
				continue;
			}
			force+= (_bodies[idx_body]._mass* _bodies[idx_body_2]._mass/ squared_dist)* body1tobody2;
		}
		_bodies[idx_body]._acceleration= (1.0f/ _bodies[idx_body]._mass)*force;
		_bodies[idx_body]._speed+= delta_t* _bodies[idx_body]._acceleration;
		_bodies[idx_body]._position+= delta_t* _bodies[idx_body]._speed;
	}
}


void ThreeBody::print() {
	for (unsigned int idx_body=0; idx_body<_bodies.size(); ++idx_body) {
		std::cout << idx_body << "\n";
		_bodies[idx_body].print();
	}
	std::cout << "\n";
}


void ThreeBody::randomize() {
	for (auto & body : _bodies) {
		body.randomize();
	}
}
