#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "utile.h"
#include "threebody.h"


using json = nlohmann::json;


// ----------------------------------------------------------------------------
BodyType::BodyType() : 
	_color(glm::vec3(0.0f)), _limit(AABB(glm::vec3(0.0f), glm::vec3(0.0f))), _mass_limit(glm::vec2(0.0f)), _radius_limit(glm::vec2(0.0f))
{}


BodyType::BodyType(glm::vec3 color, AABB limit, float friction, float max_force_squared_norm, glm::vec2 mass_limit, glm::vec2 radius_limit) :
	_color(color), _limit(limit), _friction(friction), _max_force_squared_norm(max_force_squared_norm), _mass_limit(mass_limit), _radius_limit(radius_limit)
{}


BodyType::~BodyType() {}


// ----------------------------------------------------------------------------
BodyInteraction::BodyInteraction() {}


BodyInteraction::BodyInteraction(BodyType * body_type_1, BodyType * body_type_2, float threshold, float attraction, float bias) :
	_body_type_1(body_type_1), _body_type_2(body_type_2), _threshold(threshold), _attraction(attraction), _bias(bias)
{}


BodyInteraction::~BodyInteraction() {}


// ----------------------------------------------------------------------------
Body::Body() :
	_body_type(NULL), _position(glm::vec3(0.0f)), _speed(glm::vec3(0.0f)), _acceleration(glm::vec3(0.0f)), _force(glm::vec3(0.0f)), _mass(0.0f), _radius(0.0f)
{}


Body::Body(BodyType * body_type) :
	_body_type(body_type), _position(glm::vec3(0.0f)), _speed(glm::vec3(0.0f)), _acceleration(glm::vec3(0.0f)), _force(glm::vec3(0.0f)), _mass(DEFAULT_MASS), _radius(DEFAULT_RADIUS)
{}


Body::Body(BodyType * body_type, glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass, float radius) :
	_body_type(body_type), _position(position), _speed(speed), _acceleration(acceleration), _force(glm::vec3(0.0f)), _mass(mass), _radius(radius)
{}


Body::~Body() {
}


void Body::randomize() {
	_position.x= rand_float(_body_type->_limit._vmin.x, _body_type->_limit._vmax.x);
	_position.y= rand_float(_body_type->_limit._vmin.y, _body_type->_limit._vmax.y);
	_position.z= rand_float(_body_type->_limit._vmin.z, _body_type->_limit._vmax.z);
	_speed.x= 0.0f;
	_speed.y= 0.0f;
	_speed.z= 0.0f;
	_acceleration.x= 0.0f;
	_acceleration.y= 0.0f;
	_acceleration.z= 0.0f;
	_radius= rand_float(_body_type->_radius_limit[0], _body_type->_radius_limit[1]);
	//_mass= rand_float(_body_type->_mass_limit[0], _body_type->_mass_limit[1]);
	_mass= _radius* 1.0f;
}


// --------------------------------------------------
ThreeBody::ThreeBody() {

}


ThreeBody::ThreeBody(GLuint prog_draw) : _prog_draw(prog_draw), _n_bodies(0) {
	_bodies.clear();

	glGenBuffers(2, _buffers);
		
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_world2clip_loc= glGetUniformLocation(_prog_draw, "world2clip_matrix");
}


ThreeBody::~ThreeBody() {
	clear_all();
}


BodyType * ThreeBody::add_type(glm::vec3 color, AABB limit, float friction, float max_force_squared_norm, glm::vec2 mass_limit, glm::vec2 radius_limit) {
	BodyType * body_type= new BodyType(color, limit, friction, max_force_squared_norm, mass_limit, radius_limit);
	_bodies[body_type]= std::vector<Body *>();
	return body_type;
}


BodyInteraction * ThreeBody::add_interaction(BodyType * body_type_1, BodyType * body_type_2, float threshold, float attraction, float bias) {
	_body_interactions.push_back(new BodyInteraction(body_type_1, body_type_2, threshold, attraction, bias));
	return _body_interactions[_body_interactions.size()- 1];
}


