
#ifndef THREE_BODY_H
#define THREE_BODY_H

#include <string>
#include <vector>
#include <deque>

#include <OpenGL/gl3.h>
#include <SDL2/SDL_keycode.h>
#include <glm/glm.hpp>

#include "input_state.h"


const unsigned int MAX_HISTO_SIZE= 200;


class Body {
public:
	Body();
	Body(glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass);
	~Body();
	void print();
	void randomize(float limit);
	void clear_histo();


	glm::vec3 _position;
	glm::vec3 _speed;
	glm::vec3 _acceleration;
	float _mass;
	std::deque<glm::vec3> _histo_position;
	glm::vec3 _color;
};


class ThreeBody {
public:
	ThreeBody();
	ThreeBody(GLuint prog_draw);
	~ThreeBody();
	void add_body(glm::vec3 position, glm::vec3 speed, glm::vec3 acceleration, float mass);
	void add_body();
	void clear();
	void anim(float delta_t);
	void print();
	void randomize(unsigned int n_bodies, float limit);
	void draw(const glm::mat4 & world2clip);
	void update();
	bool key_down(InputState * input_state, SDL_Keycode key);
	void set_all_z2zero();
	void read_file(std::string filepath);


	std::vector<Body *> _bodies;
	GLuint _prog_draw;
	GLint _world2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffers[2];
	unsigned int _n_pts;
};

#endif
