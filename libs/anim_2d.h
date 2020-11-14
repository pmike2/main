#ifndef ANIM_2D_H
#define ANIM_2D_H

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "gl_utils.h"
#include "bbox_2d.h"
#include "input_state.h"


// _z doit etre entre ces bornes
const float Z_NEAR= -10.0f;
const float Z_FAR= 10.0f;

const unsigned int ANIM_MODEL_SIZE= 512;

const glm::vec2 MOVE_VIEWPOINT(5.0f, 5.0f);


enum ObjectPhysics {STATIC_DESTRUCTIBLE, STATIC_INDESTRUCTIBLE, STATIC_UNSOLID, FALLING, CHECKPOINT_SOLID, CHECKPOINT_UNSOLID};


class CheckPoint {
public:
	CheckPoint();
	CheckPoint(glm::vec2 pos, float velocity);
	~CheckPoint();


	glm::vec2 _pos;
	float _velocity;
};


class Object2D {
public:
	Object2D();
	Object2D(glm::vec2 pos, glm::vec2 size, glm::vec2 footprint_offset, glm::vec2 footprint_size, ObjectPhysics physics, std::vector<CheckPoint> checkpoints = std::vector<CheckPoint>());
	Object2D(const Object2D & obj);
	~Object2D();
	void update_pos(float elapsed_time);
	void update_velocity();
	void update_footprint_pos();
	void set_aabb_pos(glm::vec2 pos);
	void set_footprint(glm::vec2 footprint_offset, glm::vec2 footprint_size);


	AABB_2D * _aabb;
	glm::vec2 _footprint_offset;
	AABB_2D * _footprint;
	glm::vec2 _velocity;
	ObjectPhysics _physics;
	std::vector<CheckPoint *> _checkpoints;
	unsigned int _idx_checkpoint;
	std::vector<Object2D *> _bottom;
	std::vector<Object2D *> _top;
	Object2D * _referential;
};


bool obj_intersect(const Object2D * anim_obj, const Object2D * static_obj, const float time_step, glm::vec2 & contact_pt, glm::vec2 & contact_normal, float & contact_time);


class Action {
public:
	Action();
	~Action();
	void print();


	std::string _name;
	std::vector<std::string> _pngs;
	unsigned int _first_idx;
	unsigned int _n_idx;
	float _anim_time;
	glm::vec2 _footprint_offset; // entre 0 et 1
	glm::vec2 _footprint_size; // entre 0 et 1
};


class Character2D;

class Texture2D {
public:
	Texture2D();
	Texture2D(GLuint prog_draw, std::string path, ScreenGL * screengl);
	virtual ~Texture2D();
	virtual void draw() = 0;
	virtual void update() = 0;
	void set_model2world(glm::mat4 model2world);


	GLuint _texture_id;
	GLuint _vbo;
	GLuint _prog_draw;
	glm::mat4 _camera2clip;
	glm::mat4 _model2world;
	ScreenGL * _screengl;
	unsigned int _n_aabbs;
	std::vector<Character2D *> _characters;
	std::vector<Action *> _actions;
	std::string _name;
};


class StaticTexture : public Texture2D {
public:
	StaticTexture();
	StaticTexture(GLuint prog_draw, std::string path, ScreenGL * screengl);
	~StaticTexture();
	void draw();
	void update();


	GLint _camera2clip_loc, _model2world_loc, _position_loc, _tex_coord_loc, _tex_loc, _alpha_loc;
	float _alpha;
};


class AnimTexture : public Texture2D {
public:
	AnimTexture();
	AnimTexture(GLuint prog_draw, std::string path, ScreenGL * screengl);
	~AnimTexture();
	void draw();
	void update();


	GLint _camera2clip_loc, _model2world_loc, _position_loc, _tex_coord_loc, _texture_array_loc, _current_layer_loc;
};


class Character2D {
public:
	Character2D();
	Character2D(Object2D * obj, Texture2D * texture, float z);
	virtual ~Character2D();


	Object2D * _obj;
	Texture2D * _texture;
	float _z;
};


class AnimatedCharacter2D : public Character2D {
public:
	AnimatedCharacter2D();
	AnimatedCharacter2D(Object2D * obj, Texture2D * texture, float z);
	~AnimatedCharacter2D();
	void anim(float elapsed_time);
	void set_action(unsigned int idx_action);
	void set_action(std::string action_name);
	std::string current_action();


	Action * _current_action;
	unsigned int _current_anim;
	float _accumulated_time;
};


class Person2D : public AnimatedCharacter2D {
public:
	Person2D();
	Person2D(Object2D * obj, Texture2D * texture, float z);
	~Person2D();
	void update_velocity();
	void update_action();
	void key_down(SDL_Keycode key);
	void key_up(SDL_Keycode key);
	void ia();
	std::string current_direction();
	std::string current_type();


	bool _left_pressed, _right_pressed, _down_pressed, _up_pressed, _lshift_pressed;
	bool _jump;
};


struct SVG_Image {
	AABB _aabb;
	std::string _href;
	float _z;
};

struct SVG_Rect {
	AABB _aabb;
	SVG_Image * _image_model;
};

struct SVG_Path {
	std::vector<glm::vec2> _pts;
	std::vector<float> _speeds;
	SVG_Image * _image;
};

class SVGParser {
public:
	SVGParser();
	SVGParser(std::string svg_path);
	~SVGParser();


	glm::vec2 _pt_hg;
	glm::vec2 _pt_bd;
	std::vector<SVG_Image *> _images;
	std::vector<SVG_Path *> _paths;
	std::vector<SVG_Rect *> _rects;
};


class Level {
public:
	Level();
	Level(GLuint prog_draw_anim, GLuint prog_draw_static, GLuint prog_draw_aabb, std::string path, ScreenGL * screengl);
	~Level();
	Texture2D * get_texture(std::string texture_name);
	void add_character(std::string texture_name, glm::vec2 pos, glm::vec2 size, float z, ObjectPhysics physics, std::string character_type, std::vector<CheckPoint> checkpoints = std::vector<CheckPoint>());
	void delete_character(Character2D * character);
	void update_model2worlds();
	void draw();
	void anim(float elapsed_time);
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);


	std::vector<Texture2D *> _textures;
	std::vector<Character2D *> _characters;
	
	unsigned int _w, _h;
	float _block_w, _block_h;
	ScreenGL * _screengl;
	Person2D * _hero;
	glm::vec2 _viewpoint;
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
