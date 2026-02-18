

#include "repere.h"

using namespace std;



// ------------------------------------------------------------------------------------------------------------------------
Repere::Repere() {
	
}


Repere::Repere(GLDrawManager * gl_draw_manager) : _gl_draw_manager(gl_draw_manager) {
	
	float data_repere[]= {
		0.0, 0.0, 0.0               , 1.0, 0.0, 0.0, 1.0,
		float(REPERE_AXIS), 0.0, 0.0, 1.0, 0.0, 0.0, 1.0,
		0.0, 0.0, 0.0               , 0.0, 1.0, 0.0, 1.0,
		0.0, float(REPERE_AXIS), 0.0, 0.0, 1.0, 0.0, 1.0,
		0.0, 0.0, 0.0               , 0.0, 0.0, 1.0, 1.0,
		0.0, 0.0, float(REPERE_AXIS), 0.0, 0.0, 1.0, 1.0
	};
	
	float data_ground[]= {
		-1.0f* float(REPERE_GROUND), -1.0f* float(REPERE_GROUND), -1.0f* float(GROUND_EPS), GROUND_COLOR.r, GROUND_COLOR.g, GROUND_COLOR.b, GROUND_COLOR.a,
		float(REPERE_GROUND), -1.0f* float(REPERE_GROUND), -1.0f* float(GROUND_EPS)       , GROUND_COLOR.r, GROUND_COLOR.g, GROUND_COLOR.b, GROUND_COLOR.a,
		float(REPERE_GROUND), float(REPERE_GROUND), -1.0f* float(GROUND_EPS)              , GROUND_COLOR.r, GROUND_COLOR.g, GROUND_COLOR.b, GROUND_COLOR.a,
		-1.0f* float(REPERE_GROUND), -1.0f* float(REPERE_GROUND), -1.0f* float(GROUND_EPS), GROUND_COLOR.r, GROUND_COLOR.g, GROUND_COLOR.b, GROUND_COLOR.a,
		float(REPERE_GROUND), float(REPERE_GROUND), -1.0f* float(GROUND_EPS)              , GROUND_COLOR.r, GROUND_COLOR.g, GROUND_COLOR.b, GROUND_COLOR.a,
		-1.0f* float(REPERE_GROUND), float(REPERE_GROUND), -1.0f* float(GROUND_EPS)       , GROUND_COLOR.r, GROUND_COLOR.g, GROUND_COLOR.b, GROUND_COLOR.a,
	};

	float data_box[]= {
		-1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(REPERE_BOX), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(REPERE_BOX), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(REPERE_BOX), float(REPERE_BOX), -1.0f * float(REPERE_BOX)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(REPERE_BOX), float(REPERE_BOX), -1.0f * float(REPERE_BOX)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(REPERE_BOX), float(REPERE_BOX), -1.0f * float(REPERE_BOX)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(REPERE_BOX), float(REPERE_BOX), -1.0f * float(REPERE_BOX)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(REPERE_BOX)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(REPERE_BOX)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(REPERE_BOX)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(REPERE_BOX), float(REPERE_BOX), float(REPERE_BOX)                        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(REPERE_BOX), float(REPERE_BOX), float(REPERE_BOX)                        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(REPERE_BOX), float(REPERE_BOX), float(REPERE_BOX)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(REPERE_BOX), float(REPERE_BOX), float(REPERE_BOX)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(REPERE_BOX)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(REPERE_BOX)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(REPERE_BOX), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a, 
		float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(REPERE_BOX)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(REPERE_BOX), float(REPERE_BOX), -1.0f * float(REPERE_BOX)        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		-1.0f * float(REPERE_BOX), float(REPERE_BOX), float(REPERE_BOX)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a,
		float(REPERE_BOX), float(REPERE_BOX), -1.0f * float(REPERE_BOX)                , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a, 
		float(REPERE_BOX), float(REPERE_BOX), float(REPERE_BOX)                        , BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a
	};

	_gl_draw_manager->set_data("repere", 6, data_repere);
	_gl_draw_manager->set_data("ground", 6, data_ground);
	_gl_draw_manager->set_data("box", 24, data_box);
}


Repere::~Repere() {
}


void Repere::draw(const mat_4d & world2clip) {
	for (auto & context_name : std::vector<std::string>{"repere", "ground", "box"}) {
		GLDrawContext * context = _gl_draw_manager->get_context(context_name);
		context->activate();
		context->set_uniform("world2clip", glm::value_ptr(glm::mat4(world2clip)));
		context->draw();
		context->deactivate();
	}
}


// ------------------------------------------------------------------------------------------------------------------------
RectSelect::RectSelect() : 
	_is_active(false), _gl_origin(pt_2d(0.0)), _gl_moving(pt_2d(0.0))
{
	for (uint i=0; i<4; ++i) {
		_norms[i]= pt_3d(0.0);
	}
}


RectSelect::~RectSelect() {

}


