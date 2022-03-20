#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include <fstream>

#include "receiver_test.h"

using namespace std;


ReceiverTest * rt;


void my_handler(sig_atomic_t s) {
	//cout << "SIGNAL" << s << "\n";

	ofstream myfile;
	myfile.open("../data/receiver_test.txt");
	for (unsigned int i=0; i<N_DEBUG; ++i) {
		if (rt->_debug[i]!= time_type::zero()) {
			myfile << time_ms(rt->_debug[i]) << "\n";
		}
	}
	myfile.close();
	
	exit(1); 
}


int main() {
	signal(SIGINT, my_handler);

	rt= new ReceiverTest();

	while (true) {
		rt->update();
	}
	
	delete rt;
	
	return 0;
}
