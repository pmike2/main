#ifndef ELEMENTS_GL_H
#define ELEMENTS_GL_H

#include <vector>
#include <map>
#include <string>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "gl_utils.h"

#include "elements.h"



class ElementsGL {
public:
	ElementsGL();
	ElementsGL(std::map<std::string, GLuint> progs);
	~ElementsGL();
	void draw_simple(const glm::mat4 & world2clip);
	void draw_light(const glm::mat4 & world2clip, const glm::vec3 & camera_position);
	void update_simple();
	void update_light();


	Forest * _forest;

	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers; // buffers OpenGL
};

#endif
