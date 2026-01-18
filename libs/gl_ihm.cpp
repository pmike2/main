#include <fstream>

#include "gl_ihm.h"


// GLIHMElement ------------------------------------------------------------------
GLIHMElement::GLIHMElement() {
	
}


GLIHMElement::GLIHMElement(pt_2d position, pt_2d size) {
	_aabb = new AABB_2D(position, size);
}


GLIHMElement::~GLIHMElement() {
	delete _aabb;
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
	for (auto & element : _elements) {
		delete element.second;
	}
	_elements.clear();
}


pt_2d GLIHMGroup::next_element_position() {

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
		GLIHMGroup * gl_group = new GLIHMGroup();
		gl_group->_name = group["name"];
		gl_group->_margin = group["margin"];
		gl_group->_position = pt_2d(group["position"][0], group["position"][1]);
		gl_group->_element_size = pt_2d(group["element_size"][0], group["element_size"][1]);

		if (group["orientation"] == "horizontal") {
			gl_group->_orientation = HORIZONTAL;
		}
		else {
			gl_group->_orientation = VERTICAL;
		}

		for (auto & button : group["buttons"]) {
			GLIHMButton * gl_button = new GLIHMButton(gl_group->next_element_position(), _element_size);
			gl_button->_name = button["name"];
			gl_button->_texture_path = button["texture"];
			gl_button->_available_percent = 100.0;
			gl_group->_elements.push_back(gl_button);
		}

		for (auto & button : group["checkboxes"]) {
			GLIHMCheckBox * gl_checkbox = new GLIHMCheckBox(gl_group->next_element_position(), _element_size);
			gl_checkbox->_name = button["name"];
			gl_checkbox->_texture_path = button["texture"];
			gl_button->_active = false;
			gl_group->_elements.push_back(gl_checkbox);
		}

		for (auto & button : group["radios"]) {
			GLIHMRadio * gl_radio = new GLIHMRadio(gl_group->next_element_position(), _element_size);
			gl_radio->_name = button["name"];
			gl_radio->_texture_path = button["texture"];
			gl_radio->_active = false;
			gl_group->_elements.push_back(gl_radio);
		}
		
		_groups.push_back(gl_group);
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
	for (auto & group : _groups) {
		delete group.second;
	}
	_groups.clear();
}


void GLIHM::update() {

}


void GLIHM::draw() {

}


void GLIHM::anim() {

}
