#ifndef SHIP_H
#define SHIP_H

#include <string>
#include <chrono>

#include <glm/glm.hpp>

#include "bbox_2d.h"

#include "ship_model.h"
#include "constantes.h"


// Vaisseau
class Ship {
public:
	Ship();
	Ship(ShipModel * model, pt_type pos, bool friendly, std::chrono::system_clock::time_point t);
	~Ship();
	void anim(std::chrono::system_clock::time_point t, bool play_sounds);
	Action * get_current_action();
	ShipModel * get_current_bullet_model();
	ActionTexture * get_current_texture();
	void set_current_action(std::string action_name, std::chrono::system_clock::time_point t);
	bool hit(std::chrono::system_clock::time_point t, bool play_sounds);
	friend std::ostream & operator << (std::ostream & os, const Ship & ship);


	ShipModel * _model; // modèle vaisseau
	AABB_2D _aabb; // AABB liéé à l'emprise totale du PNG
	AABB_2D _footprint; // AABB réduit à l'emprise du vaisseau
	float _rotation, _scale; // angle de rotation, facteur de mise à l'échelle
	bool _friendly; // est-ce le héro ou un ami
	glm::vec2 _velocity; // vecteur vitesse
	bool _shooting; // si vrai alors il faut créer une nouvelle munition
	std::string _current_action_name; // nom action courante
	unsigned int _idx_action; // indice dans la liste d'action
	unsigned int _lives; // nombre de vies restantes
	unsigned int _idx_anim; // indice dans la liste des PNG
	bool _hit; // vient t'il d'être touché
	float _hit_value; // valeur flottante pour le fragment shader montrant l'aspect hit
	bool _dead; // est t'il mort
	bool _delete; // doit il etre supprimé ; entre _dead et _delete il y a un temps d'anim de la mort
	float _alpha; // transparence
	std::chrono::system_clock::time_point _t_anim_start; // temps de début de l'affichage du PNG
	std::chrono::system_clock::time_point _t_action_start; // temps de début de l'action
	std::chrono::system_clock::time_point _t_die; // temps de mort
	std::chrono::system_clock::time_point _t_last_bullet; // temps de dernier tir
	std::chrono::system_clock::time_point _t_last_hit; // temps de dernier hit
};


#endif
