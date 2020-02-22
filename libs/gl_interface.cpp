
#include "gl_interface.h"

using namespace std;


InterfaceObject::InterfaceObject() {

}


InterfaceObject::InterfaceObject(GLuint prog_draw, string id, unsigned int i, unsigned int j, unsigned int width, unsigned int height, unsigned int screen_width, unsigned int screen_height) :
	_prog_draw(prog_draw), _id(id), _i(i), _j(j), _width(width), _height(height), _screen_width(screen_width), _screen_height(screen_height), _clicked(false), _visible(true), _type(OBJECT_TYPE_NULL)
{
	glGenBuffers(1, &_buffer);

	glUseProgram(_prog_draw);
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_camera2clip_loc= glGetUniformLocation(_prog_draw, "camera2clip_matrix");
	glUseProgram(0);

	// on veut X, Y entre -1 et 1
	glm::mat4 glm_ortho= glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	memcpy(_camera2clip, glm::value_ptr(glm_ortho), sizeof(float)* 16);
}


InterfaceObject::~InterfaceObject() {
	delete[] _data;
}


void InterfaceObject::draw() {
	if (!_visible) {
		return;
	}

	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);

	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, _camera2clip);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, _n_vertices);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


glm::vec2 InterfaceObject::screen2gl(glm::uvec2 v) {
	return glm::vec2(2.0f* float(v[0])/ float(_screen_width)- 1.0f, -2.0f* float(v[1])/ float(_screen_height)+ 1.0f);
}


bool InterfaceObject::clicked(unsigned int i, unsigned int j) {
	if (!_visible) {
		return false;
	}

	if ((i>= _i) && (i<= _i+ _width) && (j>= _j) && (j<= _j+ _height)) {
		return true;
	}
	return false;
}


// ----------------------------------------------------------------------------------------------------------------------
VerticalSlider::VerticalSlider() : InterfaceObject() {

}


VerticalSlider::VerticalSlider(GLuint prog_draw, string id, unsigned int i, unsigned int j, unsigned int width, unsigned int height, unsigned int screen_width, unsigned int screen_height, float value_min, float value_max, function<void(VerticalSlider * vs)> f) :
	InterfaceObject(prog_draw, id, i, j, width, height, screen_width, screen_height), _value_min(value_min), _value_max(value_max), _f(f)
{
	_type= OBJECT_TYPE_VERTICAL_SLIDER;
	_color_bckgnd= glm::vec3(0.3f, 0.3f, 0.3f);
	_color_cursor= glm::vec3(0.8f, 0.8f, 0.8f);
	
	_value= _value_min;
	
	_n_vertices= 9;
	_data= new float[_n_vertices* 6];
	compute_data();
}


VerticalSlider::~VerticalSlider() {
	
}


