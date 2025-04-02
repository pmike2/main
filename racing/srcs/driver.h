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


class ExpressionTexture {
public:
	ExpressionTexture();
	ExpressionTexture(std::string texture_path, unsigned int n_ms);
	~ExpressionTexture();


	std::string _texture_path;
	float _texture_idx;
	unsigned int _n_ms;
};


class Expression {
public:
	Expression();
	~Expression();


	std::vector<ExpressionTexture *> _textures;
};




class Driver {
public:
	Driver();
	Driver(std::string json_path);
	~Driver();
	void reinit(time_point t, bool set_normal_expression);
	void set_current_expression(std::string expression_name, time_point t);
	void anim(time_point t);


	std::string _name;
	std::string _json_path;
	std::map<std::string, Expression *> _expressions;
	std::string _current_expression_name, _next_expression_name;
	unsigned int _current_expression_texture_idx;
	time_point _last_anim_expression_t;
	time_point _last_change_expression_t;
	bool _angry, _happy, _tired, _sad;
};


#endif
