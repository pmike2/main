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
	cout << "key=" << data._key << " ; t_start=" << time_print(data._t_start) << " ; t_end=" << time_print(data._t_end) << " ; amplitude=" << data._amplitude << "\n";
}
