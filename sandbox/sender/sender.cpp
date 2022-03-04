/*
https://linuxhint.com/posix-shared-memory-c-programming/
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <iostream>
#include <chrono>
#include <thread>

#include "../protocol.h"
#include "keyseq.h"
#include "utile.h"


int main() {
	/*
	// create shared memory object; renvoie un file descriptor
	// O_CREAT : crée si n'existe pas ; O_EXCL : si existe déjà, erreur ; O_RDWR : read & write
	int fd= shm_open(NAME, O_CREAT | O_EXCL | O_RDWR, 0600);

	// set size
	ftruncate(fd, SIZE);

	// map file descriptor into memory
	// PROT_READ | PROT_WRITE : lecture & écriture
	// MAP_SHARED : d'autres processeurs auront accès à ce mapping
	struct Test * data= (struct Test *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	// écriture des valeurs
	data[0]._i= 0; data[0]._f= 0.0;
	data[1]._i= 1; data[1]._f= 0.1;
	data[2]._i= 2; data[2]._f= 0.2;

	// suppression du mapping
	munmap(data, SIZE);

	// fermeture file
	close(fd);
	*/

	/*using namespace std::chrono;

	system_clock::time_point tp_start= system_clock::now();
	for (unsigned int i=0; i<10000; i++) {
		system_clock::time_point tp= system_clock::now();
  		//system_clock::duration dtn = tp.time_since_epoch();
		  system_clock::duration dtn= tp- tp_start;
		
		std::cout << dtn.count() << "\n";
	}*/

	srand(time(NULL));

	Sequence * seq= new Sequence();

	for (unsigned int i=0; i<10; ++i) {
		seq->insert_event(rand_int(1, 100));
		std::this_thread::sleep_for(std::chrono::milliseconds(rand_int(10, 200)));
	}
	while (true) {
		seq->update();
	}

	/*seq->insert_event(12);
	seq->insert_event(23);
	seq->insert_event(23);*/
	
	//seq->update();
	
	delete seq;
	
	return 0;
}
