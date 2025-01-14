#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "asteroid.h"
#include "utile.h"


// ship -------------------------------------------------------------
Ship::Ship() {

}


Ship::Ship(const AABB_2D & aabb, bool friendly) : _aabb(aabb), _friendly(friendly) {
	
}


Ship::~Ship() {
	
}


void Ship::draw_aabb() {

}


void Ship::draw_texture() {

}


void Ship::anim() {

}


// level -------------------------------------------------------------
Level::Level() {

}


Level::Level(GLuint prog_draw_simple, GLuint prog_draw_texture) 
	: _pt_min(-10.0, -10.0), _pt_max(10.0, 10.0), _draw_aabb(true), _draw_texture(false), _n_pts_aabb(0),
	_key_left(false), _key_right(false), _key_up(false), _key_down(false) {

	_ships.push_back(new Ship(AABB_2D(glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0)), true));
	//unsigned int n_ennemies= rand_int(10, 20);
	unsigned int n_ennemies= 5;
	for (unsigned int i=0; i<n_ennemies; ++i) {
		pt_type pos= pt_type(rand_float(_pt_min.x, _pt_max.x), rand_float(_pt_min.y, _pt_max.y));
		pt_type size= pt_type(rand_float(0.5, 2.0), rand_float(0.5, 2.0));
		_ships.push_back(new Ship(AABB_2D(pos, size), false));
	}

	GLuint buffers[2];
	glGenBuffers(2, buffers);

	_contexts["simple"]= new DrawContext(prog_draw_simple, buffers[0],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});
	
	_contexts["texture"]= new DrawContext(prog_draw_texture, buffers[1],
		std::vector<std::string>{"position_in", "tex_coord_in", "current_layer_in"},
		std::vector<std::string>{"world2clip_matrix", "diffuse_texture_array"});
	
	update_simple();
}


Level::~Level() {

}


void Level::draw_aabb(const glm::mat4 & world2clip) {
	//std::cout << _contexts["simple"]->_prog << " ; " << _contexts["simple"]->_locs["world2clip_matrix"] << " ; " << _contexts["simple"]->_locs["position_in"] << " ; " << _contexts["simple"]->_locs["color_in"] << "\n";
	glUseProgram(_contexts["simple"]->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, _contexts["simple"]->_buffer);
	
	glUniformMatrix4fv(_contexts["simple"]->_locs["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
	glEnableVertexAttribArray(_contexts["simple"]->_locs["position_in"]);
	glEnableVertexAttribArray(_contexts["simple"]->_locs["color_in"]);

	glVertexAttribPointer(_contexts["simple"]->_locs["position_in"], 3, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)0);
	glVertexAttribPointer(_contexts["simple"]->_locs["color_in"], 4, GL_FLOAT, GL_FALSE, 7* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_LINES, 0, _n_pts_aabb);

	glDisableVertexAttribArray(_contexts["simple"]->_locs["position_in"]);
	glDisableVertexAttribArray(_contexts["simple"]->_locs["color_in"]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Level::draw_texture(const glm::mat4 & world2clip) {

}


void Level::draw(const glm::mat4 & world2clip) {
	if (_draw_texture) {
		draw_texture(world2clip);
	}
	if (_draw_aabb) {
		draw_aabb(world2clip);
	}
}


void Level::update_simple() {
	const unsigned int n_pts_per_ship= 8;
	const unsigned int n_attrs_per_pts= 7;

	_n_pts_aabb= 0;
	for (auto ship : _ships) {
		_n_pts_aabb+= n_pts_per_ship;
	}
	//std::cout << "_n_pts_aabb = " << _n_pts_aabb << "\n";

	float data[_n_pts_aabb* n_attrs_per_pts];

	for (unsigned int idx_ship=0; idx_ship<_ships.size(); ++idx_ship) {
		glm::vec4 color;
		if (_ships[idx_ship]->_friendly) {
			color= friendly_color;
		}
		else {
			color= unfriendly_color;
		}
		double positions[n_pts_per_ship* 2]= {
			_ships[idx_ship]->_aabb._pos.x, _ships[idx_ship]->_aabb._pos.y,
			_ships[idx_ship]->_aabb._pos.x+ _ships[idx_ship]->_aabb._size.x, _ships[idx_ship]->_aabb._pos.y,

			_ships[idx_ship]->_aabb._pos.x+ _ships[idx_ship]->_aabb._size.x, _ships[idx_ship]->_aabb._pos.y,
			_ships[idx_ship]->_aabb._pos.x+ _ships[idx_ship]->_aabb._size.x, _ships[idx_ship]->_aabb._pos.y+ _ships[idx_ship]->_aabb._size.y,

			_ships[idx_ship]->_aabb._pos.x+ _ships[idx_ship]->_aabb._size.x, _ships[idx_ship]->_aabb._pos.y+ _ships[idx_ship]->_aabb._size.y,
			_ships[idx_ship]->_aabb._pos.x, _ships[idx_ship]->_aabb._pos.y+ _ships[idx_ship]->_aabb._size.y,

			_ships[idx_ship]->_aabb._pos.x, _ships[idx_ship]->_aabb._pos.y+ _ships[idx_ship]->_aabb._size.y,
			_ships[idx_ship]->_aabb._pos.x, _ships[idx_ship]->_aabb._pos.y
		};

		for (unsigned int idx_pt=0; idx_pt<n_pts_per_ship; ++idx_pt) {
			data[idx_ship* n_pts_per_ship* n_attrs_per_pts+ idx_pt* n_attrs_per_pts+ 0]= positions[2* idx_pt];
			data[idx_ship* n_pts_per_ship* n_attrs_per_pts+ idx_pt* n_attrs_per_pts+ 1]= positions[2* idx_pt+ 1];
			data[idx_ship* n_pts_per_ship* n_attrs_per_pts+ idx_pt* n_attrs_per_pts+ 2]= 0.0; // z
			for (unsigned int idx_color=0; idx_color<4; ++idx_color) {
				data[idx_ship* n_pts_per_ship* n_attrs_per_pts+ idx_pt* n_attrs_per_pts+ 3+ idx_color]= color[idx_color];
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _contexts["simple"]->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* _n_pts_aabb* n_attrs_per_pts, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Level::anim() {
	float HERO_VELOCITY= 0.1;
	if (_key_left) {
		_ships[0]->_aabb._pos.x-= HERO_VELOCITY;
	}
	if (_key_right) {
		_ships[0]->_aabb._pos.x+= HERO_VELOCITY;
	}
	if (_key_down) {
		_ships[0]->_aabb._pos.y-= HERO_VELOCITY;
	}
	if (_key_up) {
		_ships[0]->_aabb._pos.y+= HERO_VELOCITY;
	}

	update_simple();
}


bool Level::key_down(InputState * input_state, SDL_Keycode key) {
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


bool Level::key_up(InputState * input_state, SDL_Keycode key) {
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
