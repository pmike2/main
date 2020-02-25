
#ifndef VMAT_H
#define VMAT_H

#include <sys/time.h>
#include <iostream>

#include "utile.h"


class VMat{
public:
	VMat();
	void anim();
	void print();
	
	float mat[16];
	float alpha;
	
	bool is_active;
};


#endif