void RectSelect::set_origin(pt_2d gl_v) {
	_gl_origin= gl_v;
	_gl_moving= _gl_origin;
}


void RectSelect::set_moving(pt_2d gl_v) {
	_gl_moving= gl_v;
}


void RectSelect::set_active(bool is_active) {
	_is_active= is_active;
}


// ------------------------------------------------------------------------------------------------------------------------
ViewSystem::ViewSystem() {

}


ViewSystem::ViewSystem(GLDrawManager * gl_draw_manager, ScreenGL * screengl) :
	_gl_draw_manager(gl_draw_manager),
	_screengl(screengl),
	_frustum_halfsize(FRUSTUM_HALFSIZE), _frustum_near(FRUSTUM_NEAR), _frustum_far(FRUSTUM_FAR),
	_target(pt_3d(0.0, 0.0, 0.0)), _phi(0.0), _theta(0.0), _rho(1.0),
	_target_constrained(false), _phi_constrained(false), _theta_constrained(false), _rho_constrained(false),
	_target_mouse_sensitivity(DEFAULT_TARGET_MOUSE_SENSIVITY), _phi_mouse_sensitivity(DEFAULT_PHI_MOUSE_SENSIVITY), _theta_mouse_sensitivity(DEFAULT_THETA_MOUSE_SENSIVITY), _rho_mouse_sensitivity(DEFAULT_RHO_MOUSE_SENSIVITY),
	_target_key_sensitivity(DEFAULT_TARGET_KEY_SENSIVITY), _phi_key_sensitivity(DEFAULT_PHI_KEY_SENSIVITY), _theta_key_sensitivity(DEFAULT_THETA_KEY_SENSIVITY), _rho_key_sensitivity(DEFAULT_RHO_KEY_SENSIVITY),
	_type(FREE_VIEW),
	_new_single_selection(false), _new_rect_selection(false),
	_target_anim(false), _phi_anim(false), _theta_anim(false), _rho_anim(false)
{
	_camera2clip = glm::frustum(-_frustum_halfsize * _screengl->_screen_width / _screengl->_screen_height, _frustum_halfsize * _screengl->_screen_width / _screengl->_screen_height, -_frustum_halfsize, _frustum_halfsize, _frustum_near, _frustum_far);

	update();

	_repere= new Repere(_gl_draw_manager);
	_rect_select= new RectSelect();
}


ViewSystem::~ViewSystem() {
	delete _repere;
	delete _rect_select;
	delete _screengl;
}


bool ViewSystem::mouse_button_down(InputState * input_state, time_point t) {
	if (input_state->_left_mouse) {
		_rect_select->set_active(true);
		_rect_select->set_origin(screen2gl(input_state->_x, input_state->_y));
		return true;
	}
	return false;
}


bool ViewSystem::mouse_button_up(InputState * input_state, time_point t) {
	if (_rect_select->_is_active) {
		_rect_select->set_active(false);
		if (glm::distance2(_rect_select->_gl_origin, _rect_select->_gl_moving)< 0.0001) {
			_new_single_selection= true;
		}
		else {
			update_selection_norms();
			_new_rect_selection= true;
		}
		return true;
	}
	return false;
}


bool ViewSystem::mouse_motion(InputState * input_state, time_point t) {
	if (input_state->_xrel == 0 && input_state->_yrel == 0) {
		return false;
	}

	if (_type== FREE_VIEW) {
		
		// sélection
		if (input_state->_left_mouse) {
			if (_rect_select->_is_active) {
				_rect_select->set_moving(screen2gl(input_state->_x, input_state->_y));
				return true;
			}
		}

		// comme dans Blender !

		// zoom
		else if (input_state->_middle_mouse && (input_state->_keys[SDLK_LCTRL] || input_state->_keys[SDLK_RCTRL] || input_state->_keys[SDLK_LGUI] || input_state->_keys[SDLK_RGUI])) {
			move_rho(input_state->_yrel);
			return true;
		}

		// translation
		else if (input_state->_middle_mouse && (input_state->_keys[SDLK_LSHIFT] || input_state->_keys[SDLK_RSHIFT])) {
			move_target(input_state->_xrel, input_state->_yrel);
			return true;
		}

		// rotation
		else if (input_state->_middle_mouse) {
			move_theta(input_state->_yrel);
			move_phi(input_state->_xrel);
			return true;
		}
	}

	// TODO : à revoir

	/*else if (_type== THIRD_PERSON_FREE) {
		if (input_state->_left_mouse) {
		}
		// zoom
		else if ((input_state->_middle_mouse) || (input_state->_keys[SDLK_LCTRL])) {
			move_rho(-(number)(input_state->_yrel)* MIDDLE_MOUSE_SENSIVITY);
			return true;
		}
		// rotations
		else if (input_state->_right_mouse) {
			move_theta((number)(-input_state->_yrel)* RIGHT_MOUSE_SENSIVITY);
			move_phi((number)(-input_state->_xrel)* RIGHT_MOUSE_SENSIVITY);
			return true;
		}
	}

	else if (_type== THIRD_PERSON_BEHIND) {
		if (input_state->_left_mouse) {
		}
		// zoom
		else if ((input_state->_middle_mouse) || (input_state->_keys[SDLK_LCTRL])) {
			move_rho(-(number)(input_state->_yrel)* MIDDLE_MOUSE_SENSIVITY);
			return true;
		}
		else if (input_state->_right_mouse) {
			
		}
		else {
			// rotation haut - bas ; la rotation gauche - droite est assurée par world.mouse_motion et view_system.anim
			move_theta((number)(-input_state->_yrel)* RIGHT_MOUSE_SENSIVITY);
			return true;
		}
	}*/

	return false;
}


