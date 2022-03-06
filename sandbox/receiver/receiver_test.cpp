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
	cout << "key=" << data._key << " ; duration=" << chrono::duration_cast<chrono::milliseconds>(data._duration).count() << " ms ; volume=" << data._volume << "\n";
}
