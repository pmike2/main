/*
https://linuxhint.com/posix-shared-memory-c-programming/
*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <thread>

#include "keyseq.h"
#include "utile.h"



int main() {
	srand(time(NULL));

	Sequence * seq= new Sequence();

	for (unsigned int i=0; i<10; ++i) {
		seq->insert_event(rand_int(1, 100));
		std::this_thread::sleep_for(std::chrono::milliseconds(rand_int(10, 200)));
	}
	while (true) {
		seq->update();
	}
	
	delete seq;
	
	return 0;
}
