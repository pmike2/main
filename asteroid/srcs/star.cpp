#include "utile.h"

#include "star.h"


// Star -----------------------------------------------------------------
Star::Star() {

}


Star::Star(pt_2d pos, pt_2d size, float z, glm::vec2 velocity, glm::vec4 color, unsigned int idx_texture) :
	_aabb(AABB_2D(pos, size)), _z(z), _velocity(velocity), _color(color), _idx_texture(idx_texture)
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


StarSystem::StarSystem(glm::vec2 pt_min, glm::vec2 pt_max, std::string pngs_dir) : _pt_min(pt_min), _pt_max(pt_max) {
	_pngs= list_files("../data/star", "png");
	for (unsigned int i=0; i<N_STARS; ++i) {
		add_random_star();
	}
	// il faut trier les étoiles par z si on veut que le blend alpha se passe bien
	sort_by_z();
}


StarSystem::~StarSystem() {
	for (auto star : _stars) {
		delete star;
	}
	_stars.clear();
}


void StarSystem::add_random_star() {
	pt_2d pos= pt_2d(rand_float(_pt_min.x, _pt_max.x), rand_float(_pt_min.y, _pt_max.y));
	float size= MIN_STAR_SIZE+ pow(rand_float(0.0, 1.0), STAR_POW_EXP)* (MAX_STAR_SIZE- MIN_STAR_SIZE);
	float z= -1.0/ size; // derrière les ships qui sont à z == 0.0 ; les + grosses sont les + proches
	//pt_2d velocity(0.0, -1.0* rand_float(0.01, 0.1));
	// taille proportionnelle à la vitesse
	pt_2d velocity(0.0, -1.0* size* STAR_SIZE_VELOCITY_RATIO);
	glm::vec4 color(rand_float(0.01, 1.0), rand_float(0.01, 1.0), rand_float(0.01, 1.0), 1.0);
	_stars.push_back(new Star(pos, pt_2d(size, size), z, velocity, color, rand_int(0, _pngs.size()- 1)));
}


void StarSystem::sort_by_z() {
	std::sort(_stars.begin(), _stars.end(), [](Star * a, Star * b) {
		return a->_z< b->_z; 
	});
}


void StarSystem::anim(std::chrono::system_clock::time_point t) {
	for (auto star : _stars) {
		star->anim(t);
		if (star->_aabb._pos.y+ star->_aabb._size.y< _pt_min.y) {
			star->_aabb._pos.y= _pt_max.y;
		}
	}
}
