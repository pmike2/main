#ifndef CAR_H
#define CAR_H

#include <iostream>
#include <string>

#include "bbox_2d.h"
#include "typedefs.h"
#include "static_object.h"



class CarModel : public StaticObjectModel {
public:
	CarModel();
	CarModel(std::string json_path);
	~CarModel();


	pt_type _forward;
	pt_type _right;
	pt_type _com2force_fwd;
	pt_type _com2force_bwd;
	number _max_wheel;
	number _wheel_increment;
	number _wheel_decrement;
	number _max_thrust;
	number _thrust_increment;
	number _thrust_decrement;
	number _max_brake;
	number _brake_increment;
	number _forward_static_friction;
	number _backward_static_friction;
	number _backward_dynamic_friction;
	number _friction_threshold;
};


class Car : public StaticObject {
public:
	Car();
	Car(CarModel * model, pt_type position, number alpha);
	~Car();
	CarModel * get_model();
	void reinit(pt_type position, number alpha);
	void update();
	void preanim_keys(bool key_left, bool key_right, bool key_down, bool key_up);
	void random_ia();
	void anim(number anim_dt);
	friend std::ostream & operator << (std::ostream & os, const Car & car);


	//CarModel * _model;

	pt_type _com2force_fwd; // vecteur com -> point ou on applique les forces
	pt_type _com2force_bwd; // vecteur com -> point ou on applique les forces
	pt_type _forward;
	pt_type _right;

	pt_type _force_fwd;
	pt_type _force_bwd;

	number _wheel;
	number _thrust;
	bool _drift;
};

#endif
