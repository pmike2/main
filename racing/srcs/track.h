#ifndef TRACK_H
#define TRACK_H

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <chrono>

#include "geom_2d.h"
#include "static_object.h"
#include "car.h"
#include "typedefs.h"
#include "input_state.h"
#include "material.h"


enum TrackMode {TRACK_WAITING, TRACK_PRECOUNT, TRACK_LIVE, TRACK_FINISHED};

// taille par défaut d'une cellule
const number DEFAULT_CELL_SIZE= 2.0;

// temps entre 2 boosts en ms
const unsigned int BOOST_DELTA_T_MS= 2000;
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

/*
	Piste de course : associée à une grille d'objets fixes, les tuiles qui composent le décor
	et à une liste d'objets 'flottants', les voitures et autres éléments interactifs
*/
class Track {
public:
	Track();
	~Track();
	void load_models(); // chgmt des modèles de tuiles, voitures et autres
	void load_json(std::string json_path);
	void save_json(std::string json_path);
	//void set_car_names();
	void set_hero(std::string name);
	void start(std::chrono::system_clock::time_point t);
	void end();
	void write_records();

	void sort_cars(); // tri des voitures par position
	unsigned int get_checkpoint_index(CheckPoint * checkpoint); // position du chkpt par rapport au start
	void collisions(); // gestion des collisions
	void surfaces(std::chrono::system_clock::time_point t);
	void repair(std::chrono::system_clock::time_point t);
	void boost(std::chrono::system_clock::time_point t);
	void checkpoints(std::chrono::system_clock::time_point t); // gestion chkpts pour toutes les voitures
	void checkpoint_ia(Car * car); // ia basée sur les chkpts
	void lap_time(std::chrono::system_clock::time_point t);
	void total_time();
	void anim(std::chrono::system_clock::time_point t, InputState * input_state); // animation

	// get / set / add / del
	void set_tile(std::string model_name, unsigned int col_idx, unsigned int row_idx);
	void set_tile(std::string model_name, unsigned int idx);
	void set_all(std::string model_name, unsigned int width, unsigned int height);
	void add_row(std::string model_name);
	void add_col(std::string model_name);
	void drop_row();
	void drop_col();
	
	StaticObject * get_floating_object(pt_type pos); // renvoie le floating object à une coord
	void delete_floating_object(StaticObject * obj); // suppression floating object
	void clear_floating_objects(); // suppression de tous les floating object

	friend std::ostream & operator << (std::ostream & os, const Track & track);


	std::map<std::string, Material *> _materials; // les matériaux
	std::map<std::string, StaticObjectModel *> _models; // modèles
	StaticObjectGrid * _grid; // grille de tuiles
	std::vector<StaticObject *> _floating_objects; // objets flottants
	unsigned int _n_laps; // nombre de tours
	CheckPoint * _start; // point de départ
	TrackMode _mode; // mode
	std::string _current_json_path;
	std::vector<std::pair<std::string, number> > _best_lap; // les 3 meilleurs temps au tour
	std::vector<std::pair<std::string, number> > _best_overall; // les 3 meilleurs en tout
	bool _new_best_lap, _new_best_overall;

	std::chrono::system_clock::time_point _last_precount_t; // pour décompte avant début course
	unsigned int _precount; // décompte avant début course

	std::vector<Car *> _sorted_cars; // les voitures triées par position
	Car * _hero;
	std::vector<pt_type> _collisions; // les positions des collisions en cours. Utile à SparkSystem
};



#endif
