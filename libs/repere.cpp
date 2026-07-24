#include "repere.h"


Repere::Repere() {
	
}


Repere::Repere(GLDrawManager * gl_draw_manager, ViewSystem * view_system, number axis_size, number ground_size, number ground_step, number box_size) :
	_gl_draw_manager(gl_draw_manager), _view_system(view_system),
	_axis_size(axis_size), _ground_size(ground_size), _ground_step(ground_step), _box_size(box_size)
{
	update_axis();
	update_ground();
	update_box();
}


Repere::~Repere() {

}


void Repere::update_axis() {
	float data_repere[] = {
		0.0, 0.0, 0.0               , 1.0, 0.0, 0.0, 1.0,
		float(_axis_size), 0.0, 0.0, 1.0, 0.0, 0.0, 1.0,
		0.0, 0.0, 0.0               , 0.0, 1.0, 0.0, 1.0,
		0.0, float(_axis_size), 0.0, 0.0, 1.0, 0.0, 1.0,
		0.0, 0.0, 0.0               , 0.0, 0.0, 1.0, 1.0,
		0.0, 0.0, float(_axis_size), 0.0, 0.0, 1.0, 1.0
	};
	
	_gl_draw_manager->set_data("repere", 6, data_repere);
}


void Repere::update_ground() {
	uint n_lines = 2 * uint(_ground_size / _ground_step) + 1;
	uint n_pts = n_lines * 2 * 2;

	float * data_ground = new float[n_pts * 7];
	float * ptr = data_ground;
	
	for (uint i=0; i<n_lines; ++i) {
		ptr[0] = float(-1.0 * _ground_size + number(i) * _ground_step);
		ptr[1] = float(-1.0 * _ground_size);
		ptr[2] = float(Z_GROUND_EPS);
		ptr[3] = GROUND_COLOR.r;
		ptr[4] = GROUND_COLOR.g;
		ptr[5] = GROUND_COLOR.b;
		ptr[6] = GROUND_COLOR.a;

		ptr[7] = float(-1.0 * _ground_size + number(i) * _ground_step);
		ptr[8] = float(_ground_size);
		ptr[9] = float(Z_GROUND_EPS);
		ptr[10] = GROUND_COLOR.r;
		ptr[11] = GROUND_COLOR.g;
		ptr[12] = GROUND_COLOR.b;
		ptr[13] = GROUND_COLOR.a;

		ptr += 14;
	}

	for (uint i=0; i<n_lines; ++i) {
		ptr[0] = float(-1.0 * _ground_size);
		ptr[1] = float(-1.0 * _ground_size + number(i) * _ground_step);
		ptr[2] = float(Z_GROUND_EPS);
		ptr[3] = GROUND_COLOR.r;
		ptr[4] = GROUND_COLOR.g;
		ptr[5] = GROUND_COLOR.b;
		ptr[6] = GROUND_COLOR.a;

		ptr[7] = float(_ground_size);
		ptr[8] = float(-1.0 * _ground_size + number(i) * _ground_step);
		ptr[9] = float(Z_GROUND_EPS);
		ptr[10] = GROUND_COLOR.r;
		ptr[11] = GROUND_COLOR.g;
		ptr[12] = GROUND_COLOR.b;
		ptr[13] = GROUND_COLOR.a;

		ptr += 14;
	}
	
	_gl_draw_manager->set_data("ground", n_pts, data_ground);
	delete[] data_ground;
}


void Repere::update_box() {
	float data_box[] = {
		-1.0f * float(_box_size), -1.0f * float(_box_size), -1.0f * float(_box_size), BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(_box_size), -1.0f * float(_box_size), -1.0f * float(_box_size)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(_box_size), -1.0f * float(_box_size), -1.0f * float(_box_size)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(_box_size), float(_box_size), -1.0f * float(_box_size)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(_box_size), float(_box_size), -1.0f * float(_box_size)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(_box_size), float(_box_size), -1.0f * float(_box_size)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(_box_size), float(_box_size), -1.0f * float(_box_size)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(_box_size), -1.0f * float(_box_size), -1.0f * float(_box_size), BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(_box_size), -1.0f * float(_box_size), float(_box_size)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(_box_size), -1.0f * float(_box_size), float(_box_size)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(_box_size), -1.0f * float(_box_size), float(_box_size)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(_box_size), float(_box_size), float(_box_size)                        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(_box_size), float(_box_size), float(_box_size)                        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(_box_size), float(_box_size), float(_box_size)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(_box_size), float(_box_size), float(_box_size)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(_box_size), -1.0f * float(_box_size), float(_box_size)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(_box_size), -1.0f * float(_box_size), -1.0f * float(_box_size), BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(_box_size), -1.0f * float(_box_size), float(_box_size)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(_box_size), -1.0f * float(_box_size), -1.0f * float(_box_size)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a, 
		float(_box_size), -1.0f * float(_box_size), float(_box_size)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(_box_size), float(_box_size), -1.0f * float(_box_size)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(_box_size), float(_box_size), float(_box_size)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(_box_size), float(_box_size), -1.0f * float(_box_size)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a, 
		float(_box_size), float(_box_size), float(_box_size)                        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a
	};

	_gl_draw_manager->set_data("box", 24, data_box);
}


void Repere::draw() {
	for (auto & context_name : std::vector<std::string>{"repere", "ground", "box"}) {
		GLDrawContext * context = _gl_draw_manager->get_context(context_name);
		context->activate();
		context->set_uniform("world2clip", glm::value_ptr(glm::mat4(_view_system->_world2clip)));
		context->draw();
		context->deactivate();
	}
}


bool Repere::key_down(InputState * input_state, SDL_Keycode key, time_point t) {
	if (key == SDLK_r) {
		_gl_draw_manager->switch_active("repere");
		return true;
	}
	else if (key == SDLK_g) {
		_gl_draw_manager->switch_active("ground");
		return true;
	}
	else if (key == SDLK_b) {
		_gl_draw_manager->switch_active("box");
		return true;
	}

	return false;
}
