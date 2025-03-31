#ifndef COLLISION_H
#define COLLISION_H


#include "static_object.h"

// impulse max, pour éviter que tout parte en cahouèt
const number MAX_IMPULSE= 3.0;


// collision entre 2 objets
bool collision(StaticObject * obj1, StaticObject * obj2, pt_type & position);


#endif
