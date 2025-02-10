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


glm::vec2 rot(glm::vec2 v, float alpha) {
	return glm::vec2(v.x* cos(alpha)- v.y* sin(alpha), v.x* sin(alpha)+ v.y* cos(alpha));
}


float norm(glm::vec2 v) {
	return sqrt(v.x* v.x+ v.y* v.y);
}


float scal(glm::vec2 u, glm::vec2 v) {
	return u.x* v.x+ u.y* v.y;
}


// Car ------------------------------------------------------------
Car::Car() {

}


Car::Car(glm::vec2 position, float alpha) {
	reinit(position, alpha);
}


Car::~Car() {

}


void Car::reinit(glm::vec2 position, float alpha) {
	_size= glm::vec2(1.0, 0.5);
	_mass= 1.0;
	_com2force_fwd_ini= glm::vec2(0.7, 0.0);
	_com2force_bwd_ini= glm::vec2(-0.7, 0.0);
	_com2bbox_center_ini= glm::vec2(0.0, 0.0);
	_right_ini= glm::vec2(0.0, -1.0);
	_forward_ini= glm::vec2(1.0, 0.0);

	_velocity= glm::vec2(0.0);
	_acceleration= glm::vec2(0.0);
	_force_fwd= glm::vec2(0.0);
	_force_bwd= glm::vec2(0.0);
	_alpha= 0.0;
	_angular_velocity= 0.0;
	_angular_acceleration= 0.0;
	_torque= 0.0;
	
	_wheel= 0.0;
	_thrust= 0.0;

	_com= position;
	_com2force_fwd= rot(_com2force_fwd_ini, alpha);
	_com2force_bwd= rot(_com2force_bwd_ini, alpha);
	_com2bbox_center= rot(_com2bbox_center_ini, alpha);
	_right= rot(_right_ini, alpha);
	_forward= rot(_forward_ini, alpha);

	_bbox= BBox_2D(_com+ _com2bbox_center, _size);
}


void Car::update_bbox() {
	_bbox._alpha= _alpha;
	_bbox._center= _com+ _com2bbox_center;
	_bbox.update();
}


void Car::preanim_keys(bool key_left, bool key_right, bool key_down, bool key_up) {
	if (key_left) {
		_wheel+= WHEEL_INCREMENT;
		if (_wheel> MAX_WHEEL) {
			_wheel= MAX_WHEEL;
		}
	}
	if (key_right) {
		_wheel-= WHEEL_INCREMENT;
		if (_wheel< -1.0* MAX_WHEEL) {
			_wheel= -1.0* MAX_WHEEL;
		}
	}
	if (!key_left && !key_right) {
		if (_wheel< -1.0* WHEEL_DECREMENT) {
			_wheel+= WHEEL_DECREMENT;
		}
		else if (_wheel> WHEEL_DECREMENT) {
			_wheel-= WHEEL_DECREMENT;
		}
		else {
			_wheel= 0.0;
		}
	}

	if (key_down) {
		_thrust-= BRAKE_INCREMENT;
		if (_thrust< -1.0* MAX_BRAKE) {
			_thrust= -1.0* MAX_BRAKE;
		}
	}
	if (key_up) {
		_thrust+= THRUST_INCREMENT;
		if (_thrust> MAX_THRUST) {
			_thrust= MAX_THRUST;
		}
	}
	if (!key_down && !key_up) {
		if (_thrust< -1.0* THRUST_DECREMENT) {
			_thrust+= THRUST_DECREMENT;
		}
		else if (_thrust> THRUST_DECREMENT) {
			_thrust-= THRUST_DECREMENT;
		}
		else {
			_thrust= 0.0;
		}
	}
}


void Car::anim() {
	const float dt= 0.05;

	_force_fwd= glm::vec2(0.0);
	_force_fwd+= _thrust* rot(_forward, _wheel);
	_force_fwd-= 0.5f* scal(_forward, _velocity)* _forward;
	//_force_fwd-= 0.5f* _velocity;

	_force_bwd= glm::vec2(0.0);
	_force_bwd-= 10.0f* scal(_right, _velocity)* _right;
	//_force_bwd-= 0.5f* _velocity;

	_acceleration= (_force_fwd+ _force_bwd)/ _mass;
	_velocity+= _acceleration* dt;
	_com+= _velocity* dt;

	_torque= 0.0;
	_torque+= _com2force_fwd.x* _force_fwd.y- _com2force_fwd.y* _force_fwd.x;
	_torque+= _com2force_bwd.x* _force_bwd.y- _com2force_bwd.y* _force_bwd.x;
	_torque-= 2.0* _angular_velocity;
	
	_angular_acceleration= 2.0* _torque;
	_angular_velocity+= _angular_acceleration* dt;
	_alpha+= _angular_velocity* dt;
	while (_alpha> M_PI* 2.0) {
		_alpha-= M_PI* 2.0;
	}
	while (_alpha< 0.0) {
		_alpha+= M_PI* 2.0;
	}

	_com2force_fwd= rot(_com2force_fwd_ini, _alpha);
	_com2force_bwd= rot(_com2force_bwd_ini, _alpha);
	_com2bbox_center= rot(_com2bbox_center_ini, _alpha);
	_right= rot(_right_ini, _alpha);
	_forward= rot(_forward_ini, _alpha);

	update_bbox();
}


