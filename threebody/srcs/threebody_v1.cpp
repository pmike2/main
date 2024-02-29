#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "utile.h"
#include "threebody_v1.h"


using json = nlohmann::json;


// ----------------------------------------------------------------------------
BodyType::BodyType() : _color(glm::vec3(0.0f)), _limit(AABB(glm::vec3(0.0f), glm::vec3(0.0f))) {}


BodyType::BodyType(glm::vec3 color, AABB limit, float friction, float max_force_squared_norm) :
	_color(color), _limit(limit), _friction(friction), _max_force_squared_norm(max_force_squared_norm)
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
	_body_type(NULL), _position(glm::vec3(0.0f)), _speed(glm::vec3(0.0f)), _acceleration(glm::vec3(0.0f)), _force(glm::vec3(0.0f)), _mass(1.0f)
{}


Body::Body(BodyType * body_type) :
	_body_type(body_type), _position(glm::vec3(0.0f)), _speed(glm::vec3(0.0f)), _acceleration(glm::vec3(0.0f)), _force(glm::vec3(0.0f)), _mass(1.0f)
{}


Body::Body(BodyType * body_type, glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass) :
	_body_type(body_type), _position(position), _speed(speed), _acceleration(acceleration), _force(glm::vec3(0.0f)), _mass(mass)
{}


Body::~Body() {
	_histo_position.clear();
}


void Body::print() {
	std::cout << "posit= (" << _position.x << " ; " << _position.y << " ; " << _position.z << ")\n";
	std::cout << "speed= (" << _speed.x << " ; " << _speed.y << " ; " << _speed.z << ")\n";
	std::cout << "accel= (" << _acceleration.x << " ; " << _acceleration.y << " ; " << _acceleration.z << ")\n";
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
}


void Body::clear_histo() {
	_histo_position.clear();
}


// --------------------------------------------------
ThreeBody::ThreeBody() {

}


ThreeBody::ThreeBody(GLuint prog_draw) : _prog_draw(prog_draw), _n_pts(0) {
	_bodies.clear();

	glGenBuffers(2, _buffers);
		
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_world2clip_loc= glGetUniformLocation(_prog_draw, "world2clip_matrix");
}


ThreeBody::~ThreeBody() {
	clear_all();
}


BodyType * ThreeBody::add_type(glm::vec3 color, AABB limit, float friction, float max_force_squared_norm) {
	_body_types.push_back(new BodyType(color, limit, friction, max_force_squared_norm));
	return _body_types[_body_types.size()- 1];
}


BodyInteraction * ThreeBody::add_interaction(BodyType * body_type_1, BodyType * body_type_2, float threshold, float attraction, float bias) {
	_body_interactions.push_back(new BodyInteraction(body_type_1, body_type_2, threshold, attraction, bias));
	_interactions_map[std::make_pair(body_type_1, body_type_2)]= _body_interactions[_body_interactions.size()- 1];
	return _body_interactions[_body_interactions.size()- 1];
}


Body * ThreeBody::add_body(BodyType * body_type, glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass) {
	_bodies.push_back(new Body(body_type, position, speed, acceleration, mass));
	return _bodies[_bodies.size()- 1];
}


Body * ThreeBody::add_body(BodyType * body_type) {
	_bodies.push_back(new Body(body_type));
	return _bodies[_bodies.size()- 1];
}


void ThreeBody::clear_bodies() {
	for (auto & body : _bodies) {
		delete body;
	}
	_bodies.clear();
}


void ThreeBody::clear_all() {
	for (auto & body : _bodies) {
		delete body;
	}
	_bodies.clear();

	for (auto & inter : _body_interactions) {
		delete inter;
	}
	_body_interactions.clear();

	for (auto & type : _body_types) {
		delete type;
	}
	_body_types.clear();
}


BodyInteraction * ThreeBody::get_interaction(BodyType * body_type_1, BodyType * body_type_2) {
	/*for (auto & inter : _body_interactions) {
		if ((inter->_body_type_1== body_type_1) && (inter->_body_type_2== body_type_2)) {
			return inter;
		}
	}
	return NULL;*/
	
	std::pair<BodyType *, BodyType *> p= std::make_pair(body_type_1, body_type_2);
	//if (_interactions_map.count(p)== 0) {
	if (_interactions_map.find(p)== _interactions_map.end()) {
		return NULL;
	}
	//return _interactions_map[p];

	return _body_interactions[0];
}


