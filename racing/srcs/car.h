#ifndef CAR_H
#define CAR_H

#include <iostream>
#include <string>
#include <chrono>

#include "bbox_2d.h"
#include "typedefs.h"
#include "static_object.h"
#include "material.h"
#include "driver.h"


// facteur d'ajustement afin d'avoir le même feeling clavier / joystick
const number JOYSTICK_FACTOR= 0.5;


// Modèle de voiture
class CarModel : public StaticObjectModel {
public:
	CarModel();
	CarModel(std::string json_path);
	~CarModel();
	void load(std::string json_path);


	pt_2d _forward; // direction avant
	pt_2d _right; // direction à droite
	pt_2d _com2force_fwd; // vecteur COM (center of mass) -> point d'application des forces avant
	pt_2d _com2force_bwd; // vecteur COM (center of mass) -> point d'application des forces arrière
	number _max_wheel; // rotation max du volant
	number _max_wheel_reverse; // rotation max du volant quand Car est en marche arrière
	number _wheel_increment; // incrément de rotation du volant lors d'un appui touche gauche-droite
	number _wheel_decrement; // décrement de rotation du volant si pas d'appui touche gauche-droite
	number _max_thrust; // max pédale accélération
	number _thrust_increment; // incrément pédale accélération lors d'un appui touche avant
	number _thrust_decrement; // décrément pédale accélération si pas d'appui touche avant
	number _max_brake; // max pédale frein
	number _brake_increment; // incrément pédale frein si appui touche arrière
	number _forward_static_friction; // friction statique avant
	number _backward_static_friction; // friction statique arrière
	number _backward_dynamic_friction; // friction dynamique arrière
	number _friction_threshold; // seuil de passage friction statique -> friction dynamique (arrière)
	number _friction_threshold_braking; // pareil mais si on freine
	number _angular_friction; // friction angulaire
	number _speed_wheel_factor; // impact de la vitesse sur _wheel (+ Car va vite, - les touches / joystick font tourner)
};


// Voiture
class Car : public StaticObject {
public:
	Car();
	Car(CarModel * model, pt_2d position, number alpha, pt_2d scale);
	~Car();
	CarModel * get_model(); // renvoie le modèle de voiture
	void reinit(pt_2d position, number alpha, pt_2d scale); // met à une position / orientation / taille
	void update(); // met à jour les données calculées à partir des autres
	void preanim_keys(bool key_left, bool key_right, bool key_down, bool key_up); // gestion touches
	void preanim_joystick(bool joystick_a, bool joystick_b, glm::vec2 joystick); // gestion joystick
	//void random_ia(); //IA aléatoire, conservée pour mémoire et tests
	void anim(number anim_dt, time_point t); // animation
	friend std::ostream & operator << (std::ostream & os, const Car & car);


	pt_2d _com2force_fwd; // vecteur com -> point ou on applique les forces à l'avant (milieu des roues avant)
	pt_2d _com2force_bwd; // vecteur com -> point ou on applique les forces à l'arrière (milieu des roues arrière)
	pt_2d _forward; // vecteur unitaire indiquant la direction
	pt_2d _right; // perpendiculaire à _forward, pointant vers la droite

	pt_2d _force_fwd; // force appliquée à l'avant
	pt_2d _force_bwd; // force appliquée à l'arrière

	number _wheel; // quantité de volant tourné
	number _thrust; // quantité de pédale d'accélération
	bool _drift; // est-ce que le véhicule est en dérapage
	number _speed; // norme de _velocity
	bool _braking; // en train de freiner ?

	CheckPoint * _next_checkpoint; // prochain chkpt objectif
	uint _n_laps; // combien de tours déjà faits
	uint _rank; // classement
	bool _finished; // a t'il fini la course
	bool _just_finished; // _just_finished et _overlap_finish servent à départager 2 cars qui auraient fini lors du même cycle d'animation
	number _overlap_finish;
	std::vector<number> _lap_times; // les temps pour chaque tour
	number _total_time; // temps total

	time_point _last_drift_t; // dernier temps de création d'un TireTrack
	time_point _last_start_t; // dernier temps de passage sur le start de la course

	float _tire_track_texture_idx; // indice de la texture de la trace de pneu courante

	Driver * _driver; // conducteur car
};

#endif
