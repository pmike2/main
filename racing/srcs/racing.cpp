#include <iomanip>
#include <fstream>
#include <sstream>
#include <functional>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "json.hpp"

#include "utile.h"

#include "racing.h"


using json = nlohmann::json;



// Racing ---------------------------------------------------------
Racing::Racing() {

}


Racing::Racing(GLuint prog_simple, GLuint prog_font, ScreenGL * screengl, bool is_joystick) :
	_draw_bbox(true), _draw_force(false), _show_info(false), _cam_mode(FIXED),
	_key_left(false), _key_right(false), _key_up(false), _key_down(false), _key_a(false), _key_z(false),
	_is_joystick(is_joystick), _joystick(glm::vec2(0.0)), _joystick_a(false), _joystick_b(false),
	_ia(false)
	{
	_pt_min= pt_type(-screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f);
	_pt_max= pt_type(screengl->_gl_width* 0.5f, screengl->_gl_height* 0.5f);
	_camera2clip= glm::ortho(-screengl->_gl_width* 0.5f, screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f, screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
	_world2camera= glm::mat4(1.0);
	_font= new Font(prog_font, "../../fonts/Silom.ttf", 48, screengl);

	unsigned int n_buffers= 2;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	_contexts["bbox"]= new DrawContext(prog_simple, _buffers[0],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix"});

	_contexts["force"]= new DrawContext(prog_simple, _buffers[1],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix", "world2camera_matrix"});

	load_models();

	//_cars.push_back(new Car(_models["car1"], pt_type(0.0, 6.0), 0.0));
	/*_cars.push_back(new Car(_models["car1"], pt_type(-4.0, 0.5), 0.0));
	_cars[1]->_thrust= 2.0;
	_cars.push_back(new Car(_models["car1"], pt_type(5.0, 0.0), M_PI* 0.5));
	_cars[2]->_thrust= 0.0;*/

	//load_json("../data/test/init.json");
	//save_json("../data/test/init.json");
	randomize();

	update_bbox();
	update_force();
}


Racing::~Racing() {
	for (auto car : _cars) {
		delete car;
	}
	_cars.clear();

	for (auto model : _models) {
		delete model.second;
	}
	_models.clear();

	delete _buffers;
}


void Racing::load_models() {
	for (auto model : _models) {
		delete model.second;
	}
	_models.clear();

	std::vector<std::string> jsons= list_files("../data/cars", "json");
	for (auto json_path : jsons) {
		_models[basename(json_path)]= new CarModel(json_path);
	}
}


void Racing::load_json(std::string json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	for (auto car : _cars) {
		delete car;
	}
	_cars.clear();

	for (auto & js_car : js) {
		Car * car= new Car(_models[js_car["model"]], pt_type(js_car["com"][0], js_car["com"][1]), js_car["alpha"]);
		car->_velocity= pt_type(js_car["velocity"][0], js_car["velocity"][1]);
		car->_acceleration= pt_type(js_car["acceleration"][0], js_car["acceleration"][1]);
		car->_angular_velocity= js_car["angular_velocity"];
		car->_angular_acceleration= js_car["angular_acceleration"];
		car->_wheel= js_car["wheel"];
		car->_thrust= js_car["thrust"];
		_cars.push_back(car);
	}
}


void Racing::save_json(std::string json_path) {
	std::ofstream ofs(json_path);
	json js;

	for (auto car : _cars) {
		json js_car;
		js_car["model"]= basename(car->_model->_json_path);
		js_car["com"]= json::array();
		js_car["com"].push_back(car->_com.x);
		js_car["com"].push_back(car->_com.y);
		js_car["velocity"]= json::array();
		js_car["velocity"].push_back(car->_velocity.x);
		js_car["velocity"].push_back(car->_velocity.y);
		js_car["acceleration"]= json::array();
		js_car["acceleration"].push_back(car->_acceleration.x);
		js_car["acceleration"].push_back(car->_acceleration.y);
		js_car["alpha"]= car->_alpha;
		js_car["angular_velocity"]= car->_angular_velocity;
		js_car["angular_acceleration"]= car->_angular_acceleration;
		js_car["wheel"]= car->_wheel;
		js_car["thrust"]= car->_thrust;

		js.push_back(js_car);
	}

	ofs << std::setw(4) << js << "\n";
}


void Racing::randomize() {
	_ia= true;

	for (auto car : _cars) {
		delete car;
	}
	_cars.clear();

	// héros
	_cars.push_back(new Car(_models["car1"], pt_type(0.0, 0.0), 0.0));

	// bords
	add_boundary();

	// ennemis
	for (unsigned int i=0; i<5; ++i) {
		_cars.push_back(new Car(_models["car1"], pt_type(rand_number(_pt_min.x, _pt_max.x), rand_number(_pt_min.y, _pt_max.y)), rand_number(0.0, M_PI* 2.0)));
	}

	// obstacles
	for (unsigned int i=0; i<3; ++i) {
		_cars.push_back(new Car(_models["obstacle"], pt_type(rand_number(_pt_min.x, _pt_max.x), rand_number(_pt_min.y, _pt_max.y)), rand_number(0.0, M_PI* 2.0)));
	}
}


void Racing::add_boundary() {
	_cars.push_back(new Car(_models["wall"], pt_type(0.0, _pt_min.y), 0.0));
	_cars.push_back(new Car(_models["wall"], pt_type(0.0, _pt_max.y), 0.0));
	_cars.push_back(new Car(_models["wall"], pt_type(_pt_min.x, 0.0), M_PI* 0.5));
	_cars.push_back(new Car(_models["wall"], pt_type(_pt_max.x, 0.0), M_PI* 0.5));
}


void Racing::draw_bbox() {
	DrawContext * context= _contexts["bbox"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	
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
	glUniformMatrix4fv(context->_locs_uniform["world2camera_matrix"], 1, GL_FALSE, glm::value_ptr(_world2camera));
	
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

	texts.push_back(Text("drift="+ std::to_string(_cars[0]->_drift), glm::vec2(6.0, -6.0), font_scale, text_color));

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
			car->_bbox->_pts[0].x, car->_bbox->_pts[0].y,
			car->_bbox->_pts[1].x, car->_bbox->_pts[1].y,

			car->_bbox->_pts[1].x, car->_bbox->_pts[1].y,
			car->_bbox->_pts[2].x, car->_bbox->_pts[2].y,

			car->_bbox->_pts[2].x, car->_bbox->_pts[2].y,
			car->_bbox->_pts[3].x, car->_bbox->_pts[3].y,

			car->_bbox->_pts[3].x, car->_bbox->_pts[3].y,
			car->_bbox->_pts[0].x, car->_bbox->_pts[0].y
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
	
	if (_ia) {
		for (unsigned int idx_car=1; idx_car<_cars.size(); ++idx_car) {
			_cars[idx_car]->random_ia();
		}
	}

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

	for (auto car : _cars) {
		car->anim(ANIM_DT);
		//std::cout << *car << "\n";

		// pour tests
		/*if (car->_com.x> _pt_max.x) {
			car->_com.x= _pt_min.x;
		}
		if (car->_com.x< _pt_min.x) {
			car->_com.x= _pt_max.x;
		}
		if (car->_com.y> _pt_max.y) {
			car->_com.y= _pt_min.y;
		}
		if (car->_com.y< _pt_min.y) {
			car->_com.y= _pt_max.y;
		}*/
	}

	collision();

	update_bbox();
	update_force();
	
	camera();
}


void Racing::collision() {
	for (unsigned int idx_car_1=0; idx_car_1<_cars.size()- 1; ++idx_car_1) {
		for (unsigned int idx_car_2=idx_car_1+ 1; idx_car_2<_cars.size(); ++idx_car_2) {
			Car * car1= _cars[idx_car_1];
			Car * car2= _cars[idx_car_2];

			if (car1->_model->_fixed && car2->_model->_fixed) {
				continue;
			}

			if (!aabb_intersects_aabb(car1->_bbox->_aabb, car2->_bbox->_aabb)) {
				continue;
			}

			pt_type axis(0.0, 0.0);
			number overlap= 0.0;
			unsigned int idx_pt= 0;
			bool is_pt_in_poly1= false;
			bool is_inter= bbox_intersects_bbox(car1->_bbox, car2->_bbox, &axis, &overlap, &idx_pt, &is_pt_in_poly1);

			// on se place comme dans le cas https://en.wikipedia.org/wiki/Collision_response
			// où la normale est celle de body1 et le point dans body2
			if (is_pt_in_poly1) {
				Car * car_tmp= car1;
				car1= car2;
				car2= car_tmp;
			}

			if (is_inter) {
				// on écarte un peu plus que de 0.5 de chaque coté ou de 1.0 dans le cas fixed
				// est-ce utile ?
				if (car1->_model->_fixed) {
					car2->_com+= overlap* 1.05* axis;
				}
				else if (car2->_model->_fixed) {
					car1->_com-= overlap* 1.05* axis;
				}
				else {
					car1->_com-= overlap* 0.55* axis;
					car2->_com+= overlap* 0.55* axis;
				}

				pt_type r1, r2;
				r1= car2->_bbox->_pts[idx_pt]- car1->_com;
				r2= car2->_bbox->_pts[idx_pt]- car2->_com;
				
				pt_type r1_norm= normalized(r1);
				pt_type r1_norm_perp(-1.0* r1_norm.y, r1_norm.x);
				pt_type contact_pt_velocity1= car1->_velocity+ car1->_angular_velocity* r1_norm_perp;

				pt_type r2_norm= normalized(r2);
				pt_type r2_norm_perp(-1.0* r2_norm.y, r2_norm.x);
				pt_type contact_pt_velocity2= car2->_velocity+ car2->_angular_velocity* r2_norm_perp;

				pt_type vr= contact_pt_velocity2- contact_pt_velocity1;

				// https://en.wikipedia.org/wiki/Coefficient_of_restitution
				// restitution doit etre entre 0 et 1 ; proche de 0 -> pas de rebond ; proche de 1 -> beaucoup de rebond
				// TODO : faire des matériaux avec des valeurs de restitution différentes
				number restitution= 0.2;


				number impulse;
				if (car1->_model->_fixed) {
					pt_type v= (cross2d(r2, axis)/ car2->_model->_inertia)* r2;
					impulse= (-(1.0+ restitution)* dot(vr, axis)) / (1.0/ car2->_model->_mass+ dot(v, axis));
				}
				else if (car2->_model->_fixed) {
					pt_type v= (cross2d(r1, axis)/ car1->_model->_inertia)* r1;
					impulse= (-(1.0+ restitution)* dot(vr, axis)) / (1.0/ car1->_model->_mass+ dot(v, axis));
				}
				else {
					pt_type v= (cross2d(r1, axis)/ car1->_model->_inertia)* r1+ (cross2d(r2, axis)/ car2->_model->_inertia)* r2;
					impulse= (-(1.0+ restitution)* dot(vr, axis)) / (1.0/ car1->_model->_mass+ 1.0/ car2->_model->_mass+ dot(v, axis));
				}

				if (abs(impulse)> 10.0) {
					std::cout << "impulse=" << impulse << "\n";
					//std::cout << "dot(vr, axis)=" << dot(vr, axis) << " ; dot(v, axis)=" << dot(v, axis) << "\n";
					save_json("../data/test/big_impulse.json");
				}

				if (!car1->_model->_fixed) {
					car1->_velocity-= (impulse/ car1->_model->_mass)* axis;
					car1->_angular_velocity-= (impulse/ car1->_model->_inertia)* cross2d(r1, axis);
				}

				if (!car2->_model->_fixed) {
					car2->_velocity+= (impulse/ car2->_model->_mass)* axis;
					car2->_angular_velocity+= (impulse/ car2->_model->_inertia)* cross2d(r2, axis);
				}

				// peut-être pas nécessaire
				car1->_acceleration= pt_type(0.0);
				car1->_angular_acceleration= 0.0;
				car2->_acceleration= pt_type(0.0);
				car2->_angular_acceleration= 0.0;
			}
		}
	}
}


void Racing::camera() {
	_com_camera.x+= CAM_INC* (_cars[0]->_com.x- _com_camera.x);
	_com_camera.y+= CAM_INC* (_cars[0]->_com.y- _com_camera.y);
	number diff_alpha= _cars[0]->_alpha- _alpha_camera;
	if (diff_alpha> M_PI) {
		_alpha_camera+= 2.0* M_PI;
		diff_alpha-= 2.0* M_PI;
	}
	if (diff_alpha< -1.0* M_PI) {
		_alpha_camera-= 2.0* M_PI;
		diff_alpha+= 2.0* M_PI;
	}
	_alpha_camera+= CAM_INC_ALPHA* diff_alpha;

	if (_cam_mode== FIXED) {
		_world2camera= glm::mat4(1.0f);
	}
	else if (_cam_mode== TRANSLATE) {
		_world2camera= glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f* float(_com_camera.x), -1.0f* float(_com_camera.y), 1.0f));
	}
	else if (_cam_mode== TRANSLATE_AND_ROTATE) {
		_world2camera= glm::translate(glm::rotate(glm::mat4(1.0f), -1.0f* float(_alpha_camera), glm::vec3(0.0, 0.0, 1.0f)), glm::vec3(-1.0f* float(_com_camera.x), -1.0f* float(_com_camera.y), 1.0f));
	}
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
		_cars[0]->reinit(pt_type(0.0, 0.0), 0.0);
		return true;
	}
	else if (key== SDLK_b) {
		_draw_bbox= !_draw_bbox;
	}
	else if (key== SDLK_f) {
		_draw_force= !_draw_force;
	}
	else if (key== SDLK_i) {
		_show_info= !_show_info;
	}
	else if (key== SDLK_l) {
		_ia= false;
		load_json("../data/test/init.json");
	}
	else if (key== SDLK_c) {
		if (_cam_mode== FIXED) {
			_cam_mode= TRANSLATE;
		}
		else if (_cam_mode== TRANSLATE) {
			_cam_mode= TRANSLATE_AND_ROTATE;
		}
		else if (_cam_mode== TRANSLATE_AND_ROTATE) {
			_cam_mode= FIXED;
		}
	}
	else if (key== SDLK_r) {
		randomize();
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

