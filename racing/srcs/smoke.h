#ifndef SMOKE_H
#define SMOKE_H

#include <chrono>

#include "geom_2d.h"
#include "car.h"
#include "typedefs.h"


const unsigned int N_SMOKES_PER_CAR= 200; // nombre de fumées par voiture
const unsigned int NEW_SMOKE_DELTA_MS= 20; // intervalle en ms de création d'une fumée
const number BUMP_SMOKE_THRESHOLD= 0.8; // à partir de quel niveau de bump on crée de la fumée sur le moteur

// config de fumée
struct SmokeConfig {
	number _size_init_factor; // facteur multiplicatif de taille initiale
	number _size_increment; // incrément de taille dans le temps
	number _opacity_init; // opacité initiale
	number _opacity_decrement; // décrément d'opacité
	number _opacity_threshold; // seuil d'opacité à partir duquel on supprime la fumée
	number _z_init; // z initial
	number _z_increment; // incrément en z
	number _dist_from_com; // distance création fumée par rapport au COM
};

// fumée pot d'échappement
const SmokeConfig EXHAUST       {0.1, 0.03, 0.8, 0.04, 0.02, -45.0, 0.01, -0.7};
// fumée moteur endommagé
const SmokeConfig ENGINE_BUMPED {0.05, 0.005, 0.8, 0.12, 0.02, -20.0, 0.01, 0.3};


class Smoke {
public:
	Smoke();
	~Smoke();
	void reinit(const SmokeConfig & config, pt_type position, number alpha, pt_type scale, unsigned int idx_texture);
	void anim();


	BBox_2D * _bbox;
	number _opacity;
	unsigned int _idx_texture;
	bool _is_alive;
	number _z;
	SmokeConfig _config;
};


class SmokeSystem {
public:
	SmokeSystem();
	SmokeSystem(Car * car, unsigned int n_pngs, std::chrono::system_clock::time_point t);
	~SmokeSystem();
	Smoke * get_free_smoke();
	void anim(std::chrono::system_clock::time_point t);
	

	Smoke ** _smokes;
	Car * _car;
	std::chrono::system_clock::time_point _last_exhaust_t;
	std::chrono::system_clock::time_point _last_bump_t;
	unsigned int _n_pngs;
};

#endif
