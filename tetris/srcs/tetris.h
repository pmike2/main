#ifndef TETRIS_H
#define TETRIS_H

#include <vector>

#include <glm/glm.hpp>

class Shape {
public:
	Shape();
	~Shape();

	
	std::vector<glm::ivec2> _indices;
};


class InstancedShape {
public:
	InstancedShape();
	~InstancedShape();


	Shape * _shape;
	uint _idx_shape;
	glm::ivec2 _offset;
	glm::vec4 _color;
};


class Level {
public:
	Level();
	~Level();


	uint _idx;
	uint _new_object_ms;
};


class Grid {
public:
	Grid();
	~Grid();


	uint _width;
	uint _height;
	std::vector<bool> _state;
};


class Tetris {
public:
	Tetris();
	~Tetris();


	Level * _current_level;
	std::vector<Shape * > _shapes;
	std::vector<InstancedShape *> _instanced_shapes;
	uint _score;
	bool _game_over;
};

#endif
