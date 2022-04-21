#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "looper.h"

using namespace std;



bool operator== (const sharedata_type& x, const sharedata_type& y) {
	//return ((x._key== y._key) && (x._t_start== y._t_start) && (x._t_end== y._t_end) && (x._amplitude== y._amplitude));
	return ((x._key== y._key) && (x._t_start== y._t_start) && (x._amplitude== y._amplitude));
}


bool operator!= (const sharedata_type& x, const sharedata_type& y) {
	//return ((x._key!= y._key) || (x._t_start!= y._t_start) || (x._t_end!= y._t_end) || (x._amplitude!= y._amplitude));
	return ((x._key!= y._key) || (x._t_start!= y._t_start) || (x._amplitude!= y._amplitude));
}


ostream & operator << (ostream & os, const sharedata_type & e) {
	os << "key= " << e._key << " ; ";
	os << "t_start= " << time_print(e._t_start) << " ; ";
	os << "t_end= " << time_print(e._t_end) << " ; ";
	os << "amplitude= " << e._amplitude;
	return os;
}


sharedata_type& sharedata_type::operator=(const sharedata_type& other) {
	_key= other._key;
	_t_start= other._t_start;
	_t_end= other._t_end;
	_amplitude= other._amplitude;
	return *this;
}


void sharedata_type::set_null() {
	_key= NULL_KEY;
	_t_start= time_type::zero();
	_t_end= time_type::zero();
	_amplitude= NULL_AMPLITUDE;
}


bool sharedata_type::is_null() {
	return _key== NULL_KEY;
}


// ------------------------------------------------------------------------
unsigned int time_ms(time_type t) {
	return chrono::duration_cast<chrono::milliseconds>(t).count();
}


string time_print(time_type t) {
	return to_string(time_ms(t))+ "ms";
}


// ------------------------------------------------------------------------
Event::Event() {
	set_null();
}


Event::~Event() {

}


void Event::set(key_type key, time_type t, amplitude_type amplitude, Event * previous, Event * next) {
	if (VERBOSE) {
		cout << "Event::set\n";
	}
	_data._key= key;
	_data._t_start= t;
	_data._t_end= t;
	_data._amplitude= amplitude;
	_previous= previous;
	_next= next;
}


void Event::set_end(time_type t) {
	if (VERBOSE) {
		cout << "Event::set_end : " << *this << "\n";
	}
	_data._t_end= t;
}


void Event::set_null() {
	/*if (VERBOSE) {
		cout << "Event::set_null\n";
	}*/
	_data.set_null();
	_previous= 0;
	_next= 0;
}


bool Event::is_null() {
	/*if (VERBOSE) {
		cout << "Event::is_null\n";
	}*/
	return _data.is_null();
}


ostream & operator << (ostream & os, const Event & e) {
	os << "addr= " << &e << " ; ";
	os << "data= " << e._data << " ; ";
	os << "previous= " << e._previous << " ; ";
	os << "next= " << e._next << " ; ";
	return os;
}


// ----------------------------------------------------------------
Track::Track() {

}


Track::Track(sharedata_type * track_data, bool infinite) :
	_current_event(0), _inserted_event(0), _current_cycle(0), _next(0), _previous(0), _data(track_data), _infinite(infinite),
	_quantize(0), _quantize_step(time_type::zero()), _current_quantize(0), _repeat(false), _hold(false), _ratio_to_master_track(1, 1)
{
	if (_infinite) {
		_duration= time_type::zero();
	}
	else {
		_duration= DEFAULT_TRACK_DURATION;
	}

	for (unsigned int idx_event=0; idx_event<N_MAX_EVENTS; ++idx_event) {
		_events[idx_event]= new Event();
	}
}


Track::~Track() {
	for (unsigned int idx_event=0; idx_event<N_MAX_EVENTS; ++idx_event) {
		delete _events[idx_event];
	}
}


time_type Track::get_relative_t(time_type t) {
	if (_infinite) {
		return t;
	}

	return t % _duration;
}


