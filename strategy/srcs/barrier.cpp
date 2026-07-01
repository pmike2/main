#include "barrier.h"


Barrier::Barrier() {

}


Barrier::Barrier(BarrierType * type, pt_2d position, number orientation, Elevation * elevation) : 
	InstancePosRot(pt_3d(position.x, position.y, elevation->get_alti(position)), glm::angleAxis(orientation, pt_3d(0.0, 0.0, 1.0)), pt_3d(1.0), type->_obj_data->_aabb), _type(type), _elevation(elevation)
{

}


Barrier::~Barrier() {

}

