#ifndef DRIVER_H
#define DRIVER_H

#include <string>
#include <vector>
#include <chrono>
#include <map>

// temps en ms que prend un chgmt d'expression other -> normal
const unsigned int EXPRESSION_CHANGE_TO_NORMAL_MS= 1000;
// temps en ms que prend un chgmt d'expression normal -> other
const unsigned int EXPRESSION_CHANGE_TO_OTHERS_MS= 50;
// seuil de bump à partir duquel le driver est fatigué
const number BUMP_TIRED_THRESHOLD= 0.6;
// noms des expressions
const std::string NORMAL_EXPRESSION= "normal";
const std::string TIRED_EXPRESSION= "tired";
const std::string ANGRY_EXPRESSION= "angry";
const std::string HAPPY_EXPRESSION= "happy";
const std::string SAD_EXPRESSION= "sad";


// texture d'expression
class ExpressionTexture {
public:
	ExpressionTexture();
	ExpressionTexture(std::string texture_path, unsigned int n_ms);
	~ExpressionTexture();


	std::string _texture_path; // chemin texture
	float _texture_idx; // indice texture
	unsigned int _n_ms; // nombre de ms d'affichage de la texture
};


// expression == liste de textures d'expression
class Expression {
public:
	Expression();
	~Expression();


	std::vector<ExpressionTexture *> _textures;
};


// conducteur
class Driver {
public:
	Driver();
	Driver(std::string json_path);
	~Driver();
	void reinit(time_point t, bool set_normal_expression);
	void set_current_expression(std::string expression_name, time_point t);
	void anim(time_point t);


	std::string _name; // nom
	std::string _json_path; // chemin json
	std::map<std::string, Expression *> _expressions; // ensemble d'expressions
	std::string _current_expression_name; // nom expression courante
	std::string _next_expression_name; // nom expression suivante
	unsigned int _current_expression_texture_idx; // indice texture d'expression courante
	time_point _last_anim_expression_t; // dernier temps de changement d'animation de texture
	time_point _last_change_expression_t; // dernier temps de changement d'expression
	bool _angry, _happy, _tired, _sad; // Driver peut ressentir plusieurs choses ; un tri par priorité est effectué
};


#endif
