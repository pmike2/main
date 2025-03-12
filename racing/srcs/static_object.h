#ifndef STATIC_OBJECT_H
#define STATIC_OBJECT_H

#include <iostream>
#include <string>

#include "geom_2d.h"
#include "bbox_2d.h"
#include "typedefs.h"


// setting en anglais == décor
// OBSTACLE_SETTING == tuile décor ; OBSTACLE_FLOATING == obstacle flottant
// HERO_CAR == voiture héros ; ENNEMY_CAR == toutes les autres voitures
// START == ligne de départ (et d'arrivée) ; CHECKPOINT == chekpt
enum ObjectType {OBSTACLE_SETTING, OBSTACLE_FLOATING, HERO_CAR, ENNEMY_CAR, START, CHECKPOINT};

// !!!
// obligé de * inertie par un facteur sinon tout pète lors des collisions
// !!!
const number INERTIA_FACTOR= 50.0;


class StaticObject;
// collision entre 2 objets
void collision(StaticObject * obj1, StaticObject * obj2);


// Modèle d'objet
class StaticObjectModel {
public:
	StaticObjectModel();
	StaticObjectModel(std::string json_path);
	~StaticObjectModel();
	void load(std::string json_path);
	friend std::ostream & operator << (std::ostream & os, const StaticObjectModel & model);


	std::string _json_path;
	ObjectType _type;
	number _mass;
	number _linear_friction; // utilisé pour les objets non figés mais qui ne sont pas des voitures
	number _angular_friction;
	Polygon2D * _footprint; // empreinte au sol, utilisée pour les collisions ; doit être convexe
	bool _fixed; // est-ce un objet figé
	bool _solid; // est-ce un objet tangible
	number _restitution; // paramètre de dureté au rebond (voir collision())
	pt_type _com2bbox_center; // vecteur centre de masse -> centre bbox ; utile pour mettre à jour StaticObject._bbox
};


// Objet générique dont hérite Car
class StaticObject {
public:
	StaticObject();
	StaticObject(StaticObjectModel * model, pt_type position, number alpha, pt_type scale);
	~StaticObject();
	void set_model(StaticObjectModel * model);
	void reinit(pt_type position, number alpha, pt_type scale); // met à une position / orientation / taille
	void update(); // met à jour les données calculées à partir des autres
	void anim(number anim_dt); // animation
	friend std::ostream & operator << (std::ostream & os, const StaticObject & obj);


	StaticObjectModel * _model;
	Polygon2D * _footprint; // empreinte au sol, utilisée pour les collisions ; doit être convexe
	BBox_2D * _bbox; // bounding box utilisée pour l'affichage des textures
	pt_type _com; // center of mass
	pt_type _velocity; // vitesse
	pt_type _acceleration; // acceleration
	pt_type _force; // force appliquée ; non utilisé pour Car
	number _alpha; // angle de rotation
	number _angular_velocity; // vitesse angulaire
	number _angular_acceleration; // accélaration angulaire
	number _torque; // torque == équivalent force pour angle
	pt_type _scale; // grossissement
	number _mass; // == model->_mass* _scale.x* _scale.y
	number _inertia; // inertie
};


// CheckPoint == point de passage des voiture ; une piste comprend plusieurs CheckPoint
class CheckPoint : public StaticObject {
public:
	CheckPoint();
	CheckPoint(StaticObjectModel * model, pt_type position, number alpha, pt_type scale);
	~CheckPoint();


	CheckPoint * _next; // le suivant
	CheckPoint * _previous; // le précédent
};


#endif