unsigned int Track::get_cycle(time_type t) {
	if (_infinite) {
		return 0;
	}

	unsigned int cycle= t / _duration;
	return cycle;
}


void Track::set_duration(time_type t) {
	if (VERBOSE) {
		cout << "Track::set_duration\n";
	}
	Event * e= get_first_event_after(t);
	while (e) {
		Event * e_next= e->_next;
		delete_event(e);
		e= e_next;
	}
	_duration= t;
}


void Track::set_ratio_to_master_track(Track * master_track, ratio_type ratio) {
	if (VERBOSE) {
		cout << "Track::set_ratio_to_master_track\n";
	}
	if (master_track== this) {
		cout << "set_ratio_to_master_track sur master track impossible\n";
		return;
	}
	_ratio_to_master_track= ratio;
	update_duration_from_ratio_to_master_track(master_track);
}


void Track::update_duration_from_ratio_to_master_track(Track * master_track) {
	float t_f= (float)(time_ms(master_track->_duration)* _ratio_to_master_track.first)/ (float)(_ratio_to_master_track.second);
	time_type t= std::chrono::milliseconds((unsigned int)(t_f));
	set_duration(t);
}


Event * Track::get_first_null_event() {
	/*if (VERBOSE) {
		cout << "Track::get_first_null_event\n";
	}*/
	for (unsigned int idx_event=0; idx_event<N_MAX_EVENTS; ++idx_event) {
		if (_events[idx_event]->is_null()) {
			return _events[idx_event];
		}
	}
	return 0;
}


Event * Track::get_first_not_null_event() {
	/*if (VERBOSE) {
		cout << "Track::get_first_not_null_event\n";
	}*/
	for (unsigned int idx_event=0; idx_event<N_MAX_EVENTS; ++idx_event) {
		if (!_events[idx_event]->is_null()) {
			return _events[idx_event];
		}
	}
	return 0;
}


Event * Track::get_last_event_before(time_type t) {
	if (VERBOSE) {
		cout << "Track::get_last_event_before\n";
	}
	Event * res= get_first_event();
	if (res== 0) {
		return 0;
	}
	if (res->_data._t_start> t) {
		return 0;
	}
	while ((res->_next) && (res->_next->_data._t_start<= t)) {
		res= res->_next;
	}

	return res;
}


Event * Track::get_first_event_after(time_type t) {
	if (VERBOSE) {
		cout << "Track::get_first_event_after\n";
	}
	Event * res= get_last_event();
	if (res== 0) {
		return 0;
	}
	if (res->_data._t_start< t) {
		return 0;
	}
	while ((res->_previous) && (res->_previous->_data._t_start>= t)) {
		res= res->_previous;
	}

	return res;
}


Event * Track::get_event_at(time_type t) {
	if (VERBOSE) {
		cout << "Track::get_event_at\n";
	}
	for (unsigned int idx_event=0; idx_event<N_MAX_EVENTS; ++idx_event) {
		if (_events[idx_event]->_data._t_start== t) {
			return _events[idx_event];
		}
	}
	return 0;
}


Event * Track::get_first_event() {
	/*if (VERBOSE) {
		cout << "Track::get_first_event\n";
	}*/
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
	if (VERBOSE) {
		cout << "Track::get_last_event\n";
	}
	Event * res= get_first_not_null_event();
	if (res== 0) {
		return 0;
	}

	while (res->_next) {
		res= res->_next;
	}

	return res;
}


void Track::insert_event(key_type key, time_type t, amplitude_type amplitude) {
	if (VERBOSE) {
		cout << "Track::insert_event\n";
	}

	t= get_relative_t(t);

	if (_inserted_event) {
		set_inserted_event_end(t);
	}

	// calage au pas de quantisation précédent
	if ((!_infinite) && (_quantize> 0)) {
		t= _quantize_step* _current_quantize;
	}

	Event * event_at_t= get_event_at(t);
	if (event_at_t) {
		delete_event(event_at_t);
	}

	_inserted_event= get_first_null_event();
	if (!_inserted_event) {
		cout << "TRACK FULL !\n";
		return;
	}

	Event * event_before_t= get_last_event_before(t);
	Event * event_after_t= get_first_event_after(t);

	_inserted_event->set(key, t, amplitude, event_before_t, event_after_t);
	
	if (event_before_t) {
		event_before_t->_next= _inserted_event;

		if (event_before_t->_data._t_end>= t) {
			event_before_t->set_end(t- chrono::milliseconds(1));
		}
	}
	if (event_after_t) {
		event_after_t->_previous= _inserted_event;
	}

	if (!_hold) {
		set_inserted_event_end(t+ DEFAULT_EVENT_DURATION);
	}
}


