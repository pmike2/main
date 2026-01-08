#ifndef ACTION_H
#define ACTION_H

#include <string>
#include <vector>

#include "typedefs.h"


enum ActionForceType {TRANSLATION, ROTATION};

// nom de l'unique séquence lorsque les séquences ne sont pas précisées
const std::string MAIN_SEQUENCE_NAME= "main_sequence";
// nom de la séquence lors d'une collision avec une voiture
const std::string CAR_CONTACT_SEQUENCE_NAME= "car_contact_sequence";
// nom de l'unique action lorsque les actions ne sont pas précisées
const std::string MAIN_ACTION_NAME= "main_action";


// affichage d'une texture liée à une action pendant n ms
class ActionTexture {
public:
	ActionTexture();
	ActionTexture(std::string texture_path, int n_ms);
	ActionTexture(std::string texture_path, int n_ms, float texture_idx);
	~ActionTexture();
	friend std::ostream & operator << (std::ostream & os, const ActionTexture & tex);


	std::string _texture_path; // chemin texture
	std::string _texture_path_bump; // chemin texture cabossée (si n'existe pas vaudra NO_PNG)
	float _texture_idx; // indice de la texture au sein du GL_TEXTURE_2D_ARRAY
	int _n_ms; // nombre de ms avant de passer à la suivante ; si vaut -1 on loope sur cette texture
};


// force à appliquer au sein d'une action
class ActionForce {
public:
	ActionForce();
	ActionForce(ActionForceType type, pt_2d force, number torque, int n_ms);
	~ActionForce();
	friend std::ostream & operator << (std::ostream & os, const ActionForce & force);


	ActionForceType _type; // translation ou rotation
	pt_2d _force; // cas translation
	number _torque; // cas rotation
	int _n_ms; // nombre de ms avant de passer à la suivante ; si vaut -1 on loope sur cette texture
};


// regroupe l'affichage de plusieurs textures et l'application de plusieurs forces
class Action {
public:
	Action();
	Action(std::string name);
	~Action();
	friend std::ostream & operator << (std::ostream & os, const Action & action);


	std::vector<ActionTexture *> _textures;
	std::vector<ActionForce * > _forces;
	std::string _name; // nom de l'action
};


// séquence d'actions
class ActionSequence {
public:
	ActionSequence();
	~ActionSequence();
	friend std::ostream & operator << (std::ostream & os, const ActionSequence & sequence);


	std::vector<std::pair<Action *, int> > _actions; // le int correspond à la durée de l'action
};


// pour effectuer des transitions différées entre séquences
class SequenceTransition {
public:
	SequenceTransition();
	SequenceTransition(std::string from, std::string to, int n_ms);
	~SequenceTransition();
	friend std::ostream & operator << (std::ostream & os, const SequenceTransition & transition);
	
	
	std::string _from; // nom séquence d'origine
	std::string _to; // nom séquence de destination
	int _n_ms; // nombre de ms à différer à partir de l'instant où on initie le passage de _from à _to
};


// randomisation des actions
class ActionRandom {
public:
	ActionRandom();
	ActionRandom(int flip, uint sequence_n_ms, number force, number torque, uint force_n_ms);
	~ActionRandom();


	bool _randomizable; // est-ce actif
	int _flip; // % de proba de flip x-> -x de la texture / action ; si < 0 alors n'est pas flippable
	int _sequence_n_ms; // sigma rand gaussienne pour le nombre de ms à attendre avant passage à nvelle séquence ; si < 0 alors inactif
	number _force; // sigma rand gaussienne pour la force
	number _torque; // sigma rand gaussienne pour le torque
	int _force_n_ms; // sigma rand gaussienne pour le nombre de ms à attendre avant passage à nvelle force; si < 0 alors inactif
};


// Modèle d'objet actionable. StaticObjectModel en hérite
class ActionableObjectModel {
public:
	ActionableObjectModel();
	ActionableObjectModel(std::string json_path);
	ActionableObjectModel(const ActionableObjectModel & model); // constructeur par copie
	~ActionableObjectModel();
	void load(std::string json_path);
	int get_transition(std::string from, std::string to);
	// renvoie une liste sans doublons des actions possibles ; utile lors du remplissage des GL_TEXTURE_2D_ARRAY
	std::vector<Action *> get_unduplicated_actions();
	friend std::ostream & operator << (std::ostream & os, const ActionableObjectModel & model);


	std::string _json_path; // chemin json de config
	std::string _name; // nom
	std::vector<SequenceTransition * > _transitions; // transitions entre séquences
	std::map<std::string, ActionSequence *> _sequences; // séquences d'action
	ActionRandom * _rand; // partie random
};


// Objet actionable ; StaticObject en hérite
class ActionableObject {
public:
	ActionableObject();
	ActionableObject(ActionableObjectModel * model);
	~ActionableObject();
	Action * get_current_action();
	void set_current_sequence(std::string sequence_name, time_point t);
	void anim_texture(time_point t);
	void anim_force(time_point t);
	void anim_action(time_point t);
	void anim_sequence(time_point t);
	void randomize();
	friend std::ostream & operator << (std::ostream & os, const ActionableObject & obj);


	ActionableObjectModel * _model_init; // modèle fixe initial, commun à plusieurs ActionableObject
	ActionableObjectModel * _model; // modèle copie de _model_init, modifiable via _model->_rand et autres rotations

	time_point _last_action_change_t; // dernier temps de changement d'action
	time_point _last_action_texture_t; // dernier temps de changement de texture
	time_point _last_action_force_t; // dernier temps de changement de force
	time_point _last_sequence_change_t; // dernier temps de changement de séquence
	std::string _current_sequence_name; // nom séquence courante
	std::string _next_sequence_name; // nom prochaine séquence
	int _current_action_texture_idx; // indice texture courant
	int _current_action_force_idx; // indice force courant
	int _current_action_idx; // indice action courant
	bool _flipped; // est-il renversé x -> -x
};

#endif
