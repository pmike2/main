#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "elements_gl.h"


// ElementsGL -------------------------------------------------------------------------------------------------------
ElementsGL::ElementsGL() {

}


ElementsGL::ElementsGL(std::map<std::string, GLuint> progs) {
	_forest = new Forest("../data");
	for (uint i=0; i<10; ++i) {
		for (uint j=0; j<10; ++j) {
			//std::cout << i << " ; " << j << "\n";
			_forest->add_tree("tree_test", pt_type_3d(number(i) * 3., number(j) * 3.0, 0.0));
		}
	}
	//std::cout << *_forest << "\n";

	GLuint buffers[2];
	glGenBuffers(2, buffers);

	_contexts["simple"]= new DrawContext(progs["repere"], buffers[0],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});

	_contexts["light"]= new DrawContext(progs["light"], buffers[1],
		std::vector<std::string>{"position_in", "color_in", "normal_in"},
		std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position"});

	update_simple();
	update_light();
}


ElementsGL::~ElementsGL() {

}


void ElementsGL::draw_simple(const glm::mat4 & world2clip) {
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


void ElementsGL::update_simple() {
	const uint n_pts_per_branch_side = BRANCH_N_POINTS_PER_CIRCLE * 6;
	const uint n_pts_per_branch_bottom = BRANCH_N_POINTS_PER_CIRCLE * 3;
	const uint n_pts_per_branch_top = BRANCH_N_POINTS_PER_CIRCLE * 3;

	uint n_branches = 0;
	for (auto tree : _forest->_trees) {
		n_branches+= tree->_branches.size();
	}
	//std::cout << n_branches << "\n";

	DrawContext * context= _contexts["simple"];
	context->_n_pts= n_branches * (n_pts_per_branch_side + n_pts_per_branch_bottom + n_pts_per_branch_top);
	context->_n_attrs_per_pts= 7;
	
	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr= data;
	glm::vec4 branch_color(0.5, 0.3, 0.2, 1.0);

	for (auto tree : _forest->_trees) {
		for (auto branch : tree->_branches) {
			for (uint i=0; i<n_pts_per_branch_side; ++i) {
				ptr[0] = float(branch->_vertices_side[i].x);
				ptr[1] = float(branch->_vertices_side[i].y);
				ptr[2] = float(branch->_vertices_side[i].z);
				ptr[3] = branch_color[0];
				ptr[4] = branch_color[1];
				ptr[5] = branch_color[2];
				ptr[6] = branch_color[3];
				ptr += 7;
			}
			for (uint i=0; i<n_pts_per_branch_bottom; ++i) {
				ptr[0] = float(branch->_vertices_bottom[i].x);
				ptr[1] = float(branch->_vertices_bottom[i].y);
				ptr[2] = float(branch->_vertices_bottom[i].z);
				ptr[3] = branch_color[0];
				ptr[4] = branch_color[1];
				ptr[5] = branch_color[2];
				ptr[6] = branch_color[3];
				ptr += 7;
			}
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
	const uint n_pts_per_branch_side= BRANCH_N_POINTS_PER_CIRCLE * 6;
	const uint n_pts_per_branch_bottom = BRANCH_N_POINTS_PER_CIRCLE * 3;
	const uint n_pts_per_branch_top = BRANCH_N_POINTS_PER_CIRCLE * 3;

	uint n_branches = 0;
	for (auto tree : _forest->_trees) {
		n_branches+= tree->_branches.size();
	}
	//std::cout << n_branches << "\n";

	DrawContext * context= _contexts["light"];
	context->_n_pts= n_branches * (n_pts_per_branch_side + n_pts_per_branch_bottom + n_pts_per_branch_top);
	context->_n_attrs_per_pts= 10;

	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr= data;
	glm::vec4 branch_color(0.5, 0.3, 0.2, 1.0);

	for (auto tree : _forest->_trees) {
		for (auto branch : tree->_branches) {
			for (uint i=0; i<n_pts_per_branch_side; ++i) {
				ptr[0] = float(branch->_vertices_side[i].x);
				ptr[1] = float(branch->_vertices_side[i].y);
				ptr[2] = float(branch->_vertices_side[i].z);
				ptr[3] = branch_color[0];
				ptr[4] = branch_color[1];
				ptr[5] = branch_color[2];
				ptr[6] = branch_color[3];
				ptr[7] = float(branch->_normals_side[i].x);
				ptr[8] = float(branch->_normals_side[i].y);
				ptr[9] = float(branch->_normals_side[i].z);
				ptr += 10;
			}
			for (uint i=0; i<n_pts_per_branch_bottom; ++i) {
				ptr[0] = float(branch->_vertices_bottom[i].x);
				ptr[1] = float(branch->_vertices_bottom[i].y);
				ptr[2] = float(branch->_vertices_bottom[i].z);
				ptr[3] = branch_color[0];
				ptr[4] = branch_color[1];
				ptr[5] = branch_color[2];
				ptr[6] = branch_color[3];
				ptr[7] = float(branch->_normals_bottom[i].x);
				ptr[8] = float(branch->_normals_bottom[i].y);
				ptr[9] = float(branch->_normals_bottom[i].z);
				ptr += 10;
			}
			for (uint i=0; i<n_pts_per_branch_top; ++i) {
				ptr[0] = float(branch->_vertices_top[i].x);
				ptr[1] = float(branch->_vertices_top[i].y);
				ptr[2] = float(branch->_vertices_top[i].z);
				ptr[3] = branch_color[0];
				ptr[4] = branch_color[1];
				ptr[5] = branch_color[2];
				ptr[6] = branch_color[3];
				ptr[7] = float(branch->_normals_top[i].x);
				ptr[8] = float(branch->_normals_top[i].y);
				ptr[9] = float(branch->_normals_top[i].z);
				ptr += 10;
			}
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
