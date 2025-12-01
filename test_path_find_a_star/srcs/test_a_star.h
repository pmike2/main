#ifndef TEST_GEOM
#define TEST_GEOM

#include <string>
#include <vector>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "path_find.h"
#include "gl_utils.h"
#include "input_state.h"
#include "font.h"
#include "typedefs.h"
#include "repere.h"


enum EDIT_MODE {ADDING_OBSTACLE, FREE};
enum UNIT_MODE {WAITING, MOVING};

const uint NEW_PT_IN_POLYGON_MS = 300;


struct UnitType {
	UnitType();
	UnitType(pt_type size, number velocity);
	~UnitType();
	
	
	pt_type _size;
	number _velocity;
};


struct Unit {
	Unit();
	Unit(UnitType * type, pt_type pos);
	~Unit();
	void clear_path();
	
	
	UnitType * _type;
	bool _selected;
	AABB_2D * _aabb;
	std::vector<pt_type> _path;
	uint _idx_path;
	UNIT_MODE _mode;
};



class TestAStar {
public:
	TestAStar();
	TestAStar(std::map<std::string, GLuint> progs);
	~TestAStar();
	void draw(const glm::mat4 & world2clip);
	void anim(time_point t, ViewSystem * view_system);
	void update();
	bool mouse_button_down(InputState * input_state, ViewSystem * view_system);
	bool mouse_button_up(InputState * input_state, ViewSystem * view_system);
	bool mouse_motion(InputState * input_state, ViewSystem * view_system, time_point t);
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);

	
	std::map<std::string, DrawContext *> _contexts; // contextes de dessin
	PathFinder * _path_finder;
	std::map<std::string, UnitType *> _unit_types;
	std::vector<Unit *> _units;
	EDIT_MODE _mode;
	std::vector<pt_type> _polygon_pts;
	time_point _last_added_pt_t;
};

#endif
