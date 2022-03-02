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
	int fd= shm_open(NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
	ftruncate(fd, SIZE);
	int * data= (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	data[0]= 0;
	data[1]= 1;
	data[2]= 2;

	munmap(data, SIZE);
	close(fd);
	return 0;
}
