#include <iostream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "utile.h"
#include "threebody.h"


Body::Body() :
	_position(glm::vec3(0.0f)), _speed(glm::vec3(0.0f)), _acceleration(glm::vec3(0.0f)), _mass(1.0f),
	_color(glm::vec3(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f)))
{}


Body::Body(glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass) :
	_position(position), _speed(speed), _acceleration(acceleration), _mass(mass),
	_color(glm::vec3(rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f), rand_float(0.0f, 1.0f)))
{}


Body::~Body() {
	_histo_position.clear();
}


void Body::print() {
	std::cout << "posit= (" << _position.x << " ; " << _position.y << " ; " << _position.z << ")\n";
	std::cout << "speed= (" << _speed.x << " ; " << _speed.y << " ; " << _speed.z << ")\n";
	std::cout << "accel= (" << _acceleration.x << " ; " << _acceleration.y << " ; " << _acceleration.z << ")\n";
}


void Body::randomize(float limit) {
	_position.x= rand_float(-1.0f* limit, limit);
	_position.y= rand_float(-1.0f* limit, limit);
	_position.z= rand_float(-1.0f* limit, limit);
	/*_speed.x= rand_float(-1.0f* limit, limit);
	_speed.y= rand_float(-1.0f* limit, limit);
	_speed.z= rand_float(-1.0f* limit, limit);*/
	_speed.x= 0.0f;
	_speed.y= 0.0f;
	_speed.z= 0.0f;
	_acceleration.x= 0.0f;
	_acceleration.y= 0.0f;
	_acceleration.z= 0.0f;
	_color.x= rand_float(0.3f, 1.0f);
	_color.y= rand_float(0.3f, 1.0f);
	_color.z= rand_float(0.3f, 1.0f);
}


void Body::clear_histo() {
	_histo_position.clear();
}


// --------------------------------------------------
ThreeBody::ThreeBody() {

}


ThreeBody::ThreeBody(GLuint prog_draw) : _prog_draw(prog_draw), _n_pts(0) {
	_bodies.clear();

	glGenBuffers(2, _buffers);
		
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_world2clip_loc= glGetUniformLocation(_prog_draw, "world2clip_matrix");
}


ThreeBody::~ThreeBody() {
	for (auto & body : _bodies) {
		delete body;
	}
	_bodies.clear();
}


void ThreeBody::add_body(glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass) {
	Body * body= new Body(position, speed, acceleration, mass);
	_bodies.push_back(body);
}


void ThreeBody::add_body() {
	Body * body= new Body();
	_bodies.push_back(body);
}


void ThreeBody::anim(float delta_t) {
	for (unsigned int idx_body=0; idx_body<_bodies.size(); ++idx_body) {
		glm::vec3 force(0.0f);
		for (unsigned int idx_body_2=0; idx_body_2<_bodies.size(); ++idx_body_2) {
			if (idx_body== idx_body_2) {
				continue;
			}
			glm::vec3 body1tobody2= _bodies[idx_body_2]->_position- _bodies[idx_body]->_position;
			float squared_dist= body1tobody2.x* body1tobody2.x+ body1tobody2.y* body1tobody2.y+ body1tobody2.z* body1tobody2.z;
			if (squared_dist< 0.001f) {
				std::cout << "Collision entre " << idx_body << " et " << idx_body_2 <<  "\n";
				continue;
			}
			force+= (_bodies[idx_body]->_mass* _bodies[idx_body_2]->_mass/ squared_dist)* body1tobody2;
		}
		_bodies[idx_body]->_acceleration= (1.0f/ _bodies[idx_body]->_mass)* force;
		_bodies[idx_body]->_speed+= delta_t* _bodies[idx_body]->_acceleration;
		_bodies[idx_body]->_position+= delta_t* _bodies[idx_body]->_speed;

		const float WARNING_LIMIT= 1000.0f;
		if (
			(_bodies[idx_body]->_position.x> WARNING_LIMIT) || (_bodies[idx_body]->_position.x< -1.0f* WARNING_LIMIT) ||
			(_bodies[idx_body]->_position.y> WARNING_LIMIT) || (_bodies[idx_body]->_position.y< -1.0f* WARNING_LIMIT) ||
			(_bodies[idx_body]->_position.z> WARNING_LIMIT) || (_bodies[idx_body]->_position.z< -1.0f* WARNING_LIMIT))
		{
			std::cout << "Body " << idx_body << " a franchi la limite " << WARNING_LIMIT << "\n";
		}

		if (_bodies[idx_body]->_histo_position.size()== MAX_HISTO_SIZE) {
			_bodies[idx_body]->_histo_position.pop_front();
		}
		_bodies[idx_body]->_histo_position.push_back(_bodies[idx_body]->_position);
	}
}