bool ViewSystem::key_down(InputState * input_state, SDL_Keycode key, time_point t) {
	
	// TODO : à revoir
	// changement de type de vue
	/*if (key== SDLK_v) {
		if (_type== FREE_VIEW) {
			_type= THIRD_PERSON_FREE;
		}
		else if (_type== THIRD_PERSON_FREE) {
			_type= THIRD_PERSON_BEHIND;
		}
		else if (_type== THIRD_PERSON_BEHIND) {
			_type= FREE_VIEW;
		}
		return true;
	}*/

	if (_type == FREE_VIEW) {
		// affichage repere
		if (key == SDLK_r) {
			_gl_draw_manager->switch_active("repere");
			_gl_draw_manager->switch_active("box");
			_gl_draw_manager->switch_active("ground");
			return true;
		}

		// déplacement
		else if (input_state->_keys[SDLK_LSHIFT] || input_state->_keys[SDLK_RSHIFT]) {
			if (key == SDLK_LEFT) {
				_target_goal = pt_2d(_target) + _target_key_sensitivity * get_target_direction(1, 0);
			}
			else if (key == SDLK_RIGHT) {
				_target_goal = pt_2d(_target) + _target_key_sensitivity * get_target_direction(-1, 0);
			}
			else if (key == SDLK_UP) {
				_target_goal = pt_2d(_target) + _target_key_sensitivity * get_target_direction(0, 1);
			}
			else if (key == SDLK_DOWN) {
				_target_goal = pt_2d(_target) + _target_key_sensitivity * get_target_direction(0, -1);
			}
			if (key == SDLK_LEFT || key == SDLK_RIGHT || key == SDLK_UP || key == SDLK_DOWN) {
				_last_key_t = t;
				_target_start = pt_2d(_target);
				_target_length = glm::length(_target_goal - _target_start);
				_target_direction = (_target_goal - _target_start) / _target_length;
				_target_anim = true;
				return true;
			}
		}

		// zoom
		else if (input_state->_keys[SDLK_LCTRL] || input_state->_keys[SDLK_LGUI] || input_state->_keys[SDLK_RCTRL] || input_state->_keys[SDLK_RGUI]) {
			if (key == SDLK_UP) {
				_rho_goal = _rho + _rho_key_sensitivity * get_delta_rho(1);
			}
			else if (key == SDLK_DOWN) {
				_rho_goal = _rho + _rho_key_sensitivity * get_delta_rho(-1);
			}
			if (key == SDLK_UP || key == SDLK_DOWN) {
				_last_key_t = t;
				_rho_start = _rho;
				_rho_anim = true;
				return true;
			}
		}

		// rotation
		else {
			if (key == SDLK_LEFT) {
				_phi_goal = _phi + _phi_key_sensitivity * get_delta_phi(1);
			}
			else if (key == SDLK_RIGHT) {
				_phi_goal = _phi + _phi_key_sensitivity * get_delta_phi(-1);
			}
			if (key == SDLK_LEFT || key == SDLK_RIGHT) {
				_last_key_t = t;
				_phi_start = _phi;
				_phi_anim = true;
				return true;
			}
			
			if (key == SDLK_UP) {
				_theta_goal = _theta + _theta_key_sensitivity * get_delta_theta(1);
				if (_theta_goal > M_PI) {
					_theta_goal = M_PI;
				}
			}
			else if (key == SDLK_DOWN) {
				_theta_goal = _theta + _theta_key_sensitivity * get_delta_theta(-1);
				if (_theta_goal < 0.0) {
					_theta_goal = 0.0;
				}
			}
			if (key == SDLK_UP || key == SDLK_DOWN) {
				_last_key_t = t;
				_theta_start = _theta;
				_theta_anim = true;
				return true;
			}
		}
	}

	/*else if (_type== THIRD_PERSON_FREE) {

	}
	else if (_type== THIRD_PERSON_BEHIND) {

	}*/
	return false;
}


bool ViewSystem::key_up(InputState * input_state, SDL_Keycode key, time_point t) {
	return false;
}


