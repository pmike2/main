#ifndef GLIHM
#define GLIHM

#include <vector>
#include <map>
#include <iostream>
#include <string>

#include "json.hpp"

#include "bbox_2d.h"
#include "typedefs.h"


using json = nlohmann::json;


enum GL_IHM_GROUP_ORIENTATION {HORINZONTAL, VERTICAL};


const number TEXTURE_SIZE = 512;


struct GLIHMElement {
	GLIHMElement();
	~GLIHMElement();


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


	std::string _name;
	std::map<std::string, GLIHMElement *> _elements;
	GL_IHM_GROUP_ORIENTATION _orientation;
	number _margin;
};


struct GLIHM {
	GLIHM();
	GLIHM(std::map<std::string, GLuint> progs, std::string json_path);
	~GLIHM();
	void update();
	void draw();
	void anim();


	std::map<std::string, GLIHMGroup *> _groups;
	std::map<std::string, DrawContext *> _contexts;
	uint _texture_idx;
	std::string _texture_root;
};


#endif
