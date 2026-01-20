#include <fstream>
#include <sstream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "geom_2d.h"

#include "gl_ihm.h"


void fill_aabb2d_data(AABB_2D * aabb, number alpha, uint texture_layer, float * data) {
	const uint idxs[6] = {0, 1, 2, 0, 2, 3};
	pt_4d l_pts[4] = {
		pt_4d(aabb->_pos.x, aabb->_pos.y, 0.0, 1.0),
		pt_4d(aabb->_pos.x + aabb->_size.x, aabb->_pos.y, 1.0, 1.0),
		pt_4d(aabb->_pos.x + aabb->_size.x, aabb->_pos.y + aabb->_size.y, 1.0, 0.0),
		pt_4d(aabb->_pos.x, aabb->_pos.y + aabb->_size.y, 0.0, 0.0)
	};

	float * ptr = data;
	for (uint i=0; i<6; ++i) {
		ptr[0] = float(l_pts[idxs[i]][0]);
		ptr[1] = float(l_pts[idxs[i]][1]);
		ptr[2] = float(l_pts[idxs[i]][2]);
		ptr[3] = float(l_pts[idxs[i]][3]);
		ptr[4] = float(alpha);
		ptr[5] = float(texture_layer);
		ptr += 6;
	}
}


// GLIHMElement ------------------------------------------------------------------
GLIHMElement::GLIHMElement() {
	
}


GLIHMElement::GLIHMElement(GLIHMGroup * group, std::string name, std::string texture_path, pt_2d position, pt_2d size) :
	_group(group), _name(name), _texture_path(texture_path), _alpha(ALPHA_INACTIVE), _active(false), _key(0) {
	_aabb = new AABB_2D(position, size);
	_active_callback = [](){};
	_inactive_callback = [](){};

	if (_group->_type == GL_IHM_SLIDER) {
		_n_pts = 12;
	}
	else {
		_n_pts = 6;
	}
	_data = new float[_n_pts * 6];
}


GLIHMElement::~GLIHMElement() {
	delete _aabb;
	delete[] _data;
}


void GLIHMElement::set_active() {
	_active = true;
	_alpha = ALPHA_ACTIVE;
	for (auto & group : _groups_visible) {
		group->_visible = true;
	}
	_active_callback();
}


void GLIHMElement::set_inactive() {
	_active = false;
	_alpha = ALPHA_INACTIVE;
	for (auto & group : _groups_visible) {
		group->_visible = false;
	}
	_inactive_callback();
}


void GLIHMElement::set_callback(std::function<void(void)> active_callback, std::function<void(void)> inactive_callback) {
	_active_callback = active_callback;
	_inactive_callback = inactive_callback;
}


std::ostream & operator << (std::ostream & os, const GLIHMElement & e) {
	os << "name = " << e._name << " ; texture_path = " << e._texture_path << " ; texture_layer = " << e._texture_layer;
	os << " ; alpha = " << e._alpha << " ; aabb = " << *e._aabb << " ; active = " << e._active;
	return os;
}


// GLIHMButton ------------------------------------------------------------------
GLIHMButton::GLIHMButton() {

}


GLIHMButton::GLIHMButton(GLIHMGroup * group, std::string name, std::string texture_path, pt_2d position, pt_2d size) :
	GLIHMElement(group, name, texture_path, position, size), _available_percent(100.0) 
{

}


GLIHMButton::~GLIHMButton() {
	
}


void GLIHMButton::click(bool verbose, pt_2d pt) {
	if (verbose) {
		std::cout << "GLIHMButton " << _name << " clicked\n";
	}

	set_active();
	set_inactive();
}


void GLIHMButton::update_data() {
	number alpha = _alpha;
	if (!_group->_visible) {
		alpha = 0.0;
	}
	fill_aabb2d_data(_aabb, alpha, _texture_layer, _data);
}


// GLIHMCheckBox ------------------------------------------------------------------
GLIHMCheckBox::GLIHMCheckBox() {

}


GLIHMCheckBox::GLIHMCheckBox(GLIHMGroup * group, std::string name, std::string texture_path, pt_2d position, pt_2d size) :
	GLIHMElement(group, name, texture_path, position, size) 
{

}