// a appeler des qu'un param est modifie
void ViewSystem::update() {
	// contraintes
	if (_target_constrained) {
		// on ne contraint pas _target.z
		for (uint i=0; i<2; ++i) {
			if (_target[i] < _target_min[i]) {
				_target[i] = _target_min[i];
			}
			if (_target[i] > _target_max[i]) {
				_target[i] = _target_max[i];
			}
		}
	}

	if (_phi_constrained) {
		if (_phi < _phi_min) {
			_phi = _phi_min;
		}
		if (_phi > _phi_max) {
			_phi = _phi_max;
		}
	}

	if (_theta_constrained) {
		if (_theta < _theta_min) {
			_theta = _theta_min;
		}
		if (_theta > _theta_max) {
			_theta = _theta_max;
		}
	}

	if (_rho_constrained) {
		if (_rho < _rho_min) {
			_rho = _rho_min;
		}
		if (_rho > _rho_max) {
			_rho = _rho_max;
		}
	}

	// phi doit être entre 0 et 2pi
	while (_phi< 0.0) {
		_phi+= M_PI* 2.0;
	}
	while (_phi> M_PI* 2.0) {
		_phi-= M_PI* 2.0;
	}

	// theta doit être entre 0 et pi
	if (_theta< 0.0) {
		_theta= 0.0;
	}
	if (_theta> M_PI) {
		_theta= M_PI;
	}

	// _rho == 0.0 crée des problèmes avec du nan dans les attributs
	const number MIN_RHO = 0.01;
	if (_rho < MIN_RHO) {
		_rho = MIN_RHO;
	}

	// https://en.wikipedia.org/wiki/Spherical_coordinate_system
	_dir= -pt_3d(cos(_phi)* sin(_theta), sin(_phi)* sin(_theta), cos(_theta));
	_eye= _target- _rho* _dir;
	_up= -pt_3d(cos(_phi)* cos(_theta), sin(_phi)* cos(_theta), -sin(_theta));
	_right= glm::cross(_dir, _up);
	_world2camera= glm::lookAt(_eye, _target, _up);

	/*std::cout << "-------------------\n";
	std::cout << "target=" << glm::to_string(_target) << "\n";
	std::cout << "rho=" << _rho << "\n";
	std::cout << "dir=" << glm::to_string(_dir) << "\n";
	std::cout << "eye=" << glm::to_string(_eye) << "\n";
	std::cout << "up=" << glm::to_string(_up) << "\n";
	std::cout << "world2camera=" << glm::to_string(_world2camera) << "\n";*/

	_world2clip= _camera2clip* _world2camera;

	_center_near= _eye+ _frustum_near* _dir;
	_center_far = _eye+ _frustum_far * _dir;
	_norm_near= _dir;
	_norm_far= -_dir;
	_norm_right= glm::cross(_up, glm::normalize(_center_near- _eye+ _right* _frustum_halfsize* number(_screengl->_screen_width)/ number(_screengl->_screen_height)));
	_norm_left = glm::cross(glm::normalize(_center_near- _eye- _right* _frustum_halfsize* number(_screengl->_screen_width)/ number(_screengl->_screen_height)), _up);
	_norm_top= glm::cross(glm::normalize(_center_near- _eye+ _up* _frustum_halfsize), _right);
	_norm_bottom= glm::cross(_right, glm::normalize(_center_near- _eye- _up* _frustum_halfsize));
}


ViewSystemState * ViewSystem::get_state() {
	ViewSystemState * state = new ViewSystemState();
	state->_target = _target;
	state->_phi = _phi;
	state->_theta = _theta;
	state->_rho = _rho;
	return state;
}


void ViewSystem::set_state(ViewSystemState * state) {
	set(state->_target, state->_phi, state->_theta, state->_rho);
}


void ViewSystem::set(const pt_3d & target, number phi, number theta, number rho) {
	_target= target;
	_phi= phi;
	_theta= theta;
	_rho= rho;

	update();
}


void ViewSystem::set_target(const pt_3d & target) {
	_target = target;
	update();
}


void ViewSystem::set_target(const pt_2d & target) {
	_target = pt_3d(target.x, target.y, 0.0);
	update();
}


void ViewSystem::set_phi(number phi) {
	_phi = phi;
	update();
}


void ViewSystem::set_theta(number theta) {
	_theta = theta;
	update();
}


void ViewSystem::set_rho(number rho) {
	_rho = rho;
	update();
}


void ViewSystem::set_2d(number rho) {
	_target= pt_3d(0.0);
	_phi= -1.0 * M_PI_2;
	_theta= 0.0;
	_rho= rho;

	update();
}


void ViewSystem::constraint_target(const pt_2d & target_min, const pt_2d & target_max) {
	_target_constrained = true;
	_target_min = target_min;
	_target_max = target_max;
}


void ViewSystem::constraint_target(const pt_2d & target) {
	_target_constrained = true;
	_target_min = target;
	_target_max = target;
}


void ViewSystem::unconstraint_target() {
	_target_constrained = false;
}