Body * ThreeBody::add_body(BodyType * body_type, glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass, float radius) {
	Body * body= new Body(body_type, position, speed, acceleration, mass, radius);
	_bodies[body_type].push_back(body);
	return body;
}


Body * ThreeBody::add_body(BodyType * body_type) {
	Body * body= new Body(body_type);
	_bodies[body_type].push_back(body);
	return body;
}


void ThreeBody::clear_bodies() {
	for (auto & x : _bodies) {
		for (auto & body : x.second) {
			delete body;
		}
		x.second.clear();
	}
}


void ThreeBody::clear_all() {
	clear_bodies();

	for (auto & x : _bodies) {
		delete x.first;
	}
	_bodies.clear();

	for (auto & inter : _body_interactions) {
		delete inter;
	}
	_body_interactions.clear();
}


/*int ThreeBody::get_body_type_idx(BodyType * body_type) {
	for (int idx=0; idx<_body_types.size(); ++idx) {
		if (_body_types[idx]== body_type) {
			return idx;
		}
	}
	return -1;
}*/


void ThreeBody::anim() {
	if (_bodies.size()== 0) {
		return;
	}

	for (auto & x : _bodies) {
		for (auto & body : x.second) {
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
				float overlap= (body1->_radius+ body2->_radius)* (body1->_radius+ body2->_radius)- squared_dist;

				// trop loin, pas d'interaction
				if (squared_dist> body_interaction->_threshold* body_interaction->_threshold) {
				}
				// trop près, collision
				else if (overlap> 0.0f ) {
					float force_coeff= 0.01f* overlap;
					body1->_force-= force_coeff* body1tobody2;
					body2->_force+= force_coeff* body1tobody2;
				}
				// ni trop loin, ni trop près
				else {
					// normalement c'est dist**3 mais avec dist**2 c'est plus joli
					float force_coeff= (body_interaction->_attraction* body1->_mass* body2->_mass/ (body_interaction->_bias+ squared_dist));
					body1->_force+= force_coeff* body1tobody2;
					body2->_force-= force_coeff* body1tobody2;
				}
			}
		}
	}

	for (auto & x : _bodies) {
		for (auto & body : x.second) {
			body->_force+= -1.0f* x.first->_friction* body->_speed;

			float squared_force_norm= body->_force.x* body->_force.x+ body->_force.y* body->_force.y+ body->_force.z* body->_force.z;
			if (squared_force_norm> x.first->_max_force_squared_norm) {
				body->_force*= (x.first->_max_force_squared_norm/ squared_force_norm);
			}

			body->_acceleration= (1.0f/ body->_mass)* body->_force;
			body->_speed+= body->_acceleration;
			body->_position+= body->_speed;

			if (body->_position.x< x.first->_limit._vmin.x) {
				body->_position.x= x.first->_limit._vmin.x;
				body->_speed.x*= -1.0f;
			}
			else if (body->_position.x> x.first->_limit._vmax.x) {
				body->_position.x= x.first->_limit._vmax.x;
				body->_speed.x*= -1.0f;
			}

			if (body->_position.y< x.first->_limit._vmin.y) {
				body->_position.y= x.first->_limit._vmin.y;
				body->_speed.y*= -1.0f;
			}
			else if (body->_position.y> x.first->_limit._vmax.y) {
				body->_position.y= x.first->_limit._vmax.y;
				body->_speed.y*= -1.0f;
			}

			if (body->_position.z< x.first->_limit._vmin.z) {
				body->_position.z= x.first->_limit._vmin.z;
				body->_speed.z*= -1.0f;
			}
			else if (body->_position.z> x.first->_limit._vmax.z) {
				body->_position.z= x.first->_limit._vmax.z;
				body->_speed.z*= -1.0f;
			}
		}
	}
}


void ThreeBody::add_random_bodies(BodyType * body_type, unsigned int n_bodies) {
	for (unsigned int idx_body=0; idx_body<n_bodies; ++idx_body) {
		Body * body= add_body(body_type);
		body->randomize();
	}
}


