#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "utile.h"
#include "threebody.h"


using json = nlohmann::json;


// ----------------------------------------------------------------------------
BodyType::BodyType() : 
	_color(glm::vec3(0.0f)), _limit(AABB(glm::vec3(0.0f), glm::vec3(0.0f))), _radius(0.0f), _mass(0.0f)
{}


BodyType::BodyType(glm::vec3 color, AABB limit, float friction, float max_force_squared_norm, float radius) :
	_color(color), _limit(limit), _friction(friction), _max_force_squared_norm(max_force_squared_norm), _radius(radius)
{
	// on relie radius et mass
	_mass= _radius* COEFF_RADIUS2MASS;
}


BodyType::~BodyType() {}


std::ostream & operator << (std::ostream & os, const BodyType & bt) {
	os << "addr=" << &bt << " ; ";
	os << "color=" << glm::to_string(bt._color) << " ; ";
	os << "limit=" << bt._limit << " ; ";
	os << "friction=" << bt._friction << " ; ";
	os << "max_force_squared_norm=" << bt._max_force_squared_norm << " ; ";
	os << "mass=" << bt._mass << " ; ";
	os << "radius=" << bt._radius;
	os << "\n";
	return os;
}


// ----------------------------------------------------------------------------
BodyInteraction::BodyInteraction() {}


BodyInteraction::BodyInteraction(BodyType * body_type_1, BodyType * body_type_2, float threshold, float attraction, float bias) :
	_body_type_1(body_type_1), _body_type_2(body_type_2), _threshold(threshold), _attraction(attraction), _bias(bias)
{}


BodyInteraction::~BodyInteraction() {}


std::ostream & operator << (std::ostream & os, const BodyInteraction & bi) {
	os << "body_type_1=" << bi._body_type_1 << " ; ";
	os << "body_type_2=" << bi._body_type_2 << " ; ";
	os << "threshold=" << bi._threshold << " ; ";
	os << "attraction=" << bi._attraction << " ; ";
	os << "bias=" << bi._bias << " ; ";
	os << "\n";
	return os;
}


// ----------------------------------------------------------------------------
Body::Body() :
	_body_type(NULL), _position(glm::vec3(0.0f)), _speed(glm::vec3(0.0f)), _acceleration(glm::vec3(0.0f)), _force(glm::vec3(0.0f))
{}


Body::Body(BodyType * body_type) :
	_body_type(body_type), _position(glm::vec3(0.0f)), _speed(glm::vec3(0.0f)), _acceleration(glm::vec3(0.0f)), _force(glm::vec3(0.0f))
{}


Body::Body(BodyType * body_type, glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration) :
	_body_type(body_type), _position(position), _speed(speed), _acceleration(acceleration), _force(glm::vec3(0.0f))
{}


Body::~Body() {
}


void Body::randomize() {
	AABB sublimit(_body_type->_limit);
	//sublimit.scale(0.5f);

	_position.x= rand_float(sublimit._vmin.x, sublimit._vmax.x);
	_position.y= rand_float(sublimit._vmin.y, sublimit._vmax.y);
	_position.z= rand_float(sublimit._vmin.z, sublimit._vmax.z);
	_speed.x= 0.0f;
	_speed.y= 0.0f;
	_speed.z= 0.0f;
	_acceleration.x= 0.0f;
	_acceleration.y= 0.0f;
	_acceleration.z= 0.0f;
}


std::ostream & operator << (std::ostream & os, const Body & b) {
	os << "body_type=" << b._body_type << " ; ";
	os << "position=" << glm::to_string(b._position) << " ; ";
	os << "speed=" << glm::to_string(b._speed) << " ; ";
	os << "acceleration=" << glm::to_string(b._acceleration) << " ; ";
	os << "\n";
	return os;
}


// --------------------------------------------------
ThreeBody::ThreeBody() {

}


