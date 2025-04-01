#ifndef DRIVER_H
#define DRIVER_H

#include <string>

//#include "car.h"


class Driver {
public:
	Driver();
	Driver(std::string name, float idx_texture_face);
	~Driver();


	//Car * _car;
	std::string _name;
	float _idx_texture_face;
};


#endif