void ThreeBody::randomize(int n_types, AABB limit, glm::vec2 friction, glm::vec2 max_force_squared_norm,
	glm::vec2 mass_limit, glm::vec2 radius_limit, glm::vec2 threshold, glm::vec2 attraction, glm::vec2 bias, 
	glm::ivec2 n_bodies) {
	
	clear_all();

	for (int i=0; i<n_types; ++i) {
		add_type(glm::vec3(rand_float(0.2f, 1.0f), rand_float(0.2f, 1.0f), rand_float(0.2f, 1.0f)), 
			limit, rand_float(friction[0], friction[1]), rand_float(max_force_squared_norm[0], max_force_squared_norm[1]),
			mass_limit, radius_limit
		);
	}
	for (auto & x : _bodies) {
		for (auto & y : _bodies) {
			add_interaction(x.first, y.first, rand_float(threshold[0], threshold[1]), rand_float(attraction[0], attraction[1]), rand_float(bias[0], bias[1]));
		}
	}
	for (auto & x : _bodies) {
		add_random_bodies(x.first, rand_int(n_bodies[0], n_bodies[1]));
	}
}


void ThreeBody::randomize_radius_per_type() {
	for (auto & x : _bodies) {
		float radius= rand_float(x.first->_radius_limit[0], x.first->_radius_limit[1]);
		for (auto & body : x.second) {
			body->_radius= radius;
		}
	}
}


void ThreeBody::dispatch_bodies(int group_size) {
	for (auto & x : _bodies) {
		int compt= group_size;
		AABB sublimit;
		for (auto & body : x.second) {
			//std::cout << sublimit << "\n";
			compt++;
			if (compt>= group_size) {
				compt= 0;
				sublimit= x.first->_limit;
				sublimit.scale(rand_float(0.1f, 0.4f));
				sublimit.translate(glm::vec3(
					rand_float(-0.9f* x.first->_limit._radius, x.first->_limit._radius* 0.9f),
					rand_float(-0.9f* x.first->_limit._radius, x.first->_limit._radius* 0.9f),
					rand_float(-0.9f* x.first->_limit._radius, x.first->_limit._radius* 0.9f)
				));
			}
			body->_position.x= rand_float(sublimit._vmin.x, sublimit._vmax.x);
			body->_position.y= rand_float(sublimit._vmin.y, sublimit._vmax.y);
			body->_position.z= rand_float(sublimit._vmin.z, sublimit._vmax.z);
		}
	}
}


