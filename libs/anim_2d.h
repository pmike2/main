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
const unsigned int MODEL_SIZE= 512;


enum obstacle_type {AIR, SOLIDE};
enum move_type {LEFT, RIGHT, UP, DOWN, WAIT};


class StaticTexture {
public:
    StaticTexture();
    StaticTexture(GLuint prog_draw, ScreenGL * screengl, std::string path);
    ~StaticTexture();
    void draw();
    void set_blocs(std::vector<AABB_2D *> blocs);


    GLuint _texture_id;
    GLuint _vbo;
	GLuint _prog_draw;
	GLint _camera2clip_loc, _model2world_loc, _position_loc, _tex_coord_loc, _tex_loc, _alpha_loc, _z_loc;
    glm::mat4 _camera2clip;
    glm::mat4 _model2world;
    ScreenGL * _screengl;
    float _alpha;
    unsigned int _n_blocs;
    float _size;
    float _z;
};


struct Action {
    void print();


    std::string _name;
    std::vector<std::string> _pngs;
    unsigned int _first_idx;
    unsigned int _n_idx;
    unsigned int _n_ms;
    move_type _move_type;
    glm::vec2 _speed_begin;
    glm::vec2 _speed_end;
    glm::vec2 _acceleration;
};


class Model {
public:
    Model();
    Model(std::string path);
    ~Model();
    Action get_action(std::string name);

    
    GLuint _texture_id;
    std::vector<Action> _actions;
    AABB_2D * _footprint;
};


class AnimTexture {
public:
    AnimTexture();
    AnimTexture(GLuint prog_draw, GLuint prog_draw_footprint, ScreenGL * screengl, Model * model);
    void draw();
    void anim(unsigned int n_ms);
    void set_action(std::string action_name);
    Action get_current_action();
    void update_model2world();
    void set_size(float size);


    GLuint _vbo, _vbo_footprint;
	GLuint _prog_draw, _prog_draw_footprint;
	GLint _camera2clip_loc, _model2world_loc, _position_loc, _tex_coord_loc, _texture_array_loc, _current_layer_loc, _next_layer_loc, _interpol_layer_loc, _z_loc;
    GLint _camera2clip_fp_loc, _model2world_fp_loc, _position_fp_loc, _color_fp_loc, _z_fp_loc;
    glm::mat4 _camera2clip;
    glm::mat4 _model2world;
    ScreenGL * _screengl;
    unsigned int _current_anim, _next_anim;
    unsigned int _first_ms;
    float _interpol_anim;
    glm::vec2 _position;
    unsigned int _current_action_idx;
    Model * _model;
    float _size;
    float _z;
	bool _go_right, _go_left, _go_up, _go_down;
	bool _falling;
	unsigned int _n_ms_start_falling;
    glm::vec2 _speed;
	glm::vec2 _acceleration;
};


class Level {
public:
    Level();
    Level(GLuint prog_draw_anim, GLuint prog_draw_footprint, GLuint prog_draw_static, ScreenGL * screengl, unsigned int w, unsigned int h);
    ~Level();
    void randomize();
    void draw();
    void anim(unsigned int n_ms);
	void ia();
	void sync_obstacles_bricks();
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);


    std::vector<AnimTexture *> _anim_textures;
    std::vector<Model *> _models;
    std::vector<StaticTexture *> _static_textures;
    std::vector<obstacle_type> _obstacles;
    unsigned int _w, _h;
	float _block_w, _block_h;
	ScreenGL * _screengl;
    bool _left_pressed, _right_pressed, _down_pressed, _up_pressed;
};

#endif
