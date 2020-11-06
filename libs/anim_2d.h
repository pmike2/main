#ifndef ANIM_2D_H
#define ANIM_2D_H

#include <string>
#include <vector>

#include "gl_utils.h"
#include "bbox_2d.h"
#include "input_state.h"


// _z doit etre entre ces bornes
const float Z_NEAR= -10.0f;
const float Z_FAR= 10.0f;

const unsigned int ANIM_MODEL_SIZE= 512;


struct Action {
	std::string direction();
	std::string type();
	void print();


	std::string _name;
	std::vector<std::string> _pngs;
	unsigned int _first_idx;
	unsigned int _n_idx;
	float _anim_time;
	glm::vec2 _footprint_offset; // entre 0 et 1
	glm::vec2 _footprint_size; // entre 0 et 1
};


class StaticObj {
public:
	StaticObj();
	StaticObj(glm::vec2 pos, glm::vec2 size, glm::vec2 footprint_offset, glm::vec2 footprint_size);
	~StaticObj();
	void update_footprint_pos();
	void set_aabb_pos(glm::vec2 pos);

	
	AABB_2D * _aabb;
	glm::vec2 _footprint_offset;
	AABB_2D * _footprint;
};


class AnimObj {
public:
	AnimObj();
	AnimObj(glm::vec2 pos, glm::vec2 size, glm::vec2 footprint_offset, glm::vec2 footprint_size);
	~AnimObj();
	void anim(float elapsed_time);
	void update_footprint_pos();
	void set_aabb_pos(glm::vec2 pos);
	void set_footprint(glm::vec2 footprint_offset, glm::vec2 footprint_size);


	AABB_2D * _aabb;
	glm::vec2 _footprint_offset;
	AABB_2D * _footprint;
	glm::vec2 _velocity;
};


bool anim_intersect_static(const AnimObj * anim_obj, const StaticObj * static_obj, const float time_step, glm::vec2 & contact_pt, glm::vec2 & contact_normal, float & contact_time);

/*
struct Collision {
	unsigned int _idx_static;
	float _contact_time;
	glm::vec2 _contact_normal;
};
*/

class StaticCharacter;

class StaticTexture {
public:
	StaticTexture();
	StaticTexture(GLuint prog_draw, std::string path, ScreenGL * screengl);
	~StaticTexture();
	void draw();
	void update();


	GLuint _texture_id;
	GLuint _vbo;
	GLuint _prog_draw;
	GLint _camera2clip_loc, _model2world_loc, _position_loc, _tex_coord_loc, _tex_loc, _alpha_loc;
	glm::mat4 _camera2clip;
	glm::mat4 _model2world;
	ScreenGL * _screengl;
	float _alpha;
	unsigned int _n_aabbs;
	glm::vec2 _footprint_offset; // entre 0 et 1
	glm::vec2 _footprint_size; // entre 0 et 1
	std::vector<StaticCharacter *> _static_characters;
};


class StaticCharacter {
public:
	StaticCharacter();
	StaticCharacter(StaticObj * static_obj, StaticTexture * static_texture, float z);
	~StaticCharacter();


	StaticObj * _static_obj;
	StaticTexture * _static_texture;
	float _z;
};


class AnimCharacter;

class AnimTexture {
public:
	AnimTexture();
	AnimTexture(GLuint prog_draw, std::string path, ScreenGL * screengl);
	~AnimTexture();
	void draw();
	void update();


	GLuint _texture_id;
	GLuint _vbo;
	GLuint _prog_draw;
	GLint _camera2clip_loc, _model2world_loc, _position_loc, _tex_coord_loc, _texture_array_loc, _current_layer_loc;
	glm::mat4 _camera2clip;
	glm::mat4 _model2world;
	ScreenGL * _screengl;
	unsigned int _n_aabbs;
	std::vector<Action *> _actions;
	std::vector<AnimCharacter *> _anim_characters;
};


class AnimCharacter {
public:
	AnimCharacter();
	AnimCharacter(AnimObj * anim_obj, AnimTexture * anim_texture, float z);
	~AnimCharacter();
	void anim(float elapsed_time);
	void update_velocity();
	void update_action();
	void key_down(SDL_Keycode key);
	void key_up(SDL_Keycode key);
	void set_action(unsigned int idx_action);
	void set_action(std::string action_name);
	std::string current_action();
	std::string current_direction();
	std::string current_type();


	AnimObj * _anim_obj;
	AnimTexture * _anim_texture;
	float _z;
	unsigned int _current_anim;
	float _accumulated_time;
	Action * _current_action;
	bool _left_pressed, _right_pressed, _down_pressed, _up_pressed, _lshift_pressed;
	bool _jump;
};


class Level {
public:
	Level();
	Level(GLuint prog_draw_anim, GLuint prog_draw_static, GLuint prog_draw_aabb, std::string path, ScreenGL * screengl);
	~Level();
	void add_static_character(unsigned int idx_texture, glm::vec2 pos, glm::vec2 size, float z);
	void add_anim_character(unsigned int idx_texture, glm::vec2 pos, glm::vec2 size, float z);
	void draw();
	void anim(float elapsed_time);
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);


	std::vector<StaticTexture *> _static_textures;
	std::vector<AnimTexture *> _anim_textures;
	std::vector<StaticObj *> _static_objs;
	std::vector<AnimObj *> _anim_objs;
	std::vector<StaticCharacter *> _static_characters;
	std::vector<AnimCharacter *> _anim_characters;
	
	unsigned int _w, _h;
	float _block_w, _block_h;
	ScreenGL * _screengl;
};


class LevelDebug {
public:
	LevelDebug();
	LevelDebug(GLuint prog_draw_aabb, Level * level, ScreenGL * screengl);
	~LevelDebug();
	void draw();
	void update();
	bool key_down(InputState * input_state, SDL_Keycode key);


	GLuint _vbo;
	GLuint _prog_draw;
	GLint _camera2clip_loc, _model2world_loc, _position_loc, _color_loc, _z_loc;
	glm::mat4 _camera2clip;
	glm::mat4 _model2world;
	ScreenGL * _screengl;
	Level * _level;
	unsigned int _n_aabbs;
	bool _draw_aabb, _draw_footprint;
};


#endif