void Track::set_inserted_event_end(time_type t) {
	if (VERBOSE) {
		cout << "Track::set_inserted_event_end\n";
	}
	if (!_inserted_event) {
		cout << "Track::set_inserted_event_end ERROR _inserted_event = 0\n";
		return;
	}
	_inserted_event->set_end(t);
	_inserted_event= 0;
}


void Track::delete_event(Event * event) {
	if (VERBOSE) {
		cout << "Track::delete_event\n";
	}
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
	/*if (VERBOSE) {
		cout << "Track::update\n";
	}*/
	unsigned int cycle= get_cycle(t);
	if (cycle!= _current_cycle) {
		_current_cycle= cycle;

		if (_inserted_event) {
			set_inserted_event_end(_duration- DEFAULT_EVENT_MARGIN);
		}

		_current_event= 0;
	}

	t= get_relative_t(t);

	if (_inserted_event) {
		while ((_inserted_event->_next) && (_inserted_event->_next->_data._t_start<= t)) {
			delete_event(_inserted_event->_next);
		}

		if ((update_current_quantize(t)) && (_repeat)) {
			key_type key= _inserted_event->_data._key;
			amplitude_type amplitude= _inserted_event->_data._amplitude;
			set_inserted_event_end(_current_quantize* _quantize_step- DEFAULT_EVENT_MARGIN);
			insert_event(key, _current_quantize* _quantize_step, amplitude);
		}
		else {
			_inserted_event->set_end(t);
		}
		
		_current_event= _inserted_event;
		emit_current();
	}
	else {
		update_current_quantize(t);

		if (!_current_event) {
			Event * first_event= get_first_event();
			if ((first_event) && (t>= first_event->_data._t_start)) {
				_current_event= first_event;
			}
		}

		if (!_current_event) {
			emit_null();
		}
		else {
			while ((_current_event->_next) && (t>= _current_event->_next->_data._t_start)) {
				_current_event= _current_event->_next;
			}

			if (t> _current_event->_data._t_end) {
				emit_null();
			}
			else {
				emit_current();
			}
		}
	}
}


void Track::set_quantize(unsigned int quantize) {
	_quantize= quantize;
	if (_quantize> 0) {
		_quantize_step= _duration/ _quantize;
	}
	else {
		_quantize_step= time_type::zero();
	}
}


bool Track::update_current_quantize(time_type t) {
	unsigned int _current_quantize_old= _current_quantize;
	if (_quantize) {
		for (unsigned int i=0; i<_quantize; ++i) {
			if (_quantize_step* (i+ 1)> t) {
				_current_quantize= i;
				break;
			}
		}
		if (_current_quantize!= _current_quantize_old) {
			return true;
		}
	}
	return false;
}


void Track::emit_current() {
	/*if (VERBOSE) {
		cout << "Track::emit_current\n";
	}*/
	
	_data->_key      = _current_event->_data._key;
	_data->_t_start  = _current_event->_data._t_start;
	_data->_t_end    = _current_event->_data._t_end;
	_data->_amplitude= _current_event->_data._amplitude;
}


void Track::emit_null() {
	/*if (VERBOSE) {
		cout << "Track::emit_null\n";
	}*/
	
	_data->_key      = NULL_KEY;
	_data->_t_start  = time_type::zero();
	_data->_t_end    = time_type::zero();
	_data->_amplitude= NULL_AMPLITUDE;
}


