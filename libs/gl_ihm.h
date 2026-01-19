#ifndef GLIHM_H
#define GLIHM_H

#include <vector>
#include <map>
#include <iostream>
#include <string>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "json.hpp"

#include "bbox_2d.h"
#include "typedefs.h"
#include "input_state.h"
#include "gl_utils.h"


using json = nlohmann::json;


enum GL_IHM_GROUP_ORIENTATION {GL_IHM_HORIZONTAL, GL_IHM_VERTICAL};
enum GL_IHM_GROUP_TYPE {GL_IHM_BUTTON, GL_IHM_CHECKBOX, GL_IHM_RADIO};


const number TEXTURE_SIZE = 512;
const float Z_NEAR = 0.0f;
const float Z_FAR = 1000.0f;
const float Z_IHM = 100.0f;
const number ALPHA_ACTIVE = 1.0;
const number ALPHA_INACTIVE = 0.5;


struct GLIHMGroup;

struct GLIHMElement {
	GLIHMElement();
	GLIHMElement(GLIHMGroup * group, std::string name, std::string texture_path, pt_2d position, pt_2d size);
	virtual ~GLIHMElement();
	virtual void click(bool verbose) = 0;
	void set_active();
	void set_inactive();
	void set_callback(std::function<void(void)> active_callback, std::function<void(void)> inactive_callback = [](){});
	friend std::ostream & operator << (std::ostream & os, const GLIHMElement & e);


	GLIHMGroup * _group;
	std::string _name;
	std::string _texture_path;
	uint _texture_layer;
	number _alpha;
	AABB_2D * _aabb;
	std::vector<GLIHMGroup *> _groups_visible;
	bool _active;
	std::function<void(void)> _active_callback, _inactive_callback;
	SDL_Keycode _key;
};


struct GLIHMButton : public GLIHMElement {
	GLIHMButton();
	GLIHMButton(GLIHMGroup * group, std::string name, std::string texture_path, pt_2d position, pt_2d size);
	~GLIHMButton();
	void click(bool verbose);


	number _available_percent;
};


struct GLIHMCheckBox : public GLIHMElement {
	GLIHMCheckBox();
	GLIHMCheckBox(GLIHMGroup * group, std::string name, std::string texture_path, pt_2d position, pt_2d size);
	~GLIHMCheckBox();
	void click(bool verbose);


};


struct GLIHMRadio : public GLIHMElement {
	GLIHMRadio();
	GLIHMRadio(GLIHMGroup * group, std::string name, std::string texture_path, pt_2d position, pt_2d size);
	~GLIHMRadio();
	void click(bool verbose);


};


struct GLIHMGroup {
	GLIHMGroup();
	GLIHMGroup(std::string name, GL_IHM_GROUP_TYPE type, GL_IHM_GROUP_ORIENTATION orientation, pt_2d position, pt_2d element_size, number margin);
	~GLIHMGroup();
	pt_2d next_element_position();
	GLIHMElement * add_element(std::string name, std::string texture_path);
	friend std::ostream & operator << (std::ostream & os, const GLIHMGroup & g);


	std::string _name;
	std::vector<GLIHMElement *> _elements;
	GL_IHM_GROUP_TYPE _type;
	GL_IHM_GROUP_ORIENTATION _orientation;
	pt_2d _position;
	pt_2d _element_size;
	number _margin;
	bool _visible;
};


struct GLIHM {
	GLIHM();
	GLIHM(std::map<std::string, GLuint> progs, ScreenGL * screengl, std::string json_path);
	~GLIHM();
	GLIHMGroup * add_group(std::string name, GL_IHM_GROUP_TYPE type, GL_IHM_GROUP_ORIENTATION orientation, pt_2d position, pt_2d element_size, number margin);
	GLIHMElement * get_element(std::string group_name, std::string element_name);
	void all_callbacks();
	void update();
	void draw();
	void anim();
	bool mouse_button_down(InputState * input_state, time_point t);
	bool key_down(InputState * input_state, SDL_Keycode key, time_point t);
	friend std::ostream & operator << (std::ostream & os, const GLIHM & ihm);


	std::vector<GLIHMGroup *> _groups;
	std::map<std::string, DrawContext *> _contexts;
	uint _texture_idx;
	std::string _texture_root;
	glm::mat4 _camera2clip;
	ScreenGL * _screengl;
	bool _verbose;
	pt_2d _default_element_size;
	number _default_margin;
};


#endif
