#ifndef ASTEROID_H
#define ASTEROID_H

#include <vector>
#include <map>

#include "bbox_2d.h"
#include "gl_utils.h"
#include "input_state.h"


const glm::vec4 friendly_color(0.0, 1.0, 0.0, 1.0);
const glm::vec4 unfriendly_color(1.0, 0.0, 0.0, 1.0);


class Ship {
public:
	Ship();
	Ship(const AABB_2D & aabb, bool friendly);
	~Ship();
	void draw_aabb();
	void draw_texture();
	void anim();


	AABB_2D _aabb;
	//glm::vec2 _velocity;
	bool _friendly;
};


class Level {
public:
	Level();
	Level(GLuint prog_draw_simple, GLuint prog_draw_texture);
	~Level();
	void draw_aabb(const glm::mat4 & world2clip);
	void draw_texture(const glm::mat4 & world2clip);
	void draw(const glm::mat4 & world2clip);
	void update_simple();
	void anim();
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);


	std::vector<Ship *> _ships;
	bool _draw_aabb;
	bool _draw_texture;
	pt_type _pt_min;
	pt_type _pt_max;
	std::map<std::string, DrawContext *> _contexts;
	unsigned int _n_pts_aabb;
	bool _key_left, _key_right, _key_up, _key_down;
};


#endif