GLIHMCheckBox::~GLIHMCheckBox() {

}


void GLIHMCheckBox::click(bool verbose, pt_2d pt) {
	if (verbose) {
		std::cout << "GLIHMCheckBox " << _name << " clicked\n";
	}

	if (_active){
		set_inactive();
	}
	else {
		set_active();
	}
}


void GLIHMCheckBox::update_data() {
	number alpha = _alpha;
	if (!_group->_visible) {
		alpha = 0.0;
	}
	fill_aabb2d_data(_aabb, alpha, _texture_layer, _data);
}


// GLIHMRadio --------------------------------------------------------------------
GLIHMRadio::GLIHMRadio() {

}


GLIHMRadio::GLIHMRadio(GLIHMGroup * group, std::string name, std::string texture_path, pt_2d position, pt_2d size) :
	GLIHMElement(group, name, texture_path, position, size) 
{

}


GLIHMRadio::~GLIHMRadio() {

}


void GLIHMRadio::click(bool verbose, pt_2d pt) {
	if (verbose) {
		std::cout << "GLIHMRadio " << _name << " clicked\n";
	}

	set_active();
	for (auto & element : _group->_elements) {
		if (element != this) {
			element->set_inactive();
		}
	}
}


void GLIHMRadio::update_data() {
	number alpha = _alpha;
	if (!_group->_visible) {
		alpha = 0.0;
	}
	fill_aabb2d_data(_aabb, alpha, _texture_layer, _data);
}


// GLIHMSlider --------------------------------------------------------------------
GLIHMSlider::GLIHMSlider() {

}


GLIHMSlider::GLIHMSlider(GLIHMGroup * group, std::string name, std::string texture_path, pt_2d position, pt_2d size) :
	GLIHMElement(group, name, texture_path, position, size), _min_value(0.0), _max_value(1.0) 
{
	
}


GLIHMSlider::~GLIHMSlider() {

}


void GLIHMSlider::click(bool verbose, pt_2d pt) {
	if (verbose) {
		std::cout << "GLIHMSlider " << _name << " clicked\n";
	}

	number lambda;
	if (_group->_orientation == GL_IHM_HORIZONTAL) {
		lambda = (pt.y - _aabb->_pos.y) / _aabb->_size.y;
	}
	else if (_group->_orientation == GL_IHM_VERTICAL) {
		lambda = (pt.x - _aabb->_pos.x) / _aabb->_size.x;
	}
	_value = _min_value + lambda * (_max_value - _min_value);

	set_active();
	set_inactive();
}


void GLIHMSlider::update_data() {
	number alpha = _alpha;
	if (!_group->_visible) {
		alpha = 0.0;
	}

	AABB_2D * aabb_cursor = new AABB_2D();
	number lambda = (_value - _min_value) / (_max_value - _min_value);
	if (_group->_orientation == GL_IHM_HORIZONTAL) {
		aabb_cursor->_size.x = aabb_cursor->_size.y = _aabb->_size.x;
		aabb_cursor->_pos.x = _aabb->_pos.x;
		aabb_cursor->_pos.y = _aabb->_pos.y + lambda * _aabb->_size.y - 0.5 * _aabb->_size.x;
		if (aabb_cursor->_pos.y < _aabb->_pos.y) {
			aabb_cursor->_pos.y = _aabb->_pos.y;
		}
		if (aabb_cursor->_pos.y > _aabb->_pos.y + _aabb->_size.y - _aabb->_size.x) {
			aabb_cursor->_pos.y = _aabb->_pos.y + _aabb->_size.y - _aabb->_size.x;
		}
	}
	else if (_group->_orientation == GL_IHM_VERTICAL) {
		aabb_cursor->_size.x = aabb_cursor->_size.y = _aabb->_size.x;
		aabb_cursor->_pos.x = _aabb->_pos.x + lambda * _aabb->_size.x - 0.5 * _aabb->_size.y;
		aabb_cursor->_pos.y = _aabb->_pos.y;
		if (aabb_cursor->_pos.x < _aabb->_pos.x) {
			aabb_cursor->_pos.x = _aabb->_pos.x;
		}
		if (aabb_cursor->_pos.x > _aabb->_pos.x + _aabb->_size.x - _aabb->_size.y) {
			aabb_cursor->_pos.x = _aabb->_pos.x + _aabb->_size.x - _aabb->_size.y;
		}
	}

	//std::cout << "aabb = " << *_aabb << "\n";
	//std::cout << "aabb_cursor = " << *aabb_cursor << "\n";
	
	fill_aabb2d_data(aabb_cursor, alpha, _texture_layer, _data);
	fill_aabb2d_data(_aabb, alpha, 0, _data + 36);
	
	delete aabb_cursor;
}


