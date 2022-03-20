#include <iostream>
#include <chrono>

#include "receiver_test.h"

using namespace std;


// ------------------------------------------------------------------
ReceiverTest::ReceiverTest() {
	for (unsigned int i=0; i<N_DEBUG; ++i) {
		_debug[i]= time_type::zero();
	}
	_debug_start_point= chrono::system_clock::now();
}


ReceiverTest::~ReceiverTest() {

}


void ReceiverTest::note_on(unsigned int idx_track) {
	//cout << "NOTE_ON : idx_track=" << idx_track << " ; key=" << _data[idx_track]._key << " ; t_start=" << time_print(_data[idx_track]._t_start) << " ; t_end=" << time_print(_data[idx_track]._t_end) << " ; amplitude=" << _data[idx_track]._amplitude << "\n";
	//_debug[_compt_debug++]= _data[idx_track]._t_start;
	time_type t= chrono::system_clock::now()- _debug_start_point;
	_debug[_compt_debug++]= t;

	if (_compt_debug>= N_DEBUG) {
		_compt_debug= 0;
	}
}


void ReceiverTest::note_off(unsigned int idx_track) {
	//cout << "NOTE_OFF : idx_track=" << idx_track << "\n";
}