int ThreeBody::get_body_type_idx(BodyType * body_type) {
	for (int idx=0; idx<_body_types.size(); ++idx) {
		if (_body_types[idx]== body_type) {
			return idx;
		}
	}
	return -1;
}


void ThreeBody::anim() {
	if (_bodies.size()== 0) {
		return;
	}

	for (auto & body : _bodies) {
		body->_force.x= 0.0f; body->_force.y= 0.0f; body->_force.z= 0.0f;
	}

	for (unsigned int idx_body_1=0; idx_body_1<_bodies.size()- 1; ++idx_body_1) {

		for (unsigned int idx_body_2=idx_body_1+ 1; idx_body_2<_bodies.size(); ++idx_body_2) {

			BodyInteraction * body_interaction= get_interaction(_bodies[idx_body_1]->_body_type, _bodies[idx_body_2]->_body_type);
			if (body_interaction== NULL) {
				continue;
			}

			glm::vec3 body1tobody2= _bodies[idx_body_2]->_position- _bodies[idx_body_1]->_position;
			float squared_dist= body1tobody2.x* body1tobody2.x+ body1tobody2.y* body1tobody2.y+ body1tobody2.z* body1tobody2.z;

			if (squared_dist> body_interaction->_threshold* body_interaction->_threshold) {
				continue;
			}

			// normalement c'est dist**3 mais avec dist**2 c'est plus joli
			float force_coeff= (body_interaction->_attraction* _bodies[idx_body_1]->_mass* _bodies[idx_body_2]->_mass/ (body_interaction->_bias+ squared_dist));
			_bodies[idx_body_1]->_force+= force_coeff* body1tobody2;
			_bodies[idx_body_2]->_force-= force_coeff* body1tobody2;
		}
	}

	for (auto & body : _bodies) {
		body->_force+= -1.0f* body->_body_type->_friction* body->_speed;

		float squared_force_norm= body->_force.x* body->_force.x+ body->_force.y* body->_force.y+ body->_force.z* body->_force.z;
		if (squared_force_norm> body->_body_type->_max_force_squared_norm) {
			body->_force*= (body->_body_type->_max_force_squared_norm/ squared_force_norm);
		}

		body->_acceleration= (1.0f/ body->_mass)* body->_force;
		body->_speed+= body->_acceleration;
		body->_position+= body->_speed;

		/*if ((body->_position.x< body->_body_type->_limit._vmin.x) ||
			(body->_position.x> body->_body_type->_limit._vmax.x)) {
			body->_position.x*= -1.0f;
		}
		if ((body->_position.y< body->_body_type->_limit._vmin.y) ||
			(body->_position.y> body->_body_type->_limit._vmax.y)) {
			body->_position.y*= -1.0f;
		}
		if ((body->_position.z< body->_body_type->_limit._vmin.z) ||
			(body->_position.z> body->_body_type->_limit._vmax.z)) {
			body->_position.z*= -1.0f;
		}*/

		if (body->_histo_position.size()== MAX_HISTO_SIZE) {
			body->_histo_position.pop_front();
		}
		body->_histo_position.push_back(body->_position);
	}
}


void ThreeBody::print() {
	for (unsigned int idx_body=0; idx_body<_bodies.size(); ++idx_body) {
		std::cout << idx_body << "\n";
		_bodies[idx_body]->print();
	}
	std::cout << "\n";
}


void ThreeBody::add_random_bodies(BodyType * body_type, unsigned int n_bodies) {
	for (unsigned int idx_body=0; idx_body<n_bodies; ++idx_body) {
		Body * body= add_body(body_type);
		body->randomize();
	}
}


