#ifndef BARRIER_H
#define BARRIER_H


#include "typedefs.h"
#include "bbox_2d.h"

#include "elevation.h"
#include "barrier_type.h"


struct Barrier : public InstancePosRot {
	Barrier();
	Barrier(BarrierType * type, pt_2d position, number orientation, Elevation * elevation);
	~Barrier();


	BarrierType * _type;
	number _life;
	Elevation * _elevation;
};


#endif