// GLIHMGroup ----------------------------------------------------------------------
GLIHMGroup::GLIHMGroup() {

}


GLIHMGroup::GLIHMGroup(std::string name, GL_IHM_GROUP_TYPE type, GL_IHM_GROUP_ORIENTATION orientation, pt_2d position, pt_2d element_size, number margin) : 
	_name(name), _type(type), _orientation(orientation), _position(position), _element_size(element_size), _margin(margin), _visible(true)
{

}


GLIHMGroup::~GLIHMGroup() {
	for (auto & element : _elements) {
		delete element;
	}
	_elements.clear();
}


pt_2d GLIHMGroup::next_element_position() {
	number n = _elements.size();
	
	pt_2d last_pos;
	number margin;
	if (n == 0) {
		last_pos = _position;
		margin = 0.0;
	}
	else {
		last_pos = _elements[n - 1]->_aabb->_pos;
		margin = _margin;
	}

	pt_2d next_pos;
	if (_orientation == GL_IHM_HORIZONTAL) {
		next_pos = last_pos + pt_2d(margin + _element_size.x, 0.0);
	}
	else if (_orientation == GL_IHM_VERTICAL) {
		next_pos = last_pos - pt_2d(0.0, margin + _element_size.y);
	}

	return next_pos;
}


GLIHMElement * GLIHMGroup::add_element(std::string name, std::string texture_path) {
	if (_type == GL_IHM_BUTTON) {
		GLIHMButton * button = new GLIHMButton(this, name, texture_path, next_element_position(), _element_size);
		_elements.push_back(button);
		return button;
	}
	else if (_type == GL_IHM_CHECKBOX) {
		GLIHMCheckBox * checkbox = new GLIHMCheckBox(this, name, texture_path, next_element_position(), _element_size);
		_elements.push_back(checkbox);
		return checkbox;
	}
	else if (_type == GL_IHM_RADIO) {
		GLIHMRadio * radio = new GLIHMRadio(this, name, texture_path, next_element_position(), _element_size);
		_elements.push_back(radio);
		return radio;
	}
	else if (_type == GL_IHM_SLIDER) {
		GLIHMSlider * slider = new GLIHMSlider(this, name, texture_path, next_element_position(), _element_size);
		_elements.push_back(slider);
		return slider;
	}
	return NULL;
}


std::ostream & operator << (std::ostream & os, const GLIHMGroup & g) {
	os << "name = " << g._name << " ; elements =\n";
	for (auto & element : g._elements) {
		os << "\t" << *element << "\n";
	}
	return os;
}


// GLIHM ---------------------------------------------------------------------------
GLIHM::GLIHM() {

}


