#ifndef PATH_FINDER_TEST_H
#define PATH_FINDER_TEST_H

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <map>

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>

#include "gl_utils.h"
#include "input_state.h"
#include "font.h"
#include "gl_draw.h"
#include "graph.h"
#include "typedefs.h"
#include "thread.h"
#include "bbox_2d.h"
#include "repere.h"

#include "path_find.h"
#include "const.h"


struct PathFinderTest {
	PathFinderTest();
	PathFinderTest(GLDrawManager * gl_draw_manager, ViewSystem * view_system, time_point t);
	~PathFinderTest();
	
	void anim(time_point t);
	
	void draw_font();
	void draw_grid_centers();
	void draw_grid_edges();
	void draw_gmos();
	void draw_gmos_path();
	void draw_select();
	void draw();
	
	void update_font();
	void update_select();
	void update_grid_centers();
	void update_grid_edges();
	void update_gmos();
	void update_gmos_path();
	void update();
	
	bool mouse_button_down(InputState * input_state, time_point t);
	bool mouse_button_up(InputState * input_state, time_point t);
	bool mouse_motion(InputState * input_state, time_point t);
	bool key_down(InputState * input_state, SDL_Keycode key, time_point t);
	bool key_up(InputState * input_state, SDL_Keycode key, time_point t);


	PathFinder * _pf;
	GLDrawManager * _gl_draw_manager;
	Font * _font;
	ViewSystem * _view_system;
};


#endif
