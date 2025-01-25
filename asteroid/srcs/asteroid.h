#ifndef ASTEROID_H
#define ASTEROID_H

#include <vector>
#include <map>
#include <chrono>

#include "json.hpp"

#include "bbox_2d.h"
#include "gl_utils.h"
#include "input_state.h"
#include "font.h"



const glm::dvec4 BORDER_COLOR(0.3, 0.3, 0.2, 1.0);
const glm::dvec4 AABB_FRIENDLY_COLOR(0.0, 1.0, 0.0, 1.0);
const glm::dvec4 AABB_UNFRIENDLY_COLOR(1.0, 0.0, 0.0, 1.0);
const glm::dvec4 FOOTPRINT_FRIENDLY_COLOR(0.0, 1.0, 1.0, 1.0);
const glm::dvec4 FOOTPRINT_UNFRIENDLY_COLOR(1.0, 0.0, 1.0, 1.0);
const float HERO_VELOCITY= 0.1;
const float Z_NEAR= -10.0f;
const float Z_FAR= 10.0f;
const unsigned int TEXTURE_SIZE= 1024;
const unsigned int HIT_UNTOUCHABLE_MS= 200;
const unsigned int DEATH_MS= 200;
const std::string MAIN_ACTION_NAME= "main";


enum ShipType {HERO, ENEMY, BULLET};
enum LevelMode {PLAYING, INACTIVE, SET_SCORE_NAME};
enum EventType {NEW_ENEMY, LEVEL_END};


class ShipModel;


class Action {
public:
	Action();
	Action(glm::vec2 direction, int t, std::string bullet_name, unsigned int t_shooting, std::string texture_name);
	~Action();
	friend std::ostream & operator << (std::ostream & os, const Action & action);


	glm::vec2 _direction;
	int _t;
	unsigned int _t_shooting;
	std::string _bullet_name;
	ShipModel * _bullet_model;
	std::string _texture_name;
};


class ActionTexture {
public:
	ActionTexture();
	ActionTexture(std::vector<std::string> & pngs, std::vector<unsigned int> & t_anims, AABB_2D & footprint);
	~ActionTexture();
	friend std::ostream & operator << (std::ostream & os, const ActionTexture & at);


	std::vector<std::string> _pngs;
	std::vector<unsigned int> _t_anims; // durées d'affichage des textures
	unsigned int _first_idx; // indice de la 1ere image liée a cette action dans la liste d'actions stockées dans un GL_TEXTURE_2D_ARRAY
	AABB_2D _footprint; // un footprint pour une action en prenant le + petit footprint des pngs de l'action
};


class ShipModel {
public:
	ShipModel();
	ShipModel(std::string json_path);
	~ShipModel();
	friend std::ostream & operator << (std::ostream & os, const ShipModel & model);


	std::string _json_path;
	ShipType _type;
	pt_type _size;
	unsigned int _score;
	unsigned int _lives;
	std::map<std::string, std::vector<Action *> > _actions;
	std::map<std::string, ActionTexture *> _textures;
};


class Ship {
public:
	Ship();
	Ship(ShipModel * model, pt_type pos, bool friendly, std::chrono::system_clock::time_point t);
	~Ship();
	void anim(std::chrono::system_clock::time_point t);
	ShipModel * get_current_bullet_model();
	ActionTexture * get_current_texture();
	void set_current_action(std::string action_name, std::chrono::system_clock::time_point t);
	bool hit(std::chrono::system_clock::time_point t);
	friend std::ostream & operator << (std::ostream & os, const Ship & ship);


	AABB_2D _aabb;
	AABB_2D _footprint;
	bool _friendly;
	glm::vec2 _velocity;
	bool _shooting;
	std::string _current_action_name;
	unsigned int _idx_action;
	std::chrono::system_clock::time_point _t_action_start;
	std::chrono::system_clock::time_point _t_last_bullet;
	ShipModel * _model;
	unsigned int _lives;
	unsigned int _idx_anim;
	std::chrono::system_clock::time_point _t_anim_start;
	std::chrono::system_clock::time_point _t_last_hit;
	bool _hit;
	float _hit_value;
	bool _dead;
	bool _delete;
	float _alpha;
	std::chrono::system_clock::time_point _t_die;
};


class Event {
public:
	Event();
	Event(EventType type, unsigned int t, glm::vec2 position, std::string enemy);
	~Event();
	friend std::ostream & operator << (std::ostream & os, const Event & event);


	EventType _type;
	unsigned int _t;
	glm::vec2 _position;
	std::string _enemy;
};


class Level {
public:
	Level();
	Level(std::string json_path, std::chrono::system_clock::time_point t);
	~Level();
	void reinit(std::chrono::system_clock::time_point t);
	friend std::ostream & operator << (std::ostream & os, const Level & level);


	std::string _json_path;
	std::chrono::system_clock::time_point _t_start;
	std::vector<Event *> _events;
	unsigned int _current_event_idx;
};


class Asteroid {
public:
	Asteroid();
	Asteroid(GLuint prog_aabb, GLuint prog_texture, GLuint prog_font, ScreenGL * screengl, std::chrono::system_clock::time_point t);
	~Asteroid();

	void load_models();
	void load_levels(std::chrono::system_clock::time_point t);
	
	void draw_border_aabb();
	void draw_ship_aabb();
	void draw_ship_footprint();
	void draw_ship_texture();
	void draw();

	void show_playing_info();
	void show_inactive_info();
	void show_set_score_name_info();

	void update_border_aabb();
	void update_ship_aabb();
	void update_ship_footprint();
	void update_ship_texture();
	
	void anim_playing(std::chrono::system_clock::time_point t);
	void anim_inactive();
	void anim_set_score_name();
	void anim(std::chrono::system_clock::time_point t);

	bool key_down(InputState * input_state, SDL_Keycode key, std::chrono::system_clock::time_point t);
	bool key_up(InputState * input_state, SDL_Keycode key, std::chrono::system_clock::time_point t);
	bool joystick_down(unsigned int button_idx, std::chrono::system_clock::time_point t);
	bool joystick_up(unsigned int button_idx, std::chrono::system_clock::time_point t);
	bool joystick_axis(unsigned int axis_idx, int value, std::chrono::system_clock::time_point t);
	
	void add_level_events(std::chrono::system_clock::time_point t);
	void add_rand_enemy(std::chrono::system_clock::time_point t);
	void reinit(std::chrono::system_clock::time_point t);
	void read_highest_scores();
	void write_highest_scores();


	std::map<std::string, ShipModel *> _models;
	std::vector<Ship *> _ships;
	bool _draw_aabb, _draw_footprint, _draw_texture;
	glm::vec2 _pt_min, _pt_max;
	std::map<std::string, DrawContext *> _contexts;
	bool _key_left, _key_right, _key_up, _key_down;
	GLuint * _buffers;
	float _joystick[2];
	unsigned int _score;
	std::vector<std::pair<std::string, unsigned int> > _highest_scores;
	Font * _font;
	glm::mat4 _camera2clip;
	LevelMode _mode;
	int _new_highest_idx;
	int _new_highest_char_idx;
	std::vector<Level *> _levels;
	unsigned int _current_level_idx;
	GLuint _texture_id;
};


#endif
