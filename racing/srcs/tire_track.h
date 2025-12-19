#ifndef TIRE_TRACK_H
#define TIRE_TRACK_H

#include <chrono>

#include "car.h"
#include "geom_2d.h"
#include "typedefs.h"


// nombre max de traces de pneus
const unsigned int N_MAX_TIRE_TRACKS= 2000;
// écart en ms entre 2 créations de trace de pneu
const unsigned int TIRE_TRACKS_DELTA_T= 20;
// opacité initiale d'une trace
const number TIRE_TRACK_OPACITY_INIT= 0.9;
// décrément d'opacité
const number TIRE_TRACK_OPACITY_DECREMENT= 0.005;
// seuil de suppression
const number TIRE_TRACK_OPACITY_THRESHOLD= 0.05;
// facteur multiplicatif par rapport au scale de Car
const number TIRE_TRACK_SCALE_FACTOR= 0.3;


// trace de pneu
class TireTrack {
public:
	TireTrack();
	~TireTrack();
	void reinit(pt_2d position, number alpha, pt_2d scale, unsigned int idx_texture);
	void anim();


	BBox_2D * _bbox; // BBox englobante
	number _opacity; // opacité
	unsigned int _idx_texture; // à quel idx de png se réfère t'elle
	bool _is_alive; // active ?
};


// Ensemble de traces de pneus
class TireTrackSystem {
public:
	TireTrackSystem();
	~TireTrackSystem();
	void reinit();
	void anim(time_point t, std::vector<Car *> cars);


	TireTrack ** _tracks; // les traces
};

#endif

