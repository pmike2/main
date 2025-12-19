#ifndef STATIC_OBJECT_H
#define STATIC_OBJECT_H

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "geom_2d.h"
#include "bbox_2d.h"
#include "typedefs.h"
#include "material.h"
#include "action.h"


// OBSTACLE_TILE = tuile obstacle
// OBSTACLE_FLOATING = obstacle flottant
// CAR = voiture
// START = ligne de départ (et d'arrivée)
// CHECKPOINT = checkpoint
// SURFACE_TILE = matériau sol liée à une tuile
// SURFACE_FLOATING = matériau sol flottant
// REPAIR = zone de réparation
// BOOST = accélérateur
// DECORATION = objet décoratif
// DIRECTION_HELP = aide flèche de direction
enum ObjectType {OBSTACLE_TILE, OBSTACLE_FLOATING, CAR, START, CHECKPOINT, 
	SURFACE_TILE, SURFACE_FLOATING, REPAIR, BOOST, DECORATION, DIRECTION_HELP};

// type de mouvement autorisé
// MVMT_FIXED = ne bouge pas
// MVMT_ROTATE = rotations autorisées
// MVMT_TRANSLATE = translations autorisées
// MVMT_TRANSLATE_CONSTRAINED = translations autorisées uniquement le long de _translation_constraint
// MVMT_ALL = rotations et translations autorisées
enum ObjectMovementType {MVMT_FIXED, MVMT_ROTATE, MVMT_TRANSLATE, MVMT_TRANSLATE_CONSTRAINED, MVMT_ALL};

// pour chargement json
const std::map<std::string, ObjectType> STR2OBJTYPE {
	{"obstacle_tile", OBSTACLE_TILE},
	{"obstacle_floating", OBSTACLE_FLOATING},
	{"car", CAR},
	{"checkpoint", CHECKPOINT},
	{"start", START},
	{"surface_tile", SURFACE_TILE},
	{"surface_floating", SURFACE_FLOATING},
	{"repair", REPAIR},
	{"boost", BOOST},
	{"decoration", DECORATION},
	{"direction_help", DIRECTION_HELP}
};

// pour chargement json
const std::map<std::string, ObjectMovementType> STR2OBJMVMTTYPE {
	{"fixed", MVMT_FIXED},
	{"rotate", MVMT_ROTATE},
	{"translate", MVMT_TRANSLATE},
	{"translate_constrained", MVMT_TRANSLATE_CONSTRAINED},
	{"all", MVMT_ALL}
};

// z d'affichage par type d'objet
const std::map<ObjectType, number> Z_OBJECTS {
	{OBSTACLE_TILE, -80.0},
	{SURFACE_TILE, -80.0},
	{START, -70.0},
	{CHECKPOINT, -70.0},
	{SURFACE_FLOATING, -70.0},
	{REPAIR, -70.0},
	{BOOST, -70.0},
	{OBSTACLE_FLOATING, -60.0},
	{DECORATION, -40.0},
	{CAR, -50.0},
	{DIRECTION_HELP, -30.0}
};

// !!!
// obligé de * inertie par un facteur sinon tout pète lors des collisions
// !!!
const number INERTIA_FACTOR= 50.0;

// nombre de bosses par objet
const unsigned int N_BUMPS= 9;
// bosse max
const number BUMP_MAX= 4.0;
// facteur multiplicatif d'application d'une bosse à la pédale d'accélération
const number BUMP_THRUST_FACTOR= 0.02;
// facteur multiplicatif d'application d'une bosse au volant
const number BUMP_WHEEL_FACTOR= 0.12;
// seuil de forçage du drift lié aux bosses
const number BUMP_DRIFT_THRESHOLD= 0.5;
// incrément de réparation des bosses
const number REPAIR_INCREMENT= 0.01;


// Modèle d'objet
class StaticObjectModel : public ActionableObjectModel {
public:
	StaticObjectModel();
	StaticObjectModel(std::string json_path);
	~StaticObjectModel();
	void load(std::string json_path);
	friend std::ostream & operator << (std::ostream & os, const StaticObjectModel & model);


	ObjectType _type; // type d'objet
	pt_2d _com2bbox_center; // vecteur centre de masse -> centre bbox ; utile pour mettre à jour StaticObject._bbox
	Polygon2D * _footprint; // empreinte au sol, utilisée pour les collisions ; doit être convexe
	ObjectMovementType _movement; // type de mouvement possible
	pt_2d _translation_constraint; // vecteur de contrainte de translation si _movement == MVMT_TRANSLATE_CONSTRAINED
	Material * _material; // matériau éventuel de l'objet
	std::string _material_name; // nom du matériau
};


// Objet générique dont hérite Car
class StaticObject : public ActionableObject {
public:
	StaticObject();
	StaticObject(StaticObjectModel * model, pt_2d position, number alpha, pt_2d scale);
	~StaticObject();
	void reinit(pt_2d position, number alpha, pt_2d scale); // met à une position / orientation / taille
	void update(); // met à jour les données calculées à partir des autres
	void set_current_surface(Material * material, time_point t);
	bool anim_surface(time_point t);
	void anim(number anim_dt, time_point t); // animation
	friend std::ostream & operator << (std::ostream & os, const StaticObject & obj);


	StaticObjectModel * _model; // modèle de l'objet

	Polygon2D * _footprint; // empreinte au sol, utilisée pour les collisions ; doit être convexe
	BBox_2D * _bbox; // bounding box utilisée pour l'affichage des textures
	pt_2d _scale; // grossissement
	number _mass; // == model->_mass* _scale.x* _scale.y
	number _inertia; // inertie

	pt_2d _com; // center of mass
	pt_2d _velocity; // vitesse
	pt_2d _acceleration; // acceleration
	pt_2d _force; // force appliquée ; non utilisé pour Car
	number _alpha; // angle de rotation
	number _angular_velocity; // vitesse angulaire
	number _angular_acceleration; // accélaration angulaire
	number _torque; // torque == équivalent force pour angle

	pt_2d _translation_constraint; // vecteur de contrainte de translation si _movement == MVMT_TRANSLATE_CONSTRAINED

	number _linear_friction_surface; // facteur multiplicatif de friction lié au sol; varie si on est sur une flaque par ex
	number _angular_friction_surface;

	number _z; // z pour affichage
	number _bumps[N_BUMPS]; // bosses dues aux chocs
	// bosses 0, 1 : arrière ; 2, 3 : côté droit ; 4, 5 : avant ; 6, 7 : côté gauche ; 8 : centre

	Material * _current_surface; // surface sur laquelle repose l'objet
	Material * _previous_surface; // surface sur laquelle reposait l'objet avant d'être sur _current_surface

	time_point _last_change_surface_t; // dernier temps de changement de surface
	time_point _last_boost_t; // dernier temps où un boost a eu lieu

	bool _car_contact; // l'objet est-t'il en contact avec une voiture
};


// CheckPoint == point de passage des voiture ; une piste comprend plusieurs CheckPoint
class CheckPoint : public StaticObject {
public:
	CheckPoint();
	CheckPoint(StaticObjectModel * model, pt_2d position, number alpha, pt_2d scale);
	~CheckPoint();


	CheckPoint * _next; // le suivant
	CheckPoint * _previous; // le précédent
};


#endif
