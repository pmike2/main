#include <glm/gtc/type_ptr.hpp>

#include "test_geom.h"


TestGeom::TestGeom() {

}


TestGeom::TestGeom(std::map<std::string, GLuint> progs) : _draw_points(true), _draw_hull(true) {
	GLuint buffers[2];
	glGenBuffers(2, buffers);

	_contexts["points"]= new DrawContext(progs["repere"], buffers[0],
		std::vector<std::string>{"position_in", "color_in"},
		std::vector<std::string>{"world2clip_matrix"});

	_contexts["hull"]= new DrawContext(progs["light"], buffers[1],
		std::vector<std::string>{"position_in", "color_in", "normal_in"},
		std::vector<std::string>{"world2clip_matrix", "light_position", "light_color", "view_position"});
	
	_hull = new ConvexHull();
}


TestGeom::~TestGeom() {
	delete _hull;
}


void TestGeom::test_simple() {
	_hull->add_pt(4.0, 10.0, 1.0);
	_hull->add_pt(3.0, 8.0, 3.0);
	_hull->add_pt(2.0, 1.0, 8.0);
	_hull->add_pt(9.0, 7.0, 2.0);
	_hull->add_pt(2.0, 1.7, 6.0);
	_hull->add_pt(9.0, 4.0, 10.0);

	_hull->compute();
	update_points();
	update_hull();
}


void TestGeom::randomize() {
	uint n_pts = 600;
	number size = 10.0;
	_hull->randomize(n_pts, 0.0, size, 0.0, size, 0.0, size);

	_hull->compute();
	update_points();
	update_hull();
}


void TestGeom::draw_points(const glm::mat4 & world2clip) {
	DrawContext * context= _contexts["points"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glUniformMatrix4fv(context->_locs_uniform["world2clip_matrix"], 1, GL_FALSE, glm::value_ptr(world2clip));
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 3, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_POINTS, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void TestGeom::draw_hull(const glm::mat4 & world2clip, const glm::vec3 & camera_position) {
	DrawContext * context= _contexts["hull"];

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


void TestGeom::draw(const glm::mat4 & world2clip, const glm::vec3 & camera_position) {
	if (_draw_points) {
		draw_points(world2clip);
	}
	if (_draw_hull) {
		draw_hull(world2clip, camera_position);
	}
}


void TestGeom::update_points() {
	DrawContext * context= _contexts["points"];
	context->_n_pts= _hull->_pts.size();
	context->_n_attrs_per_pts= 7;

	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr= data;
	glm::vec4 color(1.0f, 0.0f, 0.0f, 1.0f);
	for (auto pt : _hull->_pts) {
		ptr[0] = float(pt->_coords.x);
		ptr[1] = float(pt->_coords.y);
		ptr[2] = float(pt->_coords.z);
		ptr[3] = color[0];
		ptr[4] = color[1];
		ptr[5] = color[2];
		ptr[6] = color[3];
		ptr += 7;
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void TestGeom::update_hull() {
	DrawContext * context= _contexts["hull"];
	context->_n_pts= _hull->_faces.size() * 3;
	context->_n_attrs_per_pts= 10;
	
	float data[context->_n_pts* context->_n_attrs_per_pts];
	float * ptr= data;
	glm::vec4 color(0.0f, 1.0f, 1.0f, 0.5f);
	for (auto face : _hull->_faces) {
		for (uint i=0; i<3; ++i) {
			Pt * pt = _hull->_pts[face->_idx[i]];
			ptr[0] = float(pt->_coords.x);
			ptr[1] = float(pt->_coords.y);
			ptr[2] = float(pt->_coords.z);
			ptr[3] = color[0];
			ptr[4] = color[1];
			ptr[5] = color[2];
			ptr[6] = color[3];
			ptr[7] = float(face->_normal.x);
			ptr[8] = float(face->_normal.y);
			ptr[9] = float(face->_normal.z);
			ptr += 10;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
