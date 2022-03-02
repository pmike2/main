/*
https://linuxhint.com/posix-shared-memory-c-programming/
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "../protocol.h"


int main() {
	// create shared memory object; renvoie un file descriptor
	// O_RDONLY : lecture
	int fd= shm_open(NAME, O_RDONLY, 0666);

	// map file descriptor into memory
	// PROT_READ : lecture
	// MAP_SHARED : d'autres processeurs auront accès à ce mapping
	struct Test * data= (struct Test *)mmap(0, SIZE, PROT_READ, MAP_SHARED, fd, 0);

	// lecture des valeurs
	printf("%d %f\n", data[0]._i, data[0]._f);
	printf("%d %f\n", data[1]._i, data[1]._f);
	printf("%d %f\n", data[2]._i, data[2]._f);

	// suppression du mapping
	munmap(data, SIZE);

	// fermeture file
	close(fd);
	
	// suppression shared memory object
	shm_unlink(NAME);
	
	return 0;
}