ThreeBody::ThreeBody(GLuint prog_draw) : _prog_draw(prog_draw), _n_bodies(0), _paused(false) {
	glGenBuffers(1, &_buffer);
		
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_tex_loc= glGetAttribLocation(_prog_draw, "tex_coord_in");
	_world2clip_loc= glGetUniformLocation(_prog_draw, "world2clip_matrix");

	// sera appelé quand la connection sera établie
	/*_io.set_open_listener([&]() {
		_io.socket()->on("data", 
		[&](sio::event& ev) {
			_lock.lock();
			std::cout << "msg received\n";
			std::string msg= ev.get_message()->get_string();
			// std::cout << msg << "\n";
			json js= json::parse(msg);
			//std::cout << js["bodies_types"][0]["friction"] << "\n";
			read_json(js);
			set_all_z2zero();
			_lock.unlock();
		});
	});

	_io.connect("http://127.0.0.1:3001");*/

}


ThreeBody::~ThreeBody() {
	clear_all();
}


BodyType * ThreeBody::add_type(glm::vec3 color, AABB limit, float friction, float max_force_squared_norm, float radius) {
	BodyType * body_type= new BodyType(color, limit, friction, max_force_squared_norm, radius);
	_bodies[body_type]= std::vector<Body *>();
	return body_type;
}


BodyInteraction * ThreeBody::add_interaction(BodyType * body_type_1, BodyType * body_type_2, float threshold, float attraction, float bias) {
	_body_interactions.push_back(new BodyInteraction(body_type_1, body_type_2, threshold, attraction, bias));
	return _body_interactions[_body_interactions.size()- 1];
}


Body * ThreeBody::add_body(BodyType * body_type, glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration) {
	Body * body= new Body(body_type, position, speed, acceleration);
	_bodies[body_type].push_back(body);
	return body;
}


Body * ThreeBody::add_body(BodyType * body_type) {
	if (DEBUG) {std::cout << "add_body_start\n";};
	Body * body= new Body(body_type);
	if (DEBUG) {std::cout << "add_body_middle\n";};
	_bodies[body_type].push_back(body);
	if (DEBUG) {std::cout << "add_body_end\n";};
	return body;
}


void ThreeBody::clear_bodies() {
	for (auto & b : _bodies) {
		for (auto & body : b.second) {
			delete body;
		}
		b.second.clear();
	}
}


void ThreeBody::clear_all() {
	clear_bodies();

	for (auto & b : _bodies) {
		delete b.first;
	}
	_bodies.clear();

	for (auto & inter : _body_interactions) {
		delete inter;
	}
	_body_interactions.clear();
}


