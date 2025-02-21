#ifndef STATIC_OBJECT_H
#define STATIC_OBJECT_H

#include <iostream>
#include <string>

#include "geom_2d.h"
#include "bbox_2d.h"
#include "typedefs.h"


// setting en anglais == décor
enum ObjectType {OBSTACLE_SETTING, OBSTACLE_FLOATING, HERO_CAR, ENNEMY_CAR, CHECKPOINT, START};


class StaticObject;
void collision(StaticObject * obj1, StaticObject * obj2);



class StaticObjectModel {
public:
	StaticObjectModel();
	StaticObjectModel(std::string json_path);
	~StaticObjectModel();
	friend std::ostream & operator << (std::ostream & os, const StaticObjectModel & model);


	std::string _json_path;
	ObjectType _type;
	pt_type _com2bbox_center;
	//pt_type _halfsize;
	number _mass;
	number _inertia;
	number _linear_friction; // non utilisé pour CarModel
	number _angular_friction;
	Polygon2D * _footprint;
	bool _fixed;
	bool _solid;
};


class StaticObject {
public:
	StaticObject();
	StaticObject(StaticObjectModel * model, pt_type position, number alpha, pt_type scale);
	~StaticObject();
	void reinit(pt_type position, number alpha, pt_type scale);
	void update();
	void anim(number anim_dt);
	friend std::ostream & operator << (std::ostream & os, const StaticObject & obj);


	StaticObjectModel * _model;
	Polygon2D * _footprint;
	BBox_2D * _bbox;
	pt_type _com2bbox_center; // vecteur com -> centre bbox
	pt_type _com; // center of mass
	pt_type _velocity;
	pt_type _acceleration;
	pt_type _force; // non utilisé pour Car
	number _alpha; // angle de rotation
	number _angular_velocity;
	number _angular_acceleration;
	number _torque;
	pt_type _scale; // grossissement
};


class CheckPoint : public StaticObject {
public:
	CheckPoint();
	CheckPoint(StaticObjectModel * model, pt_type position, number alpha, pt_type scale);
	~CheckPoint();


	CheckPoint * _next;
};

#endif
