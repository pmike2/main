#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "json.hpp"

#include "utile.h"

#include "racing.h"

using json = nlohmann::json;


// Car ------------------------------------------------------------
Car::Car() {

}


Car::Car(glm::vec2 position) : _aabb(AABB_2D(position, glm::vec2(1.0, 1.7))), _velocity(glm::vec2(0.0)), _acceleration(glm::vec2(0.0)) {

}


Car::~Car() {

}


// Racing ---------------------------------------------------------
Racing::Racing() {

}


Racing::Racing(GLuint prog_aabb, GLuint prog_font, ScreenGL * screengl, bool is_joystick) :
	_draw_aabb(true),
	_key_left(false), _key_right(false), _key_up(false), _key_down(false), _key_a(false), _key_z(false),
	_is_joystick(is_joystick), _joystick(glm::vec2(0.0)), _joystick_a(false), _joystick_b(false) 
	{
	_pt_min= glm::vec2(-screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f);
	_pt_max= glm::vec2(screengl->_gl_width* 0.5f, screengl->_gl_height* 0.5f);
	_camera2clip= glm::ortho(-screengl->_gl_width* 0.5f, screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f, screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
	_font= new Font(prog_font, "../../fonts/Silom.ttf", 48, screengl);

	unsigned int n_buffers= 1;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	_contexts["aabb"]= new DrawContext(prog_aabb, _buffers[0],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix"});

	_cars.push_back(new Car(glm::vec2(0.0, 0.0)));
	/*for (unsigned int i=0; i<100; ++i) {
		_cars.push_back(new Car(glm::vec2(rand_float(_pt_min.x, _pt_max.x), rand_float(_pt_min.y, _pt_max.y))));
	}*/

	update_aabb();
}


Racing::~Racing() {
	for (auto car : _cars) {
		delete car;
	}
	_cars.clear();

	delete _buffers;
}


void Racing::draw_aabb() {
	DrawContext * context= _contexts["aabb"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Racing::draw() {
	if (_draw_aabb) {
		draw_aabb();
	}
}


void Racing::update_aabb() {
	DrawContext * context= _contexts["aabb"];
	context->_n_pts= 0;
	context->_n_attrs_per_pts= 6;

	const unsigned int n_pts_per_car= 8;

	for (auto car : _cars) {
		context->_n_pts+= n_pts_per_car;
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];

	for (unsigned int idx_car=0; idx_car<_cars.size(); ++idx_car) {
		Car * car= _cars[idx_car];

		glm::vec4 color;
		if (idx_car== 0) {
			color= glm::vec4(0.0, 1.0, 0.0, 1.0);
		}
		else {
			color= glm::vec4(1.0, 0.0, 0.0, 1.0);
		}
		number positions[n_pts_per_car* 2]= {
			car->_aabb._pos.x, car->_aabb._pos.y,
			car->_aabb._pos.x+ car->_aabb._size.x, car->_aabb._pos.y,

			car->_aabb._pos.x+ car->_aabb._size.x, car->_aabb._pos.y,
			car->_aabb._pos.x+ car->_aabb._size.x, car->_aabb._pos.y+ car->_aabb._size.y,

			car->_aabb._pos.x+ car->_aabb._size.x, car->_aabb._pos.y+ car->_aabb._size.y,
			car->_aabb._pos.x, car->_aabb._pos.y+ car->_aabb._size.y,

			car->_aabb._pos.x, car->_aabb._pos.y+ car->_aabb._size.y,
			car->_aabb._pos.x, car->_aabb._pos.y
		};

		for (unsigned int i=0; i<n_pts_per_car; ++i) {
			if (positions[2* i]> _pt_max.x) {
				positions[2* i]= _pt_max.x;
			}
			if (positions[2* i]< _pt_min.x) {
				positions[2* i]= _pt_min.x;
			}
			if (positions[2* i+ 1]> _pt_max.y) {
				positions[2* i+ 1]= _pt_max.y;
			}
			if (positions[2* i+ 1]< _pt_min.y) {
				positions[2* i+ 1]= _pt_min.y;
			}
		}

		for (unsigned int idx_pt=0; idx_pt<n_pts_per_car; ++idx_pt) {
			data[idx_car* n_pts_per_car* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 0]= float(positions[2* idx_pt]);
			data[idx_car* n_pts_per_car* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 1]= float(positions[2* idx_pt+ 1]);
			for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
				data[idx_car* n_pts_per_car* context->_n_attrs_per_pts+ idx_pt* context->_n_attrs_per_pts+ 2+ idx_color]= color[idx_color];
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::anim() {
	glm::vec2 force(0.0);

	// joystick
	if (_is_joystick) {
		if (_joystick_a) {
		}
		else if (_joystick_b) {
		}
	}
	// touches
	else {
		if (_key_left) {
			force.x-= 0.1;
		}
		if (_key_right) {
			force.x+= 0.1;
		}
		if (_key_down) {
			force.y-= 0.1;
		}
		if (_key_up) {
			force.y+= 0.1;
		}
	}

	// joueur contraint à l'écran
	if (_cars[0]->_aabb._pos.x> _pt_max.x- _cars[0]->_aabb._size.x) {
		_cars[0]->_aabb._pos.x= _pt_max.x- _cars[0]->_aabb._size.x;
	}
	if (_cars[0]->_aabb._pos.x< _pt_min.x) {
		_cars[0]->_aabb._pos.x= _pt_min.x;
	}
	if (_cars[0]->_aabb._pos.y> _pt_max.y- _cars[0]->_aabb._size.y) {
		_cars[0]->_aabb._pos.y= _pt_max.y- _cars[0]->_aabb._size.y;
	}
	if (_cars[0]->_aabb._pos.y< _pt_min.y) {
		_cars[0]->_aabb._pos.y= _pt_min.y;
	}

	update_aabb();
}


bool Racing::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_LEFT) {
		_key_left= true;
		return true;
	}
	else if (key== SDLK_RIGHT) {
		_key_right= true;
		return true;
	}
	else if (key== SDLK_UP) {
		_key_up= true;
		return true;
	}
	else if (key== SDLK_DOWN) {
		_key_down= true;
		return true;
	}
	return false;
}


bool Racing::key_up(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_LEFT) {
		_key_left= false;
		return true;
	}
	else if (key== SDLK_RIGHT) {
		_key_right= false;
		return true;
	}
	else if (key== SDLK_UP) {
		_key_up= false;
		return true;
	}
	else if (key== SDLK_DOWN) {
		_key_down= false;
		return true;
	}
	return false;
}


bool Racing::joystick_down(unsigned int button_idx) {
	if (!_is_joystick) {
		return false;
	}
	
	if (button_idx== 0) {
		_joystick_a= true;
		return true;
	}
	else if (button_idx== 1) {
		_joystick_b= true;
		return true;
	}

	return false;
}


bool Racing::joystick_up(unsigned int button_idx) {
	if (!_is_joystick) {
		return false;
	}

	if (button_idx== 0) {
		_joystick_a= false;
		return true;
	}
	else if (button_idx== 1) {
		_joystick_b= false;
		return true;
	}
	return false;
}


bool Racing::joystick_axis(unsigned int axis_idx, int value) {
	if (!_is_joystick) {
		return false;
	}

	// le joy droit a les axis_idx 2 et 3 qui ne sont pas gérés par Asteroid pour l'instant
	if (axis_idx> 1) {
		return false;
	}

	// -1 < fvalue < 1
	float fvalue= float(value)/ 32768.0;
	_joystick[axis_idx]= fvalue;
	return true;
}