void ViewSystem::constraint_phi(number phi_min, number phi_max) {
	_phi_constrained = true;
	_phi_min = phi_min;
	_phi_max = phi_max;
}


void ViewSystem::constraint_phi(number phi) {
	_phi_constrained = true;
	_phi_min = phi;
	_phi_max = phi;
}


void ViewSystem::unconstraint_phi() {
	_phi_constrained = false;
}


void ViewSystem::constraint_theta(number theta_min, number theta_max) {
	_theta_constrained = true;
	_theta_min = theta_min;
	_theta_max = theta_max;
}


void ViewSystem::constraint_theta(number theta) {
	_theta_constrained = true;
	_theta_min = theta;
	_theta_max = theta;
}


void ViewSystem::unconstraint_theta() {
	_theta_constrained = false;
}


void ViewSystem::constraint_rho(number rho_min, number rho_max) {
	_rho_constrained = true;
	_rho_min = rho_min;
	_rho_max = rho_max;
}


void ViewSystem::constraint_rho(number rho) {
	_rho_constrained = true;
	_rho_min = rho;
	_rho_max = rho;
}


void ViewSystem::unconstraint_rho() {
	_rho_constrained = false;
}


void ViewSystem::unconstraint_all() {
	_target_constrained = _phi_constrained = _theta_constrained = _rho_constrained = false;
}


ViewSystemConstraints * ViewSystem::get_constraints() {
	ViewSystemConstraints * result = new ViewSystemConstraints();
	result->_target_constrained = _target_constrained;
	result->_target_min = _target_min;
	result->_target_max = _target_max;
	result->_phi_constrained = _phi_constrained;
	result->_phi_min = _phi_min;
	result->_phi_max = _phi_max;
	result->_theta_constrained = _theta_constrained;
	result->_theta_min = _theta_min;
	result->_theta_max = _theta_max;
	result->_rho_constrained = _rho_constrained;
	result->_rho_min = _rho_min;
	result->_rho_max = _rho_max;
	return result;
}


void ViewSystem::set_constraints(ViewSystemConstraints * constraints) {
	_target_constrained = constraints->_target_constrained;
	_target_min = constraints->_target_min;
	_target_max = constraints->_target_max;
	_phi_constrained = constraints->_phi_constrained;
	_phi_min = constraints->_phi_min;
	_phi_max = constraints->_phi_max;
	_theta_constrained = constraints->_theta_constrained;
	_theta_min = constraints->_theta_min;
	_theta_max = constraints->_theta_max;
	_rho_constrained = constraints->_rho_constrained;
	_rho_min = constraints->_rho_min;
	_rho_max = constraints->_rho_max;
}


// les move sont des modif delta, contrairement a set ou anim qui font des modifs absolues
/*void ViewSystem::move_target(const pt_3d & v) {
	_target.x+= v.x* sin(_phi)- v.y* cos(_phi);
	_target.y+= -v.x* cos(_phi)- v.y* sin(_phi);
	_target.z+= v.z;
	
	update();
}*/


pt_2d ViewSystem::get_target_direction(int screen_delta_x, int screen_delta_y) {
	pt_2d v= screen2world(_screengl->_screen_width* 0.5- screen_delta_x, _screengl->_screen_height* 0.5- screen_delta_y, 0.0);
	return v - pt_2d(_target);
}


void ViewSystem::move_target(int screen_delta_x, int screen_delta_y) {
	/*
	Avant j'utilisais move_target(pt_3d) avec LEFT_MOUSE_SENSIVITY mais du coup la translation ne dépendait pas de l'altitude
	et du coup à basse alti ça bougeait vite, haute alti lentement.
	Ici il y a une meilleure corrélation entre mouvement curseur et translation terrain
	A terme fournir en argument optionnel un Terrain afin de s'ajuster à un relief plus complexe que z = 0
	*/
	pt_2d target_dir = get_target_direction(screen_delta_x, screen_delta_y);
	_target.x += _target_mouse_sensitivity * target_dir.x;
	_target.y += _target_mouse_sensitivity * target_dir.y;
	update();
}


number ViewSystem::get_delta_phi(int screen_delta_x) {
	return atan(number(screen_delta_x) / _rho);
}


// pareil que move_target, on s'adapte au niveau de zoom
void ViewSystem::move_phi(int screen_delta_x) {
	number delta_phi = get_delta_phi(screen_delta_x);
	_phi -= delta_phi * _phi_mouse_sensitivity;
	update();
}


number ViewSystem::get_delta_theta(int screen_delta_y) {
	return atan(number(screen_delta_y) / _rho);
}


// pareil que move_target, on s'adapte au niveau de zoom
void ViewSystem::move_theta(int screen_delta_y) {
	number delta_theta = get_delta_theta(screen_delta_y);
	_theta -= delta_theta * _theta_mouse_sensitivity;
	update();
}


