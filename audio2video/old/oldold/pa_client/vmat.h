
#ifndef VMAT_H
#define VMAT_H

#include <sys/time.h>
#include <iostream>

#include "utile.h"
#include "config.h"


class VMat{
public:
	VMat(struct timeval t_);
	void anim();
	void set_config(VMatConfig * config_);
	void print();
	
	VMatMorph current_values;
	VMatConfig * config;
	bool is_active;
	struct timeval t;
};


#endif
