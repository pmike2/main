#include <iostream>
#include <chrono>

#include "receiver_test.h"

using namespace std;


// ------------------------------------------------------------------
ReceiverTest::ReceiverTest() {

}


ReceiverTest::~ReceiverTest() {

}


void ReceiverTest::note_on(unsigned int idx_track) {
	cout << "NOTE_ON : idx_track=" << idx_track << " ; key=" << _data[idx_track]._key << " ; t_start=" << time_print(_data[idx_track]._t_start) << " ; t_end=" << time_print(_data[idx_track]._t_end) << " ; amplitude=" << _data[idx_track]._amplitude << "\n";
}


void ReceiverTest::note_off(unsigned int idx_track) {
	//cout << "NOTE_OFF : idx_track=" << idx_track << " ; key=" << _data[idx_track]._key << " ; t_start=" << time_print(_data[idx_track]._t_start) << " ; t_end=" << time_print(_data[idx_track]._t_end) << " ; amplitude=" << _data[idx_track]._amplitude << "\n";
	cout << "NOTE_OFF : idx_track=" << idx_track << "\n";
}