number ViewSystem::get_delta_rho(int screen_delta_y) {
	number sign = copysign(1.0, number(screen_delta_y));
	pt_2d v= screen2world(_screengl->_screen_width* 0.5, _screengl->_screen_height* 0.5- screen_delta_y, 0.0);
	return sign * glm::length(v - pt_2d(_target));
}


// pareil que move_target, on s'adapte au niveau de zoom
void ViewSystem::move_rho(int screen_delta_y) {
	number dist = get_delta_rho(screen_delta_y);
	_rho -= dist * _rho_mouse_sensitivity;
	update();
}


void ViewSystem::draw() {
	_repere->draw(_world2clip);
}


void ViewSystem::anim(time_point t) {
	if (_type== FREE_VIEW) {
		if (_target_anim || _phi_anim || _theta_anim || _rho_anim) {
			auto d = std::chrono::duration_cast<std::chrono::milliseconds>(t - _last_key_t).count();
			if (d > KEY_N_MS) {
				_target_anim = _phi_anim = _theta_anim = _rho_anim = false;
			}
		}

		if (_target_anim) {
			number dist = glm::length(_target_goal - pt_2d(_target));
			number factor = pow(dist / _target_length, KEY_TARGET_EXP);
			_target.x += factor * _target_direction.x;
			_target.y += factor * _target_direction.y;
			if (glm::dot(pt_2d(_target) - _target_start, pt_2d(_target) - _target_goal) > 0) {
				_target.x = _target_goal.x;
				_target.y = _target_goal.y;
				_target_anim = false;
			}
			update();
		}

		if (_phi_anim) {
			if (_phi_goal > _phi_start) {
				number factor = pow((_phi_goal - _phi) / (_phi_goal - _phi_start), KEY_PHI_EXP);
				_phi += factor * KEY_PHI_SPEED;
				if (_phi > _phi_goal) {
					_phi = _phi_goal;
					_phi_anim = false;
				}

				if (_phi > 2.0 * M_PI) {
					_phi -= 2.0 * M_PI;
					_phi_start -= 2.0 * M_PI;
					_phi_goal -= 2.0 * M_PI;
				}
			}
			else {
				number factor = pow((_phi - _phi_goal) / (_phi_start - _phi_goal), KEY_PHI_EXP);
				_phi -= factor * KEY_PHI_SPEED;
				if (_phi < _phi_goal) {
					_phi = _phi_goal;
					_phi_anim = false;
				}

				if (_phi < 0.0) {
					_phi += 2.0 * M_PI;
					_phi_start += 2.0 * M_PI;
					_phi_goal += 2.0 * M_PI;
				}
			}
			update();
		}

		if (_theta_anim) {
			if (_theta_goal > _theta_start) {
				number factor = pow((_theta_goal - _theta) / (_theta_goal - _theta_start), KEY_THETA_EXP);
				_theta += factor * KEY_THETA_SPEED;
				if (_theta > _theta_goal) {
					_theta = _theta_goal;
					_theta_anim = false;
				}
			}
			else {
				number factor = pow((_theta - _theta_goal) / (_theta_start - _theta_goal), KEY_THETA_EXP);
				_theta -= factor * KEY_THETA_SPEED;
				if (_theta < _theta_goal) {
					_theta = _theta_goal;
					_theta_anim = false;
				}
			}
			update();
		}

		if (_rho_anim) {
			if (_rho_goal > _rho_start) {
				number factor = pow((_rho_goal - _rho) / (_rho_goal - _rho_start), KEY_RHO_EXP);
				_rho += factor * KEY_RHO_SPEED;
				if (_rho > _rho_goal) {
					_rho = _rho_goal;
					_rho_anim = false;
				}
			}
			else {
				number factor = pow((_rho - _rho_goal) / (_rho_start - _rho_goal), KEY_RHO_EXP);
				_rho -= factor * KEY_RHO_SPEED;
				if (_rho < _rho_goal) {
					_rho = _rho_goal;
					_rho_anim = false;
				}
			}
			update();
		}
	}

	// TODO : à revoir
	/*else if (_type== THIRD_PERSON_FREE) {
		// ajuste les params de ViewSystem en fonction d'un but
		_target= target;
		update();
	}
	else if (_type== THIRD_PERSON_BEHIND) {
		// ajuste les params de ViewSystem en fonction d'un but et d'une rotation
		_target= target;
		_phi= glm::roll(rotation);
		update();
	}*/
}


// cf http://antongerdelan.net/opengl/raycasting.html
// renvoie le pt 2d qui intersecte le plan d'alti z
pt_2d ViewSystem::screen2world(pt_2d gl_coords, number z) {
	pt_4d ray_clip= pt_4d(gl_coords.x, gl_coords.y, -1.0, 1.0);
	
	pt_4d ray_eye= glm::inverse(_camera2clip)* ray_clip;
	ray_eye= pt_4d(ray_eye.x, ray_eye.y, -1.0, 0.0);
	
	pt_3d ray_world= pt_3d(glm::inverse(_world2camera)* ray_eye);
	ray_world= glm::normalize(ray_world);
	
	number lambda= (z- _eye.z)/ ray_world.z;
	
	pt_3d v= _eye+ lambda* ray_world;

	/*std::cout << "ray_clip = " << glm_to_string(ray_clip) << "\n";
	std::cout << "ray_eye = " << glm_to_string(ray_eye) << "\n";
	std::cout << "ray_world = " << glm_to_string(ray_world) << "\n";
	std::cout << "lambda = " << lambda << "\n";
	std::cout << "v = " << glm_to_string(v) << "\n";*/

	return pt_2d(v.x, v.y);
}


