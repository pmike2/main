
#ifndef SPRINGS_H
#define SPRINGS_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL.h>

#include "constantes.h"
#include "utile.h"
#include "input_state.h"



// a terme sortir la couleur de force
class Force {
public:
	Force();
	Force(glm::vec3 vec, std::string type);
	void print();


	glm::vec3 _vec;
	std::string _type;
	float _color[3];
};


class Vertex {
public:
	Vertex();
	Vertex(glm::vec3 p, float mass, float air_resist);
	void reinit();


	glm::vec3 _p;
	glm::vec3 _p_init;
	glm::vec3 _v;
	glm::vec3 _a;
	float _mass;
	float _air_resist;
	std::vector<Force> _forces;

};


class Spring {
public:
	Spring();
	Spring(uint idx1, uint idx2, float stiffness, float damping, float relaxed_size);
	Spring(uint idx1, uint idx2, float stiffness, float damping, float relaxed_size, float contract_amount, float contract_begin, float contract_end);
	void rand_contract();


	uint _idx1;
	uint _idx2;
	float _stiffness;
	float _damping;
	float _relaxed_size;
	float _contract_amount, _contract_begin, _contract_end;
};


class SpringSystem {
public:
	SpringSystem();
	friend std::ostream & operator << (std::ostream &, SpringSystem);
	friend std::istream & operator >> (std::istream &, SpringSystem &);
	void load(std::string ch);
	void save(std::string ch);
	void anim();
	void add_vertex(glm::vec3 p_, float mass_, float air_resist_);
	void add_spring(uint idx1_, uint idx2_, float stiffness_, float damping_, float relaxed_size_=-1.);
	void delete_vertex(uint idx);
	void delete_spring(uint idx);
	void delete_disconnected_vertices();
	std::vector<uint> get_ajdacents_vertices(uint idx);
	std::vector<uint> get_ajdacents_springs(uint idx);
	void print();
	void rand_contracts();
	glm::vec3 bary(bool is_init);
	float fitness();
	void reinit_vertices();

	// méthodes abstraites pas appelées dans la classe mère
	virtual void init_vertices_springs();
	virtual void rand_disposition(uint n_cubes);


	std::vector<Vertex> _vertices;
	std::vector<Spring> _springs;
	uint _tik_init;
};


class CubeSystem : public SpringSystem {
public:
	CubeSystem();
	void init_vertices_springs();
	void rand_disposition(uint n_cubes);


	std::vector< std::vector<int> > _cubes_idx;
	float _mass;
	float _air_resist;
	float _spring_size;
	float _stiffness;
	float _damping;
};


class SpringSystemGenetic {
public:
	SpringSystemGenetic();
	void next_gen();
	void save_best(std::string ch);


	std::vector<SpringSystem*> _sss;
};


class SpringSystemGL {
public:
	SpringSystemGL();
	SpringSystemGL(GLuint prog_draw);
	void draw(glm::mat4 world2clip);
	void anim();
	void update_data();
	void update_data_forces();
	void update_data_accel_speed();
	bool key_down(InputState * input_state, SDL_Keycode key);


	SpringSystem* _ss;
	GLuint _prog_draw;
	GLuint _buffer, _buffer_forces, _buffer_accel_speed;
	float _data[MAX_DATA];
	float _data_forces[MAX_DATA_FORCES];
	float _data_accel_speed[MAX_DATA_ACCEL_SPEED];
	GLint _world2clip_loc, _position_loc, _diffuse_color_loc;
	bool _is_draw_springs, _is_draw_forces, _is_draw_accel_speed;
	bool _is_paused;
};


bool sort_ss(SpringSystem* s1, SpringSystem* s2);


#endif
