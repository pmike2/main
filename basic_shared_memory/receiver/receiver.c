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
	int * data= (int *)mmap(0, SIZE, PROT_READ, MAP_SHARED, fd, 0);

	// lecture des valeurs
	printf("%d %d %d\n", data[0], data[1], data[2]);

	// suppression du mapping
	munmap(data, SIZE);

	// fermeture file
	close(fd);
	
	// suppression shared memory object
	shm_unlink(NAME);
	
	return 0;
}
