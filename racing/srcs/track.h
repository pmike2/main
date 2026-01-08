#ifndef TRACK_H
#define TRACK_H

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <chrono>

#include "geom_2d.h"
#include "static_object.h"
#include "grid.h"
#include "car.h"
#include "typedefs.h"
#include "input_state.h"
#include "material.h"
#include "driver.h"

// TRACK_WAITING = état initial
// TRACK_PRECOUNT = décompte en cours
// TRACK_LIVE = course en cours
// TRACK_FINISHED = course finie
enum TrackMode {TRACK_WAITING, TRACK_PRECOUNT, TRACK_LIVE, TRACK_FINISHED};

// taille par défaut d'une cellule
const number DEFAULT_CELL_SIZE= 2.0;

// temps entre 2 boosts en ms
const uint BOOST_DELTA_T_MS= 2000;
// amplitude d'un boost
const number BOOST_AMPLITUDE= 4.0;

// seuil de distance du checkpoint pour accélération
const number IA_THRUST_DIST_THRESHOLD= 1.0;
// si wheel est > IA_WHEEL_THRESHOLD et
// si le prod scalaire de forward et de la direction à prendre est < IA_SCALPROD_THRESHOLD
// on ne tourne pas plus; évite de tourner en rond
const number IA_WHEEL_THRESHOLD= 0.5;
const number IA_SCALPROD_THRESHOLD= -0.8;
// valeur max wheel lorsque IA tourne
const number IA_WHEEL_AMPLITUDE= 1.0;

// aide de direction
const number DIRECTION_HELP_SCALE= 1.5;
const number DIRECTION_HELP_DIST_HERO= 4.5;
const number DIRECTION_HELP_DRAWN_THRESHOLD= 8.0;


// config placement voitures ligne de départ
struct CarsPlacement {
	number _first_row_dist_start;
	number _row_dist;
	number _neighbour_dist;
	int _n_cars_per_row;
	int _n_max_rows;
};

// a terme faire un CarsPlacement par Track ?
const CarsPlacement DEFAULT_CAR_PLACEMENT {1.0, 2.0, 2.0, 3, 6};


// Infos sur une course
class TrackInfo {
public:
	TrackInfo();
	~TrackInfo();
	void parse_json(std::string json_path);


	uint _n_laps; // nombre de tours
	std::vector<std::pair<std::string, number> > _best_lap; // les 3 meilleurs temps au tour
	std::vector<std::pair<std::string, number> > _best_overall; // les 3 meilleurs en tout
};

/*
	Piste de course : associée à une grille d'objets fixes, les tuiles qui composent le décor
	et à une liste d'objets 'flottants', les voitures et autres éléments interactifs
*/
class Track {
public:
	Track();
	~Track();
	void load_models(); // chgmt des modèles de tuiles, voitures et autres
	void load_json(std::string json_path); // chgmt course
	void save_json(std::string json_path); // sauvegarde course (utilisé dans track_editor.cpp)
	void set_hero(uint idx_driver); // choix du héros
	void place_cars(); // placement des voitures à la ligne de départ
	void reinit_drivers(time_point t, bool set_normal_expression); // reinit des conducteurs
	void start(time_point t); // départ course
	void end(); // fin course
	void write_records(); // enregistrement des meilleurs temps
	void sort_cars(); // tri des voitures par position
	uint get_checkpoint_index(CheckPoint * checkpoint); // position du chkpt par rapport au start
	
	void anim_objects(time_point t); // animation des objets
	void reinit_car_contact(); // reinitialisation des contacts des objets avec les voitures
	void collisions(time_point t); // gestion des collisions
	void surfaces(time_point t); // gestion des surfaces sur lesquelles se trouvent les objets
	void repair(time_point t); // réparation des voitures
	void boost(time_point t); // boost vitesse des voitures
	void checkpoints(time_point t); // gestion chkpts pour toutes les voitures
	void checkpoint_ia(Car * car); // ia basée sur les chkpts
	void set_car_contact_action(time_point t); // met à jour l'action des objets en contact avec une voiture
	void lap_time(time_point t); // maj du temps du tour
	void total_time(); // maj du temps total
	void anim_drivers(time_point t); // anim des expressions des conducteurs
	void direction_help(); // maj de l'aide de direction
	void anim(time_point t, InputState * input_state, bool joystick_is_input); // animation

	// get / set / add / del; utilisé dans track_editor.cpp
	void set_tile(std::string model_name, uint col_idx, uint row_idx);
	void set_tile(std::string model_name, uint idx);
	void set_all(std::string model_name, uint width, uint height);
	void add_row(std::string model_name);
	void add_col(std::string model_name);
	void drop_row();
	void drop_col();
	
	StaticObject * get_floating_object(pt_2d pos); // renvoie le floating object à une coord
	void delete_floating_object(StaticObject * obj); // suppression floating object
	void clear_floating_objects(); // suppression de tous les floating object

	friend std::ostream & operator << (std::ostream & os, const Track & track);


	std::map<std::string, Material *> _materials; // les matériaux
	std::map<std::string, StaticObjectModel *> _models; // modèles
	StaticObjectGrid * _grid; // grille de tuiles
	std::vector<StaticObject *> _floating_objects; // objets flottants
	TrackInfo * _info; // infos générales course
	CheckPoint * _start; // point de départ
	TrackMode _mode; // mode
	std::string _current_json_path; // chemin json
	bool _new_best_lap; // à la fin le héros a t'il fait le meilleur temps au tour
	bool _new_best_overall; // à la fin le héros a t'il fait le meilleur temps total

	time_point _last_precount_t; // pour décompte avant début course
	uint _precount; // décompte avant début course

	std::vector<Car *> _sorted_cars; // les voitures triées par position
	Car * _hero; // voiture héros
	std::vector<Driver *> _drivers; // liste des conducteurs
	std::vector<pt_2d> _collisions; // les positions des collisions en cours. Utile à SparkSystem
	std::vector<std::vector<pt_2d> > _barriers; // les barrières de la course
};



#endif
