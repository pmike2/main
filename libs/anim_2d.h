#ifndef ANIM_2D_H
#define ANIM_2D_H

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <dirent.h>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

// cf bug : https://stackoverflow.com/questions/14113923/rapidxml-print-header-has-undefined-methods
#include "rapidxml_ext.h"

#include "utile.h"
#include "gl_utils.h"
#include "bbox_2d.h"
#include "input_state.h"


// _z doit etre entre ces bornes
const float Z_NEAR= -10.0f;
const float Z_FAR= 10.0f;

const unsigned int ANIM_MODEL_SIZE= 512;

const unsigned int LEVEL_WIDTH= 16;
const unsigned int LEVEL_HEIGHT= 16;

const float ANIM_TIME= 0.2f;



struct Action {
	void print();


	std::string _name;
	std::vector<std::string> _pngs;
	unsigned int _first_idx;
	unsigned int _n_idx;
};


class StaticObj {
public:
	StaticObj();
	StaticObj(glm::vec2 pos, glm::vec2 size, glm::vec2 footprint_offset, glm::vec2 footprint_size);
	~StaticObj();
	void update_footprint_pos();

	
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


	AABB_2D * _aabb;
	glm::vec2 _footprint_offset;
	AABB_2D * _footprint;
	glm::vec2 _velocity;
};


bool anim_intersect_static(const AnimObj * anim_obj, const StaticObj * static_obj, const float time_step, glm::vec2 & contact_pt, glm::vec2 & contact_normal, float & contact_time);


class StaticCharacter;

class StaticTexture {
public:
	StaticTexture();
	StaticTexture(GLuint prog_draw, std::string path, ScreenGL * screengl);
	~StaticTexture();
	void draw();
	void update(std::vector<StaticCharacter *> static_chars);


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
	void update(std::vector<AnimCharacter *> anim_chars);


	GLuint _texture_id;
	GLuint _vbo;
	GLuint _prog_draw;
	GLint _camera2clip_loc, _model2world_loc, _position_loc, _tex_coord_loc, _texture_array_loc, _current_layer_loc;
	glm::mat4 _camera2clip;
	glm::mat4 _model2world;
	ScreenGL * _screengl;
	unsigned int _n_aabbs;
	std::vector<Action *> _actions;
	glm::vec2 _footprint_offset; // entre 0 et 1
	glm::vec2 _footprint_size; // entre 0 et 1
};


class AnimCharacter {
public:
	AnimCharacter();
	AnimCharacter(AnimObj * anim_obj, AnimTexture * anim_texture, float z);
	~AnimCharacter();
	void anim(float elapsed_time);
	void set_action(unsigned int idx_action);
	void set_action(std::string action_name);


	AnimObj * _anim_obj;
	AnimTexture * _anim_texture;
	float _z;
	unsigned int _current_anim;
	float _accumulated_time;
	Action * _current_action;
};


class Level {
public:
	Level();
	Level(GLuint prog_draw_anim, GLuint prog_draw_static, GLuint prog_draw_aabb, ScreenGL * screengl);
	~Level();
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
	bool _left_pressed, _right_pressed, _down_pressed, _up_pressed;
};


class LevelDebug {
public:
	LevelDebug();
	LevelDebug(GLuint prog_draw_aabb, Level * level, ScreenGL * screengl);
	~LevelDebug();
	void draw();
	void update();


	GLuint _vbo;
	GLuint _prog_draw;
	GLint _camera2clip_loc, _model2world_loc, _position_loc, _color_loc, _z_loc;
	glm::mat4 _camera2clip;
	glm::mat4 _model2world;
	ScreenGL * _screengl;
	Level * _level;
	unsigned int _n_aabbs;
};


#endif