void ThreeBody::anim() {
	if (_paused) {
		return;
	}

	for (auto & b : _bodies) {
		for (auto & body : b.second) {
			body->_force.x= 0.0f; body->_force.y= 0.0f; body->_force.z= 0.0f;
		}
	}

	for (auto & body_interaction : _body_interactions) {
		std::vector<Body *> bodies1= _bodies[body_interaction->_body_type_1];
		std::vector<Body *> bodies2= _bodies[body_interaction->_body_type_2];
		int n1= bodies1.size();
		int n2= bodies2.size();

		for (unsigned int idx_body_1=0; idx_body_1<n1; ++idx_body_1) {
			for (unsigned int idx_body_2=0; idx_body_2<n2; ++idx_body_2) {
				if ((body_interaction->_body_type_1== body_interaction->_body_type_2) && (idx_body_1== idx_body_2)) {
					continue;
				}
				Body * body1= bodies1[idx_body_1];
				Body * body2= bodies2[idx_body_2];
				
				glm::vec3 body1tobody2= body2->_position- body1->_position;	
				float squared_dist= body1tobody2.x* body1tobody2.x+ body1tobody2.y* body1tobody2.y+ body1tobody2.z* body1tobody2.z;
				float overlap= (body1->_body_type->_radius+ body2->_body_type->_radius)* (body1->_body_type->_radius+ body2->_body_type->_radius)- squared_dist;

				// trop loin, pas d'interaction
				if (squared_dist> body_interaction->_threshold* body_interaction->_threshold) {
				}
				// trop près, collision
				else if (overlap> 0.0f ) {
					float force_coeff= COLLISION_FACTOR* overlap;
					body1->_force-= force_coeff* body1tobody2;
					body2->_force+= force_coeff* body1tobody2;
				}
				// ni trop loin, ni trop près
				else {
					// normalement c'est dist**3 mais avec dist**2 c'est plus joli
					float force_coeff= (body_interaction->_attraction* body1->_body_type->_mass* body2->_body_type->_mass/ (body_interaction->_bias+ squared_dist));
					body1->_force+= force_coeff* body1tobody2;
					body2->_force-= force_coeff* body1tobody2;
				}
			}
		}
	}

	for (auto & b : _bodies) {
		for (auto & body : b.second) {
			body->_force+= -1.0f* b.first->_friction* body->_speed;

			float squared_force_norm= body->_force.x* body->_force.x+ body->_force.y* body->_force.y+ body->_force.z* body->_force.z;
			if (squared_force_norm> b.first->_max_force_squared_norm) {
				body->_force*= (b.first->_max_force_squared_norm/ squared_force_norm);
			}

			body->_acceleration= (1.0f/ body->_body_type->_mass)* body->_force;
			body->_speed+= body->_acceleration;
			body->_position+= body->_speed;

			for (int i=0; i<3; ++i) {
				if (body->_position[i]< b.first->_limit._vmin[i]) {
					body->_position[i]= b.first->_limit._vmin[i]+ 0.01f;
					body->_acceleration[i]= 0.0f;
					if (body->_speed[i]< 0.0f) {
						body->_speed[i]*= -1.0f;
					}
				}
				else if (body->_position[i]> b.first->_limit._vmax[i]) {
					body->_position[i]= b.first->_limit._vmax[i]- 0.01f;
					body->_acceleration[i]= 0.0f;
					if (body->_speed[i]> 0.0f) {
						body->_speed[i]*= -1.0f;
					}
				}
			}
		}
	}
}


