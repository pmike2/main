#ifndef ELEMENT_H
#define ELEMENT_H

#include "typedefs.h"
#include "bbox.h"

#include "../elevation.h"


enum ELEMENT_TYPE {ELEMENT_TREE, ELEMENT_STONE, ELEMENT_RIVER, ELEMENT_LAKE};


class Element : public InstancePosRot {
public:
	Element();
	Element(Elevation * elevation, pt_2d position);
	virtual ~Element();
	virtual void update_data() = 0;
	//friend std::ostream & operator << (std::ostream & os, const Element & t);

	float * _data;
	uint _n_pts;
	Elevation * _elevation;
	ELEMENT_TYPE _type;
};


#endif