void ThreeBody::draw(const glm::mat4 & world2clip) {
	if (_bodies.size()== 0) {
		return;
	}

	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(world2clip));
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));
	glDrawArrays(GL_TRIANGLES, 0, _n_bodies* 6);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void ThreeBody::update() {
	if (_bodies.size()== 0) {
		return;
	}

	_n_bodies= 0;
	for (auto & x : _bodies) {
		_n_bodies+= x.second.size();
	}

	float data[36* _n_bodies];

	int compt= 0;
	for (auto & x : _bodies) {
		for (auto & body : x.second) {
			data[compt+ 0]= body->_position.x- body->_radius;
			data[compt+ 1]= body->_position.y- body->_radius;
			data[compt+ 2]= body->_position.z;
			data[compt+ 3]= x.first->_color.x;
			data[compt+ 4]= x.first->_color.y;
			data[compt+ 5]= x.first->_color.z;

			data[compt+ 6]= body->_position.x+ body->_radius;
			data[compt+ 7]= body->_position.y- body->_radius;
			data[compt+ 8]= body->_position.z;
			data[compt+ 9]= x.first->_color.x;
			data[compt+ 10]= x.first->_color.y;
			data[compt+ 11]= x.first->_color.z;

			data[compt+ 12]= body->_position.x+ body->_radius;
			data[compt+ 13]= body->_position.y+ body->_radius;
			data[compt+ 14]= body->_position.z;
			data[compt+ 15]= x.first->_color.x;
			data[compt+ 16]= x.first->_color.y;
			data[compt+ 17]= x.first->_color.z;

			data[compt+ 18]= body->_position.x- body->_radius;
			data[compt+ 19]= body->_position.y- body->_radius;
			data[compt+ 20]= body->_position.z;
			data[compt+ 21]= x.first->_color.x;
			data[compt+ 22]= x.first->_color.y;
			data[compt+ 23]= x.first->_color.z;

			data[compt+ 24]= body->_position.x+ body->_radius;
			data[compt+ 25]= body->_position.y+ body->_radius;
			data[compt+ 26]= body->_position.z;
			data[compt+ 27]= x.first->_color.x;
			data[compt+ 28]= x.first->_color.y;
			data[compt+ 29]= x.first->_color.z;

			data[compt+ 30]= body->_position.x- body->_radius;
			data[compt+ 31]= body->_position.y+ body->_radius;
			data[compt+ 32]= body->_position.z;
			data[compt+ 33]= x.first->_color.x;
			data[compt+ 34]= x.first->_color.y;
			data[compt+ 35]= x.first->_color.z;

			compt+= 36;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool ThreeBody::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_a) {
		clear_all();
		
		// color, limit, friction, max_force_squared_norm
		BodyType * type1= add_type(glm::vec3(1.0f, 0.0f, 0.0f), 
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), 0.001f, 10.0f, 
			glm::vec2(DEFAULT_MASS, DEFAULT_MASS), glm::vec2(DEFAULT_RADIUS, DEFAULT_RADIUS));

		// type1, type2, threshold, attraction, bias
		add_interaction(type1, type1, 10.0f, 0.01f, 0.1f);

		// type, n_bodies, limit
		add_random_bodies(type1, 50);
		
		set_all_z2zero();
		return true;
	}
	else if (key== SDLK_b) {
		clear_all();
		
		BodyType * type1= add_type(glm::vec3(1.0f, 0.0f, 0.0f), 
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), 0.001f, 10.0f,
			glm::vec2(DEFAULT_MASS, DEFAULT_MASS), glm::vec2(DEFAULT_RADIUS, DEFAULT_RADIUS));
		BodyType * type2= add_type(glm::vec3(0.0f, 1.0f, 0.0f), 
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), 0.001f, 10.0f,
			glm::vec2(DEFAULT_MASS, DEFAULT_MASS), glm::vec2(DEFAULT_RADIUS, DEFAULT_RADIUS));
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
		clear_all();
		
		BodyType * type1= add_type(glm::vec3(1.0f, 0.2f, 0.1f), 
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), 0.001f, 10.0f,
			glm::vec2(DEFAULT_MASS, DEFAULT_MASS), glm::vec2(DEFAULT_RADIUS, DEFAULT_RADIUS));
		BodyType * type2= add_type(glm::vec3(0.1f, 1.0f, 0.2f), 
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), 0.001f, 10.0f,
			glm::vec2(DEFAULT_MASS, DEFAULT_MASS), glm::vec2(DEFAULT_RADIUS, DEFAULT_RADIUS));
		BodyType * type3= add_type(glm::vec3(0.3f, 0.3f, 1.0f), 
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), 0.001f, 10.0f,
			glm::vec2(DEFAULT_MASS, DEFAULT_MASS), glm::vec2(DEFAULT_RADIUS, DEFAULT_RADIUS));
		add_interaction(type1, type1, 10.0f, 0.01f, 0.1f);
		add_interaction(type2, type2, 10.0f, -0.01f, 0.1f);
		add_interaction(type3, type3, 10.0f, 0.01f, 0.1f);
		add_interaction(type1, type2, 10.0f, 0.01f, 0.1f);
		add_interaction(type2, type1, 10.0f, -0.01f, 0.1f);
		add_interaction(type1, type3, 10.0f, -0.01f, 0.1f);
		add_interaction(type3, type1, 10.0f, -0.01f, 0.1f);
		add_interaction(type2, type3, 10.0f, 0.01f, 0.1f);
		add_interaction(type3, type2, 10.0f, 0.01f, 0.1f);
		add_random_bodies(type1, 20);
		add_random_bodies(type2, 40);
		add_random_bodies(type3, 80);
		
		set_all_z2zero();

		//write_json("../data/test.json");
		return true;
	}
	else if (key== SDLK_d) {
		//read_json("../data/test.json");
		return true;
	}
	else if (key== SDLK_e) {
		randomize(
			7, // n_types
			AABB(glm::vec3(-50.0f), glm::vec3(50.0f)), // limit
			glm::vec2(0.01f, 0.01f), // friction
			glm::vec2(10.0f, 10.0f), // max_squared_norm
			glm::vec2(1.0f, 1.0f), // mass_limit
			glm::vec2(0.5f, 2.0f), // radius_limit
			glm::vec2(10.0f, 60.0f), // threshold
			glm::vec2(-0.1f, 0.05f), // attraction
			glm::vec2(1.0f, 20.0f), // bias
			glm::ivec2(60, 60) // n_bodies
		);

		randomize_radius_per_type();

		dispatch_bodies(19);
		
		set_all_z2zero();
		return true;
	}

	return false;
}


