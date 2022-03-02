#ifndef PROTOCOL_H
#define PROTOCOL_H

#define NAME "/shmem-test"
#define SIZE (3 * sizeof(struct Test))


struct Test {
	int _i;
	float _f;
};

#endif
