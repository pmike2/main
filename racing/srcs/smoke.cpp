#include "utile.h"
#include "smoke.h"


// Smoke ------------------------------------------------------------------------------------
Smoke::Smoke() : _is_alive(false), _opacity(0.0), _idx_texture(0), _z(0.0), _config(EXHAUST) {
	_bbox= new BBox_2D();
}


Smoke::~Smoke() {

}


void Smoke::reinit(const SmokeConfig & config, pt_2d position, number alpha, pt_2d scale, uint idx_texture) {
	_config= config;
	_bbox->_center= position;
	_bbox->_alpha= alpha;
	_bbox->_half_size= scale;
	_bbox->update();

	_opacity= _config._opacity_init;
	_idx_texture= idx_texture;
	_z= _config._z_init;
	_is_alive= true;
}


void Smoke::anim() {
	if (!_is_alive) {
		return;
	}

	_bbox->_half_size+= _config._size_increment;
	_bbox->update();
	_z+= _config._z_increment;
	
	_opacity-= _config._opacity_decrement;
	if (_opacity< _config._opacity_threshold) {
		_opacity= 0.0;
		_is_alive= false;
	}
}


// SmokeSystem ------------------------------------------------------------------------------
SmokeSystem::SmokeSystem() {

}


SmokeSystem::SmokeSystem(Car * car, uint n_pngs, time_point t) :
	 _car(car), _n_pngs(n_pngs), _last_exhaust_t(t), _last_bump_t(t)
{
	_smokes= new Smoke *[N_SMOKES_PER_CAR];
	for (uint i=0; i<N_SMOKES_PER_CAR; ++i) {
		_smokes[i]= new Smoke();
	}
}


SmokeSystem::~SmokeSystem() {
	for (uint i=0; i<N_SMOKES_PER_CAR; ++i) {
		delete _smokes[i];
	}
	delete[] _smokes;
}


Smoke * SmokeSystem::get_free_smoke() {
	Smoke * smoke= NULL;
	for (uint i=0; i<N_SMOKES_PER_CAR; ++i) {
		if (!_smokes[i]->_is_alive) {
			smoke= _smokes[i];
			break;
		}
	}
	return smoke;
}


void SmokeSystem::anim(time_point t) {
	for (uint i=0; i<N_SMOKES_PER_CAR; ++i) {
		_smokes[i]->anim();
	}

	auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_exhaust_t).count();
	if (dt> NEW_SMOKE_DELTA_MS) {
		Smoke * smoke= get_free_smoke();
		if (smoke!= NULL) {
			_last_exhaust_t= t;
			number smoke_size= EXHAUST._size_init_factor* sqrt(_car->_velocity.x* _car->_velocity.x+ _car->_velocity.y* _car->_velocity.y);
			smoke->reinit(EXHAUST, _car->_com+ EXHAUST._dist_from_com* _car->_forward, _car->_alpha, pt_2d(smoke_size, smoke_size), rand_int(0, _n_pngs- 1));
		}
		else {
			std::cout << "SmokeSystem saturé\n";
		}
	}

	if (_car->_bumps[4]+ _car->_bumps[5]> BUMP_SMOKE_THRESHOLD) {
		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_bump_t).count();
		if (dt> NEW_SMOKE_DELTA_MS) {
			Smoke * smoke= get_free_smoke();
			if (smoke!= NULL) {
				_last_bump_t= t;
				number smoke_size= ENGINE_BUMPED._size_init_factor* (_car->_bumps[4]+ _car->_bumps[5]);
				smoke->reinit(ENGINE_BUMPED, _car->_com+ ENGINE_BUMPED._dist_from_com* _car->_forward, _car->_alpha, pt_2d(smoke_size, smoke_size), rand_int(0, _n_pngs- 1));
			}
			else {
				std::cout << "SmokeSystem saturé\n";
			}
		}
	}
}