void Track::clear() {
	if (VERBOSE) {
		cout << "Track::clear\n";
	}
	for (unsigned int idx_event=0; idx_event<N_MAX_EVENTS; ++idx_event) {
		_events[idx_event]->set_null();
	}
}


// ----------------------------------------------------------------
Sequence::Sequence() : _start_point(chrono::system_clock::now()), _mode(RECORDING) {
	init_data();

	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		bool infinite= false;
		if (idx_track== 0) {
			infinite= true;
		}
		_tracks[idx_track]= new Track(&_data[idx_track], infinite);
	}

	for (unsigned int idx_track=1; idx_track<N_MAX_TRACKS; ++idx_track) {
		if (idx_track> 1) {
			_tracks[idx_track]->_previous= _tracks[idx_track- 1];
		}
		if (idx_track< N_MAX_TRACKS- 1) {
			_tracks[idx_track]->_next= _tracks[idx_track+ 1];
		}
	}

	_current_track= _tracks[1];
}


Sequence::~Sequence() {
	close_data();
	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		delete _tracks[idx_track];
	}
}


time_type Sequence::now() {
	chrono::system_clock::time_point tp= chrono::system_clock::now();
	return tp- _start_point;
}


void Sequence::note_on(key_type key, amplitude_type amplitude) {
	if (VERBOSE) {
		cout << "Sequence::note_on\n";
	}

	/*if (_current_track->_inserted_event) {
		note_off();
	}*/

	if (_mode== RECORDING) {
		_current_track->insert_event(key, now(), amplitude);
	}
	else {
		_tracks[0]->clear();
		_tracks[0]->insert_event(key, now(), amplitude);
	}

	update();
}


void Sequence::note_off() {
	if (VERBOSE) {
		cout << "Sequence::note_off\n";
	}

	if (_mode== RECORDING) {
		if (_current_track->_inserted_event) {
			_current_track->set_inserted_event_end(_current_track->get_relative_t(now()));
		}
	}
	else {
		if (_tracks[0]->_inserted_event) {
			_tracks[0]->set_inserted_event_end(now());
		}
	}

	update();
}


void Sequence::update() {
	/*if (VERBOSE) {
		cout << "Sequence::update\n";
	}*/

	time_type now_time= now();

	if (_mode== RUNNING) {
		for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
			_tracks[idx_track]->update(now_time);
		}
	}
	else if (_mode== RECORDING) {
		for (unsigned int idx_track=1; idx_track<N_MAX_TRACKS; ++idx_track) {
			_tracks[idx_track]->update(now_time);
		}
	}
	else if (_mode== STOPPED) {
		_tracks[0]->update(now_time);
	}
}


void Sequence::clear() {
	if (VERBOSE) {
		cout << "Sequence::clear\n";
	}

	for (unsigned int idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		_tracks[idx_track]->clear();
	}
}


void Sequence::toggle_start() {
	if (VERBOSE) {
		cout << "Sequence::toggle_start\n";
	}

	if ((_mode== RUNNING) || (_mode== RECORDING)) {
		_mode= STOPPED;
	}
	else if (_mode== STOPPED) {
		_mode= RUNNING;
		_start_point= chrono::system_clock::now();
	}

	_tracks[0]->clear();
}


void Sequence::toggle_record() {
	if (VERBOSE) {
		cout << "Sequence::toggle_record\n";
	}

	if (_mode== RECORDING) {
		_mode= RUNNING;
	}
	else if (_mode== RUNNING) {
		_mode= RECORDING;
	}
}


void Sequence::set_next_track() {
	if (VERBOSE) {
		cout << "Sequence::set_next_track\n";
	}

	if (_current_track->_next) {
		_current_track= _current_track->_next;
	}
}


void Sequence::set_previous_track() {
	if (VERBOSE) {
		cout << "Sequence::set_previous_track\n";
	}

	if (_current_track->_previous) {
		_current_track= _current_track->_previous;
	}
}