std::ostream & operator << (std::ostream & os, const Car & car) {
	os << "bbox=[" << car._bbox << "] ; ";
	return os;
}


// Racing ---------------------------------------------------------
Racing::Racing() {

}


Racing::Racing(GLuint prog_bbox, GLuint prog_font, ScreenGL * screengl, bool is_joystick) :
	_draw_bbox(true), _draw_force(true), _show_info(true),
	_key_left(false), _key_right(false), _key_up(false), _key_down(false), _key_a(false), _key_z(false),
	_is_joystick(is_joystick), _joystick(glm::vec2(0.0)), _joystick_a(false), _joystick_b(false) 
	{
	_pt_min= glm::vec2(-screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f);
	_pt_max= glm::vec2(screengl->_gl_width* 0.5f, screengl->_gl_height* 0.5f);
	_camera2clip= glm::ortho(-screengl->_gl_width* 0.5f, screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f, screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
	_font= new Font(prog_font, "../../fonts/Silom.ttf", 48, screengl);

	unsigned int n_buffers= 2;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	_contexts["bbox"]= new DrawContext(prog_bbox, _buffers[0],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix"});

	_contexts["force"]= new DrawContext(prog_bbox, _buffers[1],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix"});

	_cars.push_back(new Car(glm::vec2(0.0, 0.0), 0.0));
	/*for (unsigned int i=0; i<100; ++i) {
		_cars.push_back(new Car(glm::vec2(rand_float(_pt_min.x, _pt_max.x), rand_float(_pt_min.y, _pt_max.y))));
	}*/

	update_bbox();
	update_force();
}


Racing::~Racing() {
	for (auto car : _cars) {
		delete car;
	}
	_cars.clear();

	delete _buffers;
}


void Racing::draw_bbox() {
	DrawContext * context= _contexts["bbox"];

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


void Racing::draw_force() {
	DrawContext * context= _contexts["force"];

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
	if (_draw_bbox) {
		draw_bbox();
	}
	if (_draw_force) {
		draw_force();
	}
	if (_show_info) {
		show_info();
	}
}


void Racing::show_info() {
	const float font_scale= 0.007f;
	const glm::vec4 text_color(1.0, 1.0, 1.0, 0.8);

	std::vector<Text> texts;

	texts.push_back(Text("COM PT", glm::vec2(-9.0f, 7.0f), font_scale, COM_CROSS_COLOR));
	texts.push_back(Text("FORCE_FWD PT", glm::vec2(-9.0f, 6.0f), font_scale, FORCE_FWD_CROSS_COLOR));
	texts.push_back(Text("FORCE_BWD PT", glm::vec2(-9.0f, 5.0f), font_scale, FORCE_BWD_CROSS_COLOR));
	texts.push_back(Text("BBOX PT", glm::vec2(-9.0f, 4.0f), font_scale, BBOX_CROSS_COLOR));
	texts.push_back(Text("FORCE_FWD VEC", glm::vec2(-9.0f, 3.0f), font_scale, FORCE_FWD_ARROW_COLOR));
	texts.push_back(Text("FORCE_BWD VEC", glm::vec2(-9.0f, 2.0f), font_scale, FORCE_BWD_ARROW_COLOR));
	texts.push_back(Text("ACCELERATION VEC", glm::vec2(-9.0f, 1.0f), font_scale, ACCELERATION_ARROW_COLOR));
	texts.push_back(Text("VELOCITY VEC", glm::vec2(-9.0f, 0.0f), font_scale, VELOCITY_ARROW_COLOR));
	texts.push_back(Text("FORWARD VEC", glm::vec2(-9.0f, -1.0f), font_scale, FORWARD_ARROW_COLOR));
	texts.push_back(Text("RIGHT VEC", glm::vec2(-9.0f, -2.0f), font_scale, RIGHT_ARROW_COLOR));

	texts.push_back(Text("thrust="+ std::to_string(_cars[0]->_thrust), glm::vec2(6.0, 7.0), font_scale, text_color));
	texts.push_back(Text("wheel="+ std::to_string(_cars[0]->_wheel), glm::vec2(6.0, 6.0), font_scale, text_color));
	
	texts.push_back(Text("force_fwd="+ std::to_string(norm(_cars[0]->_force_fwd)), glm::vec2(6.0, 4.0), font_scale, text_color));
	texts.push_back(Text("force_bwd="+ std::to_string(norm(_cars[0]->_force_bwd)), glm::vec2(6.0, 3.0), font_scale, text_color));
	texts.push_back(Text("acc="+ std::to_string(norm(_cars[0]->_acceleration)), glm::vec2(6.0, 2.0), font_scale, text_color));
	texts.push_back(Text("vel="+ std::to_string(norm(_cars[0]->_velocity)), glm::vec2(6.0, 1.0), font_scale, text_color));

	texts.push_back(Text("torque="+ std::to_string(_cars[0]->_torque), glm::vec2(6.0, -1.0), font_scale, text_color));
	texts.push_back(Text("ang acc="+ std::to_string(_cars[0]->_angular_acceleration), glm::vec2(6.0, -2.0), font_scale, text_color));
	texts.push_back(Text("ang vel="+ std::to_string(_cars[0]->_angular_velocity), glm::vec2(6.0, -3.0), font_scale, text_color));
	texts.push_back(Text("alpha="+ std::to_string(_cars[0]->_alpha), glm::vec2(6.0, -4.0), font_scale, text_color));

	_font->set_text(texts);
	_font->draw();
}


void Racing::update_bbox() {
	DrawContext * context= _contexts["bbox"];
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
			car->_bbox._pts[0].x, car->_bbox._pts[0].y,
			car->_bbox._pts[1].x, car->_bbox._pts[1].y,

			car->_bbox._pts[1].x, car->_bbox._pts[1].y,
			car->_bbox._pts[2].x, car->_bbox._pts[2].y,

			car->_bbox._pts[2].x, car->_bbox._pts[2].y,
			car->_bbox._pts[3].x, car->_bbox._pts[3].y,

			car->_bbox._pts[3].x, car->_bbox._pts[3].y,
			car->_bbox._pts[0].x, car->_bbox._pts[0].y
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


void Racing::update_force() {
	const unsigned int n_pts_per_car= 4* 4+ 6* 6; // 4 croix ; 6 fleches

	DrawContext * context= _contexts["force"];
	context->_n_pts= 0;
	context->_n_attrs_per_pts= 6;

	for (auto car : _cars) {
		context->_n_pts+= n_pts_per_car;
	}

	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr= data;

	for (unsigned int idx_car=0; idx_car<_cars.size(); ++idx_car) {
		Car * car= _cars[idx_car];

		ptr= draw_cross(ptr, car->_com, CROSS_SIZE, COM_CROSS_COLOR);
		ptr= draw_cross(ptr, car->_com+ car->_com2force_fwd, CROSS_SIZE, FORCE_FWD_CROSS_COLOR);
		ptr= draw_cross(ptr, car->_com+ car->_com2force_bwd, CROSS_SIZE, FORCE_BWD_CROSS_COLOR);
		ptr= draw_cross(ptr, car->_com+ car->_com2bbox_center, CROSS_SIZE, BBOX_CROSS_COLOR);
		ptr= draw_arrow(ptr, car->_com+ car->_com2force_fwd, car->_com+ car->_com2force_fwd+ car->_force_fwd, ARROW_TIP_SIZE, ARROW_ANGLE, FORCE_FWD_ARROW_COLOR);
		ptr= draw_arrow(ptr, car->_com+ car->_com2force_bwd, car->_com+ car->_com2force_bwd+ car->_force_bwd, ARROW_TIP_SIZE, ARROW_ANGLE, FORCE_BWD_ARROW_COLOR);
		ptr= draw_arrow(ptr, car->_com, car->_com+ car->_acceleration, ARROW_TIP_SIZE, ARROW_ANGLE, ACCELERATION_ARROW_COLOR);
		ptr= draw_arrow(ptr, car->_com, car->_com+ car->_velocity, ARROW_TIP_SIZE, ARROW_ANGLE, VELOCITY_ARROW_COLOR);
		ptr= draw_arrow(ptr, car->_com+ car->_com2bbox_center, car->_com+ car->_com2bbox_center+ car->_forward, ARROW_TIP_SIZE, ARROW_ANGLE, FORWARD_ARROW_COLOR);
		ptr= draw_arrow(ptr, car->_com+ car->_com2bbox_center, car->_com+ car->_com2bbox_center+ car->_right, ARROW_TIP_SIZE, ARROW_ANGLE, RIGHT_ARROW_COLOR);
	}

	/*for (int i=0; i<context->_n_pts* context->_n_attrs_per_pts; ++i) {
		std::cout << data[i] << " ; ";
	}
	std::cout << "\n";*/

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Racing::anim() {
	_cars[0]->preanim_keys(_key_left, _key_right, _key_down, _key_up);

	// joystick
	/*if (_is_joystick) {
		if (_joystick_a) {
		}
		else if (_joystick_b) {
		}
	}
	// touches
	else {*/
	//}

	// pour tests
	if (_cars[0]->_com.x> _pt_max.x) {
		_cars[0]->_com.x= _pt_min.x;
	}
	if (_cars[0]->_com.x< _pt_min.x) {
		_cars[0]->_com.x= _pt_max.x;
	}
	if (_cars[0]->_com.y> _pt_max.y) {
		_cars[0]->_com.y= _pt_min.y;
	}
	if (_cars[0]->_com.y< _pt_min.y) {
		_cars[0]->_com.y= _pt_max.y;
	}

	for (auto car : _cars) {
		car->anim();
		//std::cout << *car << "\n";
	}

	update_bbox();
	update_force();
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

	if (key== SDLK_SPACE) {
		_cars[0]->reinit(glm::vec2(0.0, 0.0), 0.0);
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