GLIHM::GLIHM(std::map<std::string, GLuint> progs, ScreenGL * screengl, std::string json_path) : _screengl(screengl), _verbose(false) {
		_contexts["gl_ihm"]= new DrawContext(progs["gl_ihm"], 
		std::vector<std::string>{"position_in:2", "tex_coord_in:2", "alpha_in:1", "current_layer_in:1"},
		std::vector<std::string>{"camera2clip_matrix", "texture_array", "z"},
		GL_STATIC_DRAW, true);

	std::vector<std::pair<GLIHMElement *, std::string> > groups_visible;

	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();
	
	_texture_root = js["texture_root"];
	_texture_size = js["texture_size"];
	std::string texture_slider_background = js["texture_slider_background"];
	_default_element_size = pt_2d(js["default_element_size"][0], js["default_element_size"][1]);
	_default_margin = js["default_margin"];

	for (auto group : js["groups"]) {
		GL_IHM_GROUP_ORIENTATION orientation;
		if (group["orientation"] == "horizontal") {
			orientation = GL_IHM_HORIZONTAL;
		}
		else if (group["orientation"] == "vertical") {
			orientation = GL_IHM_VERTICAL;
		}
		else {
			std::cerr << "GLIHM : orientation = " << group["orientation"] << " non supportée\n";
			return;
		}

		GL_IHM_GROUP_TYPE type;
		if (group["type"] == "button") {
			type = GL_IHM_BUTTON;
		}
		else if (group["type"] == "checkbox") {
			type = GL_IHM_CHECKBOX;
		}
		else if (group["type"] == "radio") {
			type = GL_IHM_RADIO;
		}
		else if (group["type"] == "slider") {
			type = GL_IHM_SLIDER;
		}
		else {
			std::cerr << "GLIHM : type = " << group["type"] << " non supporté\n";
			return;
		}

		pt_2d position = pt_2d(group["position"][0], group["position"][1]);
		pt_2d element_size = _default_element_size;
		if (group["element_size"] != nullptr) {
			element_size = pt_2d(group["element_size"][0], group["element_size"][1]);
		}
		number margin = _default_margin;
		if (group["margin"] != nullptr) {
			margin = group["margin"];
		}

		GLIHMGroup * gl_group = add_group(group["name"], type, orientation, position, element_size, margin);

		for (auto & element : group["elements"]) {
			GLIHMElement * gl_element = gl_group->add_element(element["name"], element["texture"]);
			if (element["groups_visible"] != nullptr) {
				for (auto group_visible : element["groups_visible"]) {
					groups_visible.push_back(std::make_pair(gl_element, group_visible));
				}
			}
			if (element["key"] != nullptr) {
				gl_element->_key = std::string(element["key"]).c_str()[0];
			}
			if (gl_group->_type == GL_IHM_CHECKBOX && element["checked"] != nullptr) {
				gl_element->set_active();
			}
			if (element["min_value"] != nullptr) {
				GLIHMSlider * slider = (GLIHMSlider *)(gl_element);
				slider->_min_value = element["min_value"];
				slider->_max_value = element["max_value"];
				slider->_value = slider->_min_value;
			}
		}
	}

	for (auto & group_visible : groups_visible) {
		for (auto & group : _groups) {
			if (group->_name == group_visible.second) {
				group_visible.first->_groups_visible.push_back(group);
				break;
			}
		}
	}

	for (auto & group : _groups) {
		if (group->_type == GL_IHM_RADIO) {
			group->_elements[0]->set_active();
		}
	}

	for (auto & group : _groups) {
		for (auto & element : group->_elements) {
			if (!element->_active) {
				element->set_inactive();
			}
		}
	}

	std::vector<std::string> pngs;
	pngs.push_back(_texture_root + "/" + texture_slider_background);
	uint compt = 1;
	for (auto & group : _groups) {
		for (auto & element : group->_elements) {
			pngs.push_back(_texture_root + "/" + group->_name + "/" + element->_texture_path);
			element->_texture_layer = compt++;
		}
	}
	glGenTextures(1, &_texture_idx);
	fill_texture_array(0, _texture_idx, _texture_size, pngs);

	_camera2clip= glm::ortho(float(-_screengl->_gl_width)* 0.5f, float(_screengl->_gl_width)* 0.5f, -float(_screengl->_gl_height)* 0.5f, float(_screengl->_gl_height)* 0.5f, Z_NEAR, Z_FAR);

	update();
}


GLIHM::~GLIHM() {
	for (auto & group : _groups) {
		delete group;
	}
	_groups.clear();
}


GLIHMGroup * GLIHM::add_group(std::string name, GL_IHM_GROUP_TYPE type, GL_IHM_GROUP_ORIENTATION orientation, pt_2d position, pt_2d element_size, number margin) {
	GLIHMGroup * group = new GLIHMGroup(name, type, orientation, position, element_size, margin);
	_groups.push_back(group);
	return group;
}


GLIHMElement * GLIHM::get_element(std::string group_name, std::string element_name) {
	for (auto & group : _groups) {
		if (group->_name == group_name) {
			for (auto & element : group->_elements) {
				if (element->_name == element_name) {
					return element;
				}
			}
		}
	}
	std::cerr << "GLIHM::get_element : group = " << group_name << " ; element = " << element_name << " inexistant\n";
	return NULL;
}


