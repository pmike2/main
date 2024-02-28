
#ifndef THREE_BODY_H
#define THREE_BODY_H

#include <string>
#include <vector>
#include <deque>

#include <OpenGL/gl3.h>
#include <SDL2/SDL_keycode.h>
#include <glm/glm.hpp>

#include "json.hpp"

#include "input_state.h"
#include "bbox.h"


const unsigned int MAX_HISTO_SIZE= 1;


class BodyType {
public:
	BodyType();
	BodyType(glm::vec3 color, AABB limit, float friction, float max_force_squared_norm);
	~BodyType();

	
	glm::vec3 _color;
	AABB _limit;
	float _friction;
	float _max_force_squared_norm;
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
	Body(BodyType * body_type, glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass);
	~Body();
	void print();
	void randomize();
	void clear_histo();


	BodyType * _body_type;
	glm::vec3 _position;
	glm::vec3 _speed;
	glm::vec3 _acceleration;
	float _mass;
	std::deque<glm::vec3> _histo_position;
};


class ThreeBody {
public:
	ThreeBody();
	ThreeBody(GLuint prog_draw);
	~ThreeBody();
	BodyType * add_type(glm::vec3 color, AABB limit, float friction, float max_force_squared_norm);
	void add_interaction(BodyType * body_type_1, BodyType * body_type_2, float threshold, float attraction, float bias);
	Body * add_body(BodyType * body_type, glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass);
	Body * add_body(BodyType * body_type);
	void clear_bodies();
	void clear_all();
	BodyInteraction * get_interaction(BodyType * body_type_1, BodyType * body_type_2);
	int get_body_type_idx(BodyType * body_type);
	void anim();
	void print();
	void add_random_bodies(BodyType * body_type, unsigned int n_bodies);
	void randomize(int n_types, AABB limit, glm::vec2 friction, glm::vec2 max_force_squared_norm, glm::vec2 threshold, glm::vec2 attraction, glm::vec2 bias, glm::ivec2 n_bodies);
	void draw(const glm::mat4 & world2clip);
	void update();
	bool key_down(InputState * input_state, SDL_Keycode key);
	void set_all_z2zero();
	void read_json(std::string filepath);
	void write_json(std::string filepath);


	std::vector<Body *> _bodies;
	std::vector<BodyType *> _body_types;
	std::vector<BodyInteraction *> _body_interactions;
	GLuint _prog_draw;
	GLint _world2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffers[2];
	unsigned int _n_pts;
};

#endif
