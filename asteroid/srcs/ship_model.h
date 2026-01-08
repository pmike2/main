#ifndef SHIP_MODEL_H
#define SHIP_MODEL_H

#include <string>
#include <vector>
#include <map>

#include <SDL2/SDL_mixer.h>
#include <glm/glm.hpp>

#include "bbox_2d.h"
#include "constantes.h"


class ShipModel;


// action d'un vaisseau
class Action {
public:
	Action();
	Action(glm::vec2 direction, int t, std::string bullet_name, uint t_shooting, std::string texture_name, Mix_Chunk * shoot_sound);
	~Action();
	friend std::ostream & operator << (std::ostream & os, const Action & action);


	glm::vec2 _direction; // direction du vaisseau
	int _t; // durée de l'action en ms
	uint _t_shooting; // fréquence de tir en ms si l'action implique du tir
	std::string _bullet_name; // nom du type de munition
	ShipModel * _bullet_model; // modèle munition
	std::string _texture_name; // nom de l'ActionTexture associée
	Mix_Chunk * _shoot_sound; // son tir
};


// Ensemble d'images à animer associées à une action d'un vaisseau
class ActionTexture {
public:
	ActionTexture();
	ActionTexture(std::vector<std::string> & pngs, std::vector<uint> & t_anims, AABB_2D & footprint);
	~ActionTexture();
	friend std::ostream & operator << (std::ostream & os, const ActionTexture & at);


	std::vector<std::string> _pngs; // liste des images PNG
	std::vector<uint> _t_anims; // durées d'affichage des textures
	uint _first_idx; // indice de la 1ere image liée a cette action dans la liste d'actions stockées dans un GL_TEXTURE_2D_ARRAY
	AABB_2D _footprint; // un footprint pour une action en prenant le + petit footprint des pngs de l'action
};


// Modèle vaisseau
class ShipModel {
public:
	ShipModel();
	ShipModel(std::string json_path);
	~ShipModel();
	friend std::ostream & operator << (std::ostream & os, const ShipModel & model);


	std::string _json_path; // chemin du json de paramètres
	ShipType _type; // type de vaisseau
	pt_2d _size; // taille
	uint _score_hit; // valeur remportée par le joueur lors d'un hit
	uint _score_death; // valeur remportée par le joueur lors d'une mort
	uint _lives; // nombre de vies
	std::map<std::string, std::vector<Action *> > _actions; // une action correspond en fait à une liste de Action *
	std::map<std::string, ActionTexture *> _textures; // dico des ActionTexture
	Mix_Chunk * _hit_sound, * _death_sound; // sons
};

#endif
