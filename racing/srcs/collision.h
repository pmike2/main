#ifndef COLLISION_H
#define COLLISION_H


#include "static_object.h"
#include "typedefs.h"


// impulse max, pour éviter que tout parte en cahouèt
const number MAX_IMPULSE= 3.0;

// facteur multiplicatif pour amplifier les rotations en cas de collision
const number COLLISION_ANGULAR_FACTOR= 3.0;

// collision entre 2 objets
bool collision(StaticObject * obj1, StaticObject * obj2, pt_2d & position, time_point t);


#endif
