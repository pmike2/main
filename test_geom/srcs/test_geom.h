#ifndef TEST_GEOM
#define TEST_GEOM

#include <string>
#include <vector>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "geom.h"
#include "gl_utils.h"
#include "input_state.h"
#include "font.h"
#include "typedefs.h"


const float Z_NEAR= 0.0f;
const float Z_FAR= 1000.0f;


class TestGeom {
public:
	TestGeom();
	TestGeom(std::map<std::string, GLuint> progs);
	~TestGeom();
	void test_simple();
	void randomize();
	void draw_points(const glm::mat4 & world2clip);
	void draw_hull(const glm::mat4 & world2clip, const glm::vec3 & camera_position);
	void draw(const glm::mat4 & world2clip, const glm::vec3 & camera_position);
	void update_points();
	void update_hull();
	
	
	std::map<std::string, GLDrawContext *> _contexts; // contextes de dessin
	bool _draw_points, _draw_hull;

	ConvexHull * _hull;
};

#endif
