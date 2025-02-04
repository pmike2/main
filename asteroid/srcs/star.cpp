#include "utile.h"

#include "star.h"


// Star -----------------------------------------------------------------
Star::Star() {

}


Star::Star(pt_type pos, pt_type size, glm::vec2 velocity, glm::vec4 color) :
	_aabb(AABB_2D(pos, size)), _velocity(velocity), _color(color)//, _dead(false)
{
	
}


Star::~Star() {
	
}


void Star::anim(std::chrono::system_clock::time_point t) {
	_aabb._pos+= _velocity;
}


// StarSystem ------------------------------------------------------------
StarSystem::StarSystem() {

}


StarSystem::StarSystem(glm::vec2 pt_min, glm::vec2 pt_max) : _pt_min(pt_min), _pt_max(pt_max) {
	for (unsigned int i=0; i<100; ++i) {
		add_random_star();
	}
}


StarSystem::~StarSystem() {
	for (auto star : _stars) {
		delete star;
	}
	_stars.clear();
}


void StarSystem::add_random_star() {
	pt_type pos= pt_type(rand_float(_pt_min.x, _pt_max.x), rand_float(_pt_min.y, _pt_max.y));
	float size= rand_float(0.01, 0.5);
	//pt_type velocity(0.0, -1.0* rand_float(0.01, 0.1));
	// taille proportionnelle à la vitesse
	pt_type velocity(0.0, -0.1* size);
	glm::vec4 color(rand_float(0.01, 1.0), rand_float(0.01, 1.0), rand_float(0.01, 1.0), 1.0);
	_stars.push_back(new Star(pos, pt_type(size, size), velocity, color));
}


void StarSystem::anim(std::chrono::system_clock::time_point t) {
	for (auto star : _stars) {
		star->anim(t);
		if (star->_aabb._pos.y+ star->_aabb._size.y< _pt_min.y) {
			star->_aabb._pos.y= _pt_max.y;
		}
	}
}