void ThreeBody::print() {
	for (unsigned int idx_body=0; idx_body<_bodies.size(); ++idx_body) {
		std::cout << idx_body << "\n";
		_bodies[idx_body]->print();
	}
	std::cout << "\n";
}


void ThreeBody::randomize(unsigned int n_bodies, float limit) {
	_bodies.clear();
	for (unsigned int idx_body=0; idx_body<n_bodies; ++idx_body) {
		add_body();
	}
	for (auto & body : _bodies) {
		body->randomize(limit);
	}
}


void ThreeBody::draw(const glm::mat4 & world2clip) {
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(world2clip));
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));
	//std::cout << _n_pts << "\n";
	glDrawArrays(GL_POINTS, 0, _n_pts);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void ThreeBody::update() {
	unsigned int histo_size= _bodies[0]->_histo_position.size();
	_n_pts= _bodies.size()* histo_size;
	//std::cout << "histo_size=" << histo_size << " ; _n_pts=" << _n_pts << "\n";
	float data[6* _n_pts];
	for (unsigned int idx_body=0; idx_body<_bodies.size(); ++idx_body) {
		for (unsigned int idx_position=0; idx_position<histo_size; ++idx_position) {
			/*std::cout << _bodies[idx_body]->_histo_position[idx_position].x << "\n";
			std::cout << _bodies[idx_body]->_histo_position[idx_position].y << "\n";
			std::cout << _bodies[idx_body]->_histo_position[idx_position].z << "\n";
			std::cout << _bodies[idx_body]->_color.x << "\n";
			std::cout << _bodies[idx_body]->_color.y << "\n";
			std::cout << _bodies[idx_body]->_color.z << "\n";
			std::cout << "------\n";*/
			data[(idx_body* histo_size+ idx_position)* 6+ 0]= _bodies[idx_body]->_histo_position[idx_position].x;
			data[(idx_body* histo_size+ idx_position)* 6+ 1]= _bodies[idx_body]->_histo_position[idx_position].y;
			data[(idx_body* histo_size+ idx_position)* 6+ 2]= _bodies[idx_body]->_histo_position[idx_position].z;
			data[(idx_body* histo_size+ idx_position)* 6+ 3]= _bodies[idx_body]->_color.x;
			data[(idx_body* histo_size+ idx_position)* 6+ 4]= _bodies[idx_body]->_color.y;
			data[(idx_body* histo_size+ idx_position)* 6+ 5]= _bodies[idx_body]->_color.z;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool ThreeBody::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_a) {
		randomize(3, 10.0f);
		set_all_z2zero();
		return true;
	}
	else if (key== SDLK_b) {
		randomize(3, 10.0f);
		return true;
	}
	else if (key== SDLK_c) {
		randomize(300, 10.0f);
		set_all_z2zero();
		return true;
	}
	else if (key== SDLK_d) {
		randomize(300, 10.0f);
		return true;
	}

	return false;
}


void ThreeBody::set_all_z2zero() {
	for (auto & body : _bodies) {
		body->_position.z= 0.0f;
		body->_speed.z= 0.0f;
		body->_acceleration.z= 0.0f;
	}
}

