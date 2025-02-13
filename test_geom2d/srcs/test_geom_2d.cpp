#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "test_geom_2d.h"


TestGeom2D::TestGeom2D() {
}


TestGeom2D::TestGeom2D(GLuint prog_bbox, GLuint prog_font, ScreenGL * screengl) {
	_pt_min= pt_type(-screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f);
	_pt_max= pt_type(screengl->_gl_width* 0.5f, screengl->_gl_height* 0.5f);
	_camera2clip= glm::ortho(-screengl->_gl_width* 0.5f, screengl->_gl_width* 0.5f, -screengl->_gl_height* 0.5f, screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
	_font= new Font(prog_font, "../../fonts/Silom.ttf", 48, screengl);

	unsigned int n_buffers= 1;
	_buffers= new GLuint[n_buffers];
	glGenBuffers(n_buffers, _buffers);

	_contexts["bbox"]= new DrawContext(prog_bbox, _buffers[0],
	std::vector<std::string>{"position_in", "color_in"},
	std::vector<std::string>{"camera2clip_matrix"});

	_poly1= new Polygon2D();
	_poly2= new Polygon2D();

	randomize();
}


TestGeom2D::~TestGeom2D() {
	
}


void TestGeom2D::randomize() {
	std::vector<pt_type> pts1;
	for (unsigned int i=0; i<3; ++i) {
		pts1.push_back(pt_type(0.5* rand_number(_pt_min.x, _pt_max.x), 0.5* rand_number(_pt_min.y, _pt_max.y)));
	}
	_poly1->set_points(pts1);

	std::vector<pt_type> pts2;
	for (unsigned int i=0; i<3; ++i) {
		pts2.push_back(pt_type(0.5* rand_number(_pt_min.x, _pt_max.x), 0.5* rand_number(_pt_min.y, _pt_max.y)));
	}
	_poly2->set_points(pts2);

	_inter= poly_intersects_poly(_poly1, _poly2, &_axis, &_overlap, &_idx_pt, &_is_pt_in_poly1);

	std::cout << "poly1=";
	for (auto pt : _poly1->_pts) {
		std::cout << "(" << pt.x << " , " << pt.y << ") ; ";
	}
	std::cout << "\n";

	std::cout << "poly2=";
	for (auto pt : _poly2->_pts) {
		std::cout << "(" << pt.x << " , " << pt.y << ") ; ";
	}
	std::cout << "\n";

	std::cout << "inter=" << _inter << " ; axis=(" << _axis.x << " , " << _axis.y << ") ; overlap=" << _overlap;
	std::cout << " ; idx_pt=" << _idx_pt << " ; is_pt_in_poly1=" << _is_pt_in_poly1 << "\n";

	std::cout << "----------------------------------\n";

	update();
}


void TestGeom2D::draw() {
	DrawContext * context= _contexts["bbox"];

	glUseProgram(context->_prog);
	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	
	glUniformMatrix4fv(context->_locs_uniform["camera2clip_matrix"], 1, GL_FALSE, glm::value_ptr(_camera2clip));
	
	for (auto attr : context->_locs_attrib) {
		glEnableVertexAttribArray(attr.second);
	}

	glVertexAttribPointer(context->_locs_attrib["position_in"], 2, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)0);
	glVertexAttribPointer(context->_locs_attrib["color_in"], 4, GL_FLOAT, GL_FALSE, context->_n_attrs_per_pts* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_LINES, 0, context->_n_pts);

	for (auto attr : context->_locs_attrib) {
		glDisableVertexAttribArray(attr.second);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void TestGeom2D::update() {
	const glm::vec4 POLY1_COLOR(1.0, 0.0, 0.0, 1.0);
	const glm::vec4 POLY2_COLOR(0.0, 1.0, 0.0, 1.0);
	const glm::vec4 ARROW_COLOR(0.0, 1.0, 1.0, 1.0);
	const float ARROW_ANGLE= M_PI* 0.1;
	const float ARROW_TIP_SIZE= 0.2;

	DrawContext * context= _contexts["bbox"];
	context->_n_pts= _poly1->_pts.size()* 2+ _poly2->_pts.size()* 2+ 6;
	context->_n_attrs_per_pts= 6;

	float data[context->_n_pts* context->_n_attrs_per_pts];

	float * ptr= data;
	ptr= draw_polygon(ptr, _poly1->_pts, POLY1_COLOR);
	ptr= draw_polygon(ptr, _poly2->_pts, POLY2_COLOR);
	if (_inter) {
		pt_type start, end;
		if (_is_pt_in_poly1) {
			start= _poly1->_pts[_idx_pt];
		}
		else {
			start= _poly2->_pts[_idx_pt];
		}
		end= start+ _overlap* _axis;
		ptr= draw_arrow(ptr, start, end, ARROW_TIP_SIZE, ARROW_ANGLE, ARROW_COLOR);
	}
	else {
		ptr= draw_arrow(ptr, pt_type(1000.0, 1000.0), pt_type(1000.0, 1000.0), ARROW_TIP_SIZE, ARROW_ANGLE, ARROW_COLOR);
	}

	glBindBuffer(GL_ARRAY_BUFFER, context->_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* context->_n_pts* context->_n_attrs_per_pts, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
