#include "utile.h"
#include "smoke.h"


// Smoke ------------------------------------------------------------------------------------
Smoke::Smoke() : _is_alive(false), _opacity(0.0), _idx_texture(0), _z(0.0) {
	_bbox= new BBox_2D();
}


Smoke::~Smoke() {

}


void Smoke::reinit(pt_type position, number alpha, pt_type scale, unsigned int idx_texture) {
	_bbox->_center= position;
	_bbox->_alpha= alpha;
	_bbox->_half_size= scale;
	_bbox->update();

	_opacity= SMOKE_OPACITY_INIT;
	_idx_texture= idx_texture;
	_z= SMOKE_Z_INIT;
	_is_alive= true;
}


void Smoke::anim() {
	if (!_is_alive) {
		return;
	}

	_bbox->_half_size+= SMOKE_SIZE_INCREMENT;
	_bbox->update();
	_z+= SMOKE_Z_INCREMENT;
	
	_opacity-= SMOKE_OPACITY_DECREMENT;
	if (_opacity< SMOKE_OPACITY_THRESHOLD) {
		_opacity= 0.0;
		_is_alive= false;
	}
}


// SmokeSystem ------------------------------------------------------------------------------
SmokeSystem::SmokeSystem() {

}


SmokeSystem::SmokeSystem(Car * car, unsigned int n_pngs, std::chrono::system_clock::time_point t) :
	 _car(car), _n_pngs(n_pngs), _last_creation_t(t)
{
	_smokes= new Smoke *[N_SMOKES_PER_CAR];
	for (unsigned int i=0; i<N_SMOKES_PER_CAR; ++i) {
		_smokes[i]= new Smoke();
	}
}


SmokeSystem::~SmokeSystem() {
	for (unsigned int i=0; i<N_SMOKES_PER_CAR; ++i) {
		delete _smokes[i];
	}
	delete[] _smokes;
}


void SmokeSystem::anim(std::chrono::system_clock::time_point t) {
	for (unsigned int i=0; i<N_SMOKES_PER_CAR; ++i) {
		_smokes[i]->anim();
	}
	auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_creation_t).count();
	if (dt> NEW_SMOKE_DELTA_MS) {
		Smoke * smoke= NULL;
		for (unsigned int i=0; i<N_SMOKES_PER_CAR; ++i) {
			if (!_smokes[i]->_is_alive) {
				smoke= _smokes[i];
				break;
			}
		}
		if (smoke!= NULL) {
			_last_creation_t= t;
			number smoke_size= SMOKE_SIZE_INIT_FACTOR* sqrt(_car->_velocity.x* _car->_velocity.x+ _car->_velocity.y* _car->_velocity.y);
			smoke->reinit(_car->_com- SMOKE_DIST_FROM_COM* _car->_forward, _car->_alpha, pt_type(smoke_size, smoke_size), rand_int(0, _n_pngs- 1));
		}
	}
}