pt_2d ViewSystem::screen2world(uint x, uint y, number z) {
	return screen2world(screen2gl(x, y), z);
}


pt_3d ViewSystem::screen2world_depthbuffer(pt_2d gl_coords) {
	pt_4d ray_clip= pt_4d(gl_coords.x, gl_coords.y, -1.0, 1.0);
	pt_4d ray_eye= glm::inverse(_camera2clip)* ray_clip;
	ray_eye= pt_4d(ray_eye.x, ray_eye.y, -1.0, 0.0);
	pt_3d ray_world= pt_3d(glm::inverse(_world2camera)* ray_eye);
	ray_world= glm::normalize(ray_world);

	glm::uvec2 screen_coords= gl2screen(gl_coords);
	float buffer_depth;
	// attention au height- y
	glReadPixels(screen_coords.x, _screengl->_screen_height- screen_coords.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &buffer_depth);
	number world_depth= depthbuffer2world(number(buffer_depth));
	number lambda= world_depth / glm::dot(ray_world, _dir);
	
	pt_3d result = _eye + lambda * ray_world;

	return _eye + lambda * ray_world;
}


pt_3d ViewSystem::screen2world_depthbuffer(uint x, uint y) {
	return screen2world_depthbuffer(screen2gl(x, y));
}


pt_2d ViewSystem::screen2gl(uint x, uint y) {
	// les coords OpenGL sont [-1, 1] x [-1, 1]
	number x_gl= 2.0* (number)(x)/ (number)(_screengl->_screen_width)- 1.0;
	number y_gl= 1.0- 2.0* (number)(y)/ (number)(_screengl->_screen_height);
	return pt_2d(x_gl, y_gl);
}


glm::uvec2 ViewSystem::gl2screen(pt_2d gl_coords) {
	uint x= (uint)((gl_coords.x+ 1.0)* (number)(_screengl->_screen_width)* 0.5);
	uint y= (uint)((1.0- gl_coords.y)* (number)(_screengl->_screen_height)* 0.5);
	return pt_2d(x, y);
}


number ViewSystem::depthbuffer2world(number depth) {
	return _frustum_near* _frustum_far* 2.0/ (_frustum_far+ _frustum_near - (2.0* depth- 1.0)* (_frustum_far- _frustum_near));
}


// cf http://cgvr.cs.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
/*bool ViewSystem::contains_point(const pt_3d & pos) {
	pt_3d pos2= pt_3d(_world2camera* pt_4d(pos.x, pos.y, pos.z, 1.0));

	// z pointe vers l'extérieur il faut donc l'inverser
	number y= -pos2.z* _frustum_halfsize/ _frustum_near;
	number x= y* (number)(_screen_width)/ (number)(_screen_height);

	x+= CONTAINS_POINT_TOLERANCE;
	y+= CONTAINS_POINT_TOLERANCE;

	cout << glm::to_string(pos2) << " ; " << _frustum_near << " ; " << _frustum_far << " ; " << x << " ; " << y << endl;
	cout << (-pos2.z> _frustum_near) << " : " << (-pos2.z< _frustum_far) << " : " << (pos2.x> -x) << " : " << (pos2.x< x) << " : " << (pos2.y> -y) << " : " << (pos2.y< y) << endl;

	return ((-pos2.z> _frustum_near) && (-pos2.z< _frustum_far) && (pos2.x> -x) && (pos2.x< x) && (pos2.y> -y) && (pos2.y< y));
}
*/


// plusieurs versions de test d'intersection
bool ViewSystem::intersects_bbox(BBox * bbox, bool selection) {
	return intersects_pts(bbox->_pts, 8, selection);
}


bool ViewSystem::intersects_aabb(AABB * aabb, bool selection) {
	return intersects_pts(aabb->_pts, 8, selection);
}


bool ViewSystem::intersects_aabb(AABB * aabb, const mat_4d & model2world_matrix, bool selection) {
	std::vector<pt_3d> pts;
	for (uint i=0; i<8; ++i) {
		pts.push_back(pt_3d(model2world_matrix* pt_4d(pt_3d(aabb->_pts[i]), 1.0)));
	}

	return intersects_pts(pts, selection);
}


bool ViewSystem::intersects_pts(std::vector<pt_3d> pts, bool selection) {
	pt_3d * add = pts.data();
	return intersects_pts(add, pts.size(), selection);
}


