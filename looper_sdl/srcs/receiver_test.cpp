#include <iostream>
#include <chrono>

#include "receiver_test.h"

using namespace std;


// ------------------------------------------------------------------
ReceiverTest::ReceiverTest() {

}


ReceiverTest::~ReceiverTest() {

}


void ReceiverTest::on_new_data(sharedata_type data) {
	cout << "key=" << data._key << " ; duration=" << time_print(data._duration) << " ; volume=" << data._volume << "\n";
}