void GLIHM::all_callbacks() {
	for (auto & group : _groups) {
		for (auto & element : group->_elements) {
			// pour les sliders on force l'appel au callback actif pour qu'il init bien le prog appelant
			if (element->_active || group->_type == GL_IHM_SLIDER) {
				if (_verbose) {
					std::cout << "GLIHM active callback " << group->_name << " / " << element->_name << "\n";
				}
				element->_active_callback();
			}
			else {
				if (_verbose) {
					std::cout << "GLIHM inactive callback " << group->_name << " / " << element->_name << "\n";
				}
				element->_inactive_callback();
			}
		}
	}
}


void GLIHM::update() {
	DrawContext * context= _contexts["gl_ihm"];

	context->_n_pts = 0;
	for (auto & group : _groups) {
		for (auto & element : group->_elements) {
			context->_n_pts += element->_n_pts;
		}
	}

	float * data = new float[context->data_size()];
	float * ptr = data;
	//const uint idxs[6] = {0, 1, 2, 0, 2, 3};
	for (auto & group : _groups) {
		for (auto & element : group->_elements) {
			/*pt_4d l_pts[4] = {
				pt_4d(element->_aabb->_pos.x, element->_aabb->_pos.y, 0.0, 1.0),
				pt_4d(element->_aabb->_pos.x + element->_aabb->_size.x, element->_aabb->_pos.y, 1.0, 1.0),
				pt_4d(element->_aabb->_pos.x + element->_aabb->_size.x, element->_aabb->_pos.y + element->_aabb->_size.y, 1.0, 0.0),
				pt_4d(element->_aabb->_pos.x, element->_aabb->_pos.y + element->_aabb->_size.y, 0.0, 0.0)
			};

			number alpha = element->_alpha;
			if (!group->_visible) {
				alpha = 0.0;
			}

			for (uint i=0; i<6; ++i) {
				ptr[0] = float(l_pts[idxs[i]][0]);
				ptr[1] = float(l_pts[idxs[i]][1]);
				ptr[2] = float(l_pts[idxs[i]][2]);
				ptr[3] = float(l_pts[idxs[i]][3]);
				ptr[4] = float(alpha);
				ptr[5] = float(element->_texture_layer);
				ptr += 6;
			}*/
			element->update_data();
			for (uint i=0; i<element->_n_pts * 6; ++i) {
				ptr[i] = element->_data[i];
			}
			ptr += element->_n_pts * 6;
		}
	}
	context->set_data(data);
	delete[] data;
	//context->show_data();
}


void GLIHM::draw() {
	DrawContext * context= _contexts["gl_ihm"];
	if (!context->_active) {
		return;
	}

	context->activate();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_idx);
	glActiveTexture(0);
	glUniform1i(context->_locs_uniform["texture_array"], 0);
	glUniform1f(context->_locs_uniform["z"], Z_IHM);
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);
	context->deactivate();
}


void GLIHM::anim() {

}


bool GLIHM::mouse_button_down(InputState * input_state, time_point t) {
	pt_2d pt = _screengl->screen2gl(input_state->_x, input_state->_y);
	for (auto & group : _groups) {
		if (!group->_visible) {
			continue;
		}
		for (auto & element : group->_elements) {
			if (point_in_aabb2d(pt, element->_aabb)) {
				element->click(_verbose, pt);
				update();
				return true;
			}
		}
	}
	return false;
}


bool GLIHM::key_down(InputState * input_state, SDL_Keycode key, time_point t) {
	for (auto & group : _groups) {
		if (!group->_visible) {
			continue;
		}
		for (auto & element : group->_elements) {
			if (element->_key == key) {
				element->click(_verbose, pt_2d(0.0));
				update();
				return true;
			}
		}
	}
	return false;
}


std::ostream & operator << (std::ostream & os, const GLIHM & ihm) {
	os << "texture_idx = " << ihm._texture_idx << " ; texture_root = " << ihm._texture_root << " ; groups =\n";
	for (auto & group : ihm._groups) {
		os << *group;
	}
	return os;
}
