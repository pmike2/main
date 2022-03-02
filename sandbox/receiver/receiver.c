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
	int fd= shm_open(NAME, O_RDONLY, 0666);
	int * data= (int *)mmap(0, SIZE, PROT_READ, MAP_SHARED, fd, 0);
	printf("%d %d %d\n", data[0], data[1], data[2]);

	munmap(data, SIZE);
	close(fd);
	shm_unlink(NAME);
	return 0;
}
