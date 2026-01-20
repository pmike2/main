#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "elements_gl.h"


// ElementsGL -------------------------------------------------------------------------------------------------------
ElementsGL::ElementsGL() {

}


ElementsGL::ElementsGL(std::map<std::string, GLuint> progs) {
	_elements = new Elements("../data");
	for (uint i=0; i<10; ++i) {
		for (uint j=0; j<10; ++j) {
			//std::cout << i << " ; " << j << "\n";
			_elements->add_tree("tree_test", pt_3d(number(i) * 3.0, number(j) * 3.0, 0.0), pt_3d(1.0, 1.0, 4.0));
		}
	}
	
	for (uint i=0; i<10; ++i) {
		_elements->add_stone(pt_3d(number(i) * 3.0, -2.0, 0.0), pt_3d(1.0, 1.0, 1.0));
	}

	GLuint buffers[2];
	glGenBuffers(2, buffers);

	_contexts["bbox"]= new DrawContext(progs["repere"], buffers[0],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});

	_contexts["light"]= new DrawContext(progs["light"], buffers[1],
		std::vector<std::string>{"position_in", "color_in", "normal_in"},
		std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position"});

	update_bbox();
	update_light();
}


ElementsGL::~ElementsGL() {

}


void ElementsGL::draw_bbox(const glm::mat4 & world2clip) {
	DrawContext * context= _contexts["bbox"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void ElementsGL::draw_light(const glm::mat4 & world2clip, const glm::vec3 & camera_position) {
	DrawContext * context= _contexts["light"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glm::vec3 light_position(5.0f, 5.0f, 50.0f);
	glm::vec3 light_color(1.0f);

	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	glUniform3fv(context->_locs_uniform["light_position"], 1, glm::value_ptr(light_position));
	glUniform3fv(context->_locs_uniform["light_color"], 1, glm::value_ptr(light_color));
	glUniform3fv(context->_locs_uniform["view_position"], 1, glm::value_ptr(camera_position));
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(context->_locs_attrib["normal_in"], 3, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(7* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void ElementsGL::update_bbox() {
	DrawContext * context= _contexts["bbox"];
	context->_n_pts= _elements->_elements.size() * 48;
	context->_n_attrs_per_pts= 7;
	
	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr= data;
	glm::vec4 bbox_color(1.0, 0.8, 0.2, 1.0);

	for (auto element : _elements->_elements) {
		AABB * aabb = element->_aabb;
		std::vector<pt_3d> pts = aabb->segments();
		for (auto pt : pts) {
			ptr[0] = float(pt.x);
			ptr[1] = float(pt.y);
			ptr[2] = float(pt.z);
			ptr[3] = bbox_color[0];
			ptr[4] = bbox_color[1];
			ptr[5] = bbox_color[2];
			ptr[6] = bbox_color[3];
			ptr += 7;
		}
	}

	/*for (int i=0; i<context->_n_pts* context->_n_attrs_per_pts; ++i) {
		std::cout << data[i] << " ; ";
	}
	std::cout << "\n";*/

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void ElementsGL::update_light() {

	uint n_pts = 0;
	for (auto element : _elements->_elements) {
		n_pts += element->_n_pts;
	}

	DrawContext * context= _contexts["light"];
	context->_n_pts= n_pts;
	context->_n_attrs_per_pts= 10;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	uint compt = 0;
	for (auto element : _elements->_elements) {
		for (uint i=0; i<element->_n_pts * context->_n_attrs_per_pts; ++i) {
			data[compt++] = element->_data[i];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
