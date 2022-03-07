#include <iostream>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "looper.h"

using namespace std;


string time_print(time_type t) {
	return to_string(chrono::duration_cast<chrono::milliseconds>(t).count())+ "ms";
}


// ------------------------------------------------------------------------
Event::Event() {
	set_null();
}


Event::~Event() {

}


void Event::set(time_type t, Event * previous, Event * next, key_type key) {
	_t_start= t;
	_t_end= time_type::zero();
	_previous= previous;
	_next= next;
	_key= key;
}


void Event::set_null() {
	_t_start= time_type::zero();
	_t_end= time_type::zero();
	_previous= 0;
	_next= 0;
	_key= NULL_KEY;
}


ostream & operator << (ostream & os, const Event & e) {
	os << "addr= " << &e << " ; ";
	os << "key= " << e._key << " ; ";
	os << "t_start= " << time_print(e._t_start) << " ; ";
	os << "t_end= " << time_print(e._t_end) << " ; ";
	os << "previous= " << e._previous << " ; ";
	os << "next= " << e._next;
	return os;
}


// ----------------------------------------------------------------
Track::Track() : /*_duration(time_type::zero()),*/ _duration(DEFAULT_TRACK_DURATION), _last_event(0), _current_cycle(0) {
	for (unsigned int i=0; i<N_MAX_EVENTS; ++i) {
		_events[i]= new Event();
	}
}


Track::~Track() {
	for (unsigned int i=0; i<N_MAX_EVENTS; ++i) {
		delete _events[i];
	}
}


time_type Track::get_relative_t(time_type t) {
	return t % _duration;
}


unsigned int Track::get_cycle(time_type t) {
	unsigned int cycle= t / _duration;
	return cycle;
}


void Track::set_duration(time_type duration) {
	_duration= duration;
}


Event * Track::get_first_null_event() {
	for (unsigned int i=0; i<N_MAX_EVENTS; ++i) {
		if (_events[i]->_key== NULL_KEY) {
			return _events[i];
		}
	}
	return 0;
}


Event * Track::get_first_not_null_event() {
	for (unsigned int i=0; i<N_MAX_EVENTS; ++i) {
		if (_events[i]->_key!= NULL_KEY) {
			return _events[i];
		}
	}
	return 0;
}


Event * Track::get_last_event_before(time_type t) {
	Event * res= get_first_event();
	if (res== 0) {
		return 0;
	}
	if (res->_t_start> t) {
		return 0;
	}
	while ((res->_next) && (res->_next->_t_start<= t)) {
		res= res->_next;
	}

	return res;
}


