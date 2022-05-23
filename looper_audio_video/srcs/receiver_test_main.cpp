#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>

#include "receiver_test.h"

using namespace std;


ReceiverTest * rt;


void interruption_handler(sig_atomic_t s) {
	delete rt;
	exit(1); 
}


int main() {
	signal(SIGINT, interruption_handler);

	rt= new ReceiverTest();

	while (true) {
		rt->update();
	}
	
	return 0;
}
