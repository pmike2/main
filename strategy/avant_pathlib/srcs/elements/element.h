#ifndef ELEMENT_H
#define ELEMENT_H

#include "json.hpp"

#include "typedefs.h"
#include "bbox.h"

#include "../const.h"
#include "../elevation.h"


using json = nlohmann::json;


class Element : public InstancePosRot {
public:
	Element();
	Element(Elevation * elevation, pt_2d position);
	virtual ~Element();
	virtual void update_data() = 0;
	virtual json get_json() = 0;


	float * _data;
	uint _n_pts;
	Elevation * _elevation;
	ELEMENT_TYPE _type;
	bool _delete;
};


#endif
