#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>

#include "typedefs.h"


// matériau
class Material {
public:
	Material();
	Material(std::string json_path);
	~Material();


	std::string _json_path; // chemin json de config
	std::string _name; // nom
	number _density; // densité du matériau
	number _linear_friction; // friction linéaire (non utilisé pour Car)
	number _angular_friction; // friction angulaire (non utilisé pour Car)
	number _slippery; // est-t'il glissant
	bool _solid; // est-ce un objet tangible
	bool _bumpable; // est-ce abimable
	number _restitution; // paramètre de dureté au rebond (voir collisions)
	number _collision_thrust; // fixe la valeur max de car->_thrust (voir collisions)
	number _damage; // a quel point ce matériau abime lors d'une collision
	unsigned int _surface_change_ms; // temps en ms pendant lequel des traces différentes vont être faites à partir du moment de sortie du matériau
	float _tire_track_texture_idx; // indice texture trace de pneus liée au matériau
};


#endif
