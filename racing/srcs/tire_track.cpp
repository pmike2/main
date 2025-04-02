#include "tire_track.h"


// TireTrack ---------------------------------------------------------------------------------
TireTrack::TireTrack() : _is_alive(false), _opacity(0.0), _idx_texture(0) {
	_bbox= new BBox_2D();
}


TireTrack::~TireTrack() {
	delete _bbox;
}


void TireTrack::reinit(pt_type position, number alpha, pt_type scale, unsigned int idx_texture) {
	_bbox->_center= position;
	_bbox->_alpha= alpha;
	_bbox->_half_size= scale;
	_bbox->update();

	_opacity= TIRE_TRACK_OPACITY_INIT;
	_idx_texture= idx_texture;
	_is_alive= true;
}


void TireTrack::anim() {
	if (!_is_alive) {
		return;
	}

	_opacity-= TIRE_TRACK_OPACITY_DECREMENT;
	if (_opacity< TIRE_TRACK_OPACITY_THRESHOLD) {
		_opacity= 0.0;
		_is_alive= false;
	}
}


// TireTrackSystem ----------------------------------------------------------------------------
TireTrackSystem::TireTrackSystem() {
	_tracks= new TireTrack *[N_MAX_TIRE_TRACKS];
	for (unsigned int i=0; i<N_MAX_TIRE_TRACKS; ++i) {
		_tracks[i]= new TireTrack();
	}
}


TireTrackSystem::~TireTrackSystem() {
	for (unsigned int i=0; i<N_MAX_TIRE_TRACKS; ++i) {
		delete _tracks[i];
	}
	delete[] _tracks;
}


void TireTrackSystem::reinit() {
	for (unsigned int i=0; i<N_MAX_TIRE_TRACKS; ++i) {
		_tracks[i]->_is_alive= false;
	}
}


void TireTrackSystem::anim(time_point t, std::vector<Car *> cars) {
	for (unsigned int i=0; i<N_MAX_TIRE_TRACKS; ++i) {
		_tracks[i]->anim();
	}

	for (auto car : cars) {
		if (!car->_drift && car->_previous_surface->_name== "road") {
			continue;
		}

		auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- car->_last_drift_t).count();
		if (dt> TIRE_TRACKS_DELTA_T) {
			TireTrack * tire_track= NULL;
			for (unsigned int i=0; i<N_MAX_TIRE_TRACKS; ++i) {
				if (!_tracks[i]->_is_alive) {
					tire_track= _tracks[i];
					break;
				}
			}
			if (tire_track!= NULL) {
				tire_track->reinit(car->_com, car->_alpha, TIRE_TRACK_SCALE_FACTOR* car->_scale, car->_tire_track_texture_idx);
				car->_last_drift_t= t;
			}
			else {
				std::cout << "TireTrackSystem saturé\n";
			}
		}
	}
}
