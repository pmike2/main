
#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>
#include <sstream>
#include <stack>
#include <iostream>
#include <fstream>

#include "constantes.h"
#include "utile.h"


class Pt {
public:
	Pt();
	Pt(float x_, float y_);
	float x, y;
};


// ------------------------------------------------------------------------------------------
class ActionConfig {
public:
	ActionConfig();
	virtual ~ActionConfig(); // destructeur virtuel necessaire a cause des methodes virtuelles
	virtual void load(std::string ch) = 0;
	virtual void save(std::string ch) = 0;
	virtual void randomize() = 0;
	virtual void defaultize() = 0;
	
	channel_type type;
	unsigned int lifetime; // en millisecondes
	bool active;
	bool retrig;
	std::vector<Pt>morph_pos;
};


// ------------------------------------------------------------------------------------------
struct ModelObjMorph {
	float model2world[16];
	float ambient[3]; // couleur des reflets ambiants, géré par objet
	float diffuse[3]; // couleur des reflets diffus, géré par sommet (à faire évoluer), inutilisé pour l'instant (fichier .mat remplace)
	float shininess; // eclat (entre 0. et 128., 0. étant le + éclatant), géré par objet
};


class ModelObjConfig : public ActionConfig {
public:
	ModelObjConfig();
	friend std::ostream & operator << (std::ostream &, ModelObjConfig);
	friend std::istream & operator >> (std::istream &, ModelObjConfig &);
	void load(std::string ch);
	void save(std::string ch);
	void randomize();
	void defaultize();
	
	ModelObjMorph init_values, final_values;
	std::string ch_obj;
	std::string ch_mat;
	std::vector<Pt>alpha_pos;
};


// ------------------------------------------------------------------------------------------
struct LightMorph {
	float color[3];
	float position_world[4];
	// de taille 3 (contrairement a position_world) car comme normal, il s'agit d'une direction
	float spot_cone_direction_world[3]; 
};


class LightConfig : public ActionConfig {
public:
	LightConfig();
	friend std::ostream & operator << (std::ostream &, LightConfig);
	friend std::istream & operator >> (std::istream &, LightConfig &);
	void load(std::string ch);
	void save(std::string ch);
	void randomize();
	void defaultize();

	LightMorph init_values, final_values;
};


// ------------------------------------------------------------------------------------------
struct VMatMorph {
	float mat[16];
};


class VMatConfig : public ActionConfig {
public:
	VMatConfig();
	friend std::ostream & operator << (std::ostream &, VMatConfig);
	friend std::istream & operator >> (std::istream &, VMatConfig &);
	void load(std::string ch);
	void save(std::string ch);
	void randomize();
	void defaultize();

	VMatMorph init_values, final_values;
	std::vector<Pt>alpha_pos;
};


// ------------------------------------------------------------------------------------------
class ChannelConfig {
public:
	ChannelConfig();
	friend std::ostream & operator << (std::ostream &, ChannelConfig);
	friend std::istream & operator >> (std::istream &, ChannelConfig &);
	void load(std::string ch);
	void save(std::string ch);
	void randomize();
	void release();
	
	std::vector<ActionConfig *> actions;
};


// ------------------------------------------------------------------------------------------
class A2VConfig {
public:
	A2VConfig();
	friend std::ostream & operator << (std::ostream &, A2VConfig);
	friend std::istream & operator >> (std::istream &, A2VConfig &);
	void load(std::string ch);
	void save(std::string ch);
	void randomize();
	void release();
	
	std::vector<ChannelConfig> channels_configs;
};


#endif
