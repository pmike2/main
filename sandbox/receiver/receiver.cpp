/*
https://linuxhint.com/posix-shared-memory-c-programming/
*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "receiver_test.h"



int main() {
	ReceiverTest * rt= new ReceiverTest();

	while (true) {
		rt->update();
	}
	
	delete rt;
	
	return 0;
}