void ThreeBody::set_all_z2zero() {
	for (auto & x : _bodies) {
		for (auto & body : x.second) {
			body->_position.z= 0.0f;
			body->_speed.z= 0.0f;
			body->_acceleration.z= 0.0f;
		}
	}
}

/*
void ThreeBody::read_json(std::string filepath) {
	std::ifstream ifs(filepath);
	json js= json::parse(ifs);

	clear_all();

	for (auto & type : js["types"]) {
		add_type(
			glm::vec3(type["color"][0], type["color"][1], type["color"][2]), 
			AABB(
				glm::vec3(type["limit"][0], type["limit"][1], type["limit"][2]),
				glm::vec3(type["limit"][3], type["limit"][4], type["limit"][5])
			),
			type["friction"], type["max_force_squared_norm"]
		);
	}
	
	for (auto & inter : js["interactions"]) {
		add_interaction(
			_body_types[inter["body_type_1"]],
			_body_types[inter["body_type_2"]],
			inter["threshold"],
			inter["attraction"],
			inter["bias"]
		);
	}

	for (auto & body : js["bodies"]) {
		add_body(
			_body_types[body["body_type"]],
			glm::vec3(body["position"][0], body["position"][1], body["position"][2]),
			glm::vec3(body["speed"][0], body["speed"][1], body["speed"][2]),
			glm::vec3(body["acceleration"][0], body["acceleration"][1], body["acceleration"][2]),
			body["mass"]
		);
	}
}


void ThreeBody::write_json(std::string filepath) {
	json js;
	js["interactions"]= {};
	js["types"]= {};

	for (auto & inter : _body_interactions) {
		int idx1= get_body_type_idx(inter->_body_type_1);
		int idx2= get_body_type_idx(inter->_body_type_2);
		js["interactions"].push_back({
			{"body_type_1", idx1},
			{"body_type_2", idx2},
			{"threshold", inter->_threshold},
			{"attraction", inter->_attraction},
			{"bias", inter->_bias}
		});
	}

	for (auto & x : _bodies) {
		json js_bodies;
		for (auto & body : x.second) {
			js_bodies.push_back({
				{"position", {body->_position.x, body->_position.y, body->_position.z}},
				{"speed", {body->_speed.x, body->_speed.y, body->_speed.z}},
				{"acceleration", {body->_acceleration.x, body->_acceleration.y, body->_acceleration.z}},
				{"mass", body->_mass}
			});
		}
		js["types"].push_back({
			{"color", {x.first->_color.x, x.first->_color.y, x.first->_color.z}},
			{"limit", {
				x.first->_limit._vmin.x, x.first->_limit._vmin.y, x.first->_limit._vmin.z, 
				x.first->_limit._vmax.x, x.first->_limit._vmax.y, x.first->_limit._vmax.z
			}},
			{"friction", x.first->_friction},
			{"max_force_squared_norm", x.first->_max_force_squared_norm},
			{"bodies", js_bodies}
		});
	}

	std::ofstream ofs(filepath);
	ofs << std::setw(4) << js << std::endl;
}
*/