// cf https://zeux.io/2009/01/31/view-frustum-culling-optimization-introduction
bool ViewSystem::intersects_pts(pt_3d * pts, uint n_pts, bool selection) {
	bool b= false;
	pt_3d norms[4];
	if (selection) {
		for (uint i=0; i<4; ++i) {
			norms[i]= _rect_select->_norms[i];
		}
	}
	else {
		// faut-il ajouter _norm_far et _norm_near ? sinon le cone est infini
		norms[0]= _norm_left;
		norms[1]= _norm_right;
		norms[2]= _norm_top;
		norms[3]= _norm_bottom;
	}

	for (uint i=0; i<4; ++i) {
		b= false;
		for (uint j=0; j<n_pts; ++j) {
			if (glm::dot(pts[j]- _eye, norms[i])> 0.0) {
				b= true;
				break;
			}
		}
		if (!b) {
			return false;
		}
	}
	return true;
}


void ViewSystem::update_selection_norms() {
	number xmin= min(_rect_select->_gl_origin.x, _rect_select->_gl_moving.x);
	number ymin= min(_rect_select->_gl_origin.y, _rect_select->_gl_moving.y);
	number xmax= max(_rect_select->_gl_origin.x, _rect_select->_gl_moving.x);
	number ymax= max(_rect_select->_gl_origin.y, _rect_select->_gl_moving.y);
	
	pt_2d v1= screen2world(pt_2d(xmin, ymin), 0.0);
	pt_2d v2= screen2world(pt_2d(xmax, ymin), 0.0);
	pt_2d v3= screen2world(pt_2d(xmax, ymax), 0.0);
	pt_2d v4= screen2world(pt_2d(xmin, ymax), 0.0);

	pt_3d dir1= pt_3d(v1, 0.0)- _eye;
	pt_3d dir2= pt_3d(v2, 0.0)- _eye;
	pt_3d dir3= pt_3d(v3, 0.0)- _eye;
	pt_3d dir4= pt_3d(v4, 0.0)- _eye;

	_rect_select->_norms[0]= glm::cross(dir2, dir1);
	_rect_select->_norms[1]= glm::cross(dir3, dir2);
	_rect_select->_norms[2]= glm::cross(dir4, dir3);
	_rect_select->_norms[3]= glm::cross(dir1, dir4);
}


bool ViewSystem::pt_2d_intersects_aabb(pt_2d pt, AABB * aabb, bool check_depth) {
	pt_2d click_world= screen2world(pt, 0.0);
	number t_hit;
	bool intersect= ray_intersects_aabb(_eye, pt_3d(click_world, 0.0)- _eye, aabb, t_hit);
	if (!intersect) {
		return false;
	}

	if (!check_depth) {
		return true;
	}

	glm::uvec2 screen_coords= gl2screen(pt);
	float buffer_depth;
	// attention au height- y
	glReadPixels(screen_coords.x, _screengl->_screen_height- screen_coords.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &buffer_depth);
	number world_depth= depthbuffer2world(number(buffer_depth));
	if (abs(abs(world_depth)- t_hit)< 40.0) {
		return true;
	}

	return false;
}


bool ViewSystem::single_selection_intersects_aabb(AABB * aabb, bool check_depth) {
	/*pt_2d click_world= screen2world(_rect_select->_gl_origin, 0.0);
	number t_hit;
	bool intersect= ray_intersects_aabb(_eye, pt_3d(click_world, 0.0)- _eye, aabb, t_hit);
	if (!intersect) {
		return false;
	}

	if (!check_depth) {
		return true;
	}

	glm::uvec2 screen_coords= gl2screen(_rect_select->_gl_origin);
	float buffer_depth;
	// attention au height- y
	glReadPixels(screen_coords.x, _screengl->_screen_height- screen_coords.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &buffer_depth);
	number world_depth= depthbuffer2world(number(buffer_depth));
	if (abs(abs(world_depth)- t_hit)< 40.0) {
		return true;
	}

	return false;*/
	return pt_2d_intersects_aabb(_rect_select->_gl_origin, aabb, check_depth);
}


bool ViewSystem::rect_selection_intersects_bbox(BBox * bbox, bool check_depth) {
	if (!intersects_bbox(bbox, true)) {
		return false;
	}

	if (!check_depth) {
		return true;
	}

	pt_3d center= 0.5* (bbox->_aabb->_vmin+ bbox->_aabb->_vmax);
	pt_4d v= _world2clip* pt_4d(center, 1.0);
	glm::uvec2 screen_coords= gl2screen(pt_2d(v.x/ v.w, v.y/ v.w));
	float buffer_depth;
	// attention au height- y
	glReadPixels(screen_coords.x, _screengl->_screen_height- screen_coords.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &buffer_depth);
	number world_depth= depthbuffer2world(number(buffer_depth));
	number t_hit= glm::distance(center, _eye);
	if (abs(abs(world_depth)- t_hit)< 40.0) {
		return true;
	}

	return false;
}
