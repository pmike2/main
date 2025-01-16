#ifndef ASTEROID_H
#define ASTEROID_H

#include <vector>
#include <map>
#include <chrono>

#include "json.hpp"

#include "bbox_2d.h"
#include "gl_utils.h"
#include "input_state.h"


const glm::dvec4 friendly_color(0.0, 1.0, 0.0, 1.0);
const glm::dvec4 unfriendly_color(1.0, 0.0, 0.0, 1.0);
const glm::dvec4 border_color(0.3, 0.3, 0.2, 1.0);
const unsigned int DELTA_BULLET= 100;


class Action {
public:
	Action();
	Action(glm::vec2 direction, unsigned int t, bool shooting, unsigned int t_shooting);
	~Action();


	glm::vec2 _direction;
	unsigned int _t;
	bool _shooting;
	unsigned int _t_shooting;
};


class ShipModel {
public:
	ShipModel();
	ShipModel(std::string json_path);
	~ShipModel();


	glm::vec2 _size;
	std::vector<Action *> _actions;
};


class Bullet {
public:
	Bullet();
	Bullet(const AABB_2D & aabb, bool friendly, glm::vec2 velocity);
	~Bullet();
	void anim();


	AABB_2D _aabb;
	bool _friendly;
	glm::vec2 _velocity;
	bool _dead;
};


class Ship {
public:
	Ship();
	Ship(ShipModel * model, pt_type pos, bool friendly, glm::vec2 velocity);
	~Ship();
	Bullet * anim();


	AABB_2D _aabb;
	bool _friendly;
	glm::vec2 _velocity;
	bool _dead;
	
	unsigned int _idx_action;
	std::chrono::system_clock::time_point _t_action_start;
	std::chrono::system_clock::time_point _t_last_bullet;
	ShipModel * _model;
};


class Level {
public:
	Level();
	Level(GLuint prog_draw_simple, GLuint prog_draw_texture);
	~Level();
	
	void draw_border_aabb(const glm::mat4 & world2clip);
	void draw_ship_aabb(const glm::mat4 & world2clip);
	void draw_bullet_aabb(const glm::mat4 & world2clip);
	void draw_texture(const glm::mat4 & world2clip);
	void draw(const glm::mat4 & world2clip);
	
	void update_ship_aabb();
	void update_bullet_aabb();
	void update_border_aabb();
	
	void anim();
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	bool joystick_down(unsigned int button_idx);
	bool joystick_up(unsigned int button_idx);
	bool joystick_axis(unsigned int axis_idx, int value);
	void add_rand_ennemy();
	void reinit();


	std::map<std::string, ShipModel *> _models;
	std::vector<Ship *> _ships;
	std::vector<Bullet * > _bullets;
	bool _draw_aabb;
	bool _draw_texture;
	glm::vec2 _pt_min, _pt_max;
	std::map<std::string, DrawContext *> _contexts;
	bool _key_left, _key_right, _key_up, _key_down;
	bool _shooting;
	std::chrono::system_clock::time_point _t_last_shooting;
	GLuint * _buffers;
	bool _gameover;
	float _joystick[2];
	unsigned int _score;
};


#endif
