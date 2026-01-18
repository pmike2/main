#include <fstream>

#include "gl_ihm.h"


// GLIHMElement ------------------------------------------------------------------
GLIHMElement::GLIHMElement() {

}


GLIHMElement::~GLIHMElement() {

}


// GLIHMButton ------------------------------------------------------------------
GLIHMButton::GLIHMButton() {

}


GLIHMButton::~GLIHMButton() {
	
}


// GLIHMCheckBox ------------------------------------------------------------------
GLIHMCheckBox::GLIHMCheckBox() {

}


GLIHMCheckBox::~GLIHMCheckBox() {

}


// GLIHMRadio --------------------------------------------------------------------
GLIHMRadio::GLIHMRadio() {

}


GLIHMRadio::~GLIHMRadio() {

}


// GLIHMGroup ----------------------------------------------------------------------
GLIHMGroup::GLIHMGroup() {

}


GLIHMGroup::~GLIHMGroup() {
	
}


// GLIHM ---------------------------------------------------------------------------
GLIHM::GLIHM() {

}


GLIHM::GLIHM(std::map<std::string, GLuint> progs, std::string json_path) {
		_contexts["icon"]= new DrawContext(progs["icon"], 
		std::vector<std::string>{"position_in:3", "tex_coord_in:2", "current_layer_in:1"},
		std::vector<std::string>{"camera2clip_matrix", "texture_array"},
		GL_STATIC_DRAW, true);

	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();
	
	_texture_root = js["textures_root"];
	for (auto group : js["groups"]) {
	}

	std::vector<std::string> pngs;
	uint compt = 0;
	for (auto & group : _groups) {
		for (auto & element : group->_elements) {
			pngs.push_back(_texture_root + "/" + element->_texture_path);
			element->_texture_layer = compt++;
		}
	}
	glGenTextures(1, &_texture_idx);
	fill_texture_array(0, _texture_idx, TEXTURE_SIZE, pngs);
}


GLIHM::~GLIHM() {

}


void GLIHM::update() {

}


void GLIHM::draw() {

}


void GLIHM::anim() {

}
