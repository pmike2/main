#ifndef ELEMENTS_GL_H
#define ELEMENTS_GL_H

#include <vector>
#include <map>
#include <string>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "gl_utils.h"

#include "elements.h"


uint TREE_N_POINTS_PER_CIRCLE = 6;


class ForestGL {
public:
	ForestGL();
	ForestGL(std::map<std::string, GLuint> progs);
	~ForestGL();
	void draw_simple(const glm::mat4 & world2clip);
	void update_simple();


	std::map<std::string, TreeSpecies *> _species;
	std::vector<Tree *> _trees;
	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers; // buffers OpenGL
};

#endif
