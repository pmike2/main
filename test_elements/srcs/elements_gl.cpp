#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "elements_gl.h"


// ForestGL -------------------------------------------------------------------------------------------------------
ForestGL::ForestGL() {

}


ForestGL::ForestGL(std::map<std::string, GLuint> progs) {
	GLuint buffers[2];
	glGenBuffers(2, buffers);

	_contexts["simple"]= new DrawContext(progs["repere"], buffers[0],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});

}


ForestGL::~ForestGL() {

}


void ForestGL::draw_simple(const glm::mat4 & world2clip) {
	DrawContext * context= _contexts["simple"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void ForestGL::update_simple() {
	const unsigned int n_pts_per_obj= 8;

	DrawContext * context= _contexts["simple"];
	context->_n_pts= n_pts_per_obj* ;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr= data;
	
	for (auto tree : _trees) {
		for (auto branch : tree->_branches) {
			float circle_base[TREE_N_POINTS_PER_CIRCLE] = {};
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
