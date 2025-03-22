#ifndef STATIC_OBJECT_H
#define STATIC_OBJECT_H

#include <iostream>
#include <string>

#include "geom_2d.h"
#include "bbox_2d.h"
#include "typedefs.h"
#include "material.h"


// OBSTACLE_TILE == tuile obstacle ; OBSTACLE_FLOATING == obstacle flottant
// HERO_CAR == voiture héros ; ENNEMY_CAR == toutes les autres voitures
// START == ligne de départ (et d'arrivée) ; CHECKPOINT == chekpt
// MATERIAL == matériau sol
// TIRE_TRACKS == traces de pneu
enum ObjectType {OBSTACLE_TILE, OBSTACLE_FLOATING, HERO_CAR, ENNEMY_CAR, START, CHECKPOINT, SURFACE_TILE, SURFACE_FLOATING, REPAIR};

// type de grille : verticale, horizontale
enum GridType {VERTICAL_GRID, HORIZONTAL_GRID};

// !!!
// obligé de * inertie par un facteur sinon tout pète lors des collisions
// !!!
const number INERTIA_FACTOR= 50.0;
const number BUMP_INCREMENT= 0.15;
const number BUMP_MAX= 4.0;
const number REPAIR_INCREMENT= 0.01;
const number BUMP_THRUST_FACTOR= 0.02;
const number BUMP_WHEEL_FACTOR= 0.12;
const number BUMP_DRIFT_THRESHOLD= 0.5;



// Modèle d'objet
class StaticObjectModel {
public:
	StaticObjectModel();
	StaticObjectModel(std::string json_path);
	~StaticObjectModel();
	void load(std::string json_path);
	friend std::ostream & operator << (std::ostream & os, const StaticObjectModel & model);


	std::string _json_path;
	std::string _name;
	ObjectType _type;
	pt_type _com2bbox_center; // vecteur centre de masse -> centre bbox ; utile pour mettre à jour StaticObject._bbox
	Polygon2D * _footprint; // empreinte au sol, utilisée pour les collisions ; doit être convexe
	float _texture_idx; // pour accélerer update()
	bool _fixed; // est-ce un objet figé

	Material * _material;
	std::string _material_name;
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
	void set_current_surface(Material * material);
	void anim(number anim_dt); // animation
	friend std::ostream & operator << (std::ostream & os, const StaticObject & obj);


	StaticObjectModel * _model;

	Polygon2D * _footprint; // empreinte au sol, utilisée pour les collisions ; doit être convexe
	BBox_2D * _bbox; // bounding box utilisée pour l'affichage des textures
	pt_type _scale; // grossissement
	number _mass; // == model->_mass* _scale.x* _scale.y
	number _inertia; // inertie

	pt_type _com; // center of mass
	pt_type _velocity; // vitesse
	pt_type _acceleration; // acceleration
	pt_type _force; // force appliquée ; non utilisé pour Car
	number _alpha; // angle de rotation
	number _angular_velocity; // vitesse angulaire
	number _angular_acceleration; // accélaration angulaire
	number _torque; // torque == équivalent force pour angle

	number _z;
	number _bumps[9];

	Material * _current_surface;
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


// grille d'objets
class StaticObjectGrid {
public:
	StaticObjectGrid();
	StaticObjectGrid(number cell_size, GridType type);
	~StaticObjectGrid();
	void clear();
	
	// méthodes de chgmt de système de coord
	unsigned int coord2idx(unsigned int col_idx, unsigned int row_idx);
	std::pair<unsigned int, unsigned int> idx2coord(unsigned int idx);
	std::pair<int, int> number2coord(pt_type pos);
	pt_type coord2number(unsigned int col_idx, unsigned int row_idx);
	pt_type idx2number(unsigned int idx);
	
	// get / set / add / del
	StaticObject * get_tile(unsigned int col_idx, unsigned int row_idx);
	void push_tile(StaticObjectModel * model);
	void set_tile(StaticObjectModel * model, unsigned int col_idx, unsigned int row_idx);
	void set_tile(StaticObjectModel * model, unsigned int idx);
	void set_all(StaticObjectModel * model, unsigned int width, unsigned int height);
	void add_row(StaticObjectModel * model);
	void add_col(StaticObjectModel * model);
	void drop_row();
	void drop_col();


	std::vector<StaticObject *> _objects;
	unsigned int _width; // dimensions
	unsigned int _height;
	number _cell_size; // taille cellule
	GridType _type; // type
};


#endif
