#ifndef GL_INTERFACE_H
#define GL_INTERFACE_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <math.h>
#include <functional>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SDL2/SDL_keycode.h>

#include "input_state.h"


const float DELTA_SLIDER= 0.01f;
const uint VSLIDER_DEFAULT_WIDTH= 15;
const uint VSLIDER_DEFAULT_HEIGHT= 300;
const uint BUTTON_DEFAULT_WIDTH= 15;
const uint BUTTON_DEFAULT_HEIGHT= 15;
const glm::vec3 BUTTON_COLOR_CLICKED= glm::vec3(0.3f, 0.9f, 0.9f);
const glm::vec3 BUTTON_COLOR_UNCLICKED= glm::vec3(0.3f, 0.5f, 0.5f);
const glm::vec3 SWITCH_COLOR_ACTIVE= glm::vec3(0.9f, 0.9f, 0.3f);
const glm::vec3 SWITCH_COLOR_INACTIVE= glm::vec3(0.5f, 0.5f, 0.3f);

enum InterfaceObjectType {OBJECT_TYPE_NULL, OBJECT_TYPE_VERTICAL_SLIDER, OBJECT_TYPE_BUTTON, OBJECT_TYPE_SWITCH};


class InterfaceObject {
public:
	InterfaceObject();
	InterfaceObject(GLuint prog_draw, std::string id, uint i, uint j, uint width, uint height, uint screen_width, uint screen_height);
	~InterfaceObject();
	virtual void compute_data() = 0;
	void draw();
	glm::vec2 screen2gl(glm::uvec2 v);
	bool clicked(uint i, uint j);


	std::string _id;
	bool _clicked, _visible;
	uint _i, _j, _width, _height, _screen_width, _screen_height;
	InterfaceObjectType _type;

	GLuint _prog_draw;
	GLint _camera2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffer;
	uint _n_vertices;
	float * _data;
	float _camera2clip[16];
};


class VerticalSlider : public InterfaceObject {
public:
	VerticalSlider();
	VerticalSlider(GLuint prog_draw, std::string id, uint i, uint j, uint width, uint height, uint screen_width, uint screen_height, float value_min, float value_max, std::function<void(VerticalSlider * vs)> f);
	~VerticalSlider();
	void compute_data();
	float value2gl();
	void update_value(uint j);
	

	std::function<void(VerticalSlider * vs)> _f;
	float _value, _value_min, _value_max;
	glm::vec3 _color_bckgnd, _color_cursor;
};


class Button : public InterfaceObject {
public:
	Button();
	Button(GLuint prog_draw, std::string id, uint i, uint j, uint width, uint height, uint screen_width, uint screen_height, std::function<void(Button * b)> f);
	~Button();
	void compute_data();


	std::function<void(Button * b)> _f;	
	glm::vec3 _color_clicked, _color_unclicked;
};


class Switch : public InterfaceObject {
public:
	Switch();
	Switch(GLuint prog_draw, std::string id, uint i, uint j, uint width, uint height, uint screen_width, uint screen_height, std::function<void(Switch * s)> f);
	~Switch();
	void compute_data();


	std::function<void(Switch * s)> _f;
	bool _active;
	glm::vec3 _color_active, _color_inactive;
};


class IHM {
public:
	IHM();
	IHM(GLuint prog_draw, uint screen_width, uint screen_height);
	~IHM();
	void draw();
	void add_vslider(std::string id, uint i, uint j, float value_min, float value_max, std::function<void(VerticalSlider * vs)> f);
	void add_button(std::string id, uint i, uint j, std::function<void(Button * b)> f);
	void add_switch(std::string id, uint i, uint j, std::function<void(Switch * s)> f);
	bool mouse_button_down(InputState * input_state);
	bool mouse_button_up(InputState * input_state);
	bool mouse_motion(InputState * input_state);
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	void toggle_visible();


	GLuint _prog_draw;
	uint _screen_width, _screen_height;
	std::vector<VerticalSlider *> _vsliders;
	std::vector<Button *> _buttons;
	std::vector<Switch *> _switchs;
};


#endif