void VerticalSlider::compute_data() {
	glm::vec2 vmin= screen2gl(glm::vec2(_i, _j+ _height));
	glm::vec2 vmax= screen2gl(glm::vec2(_i+ _width, _j));
	float y= value2gl();

	_data[0] = vmin.x; _data[1] = vmin.y; _data[2] =0.0f; _data[3] = _color_bckgnd.x; _data[4] = _color_bckgnd.y; _data[5] = _color_bckgnd.z;
	_data[6] = vmax.x; _data[7] = vmin.y; _data[8] =0.0f; _data[9] = _color_bckgnd.x; _data[10] = _color_bckgnd.y; _data[11] = _color_bckgnd.z;
	_data[12]= vmin.x; _data[13]= vmax.y; _data[14] =0.0f; _data[15]= _color_bckgnd.x; _data[16]= _color_bckgnd.y; _data[17]= _color_bckgnd.z;
	
	_data[18]= vmax.x; _data[19]= vmin.y; _data[20] =0.0f; _data[21]= _color_bckgnd.x; _data[22]= _color_bckgnd.y; _data[23]= _color_bckgnd.z;
	_data[24]= vmax.x; _data[25]= vmax.y; _data[26] =0.0f; _data[27]= _color_bckgnd.x; _data[28]= _color_bckgnd.y; _data[29]= _color_bckgnd.z;	
	_data[30]= vmin.x; _data[31]= vmax.y; _data[32] =0.0f; _data[33]= _color_bckgnd.x; _data[34]= _color_bckgnd.y; _data[35]= _color_bckgnd.z;
	
	// z > 0.0f pour etre affiché par dessus
	_data[36]= vmin.x; _data[37]= y- DELTA_SLIDER; _data[38] =0.5f; _data[39]= _color_cursor.x; _data[40]= _color_cursor.y; _data[41]= _color_cursor.z;
	_data[42]= vmax.x; _data[43]= y;               _data[44] =0.5f; _data[45]= _color_cursor.x; _data[46]= _color_cursor.y; _data[47]= _color_cursor.z;
	_data[48]= vmin.x; _data[49]= y+ DELTA_SLIDER; _data[50] =0.5f; _data[51]= _color_cursor.x; _data[52]= _color_cursor.y; _data[53]= _color_cursor.z;

	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, 54* sizeof(float), _data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


float VerticalSlider::value2gl() {
	unsigned int j= (unsigned int)((float)(_j+ _height)- ((_value- _value_min)/ (_value_max- _value_min))* (float)(_height));
	return screen2gl(glm::uvec2(0, j)).y;
}


void VerticalSlider::update_value(unsigned int j) {
	if (j<_j) {
		j= _j;
	}
	if (j> _height+ _j) {
		j= _height+ _j;
	}
	_value= _value_min+ ((float)(_height+ _j- j)/ (float)(_height))* (_value_max- _value_min);
	compute_data();
}


// ------------------------------------------------------------------------------------------------------------------------
Button::Button() {

}


Button::Button(GLuint prog_draw, std::string id, unsigned int i, unsigned int j, unsigned int width, unsigned int height, unsigned int screen_width, unsigned int screen_height, function<void(Button * b)> f) :
	InterfaceObject(prog_draw, id, i, j, width, height, screen_width, screen_height), _f(f)
{
	_type= OBJECT_TYPE_BUTTON;
	_color_clicked= BUTTON_COLOR_CLICKED;
	_color_unclicked= BUTTON_COLOR_UNCLICKED;
	
	_n_vertices= 6;
	_data= new float[_n_vertices* 6];
	compute_data();
}


Button::~Button() {

}


void Button::compute_data() {
	glm::vec2 vmin= screen2gl(glm::vec2(_i, _j+ _height));
	glm::vec2 vmax= screen2gl(glm::vec2(_i+ _width, _j));
	glm::vec3 color;
	if (_clicked) {
		color= _color_clicked;
	}
	else {
		color= _color_unclicked;
	}

	_data[0] = vmin.x; _data[1] = vmin.y; _data[2] =0.0f; _data[3] = color.x; _data[4] = color.y; _data[5] = color.z;
	_data[6] = vmax.x; _data[7] = vmin.y; _data[8] =0.0f; _data[9] = color.x; _data[10]= color.y; _data[11]= color.z;
	_data[12]= vmin.x; _data[13]= vmax.y; _data[14]=0.0f; _data[15]= color.x; _data[16]= color.y; _data[17]= color.z;
	
	_data[18]= vmax.x; _data[19]= vmin.y; _data[20] =0.0f; _data[21]= color.x; _data[22]= color.y; _data[23]= color.z;
	_data[24]= vmax.x; _data[25]= vmax.y; _data[26] =0.0f; _data[27]= color.x; _data[28]= color.y; _data[29]= color.z;	
	_data[30]= vmin.x; _data[31]= vmax.y; _data[32] =0.0f; _data[33]= color.x; _data[34]= color.y; _data[35]= color.z;

	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, 36* sizeof(float), _data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// ------------------------------------------------------------------------------------------------------------------------
Switch::Switch() {

}


Switch::Switch(GLuint prog_draw, std::string id, unsigned int i, unsigned int j, unsigned int width, unsigned int height, unsigned int screen_width, unsigned int screen_height, function<void(Switch * s)> f) :
	InterfaceObject(prog_draw, id, i, j, width, height, screen_width, screen_height), _active(false), _f(f)
{
	_type= OBJECT_TYPE_SWITCH;
	_color_active= SWITCH_COLOR_ACTIVE;
	_color_inactive= SWITCH_COLOR_INACTIVE;
	
	_n_vertices= 6;
	_data= new float[_n_vertices* 6];
	compute_data();
}


Switch::~Switch() {

}


void Switch::compute_data() {
	glm::vec2 vmin= screen2gl(glm::vec2(_i, _j+ _height));
	glm::vec2 vmax= screen2gl(glm::vec2(_i+ _width, _j));
	glm::vec3 color;
	if (_active) {
		color= _color_active;
	}
	else {
		color= _color_inactive;
	}

	_data[0] = vmin.x; _data[1] = vmin.y; _data[2] =0.0f; _data[3] = color.x; _data[4] = color.y; _data[5] = color.z;
	_data[6] = vmax.x; _data[7] = vmin.y; _data[8] =0.0f; _data[9] = color.x; _data[10]= color.y; _data[11]= color.z;
	_data[12]= vmin.x; _data[13]= vmax.y; _data[14]=0.0f; _data[15]= color.x; _data[16]= color.y; _data[17]= color.z;
	
	_data[18]= vmax.x; _data[19]= vmin.y; _data[20] =0.0f; _data[21]= color.x; _data[22]= color.y; _data[23]= color.z;
	_data[24]= vmax.x; _data[25]= vmax.y; _data[26] =0.0f; _data[27]= color.x; _data[28]= color.y; _data[29]= color.z;	
	_data[30]= vmin.x; _data[31]= vmax.y; _data[32] =0.0f; _data[33]= color.x; _data[34]= color.y; _data[35]= color.z;

	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, 36* sizeof(float), _data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// ------------------------------------------------------------------------------------------------------------------------
IHM::IHM() {

}


IHM::IHM(GLuint prog_draw, unsigned int screen_width, unsigned int screen_height) : _prog_draw(prog_draw), _screen_width(screen_width), _screen_height(screen_height) {


}


IHM::~IHM() {
	for (auto slider : _vsliders) {
		delete slider;
	}
	_vsliders.clear();
	for (auto button : _buttons) {
		delete button;
	}
	_buttons.clear();
	for (auto switch_ : _switchs) {
		delete switch_;
	}
	_switchs.clear();
}


void IHM::draw() {
	for (auto slider : _vsliders) {
		slider->draw();
	}
	for (auto button : _buttons) {
		button->draw();
	}
	for (auto switch_ : _switchs) {
		switch_->draw();
	}
}


void IHM::add_vslider(string id, unsigned int i, unsigned int j, float value_min, float value_max, function<void(VerticalSlider * vs)> f) {
	_vsliders.push_back(new VerticalSlider(_prog_draw, id, i, j, VSLIDER_DEFAULT_WIDTH, VSLIDER_DEFAULT_HEIGHT, _screen_width, _screen_height, value_min, value_max, f));
}


void IHM::add_button(std::string id, unsigned int i, unsigned int j, function<void(Button * b)> f) {
	_buttons.push_back(new Button(_prog_draw, id, i, j, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT, _screen_width, _screen_height, f));
}


void IHM::add_switch(std::string id, unsigned int i, unsigned int j, function<void(Switch * s)> f) {
	_switchs.push_back(new Switch(_prog_draw, id, i, j, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT, _screen_width, _screen_height, f));
}


bool IHM::mouse_button_down(InputState * input_state) {
	for (auto slider : _vsliders) {
		if (slider->clicked(input_state->_x, input_state->_y)) {
			slider->_clicked= true;
			slider->update_value(input_state->_y);
			return true;
		}
	}

	for (auto button : _buttons) {
		if (button->clicked(input_state->_x, input_state->_y)) {
			button->_clicked= true;
			button->compute_data();
			return true;
		}
	}

	for (auto switch_ : _switchs) {
		if (switch_->clicked(input_state->_x, input_state->_y)) {
			switch_->_clicked= true;
			switch_->_active= !switch_->_active;
			switch_->compute_data();
			return true;
		}
	}

	return false;
}


bool IHM::mouse_button_up(InputState * input_state) {
	for (auto slider : _vsliders) {
		if (slider->_clicked) {
			slider->_clicked= false;
			slider->_f(slider);
			return true;
		}
	}
	
	for (auto button : _buttons) {
		if (button->_clicked) {
			button->_clicked= false;
			button->compute_data();
			button->_f(button);
			return true;
		}
	}

	for (auto switch_ : _switchs) {
		if (switch_->_clicked) {
			switch_->_clicked= false;
			switch_->_f(switch_);
			return true;
		}
	}

	return false;
}


bool IHM::mouse_motion(InputState * input_state) {
	for (auto slider : _vsliders) {
		if (slider->_clicked) {
			slider->update_value(input_state->_y);
			return true;
		}
	}
	
	for (auto button : _buttons) {
		if (button->_clicked) {
			return true;
		}
	}

	for (auto switch_ : _switchs) {
		if (switch_->_clicked) {
			return true;
		}
	}

	return false;
}


bool IHM::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_i) {
		toggle_visible();
		return true;
	}

	return false;
}


bool IHM::key_up(InputState * input_state, SDL_Keycode key) {

	return false;
}


void IHM::toggle_visible() {
	for (auto slider : _vsliders) {
		slider->_visible= !slider->_visible;
	}

	for (auto button : _buttons) {
		button->_visible= !button->_visible;
	}

	for (auto switch_ : _switchs) {
		switch_->_visible= !switch_->_visible;
	}

}

