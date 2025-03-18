#ifndef CAR_H
#define CAR_H

#include <iostream>
#include <string>
#include <chrono>

#include "bbox_2d.h"
#include "typedefs.h"
#include "static_object.h"


// Modèle de voiture
class CarModel : public StaticObjectModel {
public:
	CarModel();
	CarModel(std::string json_path);
	~CarModel();
	void load(std::string json_path);


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


// Voiture
class Car : public StaticObject {
public:
	Car();
	Car(CarModel * model, pt_type position, number alpha, pt_type scale);
	~Car();
	CarModel * get_model(); // renvoie le modèle de voiture
	void reinit(pt_type position, number alpha, pt_type scale); // met à une position / orientation / taille
	void update(); // met à jour les données calculées à partir des autres
	void preanim_keys(bool key_left, bool key_right, bool key_down, bool key_up); // gestion touches
	void preanim_joystick(bool joystick_a, bool joystick_b, glm::vec2 joystick); // gestion joystick
	void random_ia(); //IA aléatoire, conservée pour mémoire et tests
	void anim(number anim_dt); // animation
	friend std::ostream & operator << (std::ostream & os, const Car & car);


	pt_type _com2force_fwd; // vecteur com -> point ou on applique les forces à l'avant (milieu des roues avant)
	pt_type _com2force_bwd; // vecteur com -> point ou on applique les forces à l'arrière (milieu des roues arrière)
	pt_type _forward; // vecteur unitaire indiquant la direction
	pt_type _right; // perpendiculaire à _forward, pointant vers la droite

	pt_type _force_fwd; // force appliquée à l'avant
	pt_type _force_bwd; // force appliquée à l'arrière

	number _wheel; // quantité de volant tourné
	number _thrust; // quantité de pédale d'accélération
	bool _drift; // est-ce que le véhicule est en dérapage

	CheckPoint * _next_checkpoint; // prochain chkpt objectif
	unsigned int _n_laps; // combien de tours déjà faits
	std::string _name; // nom du véhicule pour affichage classement

	number _linear_friction_material; // facteur multiplicatif de friction lié au sol; varie si on est sur une flaque par ex
	number _angular_friction_material;

	std::chrono::system_clock::time_point _last_drift_t;
	std::string _current_tracks;
	std::chrono::system_clock::time_point _last_track_t;

	unsigned int _rank;
	bool _finished;
	std::vector<number> _lap_times;
	std::chrono::system_clock::time_point _last_start_t;
};

#endif
