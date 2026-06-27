#include "element.h"


Element::Element() {

}


Element::Element(Elevation * elevation, pt_2d position, std::string type) : 
	InstancePosRot(pt_3d(position.x, position.y, elevation->get_alti(position)), quat(1.0, 0.0, 0.0, 0.0), pt_3d(1.0)),
	_elevation(elevation), _n_pts(0), _delete(false), _valid(true), _type(type), _data(NULL)
{
	
}


Element::~Element() {
	if (_data != NULL) {
		delete _data;
	}
}

