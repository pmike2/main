
#ifndef THREE_BODY_H
#define THREE_BODY_H

#include <string>
#include <vector>
#include <map>
#include <utility>

#include <OpenGL/gl3.h>
#include <SDL2/SDL_keycode.h>
#include <glm/glm.hpp>

#include "json.hpp"

#include "sio_client.h"

#include "input_state.h"
#include "bbox.h"


const float DEFAULT_MASS= 1.0f;
const float DEFAULT_RADIUS= 1.0f;
const float COLLISION_FACTOR= 0.001f;


class BodyType {
public:
	BodyType();
	BodyType(glm::vec3 color, AABB limit, float friction, float max_force_squared_norm, glm::vec2 mass_limit, glm::vec2 radius_limit);
	~BodyType();

	
	glm::vec3 _color;
	AABB _limit;
	float _friction;
	float _max_force_squared_norm;
	glm::vec2 _mass_limit;
	glm::vec2 _radius_limit;
};


class BodyInteraction {
public:
	BodyInteraction();
	BodyInteraction(BodyType * body_type_1, BodyType * body_type_2, float threshold, float attraction, float bias);
	~BodyInteraction();


	BodyType * _body_type_1;
	BodyType * _body_type_2;
	float _threshold;
	float _attraction;
	float _bias;
};


class Body {
public:
	Body();
	Body(BodyType * body_type);
	Body(BodyType * body_type, glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass, float radius);
	~Body();
	void randomize();


	BodyType * _body_type;
	glm::vec3 _position;
	glm::vec3 _speed;
	glm::vec3 _acceleration;
	glm::vec3 _force;
	float _mass;
	float _radius;
};


class ThreeBody {
public:
	ThreeBody();
	ThreeBody(GLuint prog_draw);
	~ThreeBody();
	BodyType * add_type(glm::vec3 color, AABB limit, float friction, float max_force_squared_norm, glm::vec2 mass_limit, glm::vec2 radius_limit);
	BodyInteraction * add_interaction(BodyType * body_type_1, BodyType * body_type_2, float threshold, float attraction, float bias);
	Body * add_body(BodyType * body_type, glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass, float radius);
	Body * add_body(BodyType * body_type);
	void clear_bodies();
	void clear_all();
	void anim();
	void add_random_bodies(BodyType * body_type, unsigned int n_bodies);
	void randomize(int n_types, AABB limit, glm::vec2 friction, glm::vec2 max_force_squared_norm, 
		glm::vec2 mass_limit, glm::vec2 radius_limit, glm::vec2 threshold, glm::vec2 attraction, 
		glm::vec2 bias, glm::ivec2 n_bodies);
	void randomize_radius_per_type();
	void prune_with_radius();
	void dispatch_bodies(int group_size);
	void draw(const glm::mat4 & world2clip);
	void update();
	bool key_down(InputState * input_state, SDL_Keycode key);
	void set_all_z2zero();
	void read_json_file(std::string filepath);
	void read_json(nlohmann::json js);
	void write_json(std::string filepath);


	std::map<BodyType *, std::vector<Body *> > _bodies;
	std::vector<BodyInteraction *> _body_interactions;
	bool _paused;

	GLuint _prog_draw;
	GLint _world2clip_loc, _position_loc, _color_loc, _tex_loc;
	GLuint _buffer;
	unsigned int _n_bodies;
	sio::client _io;
};

#endif