void ThreeBody::randomize(int n_types, AABB limit, glm::vec2 friction, glm::vec2 max_force_squared_norm, glm::vec2 threshold, glm::vec2 attraction, glm::vec2 bias, glm::ivec2 n_bodies) {
	clear_all();

	for (int i=0; i<n_types; ++i) {
		AABB sublimit(limit);
		sublimit.scale(rand_float(0.1f, 0.4f));
		sublimit.translate(glm::vec3(
			rand_float(-0.9f* limit._radius, limit._radius* 0.9f),
			rand_float(-0.9f* limit._radius, limit._radius* 0.9f),
			rand_float(-0.9f* limit._radius, limit._radius* 0.9f)
		));
		//std::cout << sublimit << "\n";
		add_type(glm::vec3(rand_float(0.2f, 1.0f), rand_float(0.2f, 1.0f), rand_float(0.2f, 1.0f)), 
			sublimit, rand_float(friction[0], friction[1]), rand_float(max_force_squared_norm[0], max_force_squared_norm[1]));
	}
	for (int i1=0; i1<_body_types.size(); ++i1) {
		for (int i2=0; i2<_body_types.size(); ++i2) {
			add_interaction(_body_types[i1], _body_types[i2], rand_float(threshold[0], threshold[1]), rand_float(attraction[0], attraction[1]), rand_float(bias[0], bias[1]));
		}
	}
	for (auto & type : _body_types) {
		add_random_bodies(type, rand_int(n_bodies[0], n_bodies[1]));
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
	glDrawArrays(GL_POINTS, 0, _n_pts);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void ThreeBody::update() {
	if (_bodies.size()== 0) {
		return;
	}

	unsigned int histo_size= _bodies[0]->_histo_position.size();
	_n_pts= _bodies.size()* histo_size;
	float data[6* _n_pts];
	for (unsigned int idx_body=0; idx_body<_bodies.size(); ++idx_body) {
		for (unsigned int idx_position=0; idx_position<histo_size; ++idx_position) {
			data[(idx_body* histo_size+ idx_position)* 6+ 0]= _bodies[idx_body]->_histo_position[idx_position].x;
			data[(idx_body* histo_size+ idx_position)* 6+ 1]= _bodies[idx_body]->_histo_position[idx_position].y;
			data[(idx_body* histo_size+ idx_position)* 6+ 2]= _bodies[idx_body]->_histo_position[idx_position].z;
			data[(idx_body* histo_size+ idx_position)* 6+ 3]= _bodies[idx_body]->_body_type->_color.x;
			data[(idx_body* histo_size+ idx_position)* 6+ 4]= _bodies[idx_body]->_body_type->_color.y;
			data[(idx_body* histo_size+ idx_position)* 6+ 5]= _bodies[idx_body]->_body_type->_color.z;
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
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), 0.001f, 10.0f);

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
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), 0.001f, 10.0f);
		BodyType * type2= add_type(glm::vec3(0.0f, 1.0f, 0.0f), 
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), 0.001f, 10.0f);
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
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), 0.001f, 10.0f);
		BodyType * type2= add_type(glm::vec3(0.1f, 1.0f, 0.2f), 
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), 0.001f, 10.0f);
		BodyType * type3= add_type(glm::vec3(0.3f, 0.3f, 1.0f), 
			AABB(glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f)), 0.001f, 10.0f);
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

		write_json("../data/test.json");
		return true;
	}
	else if (key== SDLK_d) {
		read_json("../data/test.json");
		return true;
	}
	else if (key== SDLK_e) {
		float limit= 5.0f;
		randomize(
			8, // n_types
			AABB(glm::vec3(-1.0f* limit), glm::vec3(limit)), // limit
			glm::vec2(0.1f, 0.5f), // friction
			glm::vec2(0.5f, 5.5f), // max_squared_norm
			glm::vec2(20.0f, 40.0f), // threshold
			glm::vec2(-1.0f, 1.0f), // attraction
			glm::vec2(5.0f, 50.0f), // bias
			glm::ivec2(50, 50) // n_bodies
		);
		
		set_all_z2zero();
		return true;
	}

	return false;
}


void ThreeBody::set_all_z2zero() {
	for (auto & body : _bodies) {
		body->_position.z= 0.0f;
		body->_speed.z= 0.0f;
		body->_acceleration.z= 0.0f;
	}
}


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
	js["types"]= {};
	js["interactions"]= {};
	js["bodies"]= {};

	for (auto & type : _body_types) {
		js["types"].push_back({
			{"color", {type->_color.x, type->_color.y, type->_color.z}},
			{"limit", {
				type->_limit._vmin.x, type->_limit._vmin.y, type->_limit._vmin.z, 
				type->_limit._vmax.x, type->_limit._vmax.y, type->_limit._vmax.z
			}},
			{"friction", type->_friction},
			{"max_force_squared_norm", type->_max_force_squared_norm}
		});
	}

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

	for (auto & body : _bodies) {
		int idx= get_body_type_idx(body->_body_type);
		js["bodies"].push_back({
			{"body_type", idx},
			{"position", {body->_position.x, body->_position.y, body->_position.z}},
			{"speed", {body->_speed.x, body->_speed.y, body->_speed.z}},
			{"acceleration", {body->_acceleration.x, body->_acceleration.y, body->_acceleration.z}},
			{"mass", body->_mass}
		});
	}

	std::ofstream ofs(filepath);
	ofs << std::setw(4) << js << std::endl;
}

