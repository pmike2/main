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
	unsigned int _idx_shape;
	glm::ivec2 _offset;
	glm::vec4 _color;
};


class Level {
public:
	Level();
	~Level();


	unsigned int _idx;
	unsigned int _new_object_ms;
};


class Grid {
public:
	Grid();
	~Grid();


	unsigned int _width;
	unsigned int _height;
	std::vector<bool> _state;
};


class Tetris {
public:
	Tetris();
	~Tetris();


	Level * _current_level;
	std::vector<Shape * > _shapes;
	std::vector<InstancedShape *> _instanced_shapes;
	unsigned int _score;
	bool _game_over;
};

#endif
