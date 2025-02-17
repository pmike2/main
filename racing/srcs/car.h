#ifndef CAR_H
#define CAR_H

#include <iostream>
#include <string>

#include "bbox_2d.h"
#include "typedefs.h"


class CarModel {
public:
	CarModel();
	CarModel(std::string json_path);
	~CarModel();


	std::string _json_path;

	pt_type _forward;
	pt_type _right;
	pt_type _com2force_fwd;
	pt_type _com2force_bwd;
	pt_type _com2bbox_center;
	pt_type _halfsize;
	number _mass;
	number _inertia;
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
	number _angular_friction;

	bool _fixed;
};


class Car {
public:
	Car();
	Car(CarModel * model, pt_type position, number alpha);
	~Car();
	void reinit(pt_type position, number alpha);
	void update_direction();
	void update_bbox();
	void preanim_keys(bool key_left, bool key_right, bool key_down, bool key_up);
	void random_ia();
	void anim(number anim_dt);
	friend std::ostream & operator << (std::ostream & os, const Car & car);


	CarModel * _model;
	BBox_2D * _bbox;

	pt_type _com2force_fwd; // vecteur com -> point ou on applique les forces
	pt_type _com2force_bwd; // vecteur com -> point ou on applique les forces
	pt_type _com2bbox_center; // vecteur com -> centre bbox
	pt_type _forward;
	pt_type _right;

	pt_type _com; // center of mass
	pt_type _velocity;
	pt_type _acceleration;
	pt_type _force_fwd;
	pt_type _force_bwd;

	number _alpha; // angle de rotation
	number _angular_velocity;
	number _angular_acceleration;
	number _torque;

	number _wheel;
	number _thrust;
	bool _drift;
};

#endif
