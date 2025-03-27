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


// OBSTACLE_TILE == tuile obstacle ; OBSTACLE_FLOATING == obstacle flottant
// HERO_CAR == voiture héros ; ENNEMY_CAR == toutes les autres voitures
// START == ligne de départ (et d'arrivée) ; CHECKPOINT == chekpt
// SURFACE_TILE, SURFACE_FLOATING == matériau sol
// REPAIR : zone de réparation
// BOOST : accélérateur
enum ObjectType {OBSTACLE_TILE, OBSTACLE_FLOATING, HERO_CAR, ENNEMY_CAR, START, CHECKPOINT, 
	SURFACE_TILE, SURFACE_FLOATING, REPAIR, BOOST};

// type de grille : verticale, horizontale
enum GridType {VERTICAL_GRID, HORIZONTAL_GRID};

const std::map<std::string, ObjectType> STR2OBJTYPE {
	{"obstacle_tile", OBSTACLE_TILE},
	{"obstacle_floating", OBSTACLE_FLOATING},
	{"hero_car", HERO_CAR},
	{"ennemy_car", ENNEMY_CAR},
	{"checkpoint", CHECKPOINT},
	{"start", START},
	{"surface_tile", SURFACE_TILE},
	{"surface_floating", SURFACE_FLOATING},
	{"repair", REPAIR},
	{"boost", BOOST}
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
	{HERO_CAR, -40.0},
	{ENNEMY_CAR, -40.0}
};

const std::string MAIN_ACTION_NAME= "main";


class ActionTexture {
public:
	ActionTexture();
	ActionTexture(std::string texture_path, unsigned int n_ms);
	~ActionTexture();


	std::string _texture_path;
	std::string _texture_path_bump;
	float _texture_idx;
	unsigned int _n_ms;
};


class Action {
public:
	Action();
	~Action();


	std::vector<ActionTexture *> _textures;
};


// Modèle d'objet
class StaticObjectModel {
public:
	StaticObjectModel();
	StaticObjectModel(std::string json_path);
	~StaticObjectModel();
	void load(std::string json_path);
	friend std::ostream & operator << (std::ostream & os, const StaticObjectModel & model);


	std::string _json_path; // chemin json de config
	std::string _name; // nom
	ObjectType _type; // type d'objet
	pt_type _com2bbox_center; // vecteur centre de masse -> centre bbox ; utile pour mettre à jour StaticObject._bbox
	Polygon2D * _footprint; // empreinte au sol, utilisée pour les collisions ; doit être convexe
	bool _fixed; // est-ce un objet figé
	Material * _material; // matériau éventuel de l'objet
	std::string _material_name; // nom du matériau

	//float _texture_idx;
	std::map<std::string, Action *> _actions;
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
	void set_current_surface(Material * material, std::chrono::system_clock::time_point t);
	void set_current_action(std::string action_name, std::chrono::system_clock::time_point t);
	bool anim_surface(std::chrono::system_clock::time_point t);
	void anim_texture(std::chrono::system_clock::time_point t);
	void anim(number anim_dt, std::chrono::system_clock::time_point t); // animation
	friend std::ostream & operator << (std::ostream & os, const StaticObject & obj);


	StaticObjectModel * _model; // modèle de l'objet

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

	number _linear_friction_surface; // facteur multiplicatif de friction lié au sol; varie si on est sur une flaque par ex
	number _angular_friction_surface;

	number _z; // z pour affichage
	number _bumps[N_BUMPS]; // bosses dues aux chocs
	// bosses 0, 1 : arrière ; 2, 3 : côté droit ; 4, 5 : avant ; 6, 7 : côté gauche ; 8 : centre

	Material * _current_surface; // surface sur laquelle repose l'objet
	Material * _previous_surface; // surface sur laquelle reposait l'objet avant d'être sur _current_surface

	std::chrono::system_clock::time_point _last_change_surface_t; // dernier temps de changement de surface
	std::chrono::system_clock::time_point _last_boost_t; // dernier temps où un boost a eu lieu

	std::string _current_action_name;
	unsigned int _current_action_texture_idx;
	std::chrono::system_clock::time_point _last_anim_action_t;
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
