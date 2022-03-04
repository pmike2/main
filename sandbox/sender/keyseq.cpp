#include <iostream>

#include "keyseq.h"

using namespace std;
using namespace std::chrono;


Event::Event() {
	set_null();
}


Event::~Event() {

}


void Event::set(system_clock::duration t, Event * previous, Event * next, key_type key) {
	_t= t;
	_previous= previous;
	_next= next;
	_key= key;
}


void Event::set_null() {
	_t= system_clock::duration::zero();
	_previous= 0;
	_next= 0;
	_key= 0;
}


// ----------------------------------------------------------------
Track::Track() : /*_duration(system_clock::duration::zero()),*/ _duration(DEFAULT_TRACK_DURATION), _last_event(0), _current_cycle(0) {
	for (unsigned int i=0; i<N_MAX_EVENTS; ++i) {
		_events[i]= new Event();
	}
}


Track::~Track() {
	for (unsigned int i=0; i<N_MAX_EVENTS; ++i) {
		delete _events[i];
	}
}


system_clock::duration Track::get_relative_t(system_clock::duration t) {
	return t % _duration;
}


unsigned int Track::get_cycle(system_clock::duration t) {
	unsigned int cycle= t / _duration;
	return cycle;
}


void Track::set_duration(system_clock::duration duration) {
	_duration= duration;
}


Event * Track::get_first_null_event() {
	for (unsigned int i=0; i<N_MAX_EVENTS; ++i) {
		if (_events[i]->_key== 0) {
			return _events[i];
		}
	}
	return 0;
}


Event * Track::get_first_not_null_event() {
	for (unsigned int i=0; i<N_MAX_EVENTS; ++i) {
		if (_events[i]->_key!= 0) {
			return _events[i];
		}
	}
	return 0;
}


Event * Track::get_last_event_before(system_clock::duration t) {
	/*Event * res= get_first_not_null_event();
	if (res== 0) {
		return 0;
	}
	
	t= get_relative_t(t);
	if (res->_t < t) {
		while ((res->_next) && (res->_next->_t <= t)) {
			res= res->_next;
		}
	}
	else {
		while ((res->_previous) && (res->_t > t)) {
			res= res->_previous;
		}
	}
	
	if (res->_t > t) {
		return 0;
	}*/

	Event * res= get_first_event();
	if (res== 0) {
		return 0;
	}
	while ((res->_next) && (res->_next->_t<= t)) {
		res= res->_next;
	}
	if (res->_t> t) {
		return 0;
	}

	return res;
}


Event * Track::get_first_event_after(system_clock::duration t) {
	/*Event * res= get_first_not_null_event();
	if (res== 0) {
		return 0;
	}

	t= get_relative_t(t);
	if (res->_t < t) {
		while ((res->_next) && (res->_t < t)) {
			res= res->_next;
		}
	}
	else {
		while ((res->_previous) && (res->_previous->_t >= t)) {
			res= res->_previous;
		}
	}

	if (res->_t < t) {
		return 0;
	}*/

	Event * res= get_last_event_before(t);
	if ((res== 0) || (res->_next== 0)) {
		return 0;
	}

	return res;
}


Event * Track::get_first_event() {
	Event * res= get_first_not_null_event();
	if (res== 0) {
		return 0;
	}

	while (res->_previous) {
		res= res->_previous;
	}

	return res;
}


void Track::insert_event(key_type key, system_clock::duration t) {
	//cout << "Insertion starts " << key << "\n";
	Event * event2insert= get_first_null_event();
	if (event2insert== 0) {
		cout << "TRACK FULL !\n";
		return;
	}

	Event * event_before_t= get_last_event_before(t);
	Event * event_after_t= get_first_event_after(t);
	event2insert->set(get_relative_t(t), event_before_t, event_after_t, key);
	if (event_before_t!= 0) {
		event_before_t->_next= event2insert;
	}
	if (event_after_t!= 0) {
		event_after_t->_previous= event2insert;
	}
	//cout << "Insertion ends " << key << "\n";
}


void Track::delete_event(Event * event) {
	event->set_null();
}


void Track::update(system_clock::duration t) {
	unsigned int cycle= get_cycle(t);
	if (cycle!= _current_cycle) {
		_current_cycle= cycle;
		_last_event= 0;
	}

	if (_last_event) {
		if (_last_event->_next) {
			if (get_relative_t(t)>= _last_event->_next->_t) {
				_last_event= _last_event->_next;
				cout << "Event " << _last_event->_key << "\n";
			}
		}
		else {
			//_last_event= 0;
		}
	}
	else {
		Event * first_event= get_first_event();
		//if ((first_event) && (_last_event!= first_event) {
		if ((first_event) && (get_relative_t(t)>= first_event->_t)) {
			_last_event= first_event;
			cout << "Event " << _last_event->_key << "\n";
		}
	}
}


// ----------------------------------------------------------------
Sequence::Sequence() {
	_start_point= system_clock::now();
	for (unsigned int i=0; i<N_MAX_TRACKS; ++i) {
		_tracks[i]= new Track();
	}
	_current_track= _tracks[0];
}


Sequence::~Sequence() {
	for (unsigned int i=0; i<N_MAX_TRACKS; ++i) {
		delete _tracks[i];
	}
}


system_clock::duration Sequence::now() {
	system_clock::time_point tp= system_clock::now();
	return tp- _start_point;
}


void Sequence::insert_event(key_type key) {
	_current_track->insert_event(key, now());
}


void Sequence::update() {
	for (unsigned int i=0; i<N_MAX_TRACKS; ++i) {
		_tracks[i]->update(now());
	}
}


void Sequence::set_track(Track * track) {
	_current_track= track;
}
