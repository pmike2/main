#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>

#include "typedefs.h"


// temps en ms pendant lequel des traces différentes vont être faites à partir du moment de sortie du matériau
const unsigned int SURFACE_CHANGE_DT= 1000;
const number LINEAR_FRICTION_MATERIAL_INCREMENT= 0.1;
const number ANGULAR_FRICTION_MATERIAL_INCREMENT= 0.1;



class Material {
public:
	Material();
	Material(std::string json_path);
	~Material();


	std::string _json_path;
	std::string _name;
	number _density;
	number _linear_friction;
	number _angular_friction;
	bool _solid; // est-ce un objet tangible
	number _restitution; // paramètre de dureté au rebond (voir collisions)

	float _tire_track_texture_idx;
};


#endif
