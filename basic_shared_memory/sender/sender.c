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
	// O_CREAT : crée si n'existe pas ; O_EXCL : si existe déjà, erreur ; O_RDWR : read & write
	int fd= shm_open(NAME, O_CREAT | O_EXCL | O_RDWR, 0600);

	// set size
	ftruncate(fd, SIZE);

	// map file descriptor into memory
	// PROT_READ | PROT_WRITE : lecture & écriture
	// MAP_SHARED : d'autres processeurs auront accès à ce mapping
	int * data= (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	// écriture des valeurs
	data[0]= 0;
	data[1]= 1;
	data[2]= 2;

	// suppression du mapping
	munmap(data, SIZE);

	// fermeture file
	close(fd);
	
	return 0;
}
