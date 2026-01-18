#ifndef GLIHM
#define GLIHM

#include <vector>
#include <map>
#include <iostream>
#include <string>

#include "json.hpp"

#include "bbox_2d.h"
#include "typedefs.h"
#include "input_state.h"


using json = nlohmann::json;


enum GL_IHM_GROUP_ORIENTATION {HORINZONTAL, VERTICAL};


const number TEXTURE_SIZE = 512;


struct GLIHMElement {
	GLIHMElement();
	GLIHMElement(pt_2d position, pt_2d size);
	~GLIHMElement();
	virtual void click() = 0;


	std::string _name;
	std::string _texture_path;
	AABB_2D * _aabb;
	uint _texture_layer;
};


struct GLIHMButton : public GLIHMElement {
	GLIHMButton();
	~GLIHMButton();


	number _available_percent;
};


struct GLIHMCheckBox : public GLIHMElement {
	GLIHMCheckBox();
	~GLIHMCheckBox();


	bool _active;
};


struct GLIHMRadio : public GLIHMElement {
	GLIHMRadio();
	~GLIHMRadio();


	bool _active;
};


struct GLIHMGroup {
	GLIHMGroup();
	~GLIHMGroup();
	pt_2d next_element_position();


	std::string _name;
	std::map<std::string, GLIHMElement *> _elements;
	GL_IHM_GROUP_ORIENTATION _orientation;
	number _margin;
	pt_2d _position;
	pt_2d _element_size;
};


struct GLIHM {
	GLIHM();
	GLIHM(std::map<std::string, GLuint> progs, std::string json_path);
	~GLIHM();
	void update();
	void draw();
	void anim();
	bool mouse_button_down(InputState * input_state, time_point t);
	bool key_down(InputState * input_state, SDL_Keycode key, time_point t);


	std::map<std::string, GLIHMGroup *> _groups;
	std::map<std::string, DrawContext *> _contexts;
	uint _texture_idx;
	std::string _texture_root;
};


#endif
