#ifndef TEST_GEOM_2D
#define TEST_GEOM_2D

#include <string>
#include <vector>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "bbox_2d.h"
#include "geom_2d.h"
#include "gl_utils.h"
#include "input_state.h"
#include "font.h"
#include "typedefs.h"


const float Z_NEAR= 0.0f;
const float Z_FAR= 1000.0f;


class TestGeom2D {
public:
	TestGeom2D();
	TestGeom2D(std::map<std::string, GLuint> progs, ScreenGL * screengl);
	~TestGeom2D();
	void randomize();
	void draw();
	void update();
	
	
	pt_type _pt_min, _pt_max;
	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	GLuint * _buffers;
	glm::mat4 _camera2clip; // glm::ortho
	Font * _font; // font pour écriture textes

	Polygon2D * _poly1;
	Polygon2D * _poly2;
	bool _inter;
	pt_type _axis;
	number _overlap;
	unsigned int _idx_pt;
	bool _is_pt_in_poly1;
};

#endif