Event * Track::get_first_event_after(time_type t) {
	Event * res= get_last_event();
	if (res== 0) {
		return 0;
	}
	if (res->_t_start< t) {
		return 0;
	}
	while ((res->_previous) && (res->_previous->_t_start>= t)) {
		res= res->_previous;
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


Event * Track::get_last_event() {
	Event * res= get_first_not_null_event();
	if (res== 0) {
		return 0;
	}

	while (res->_next) {
		res= res->_next;
	}

	return res;
}


void Track::insert_event(key_type key, time_type t, bool hold) {
	//cout << "insert_event " << key << "\n";

	t= get_relative_t(t);

	Event * event2insert= get_first_null_event();
	if (event2insert== 0) {
		cout << "TRACK FULL !\n";
		return;
	}

	Event * event_before_t= get_last_event_before(t);
	Event * event_after_t= get_first_event_after(t);
	event2insert->set(t, event_before_t, event_after_t, key);
	if (event_before_t!= 0) {
		//cout << "before : " << *event_before_t << "\n";
		event_before_t->_next= event2insert;
	}
	if (event_after_t!= 0) {
		//cout << "after : " << *event_after_t << "\n";
		event_after_t->_previous= event2insert;
	}

	if (!hold) {
		event2insert->_t_end= event2insert->_t_start+ MIN_EVENT_DURATION;
	}
}


void Track::set_event_end(Event * event, time_type t) {
	t= get_relative_t(t);
	
	event->_t_end= t;
}


void Track::delete_event(Event * event) {
	//cout << "delete event\n";
	if (event->_previous) {
		if (event->_next) {
			event->_previous->_next= event->_next;
			event->_next->_previous= event->_previous;
		}
		else {
			event->_previous->_next= 0;
		}
	}
	else {
		if (event->_next) {
			event->_next->_previous= 0;
		}
		else {
			
		}
	}
	
	event->set_null();
}


void Track::update(time_type t) {
	unsigned int cycle= get_cycle(t);
	if (cycle!= _current_cycle) {
		_current_cycle= cycle;
		_last_event= 0;
	}

	t= get_relative_t(t);

	if (_last_event) {
		if (t> _last_event->_t_end) {
			//_last_event= 0;
		}
		else {
			while ((_last_event->_next) && (_last_event->_next->_t_start< t)) {
				delete_event(_last_event->_next);
			}
		}
		if ((_last_event->_next) && (t>= _last_event->_next->_t_start)) {
			_last_event= _last_event->_next;
			//cout << "Event(1) " << _last_event->_key << "\n";
		}
	}
	else {
		Event * first_event= get_first_event();
		if ((first_event) && (t>= first_event->_t_start)) {
			_last_event= first_event;
			//cout << "Event(2) " << _last_event->_key << "\n";
		}
	}
}


// ----------------------------------------------------------------
Sequence::Sequence() : _start_point(chrono::system_clock::now()) {
	for (unsigned int i=0; i<N_MAX_TRACKS; ++i) {
		_tracks[i]= new Track();
	}
	_current_track= _tracks[0];

	init_data2send();
}


Sequence::~Sequence() {
	close_data2send();
	for (unsigned int i=0; i<N_MAX_TRACKS; ++i) {
		delete _tracks[i];
	}
}


time_type Sequence::now() {
	chrono::system_clock::time_point tp= chrono::system_clock::now();
	return tp- _start_point;
}


void Sequence::insert_event(key_type key) {
	_current_track->insert_event(key, now());
}


void Sequence::update() {
	time_type now_time= now();
	for (unsigned int i=0; i<N_MAX_TRACKS; ++i) {
		_tracks[i]->update(now_time);
		if (_tracks[i]->_last_event!= 0) {
			_data2send[i]._key= _tracks[i]->_last_event->_key;
			_data2send[i]._duration= _tracks[i]->_last_event->_t_end- _tracks[i]->_last_event->_t_start;
			_data2send[i]._volume= 0; // TODO
		}
	}
}


void Sequence::set_track(Track * track) {
	_current_track= track;
}


void Sequence::init_data2send() {
	//https://linuxhint.com/posix-shared-memory-c-programming/

	// create shared memory object; renvoie un file descriptor
	// O_CREAT : crée si n'existe pas ; O_RDWR : read & write
	// O_EXCL permet de générer une erreur si l'objet existe déjà mais c'est pénible d'avoir à supprimer l'objet à la main
	_fd= shm_open(SHARED_MEM_OBJ_NAME.c_str(), O_CREAT | O_RDWR, 0600);
	if (_fd< 0) {
		perror("ERROR shm_open");
		return;
	}

	// set size
	ftruncate(_fd, DATA_SIZE);

	// map file descriptor into memory
	// PROT_READ | PROT_WRITE : lecture & écriture
	// MAP_SHARED : d'autres processeurs auront accès à ce mapping
	_data2send= (sharedata_type *)mmap(0, DATA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
	//cout << _data2send << "\n";

	for (unsigned i=0; i<N_MAX_TRACKS; ++i) {
		_data2send[i]._key= NULL_KEY;
		_data2send[i]._duration= time_type::zero();
		_data2send[i]._volume= 0;
	}
}


void Sequence::close_data2send() {
	// suppression du mapping
	munmap(_data2send, DATA_SIZE);

	// fermeture file
	close(_fd);

	// suppression shared memory object
	shm_unlink(SHARED_MEM_OBJ_NAME.c_str());
}


void Sequence::debug() {
	for (unsigned i=0; i<N_MAX_TRACKS; ++i) {
		for (unsigned j=0; j<N_MAX_EVENTS; ++j) {
			if (_tracks[i]->_events[j]->_key!= NULL_KEY) {
				cout << *_tracks[i]->_events[j] << "\n";
			}
		}
		break;
	}
}


// ------------------------------------------------------------------
Receiver::Receiver() {
	init_data2receive();

	_data2receive_current= new sharedata_type[N_MAX_TRACKS];
	for (unsigned i=0; i<N_MAX_TRACKS; ++i) {
		_data2receive_current[i]._key= NULL_KEY;
		_data2receive_current[i]._duration= time_type::zero();
		_data2receive_current[i]._volume= 0;
	}

}

Receiver::~Receiver() {
	close_data2receive();
	delete[] _data2receive_current;
}


void Receiver::init_data2receive() {
	// create shared memory object; renvoie un file descriptor
	// O_RDONLY : lecture
	_fd= shm_open(SHARED_MEM_OBJ_NAME.c_str(), O_RDONLY, 0666);

	// map file descriptor into memory
	// PROT_READ : lecture
	// MAP_SHARED : d'autres processeurs auront accès à ce mapping
	_data2receive= (sharedata_type *)mmap(0, DATA_SIZE, PROT_READ, MAP_SHARED, _fd, 0);
}


void Receiver::close_data2receive() {
	// suppression du mapping
	munmap(_data2receive, DATA_SIZE);

	// fermeture file
	close(_fd);
}


void Receiver::update() {
	for (unsigned i=0; i<N_MAX_TRACKS; ++i) {
		if ((_data2receive[i]._key!= NULL_KEY) && (_data2receive[i]._key!= _data2receive_current[i]._key)) {
			_data2receive_current[i]._key= _data2receive[i]._key;
			_data2receive_current[i]._duration= _data2receive[i]._duration;
			_data2receive_current[i]._volume= _data2receive[i]._volume;
			on_new_data(_data2receive_current[i]);
		}
	}
}