void Sequence::set_master_track_duration(time_type t) {
	if (VERBOSE) {
		cout << "Sequence::set_master_track_duration\n";
	}

	_tracks[1]->set_duration(t);
	for (unsigned int idx_track=2; idx_track<N_MAX_TRACKS; ++idx_track) {
		_tracks[idx_track]->update_duration_from_ratio_to_master_track(_tracks[1]);
	}
}


void Sequence::init_data() {
	//https://linuxhint.com/posix-shared-memory-c-programming/

	// create shared memory object; renvoie un file descriptor
	// O_CREAT : crée si n'existe pas ; O_RDWR : read & write
	// O_EXCL permet de générer une erreur si l'objet existe déjà mais c'est pénible d'avoir à supprimer l'objet à la main
	_fd= shm_open(SHARED_MEM_OBJ_NAME.c_str(), O_CREAT | O_RDWR, 0600);
	if (_fd< 0) {
		perror("Sequence : ERROR shm_open");
		return;
	}

	// set size
	ftruncate(_fd, DATA_SIZE);

	// map file descriptor into memory
	// PROT_READ | PROT_WRITE : lecture & écriture
	// MAP_SHARED : d'autres processeurs auront accès à ce mapping
	_data= (sharedata_type *)mmap(0, DATA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);

	for (unsigned idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		_data[idx_track].set_null();
	}
}


void Sequence::close_data() {
	// suppression du mapping
	munmap(_data, DATA_SIZE);

	// fermeture file
	close(_fd);

	// suppression shared memory object
	shm_unlink(SHARED_MEM_OBJ_NAME.c_str());
}


void Sequence::debug() {
	for (unsigned idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		for (unsigned idx_event=0; idx_event<N_MAX_EVENTS; ++idx_event) {
			if (!_tracks[idx_track]->_events[idx_event]->is_null()) {
				cout << "track " << idx_track << " : " << *_tracks[idx_track]->_events[idx_event] << "\n";
			}
		}
	}
}


// ------------------------------------------------------------------
Receiver::Receiver() {
	init_data();

	_data_current= new sharedata_type[N_MAX_TRACKS];
	for (unsigned idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		_data_current[idx_track].set_null();
	}

	if (DEBUG) {
		for (unsigned int i=0; i<N_DEBUG; ++i) {
			_debug[i]= time_type::zero();
		}
		_debug_start_point= chrono::system_clock::now();
		// _debug_path doit être spécifié dans les classes filles 
		_debug_path= "";
	}
}

Receiver::~Receiver() {
	close_data();
	delete[] _data_current;

	if (DEBUG) {
		if (_debug_path.empty()) {
			cout << "_debug_path n'est pas précisé\n";
			return;
		}
		ofstream myfile;
		myfile.open(_debug_path);
		for (unsigned int i=0; i<N_DEBUG; ++i) {
			if (_debug[i]!= time_type::zero()) {
				myfile << time_ms(_debug[i]) << "\n";
			}
		}
		myfile.close();
	}
}


void Receiver::init_data() {
	// create shared memory object; renvoie un file descriptor
	// O_RDONLY : lecture
	// le while sert ici à retenter tant que le shm_open de looper n'a pas été créé
	_fd= -1;
	while (_fd< 0) {
		_fd= shm_open(SHARED_MEM_OBJ_NAME.c_str(), O_RDONLY, 0666);
	}

	// map file descriptor into memory
	// PROT_READ : lecture
	// MAP_SHARED : d'autres processeurs auront accès à ce mapping
	_data= (sharedata_type *)mmap(0, DATA_SIZE, PROT_READ, MAP_SHARED, _fd, 0);
}


void Receiver::close_data() {
	// suppression du mapping
	munmap(_data, DATA_SIZE);

	// fermeture file
	close(_fd);
}


void Receiver::update() {
	for (unsigned idx_track=0; idx_track<N_MAX_TRACKS; ++idx_track) {
		if (_data_current[idx_track]!= _data[idx_track]) {
			if (!_data_current[idx_track].is_null()) {
				note_off(idx_track);
			}

			_data_current[idx_track]= _data[idx_track];

			if (!_data_current[idx_track].is_null()) {
				note_on(idx_track);
			}
		}
	}
}