void ThreeBody::draw(const glm::mat4 & world2clip) {
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_color_loc);
	glEnableVertexAttribArray(_tex_loc);

	glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(world2clip));
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 8* sizeof(float), (void*)0);
	glVertexAttribPointer(_color_loc, 3, GL_FLOAT, GL_FALSE, 8* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(_tex_loc, 2, GL_FLOAT, GL_FALSE, 8* sizeof(float), (void*)(6* sizeof(float)));
	
	glDrawArrays(GL_TRIANGLES, 0, _n_bodies* 6);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_color_loc);
	glDisableVertexAttribArray(_tex_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void ThreeBody::update() {
	_n_bodies= 0;
	for (auto & b : _bodies) {
		_n_bodies+= b.second.size();
	}
	if (_n_bodies== 0) {
		return;
	}

	float data[48* _n_bodies];

	int compt= 0;
	for (auto & b : _bodies) {
		for (auto & body : b.second) {
			data[compt++]= body->_position.x- body->_body_type->_radius;
			data[compt++]= body->_position.y- body->_body_type->_radius;
			data[compt++]= body->_position.z;
			data[compt++]= b.first->_color.x;
			data[compt++]= b.first->_color.y;
			data[compt++]= b.first->_color.z;
			data[compt++]= 0.0f;
			data[compt++]= 0.0f;

			data[compt++]= body->_position.x+ body->_body_type->_radius;
			data[compt++]= body->_position.y- body->_body_type->_radius;
			data[compt++]= body->_position.z;
			data[compt++]= b.first->_color.x;
			data[compt++]= b.first->_color.y;
			data[compt++]= b.first->_color.z;
			data[compt++]= 1.0f;
			data[compt++]= 0.0f;

			data[compt++]= body->_position.x+ body->_body_type->_radius;
			data[compt++]= body->_position.y+ body->_body_type->_radius;
			data[compt++]= body->_position.z;
			data[compt++]= b.first->_color.x;
			data[compt++]= b.first->_color.y;
			data[compt++]= b.first->_color.z;
			data[compt++]= 1.0f;
			data[compt++]= 1.0f;

			data[compt++]= body->_position.x- body->_body_type->_radius;
			data[compt++]= body->_position.y- body->_body_type->_radius;
			data[compt++]= body->_position.z;
			data[compt++]= b.first->_color.x;
			data[compt++]= b.first->_color.y;
			data[compt++]= b.first->_color.z;
			data[compt++]= 0.0f;
			data[compt++]= 0.0f;

			data[compt++]= body->_position.x+ body->_body_type->_radius;
			data[compt++]= body->_position.y+ body->_body_type->_radius;
			data[compt++]= body->_position.z;
			data[compt++]= b.first->_color.x;
			data[compt++]= b.first->_color.y;
			data[compt++]= b.first->_color.z;
			data[compt++]= 1.0f;
			data[compt++]= 1.0f;

			data[compt++]= body->_position.x- body->_body_type->_radius;
			data[compt++]= body->_position.y+ body->_body_type->_radius;
			data[compt++]= body->_position.z;
			data[compt++]= b.first->_color.x;
			data[compt++]= b.first->_color.y;
			data[compt++]= b.first->_color.z;
			data[compt++]= 0.0f;
			data[compt++]= 1.0f;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool ThreeBody::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_a) {
		clear_all();
		
		BodyType * type1= add_type(
			glm::vec3(1.0f, 0.0f, 0.0f), // color
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), // limit
			0.001f, // friction
			10.0f, // max_force_squared_norm
			1.0f // radius
		);

		BodyType * type2= add_type(
			glm::vec3(0.0f, 1.0f, 0.0f), // color
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), // limit
			0.001f, // friction
			10.0f, // max_force_squared_norm
			1.0f // radius_limit
		);

		// type1, type2, threshold, attraction, bias
		//add_interaction(type1, type1, 10.0f, 0.01f, 0.1f);

		// type, n_bodies, limit
		//add_random_bodies(type1, 50);

		add_body(type1, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		add_body(type2, glm::vec3(10.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		
		//set_all_z2zero();
		return true;
	}
	else if (key== SDLK_b) {
		clear_all();
		
		BodyType * type1= add_type(glm::vec3(1.0f, 0.0f, 0.0f), 
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), 0.001f, 10.0f, 1.0f);
		BodyType * type2= add_type(glm::vec3(0.0f, 1.0f, 0.0f), 
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), 0.001f, 10.0f, 1.0f);
		add_interaction(type1, type1, 10.0f, 0.01f, 0.1f);
		add_interaction(type2, type2, 10.0f, 0.01f, 0.1f);
		add_interaction(type1, type2, 5.0f, -0.1f, 0.1f);
		add_interaction(type2, type1, 5.0f, -0.1f, 0.1f);
		add_random_bodies(type1, 20);
		add_random_bodies(type2, 20);
		
		set_all_z2zero();
		return true;
	}
	else if (key== SDLK_c) {
		randomize(
			7, // n_types
			AABB(glm::vec3(-120.0f), glm::vec3(120.0f)), // limit
			glm::vec2(0.05f, 0.1f), // friction
			glm::vec2(1.0f, 1.0f), // max_squared_norm
			glm::vec2(0.4f, 4.0f), // radius
			glm::vec2(10.0f, 40.0f), // threshold
			glm::vec2(-0.8f, 0.5f), // attraction
			glm::vec2(1000.0f, 1000.0f) // bias
		);

		add_random_bodies(glm::ivec2(100, 100));
		//randomize_radius_per_type();
		//prune_with_radius();
		dispatch_bodies(10);
		set_all_z2zero();

		return true;
	}
	else if (key== SDLK_d) {
		std::cout << *this;
	}
	else if (key== SDLK_p) {
		_paused= !_paused;
		return true;
	}
	else if (key== SDLK_w) {
		write_json("../data/test.json");
		return true;
	}
	else if (key== SDLK_x) {
		read_json_file("../data/test.json");
		return true;
	}

	return false;
}


void ThreeBody::set_all_z2zero() {
	for (auto & b : _bodies) {
		for (auto & body : b.second) {
			body->_position.z= 0.0f;
			body->_speed.z= 0.0f;
			body->_acceleration.z= 0.0f;
		}
	}
}


void ThreeBody::read_json(json js) {
	std::map<unsigned long, BodyType *> body_type_map;

	if (DEBUG) {std::cout << "read_json_start\n";};

	clear_all();

	for (auto & type : js["types"]) {
		if (DEBUG) {std::cout << "ok1\n";};
		BodyType * body_type= add_type(
			glm::vec3(type["color"][0], type["color"][1], type["color"][2]), 
			AABB(
				glm::vec3(type["limit"][0], type["limit"][1], type["limit"][2]),
				glm::vec3(type["limit"][3], type["limit"][4], type["limit"][5])
			),
			type["friction"], type["max_force_squared_norm"], type["radius"]
		);
		if (DEBUG) {std::cout << "ok2\n";};
		if (type["bodies"]!= nullptr) {
			for (auto & body : type["bodies"]) {
				add_body(
					body_type,
					glm::vec3(body["position"][0], body["position"][1], body["position"][2]),
					glm::vec3(body["speed"][0], body["speed"][1], body["speed"][2]),
					glm::vec3(body["acceleration"][0], body["acceleration"][1], body["acceleration"][2])
				);
			}
		}
		else if (type["n_bodies"]!= nullptr) {
			add_random_bodies(body_type, type["n_bodies"]);
		}
		if (DEBUG) {std::cout << "ok3\n";};
		body_type_map[type["id"]]= body_type;
	}
	
	for (auto & inter : js["interactions"]) {
		if (DEBUG) {std::cout << "ok4\n";};
		add_interaction(
			body_type_map[inter["body_type_1"]],
			body_type_map[inter["body_type_2"]],
			inter["threshold"],
			inter["attraction"],
			inter["bias"]
		);
		if (DEBUG) {std::cout << "ok5\n";};
	}

	if (DEBUG) {std::cout << "read_json_end\n";};
}


void ThreeBody::read_json(std::string s) {
	read_json(json::parse(s));
}


void ThreeBody::read_json_file(std::string filepath) {
	std::ifstream ifs(filepath);
	json js= json::parse(ifs);
	read_json(js);
}


void ThreeBody::write_json(std::string filepath) {
	json js;
	js["types"]= {};
	js["interactions"]= {};

	for (auto & b : _bodies) {
		json js_bodies;
		for (auto & body : b.second) {
			js_bodies.push_back({
				{"position", {body->_position.x, body->_position.y, body->_position.z}},
				{"speed", {body->_speed.x, body->_speed.y, body->_speed.z}},
				{"acceleration", {body->_acceleration.x, body->_acceleration.y, body->_acceleration.z}},
			});
		}
		js["types"].push_back({
			{"id", (unsigned long)(b.first)},
			{"color", {b.first->_color.x, b.first->_color.y, b.first->_color.z}},
			{"limit", {
				b.first->_limit._vmin.x, b.first->_limit._vmin.y, b.first->_limit._vmin.z, 
				b.first->_limit._vmax.x, b.first->_limit._vmax.y, b.first->_limit._vmax.z
			}},
			{"friction", b.first->_friction},
			{"max_force_squared_norm", b.first->_max_force_squared_norm},
			{"radius", b.first->_radius},
			{"bodies", js_bodies}
		});
	}

	for (auto & inter : _body_interactions) {
		js["interactions"].push_back({
			{"body_type_1", (unsigned long)(inter->_body_type_1)},
			{"body_type_2", (unsigned long)(inter->_body_type_2)},
			{"threshold", inter->_threshold},
			{"attraction", inter->_attraction},
			{"bias", inter->_bias}
		});
	}

	std::ofstream ofs(filepath);
	ofs << std::setw(4) << js << std::endl;
}


void ThreeBody::add_random_bodies(BodyType * body_type, unsigned int n_bodies) {
	for (unsigned int idx_body=0; idx_body<n_bodies; ++idx_body) {
		if (DEBUG) {std::cout << "add_random_bodies_start\n";};
		Body * body= add_body(body_type);
		if (DEBUG) {std::cout << "add_random_bodies_middle\n";};
		body->randomize();
		if (DEBUG) {std::cout << "add_random_bodies_end\n";};
	}
}


void ThreeBody::add_random_bodies(glm::vec2 n_bodies) {
	float min_radius= 1e6;
	float max_radius= -1e6;
	for (auto & b : _bodies) {
		if (b.first->_radius> max_radius) {
			max_radius= b.first->_radius;
		}
		if (b.first->_radius< min_radius) {
			min_radius= b.first->_radius;
		}
	}

	for (auto & b : _bodies) {
		// nombre de bodies inversement proportionnel au radius
		int n= n_bodies[0]+ (n_bodies[1]- n_bodies[0])* (b.first->_radius- min_radius)/ (max_radius- min_radius);
		add_random_bodies(b.first, n);
	}
}


void ThreeBody::randomize(int n_types, AABB limit, glm::vec2 friction, glm::vec2 max_force_squared_norm,
	glm::vec2 radius, glm::vec2 threshold, glm::vec2 attraction, glm::vec2 bias) {
	
	clear_all();

	for (int i=0; i<n_types; ++i) {
		add_type(glm::vec3(rand_float(0.2f, 1.0f), rand_float(0.2f, 1.0f), rand_float(0.2f, 1.0f)), 
			limit, rand_float(friction[0], friction[1]), rand_float(max_force_squared_norm[0], max_force_squared_norm[1]),
			rand_float(radius[0], radius[1])
		);
	}
	for (auto & b : _bodies) {
		for (auto & y : _bodies) {
			add_interaction(b.first, y.first, rand_float(threshold[0], threshold[1]), rand_float(attraction[0], attraction[1]), rand_float(bias[0], bias[1]));
		}
	}
}


/*void ThreeBody::randomize_radius_per_type() {
	for (auto & b : _bodies) {
		float radius= rand_float(b.first->_radius_limit[0], b.first->_radius_limit[1]);
		for (auto & body : b.second) {
			body->_radius= radius;
		}
	}
}*/


/*void ThreeBody::prune_with_radius() {
	float min_radius= 1e6;
	float max_radius= -1e6;
	for (auto & b : _bodies) {
		for (auto & body : b.second) {
			if (body->_body_type->_radius> max_radius) {
				max_radius= body->_body_type->_radius;
			}
			if (body->_body_type->_radius< min_radius) {
				min_radius= body->_body_type->_radius;
			}
		}
	}
	for (auto & b : _bodies) {
		b.second.erase(std::remove_if(b.second.begin(), b.second.end(), [min_radius, max_radius](Body * body) {
			float x= rand_float(0.0f, 1.1f); // si 1.0f tous les objets de radius max seront supprimés
			return x< (body->_body_type->_radius- min_radius)/ (max_radius- min_radius);
		}), b.second.end());
	}
}*/


void ThreeBody::dispatch_bodies(int group_size) {
	for (auto & b : _bodies) {
		int compt= group_size;
		AABB sublimit;
		for (auto & body : b.second) {
			//std::cout << sublimit << "\n";
			compt++;
			if (compt>= group_size) {
				compt= 0;
				sublimit= b.first->_limit;
				sublimit.scale(rand_float(0.1f, 0.2f));
				float t= 0.9f;
				sublimit.translate(glm::vec3(
					rand_float(-1.0f* t* b.first->_limit._radius, t* b.first->_limit._radius),
					rand_float(-1.0f* t* b.first->_limit._radius, t* b.first->_limit._radius),
					rand_float(-1.0f* t* b.first->_limit._radius, t* b.first->_limit._radius)
				));
			}
			body->_position.x= rand_float(sublimit._vmin.x, sublimit._vmax.x);
			body->_position.y= rand_float(sublimit._vmin.y, sublimit._vmax.y);
			body->_position.z= rand_float(sublimit._vmin.z, sublimit._vmax.z);
		}
	}
}


std::ostream & operator << (std::ostream & os, const ThreeBody & tb) {
	os << "bodies -------------\n";
	for (auto & b : tb._bodies) {
		os << *b.first;
		/*for (auto & body : b.second) {
			os << *body;
		}*/
	}
	os << "interactions -------------\n";
	for (auto & inter : tb._body_interactions) {
		os << *inter;
	}
	os << "\n";
	return os;
}

