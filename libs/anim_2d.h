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


struct Action {
    void print();


    std::string _name;
    std::vector<std::string> _pngs;
    unsigned int _first_idx;
    unsigned int _n_idx;
    unsigned int _n_ms;
    float _move;
};


class Model {
public:
    Model();
    Model(std::string path);
    ~Model();

    
    unsigned int _texture_id;
    std::vector<Action> _actions;
};


class Test {
public:
    Test();
    Test(GLuint prog_draw, ScreenGL * screengl, Model * model);
    void draw();
    void anim(unsigned int n_ms);
    void set_action(std::string action_name);
    void update_model2world();
    void set_size(float size);


    GLuint _vbo;
	GLuint _prog_draw;
	GLint _camera2clip_loc, _model2world_loc, _position_loc, _tex_coord_loc, _texture_array_loc, _current_layer_loc, _next_layer_loc, _interpol_layer_loc;
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
};


class StaticTexture {
public:
    StaticTexture();
    StaticTexture(GLuint prog_draw, ScreenGL * screengl, std::string path);
    ~StaticTexture();


    GLuint _vbo;
	GLuint _prog_draw;
	GLint _camera2clip_loc, _model2world_loc, _position_loc, _tex_coord_loc, _texture_loc;
    glm::mat4 _camera2clip;
    glm::mat4 _model2world;
    ScreenGL * _screengl;
};


#endif
