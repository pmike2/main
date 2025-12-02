

#include "repere.h"

using namespace std;



// ------------------------------------------------------------------------------------------------------------------------
Repere::Repere() {
	
}


Repere::Repere(GLuint prog_draw) : _prog_draw(prog_draw), _is_repere(true), _is_ground(true), _is_box(true) {
	uint i;
	
	float data_repere[]= {
		0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
		float(REPERE_AXIS), 0.0, 0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
		0.0, float(REPERE_AXIS), 0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 0.0, 1.0,
		0.0, 0.0, float(REPERE_AXIS), 0.0, 0.0, 1.0
	};
	
	number EPS= 0.01;
	float data_ground[]= {
		-1.0f* float(REPERE_GROUND), -1.0f* float(REPERE_GROUND), -1.0f* float(EPS), float(GROUND_COLOR[0]), float(GROUND_COLOR[1]), float(GROUND_COLOR[2]),
		float(REPERE_GROUND), -1.0f* float(REPERE_GROUND), -1.0f* float(EPS), float(GROUND_COLOR[0]), float(GROUND_COLOR[1]), float(GROUND_COLOR[2]),
		float(REPERE_GROUND), float(REPERE_GROUND), -1.0f* float(EPS), float(GROUND_COLOR[0]), float(GROUND_COLOR[1]), float(GROUND_COLOR[2]),
		-1.0f* float(REPERE_GROUND), -1.0f* float(REPERE_GROUND), -1.0f* float(EPS), float(GROUND_COLOR[0]), float(GROUND_COLOR[1]), float(GROUND_COLOR[2]),
		float(REPERE_GROUND), float(REPERE_GROUND), -1.0f* float(EPS), float(GROUND_COLOR[0]), float(GROUND_COLOR[1]), float(GROUND_COLOR[2]),
		-1.0f* float(REPERE_GROUND), float(REPERE_GROUND), -1.0f* float(EPS), float(GROUND_COLOR[0]), float(GROUND_COLOR[1]), float(GROUND_COLOR[2])
	};

	float data_box[]= {
		-1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]), float(REPERE_BOX), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX) , float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]),
		float(REPERE_BOX), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]), float(REPERE_BOX), float(REPERE_BOX), -1.0f * float(REPERE_BOX) , float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]),
		float(REPERE_BOX), float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]), -1.0f * float(REPERE_BOX), float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]),
		-1.0f * float(REPERE_BOX), float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]),

		-1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]), float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(REPERE_BOX) , float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]),
		float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]), float(REPERE_BOX), float(REPERE_BOX), float(REPERE_BOX) , float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]),
		float(REPERE_BOX), float(REPERE_BOX), float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]), -1.0f * float(REPERE_BOX), float(REPERE_BOX), float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]),
		-1.0f * float(REPERE_BOX), float(REPERE_BOX), float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]),
		
		-1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(REPERE_BOX) , float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]),
		float(REPERE_BOX), -1.0f * float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]), float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(REPERE_BOX) , float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]),
		-1.0f * float(REPERE_BOX), float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]), -1.0f * float(REPERE_BOX), float(REPERE_BOX), float(REPERE_BOX) , float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]),
		float(REPERE_BOX), float(REPERE_BOX), -1.0f * float(REPERE_BOX), float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2]), float(REPERE_BOX), float(REPERE_BOX), float(REPERE_BOX) , float(BOX_COLOR[0]), float(BOX_COLOR[1]), float(BOX_COLOR[2])
	};

	glGenBuffers(3, _buffers);
	
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data_repere), data_repere, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, _buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data_ground), data_ground, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data_box), data_box, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_world2clip_loc= glGetUniformLocation(_prog_draw, "world2clip_matrix");
}


Repere::~Repere() {
	
}


void Repere::draw(const mat_4d & world2clip) {
	if (_is_repere) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_diffuse_color_loc);

		glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(glm::mat4(world2clip)));
		glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));

		glDrawArrays(GL_LINES, 0, 6);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_diffuse_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}
	
	if (_is_ground) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[1]);

		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_diffuse_color_loc);
		
		glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(glm::mat4(world2clip)));
		glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_diffuse_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);		
		glUseProgram(0);
	}

	if (_is_box) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[2]);

		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_diffuse_color_loc);
		
		glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(glm::mat4(world2clip)));
		glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
		glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));

		glDrawArrays(GL_LINES, 0, 24);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_diffuse_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);		
		glUseProgram(0);
	}
}


// ------------------------------------------------------------------------------------------------------------------------
RectSelect::RectSelect() {

}


RectSelect::RectSelect(GLuint prog_draw) : 
	_prog_draw(prog_draw), _is_active(false), _gl_origin(pt_type(0.0)), _gl_moving(pt_type(0.0)),	_color(pt_type_3d(1.0, 1.0, 0.0))
{
	glGenBuffers(1, &_buffer);
	
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_color_loc= glGetAttribLocation(_prog_draw, "color_in");

	for (uint i=0; i<4; ++i) {
		_norms[i]= pt_type_3d(0.0);
	}

	update_draw();
}


RectSelect::~RectSelect() {

}


void RectSelect::draw() {
	if (_is_active) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffer);

		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_color_loc);
		
		glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
		glVertexAttribPointer(_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

		glDrawArrays(GL_LINES, 0, 8);

		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_color_loc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);		
		glUseProgram(0);
	}
}


void RectSelect::set_origin(pt_type gl_v) {
	_gl_origin= gl_v;
	_gl_moving= _gl_origin;
	update_draw();
}


void RectSelect::set_moving(pt_type gl_v) {
	_gl_moving= gl_v;
	update_draw();
}


void RectSelect::set_active(bool is_active) {
	_is_active= is_active;
}


void RectSelect::update_draw() {
	float data_selection[]= {
		float(_gl_origin.x), float(_gl_origin.y), float(_color.x), float(_color.y), float(_color.z),
		float(_gl_moving.x), float(_gl_origin.y), float(_color.x), float(_color.y), float(_color.z),
		
		float(_gl_moving.x), float(_gl_origin.y), float(_color.x), float(_color.y), float(_color.z),
		float(_gl_moving.x), float(_gl_moving.y), float(_color.x), float(_color.y), float(_color.z),
		
		float(_gl_moving.x), float(_gl_moving.y), float(_color.x), float(_color.y), float(_color.z),
		float(_gl_origin.x), float(_gl_moving.y), float(_color.x), float(_color.y), float(_color.z),

		float(_gl_origin.x), float(_gl_moving.y), float(_color.x), float(_color.y), float(_color.z),
		float(_gl_origin.x), float(_gl_origin.y), float(_color.x), float(_color.y), float(_color.z),
	};

	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data_selection), data_selection, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// ------------------------------------------------------------------------------------------------------------------------
ViewSystem::ViewSystem() {

}


ViewSystem::ViewSystem(std::map<std::string, GLuint> progs, ScreenGL * screengl) :
	_screengl(screengl),
	_target(pt_type_3d(0.0, 0.0, 0.0)), _eye(pt_type_3d(0.0, 0.0, 0.0)), _up(pt_type_3d(0.0, 0.0, 0.0)), 
	_phi(0.0), _theta(0.0), _rho(1.0),
	_type(FREE_VIEW), _frustum_halfsize(FRUSTUM_HALFSIZE), _frustum_near(FRUSTUM_NEAR), _frustum_far(FRUSTUM_FAR),
	_new_single_selection(false), _new_rect_selection(false), _new_destination(false), _free_view_x(0.0), _free_view_y(0.0)
{
	_camera2clip= glm::frustum(-_frustum_halfsize* _screengl->_screen_width/ _screengl->_screen_height, _frustum_halfsize* _screengl->_screen_width/ _screengl->_screen_height, -_frustum_halfsize, _frustum_halfsize, _frustum_near, _frustum_far);

	update();

	_repere= new Repere(progs["repere"]);
	_rect_select= new RectSelect(progs["select"]);

	_font= new Font(progs["font"], "../../fonts/Silom.ttf", 48, screengl);
	//_font->_z= 100.0;
}


ViewSystem::~ViewSystem() {
	delete _repere;
	delete _rect_select;
	delete _font;
	delete _screengl;
}


bool ViewSystem::mouse_button_down(InputState * input_state) {
	_free_view_x= 0.0;
	_free_view_y= 0.0;

	if (input_state->_left_mouse) {
		if (input_state->_keys[SDLK_m]) {
			_new_destination= true;
			_destination= pt_type_3d(screen2world(input_state->_x, input_state->_y, 0.0), 0.0);
			return true;
		}
		else {
			_rect_select->set_active(true);
			_rect_select->set_origin(screen2gl(input_state->_x, input_state->_y));
			return true;
		}
	}
	return false;
}


bool ViewSystem::mouse_button_up(InputState * input_state) {
	_free_view_x= number(input_state->_xrel);
	_free_view_y= number(input_state->_yrel);

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


bool ViewSystem::mouse_motion(InputState * input_state) {
	if (_type== FREE_VIEW) {
		
		if (input_state->_left_mouse) {
			// translation
			if (input_state->_keys[SDLK_LSHIFT]) {
				move_target(input_state->_xrel, input_state->_yrel, 0.0);
				return true;
			}
			// selection rectangulaire
			else if (_rect_select->_is_active) {
				_rect_select->set_moving(screen2gl(input_state->_x, input_state->_y));
				return true;
			}
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

	else if (_type== THIRD_PERSON_FREE) {
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
	}
	return false;
}


bool ViewSystem::key_down(InputState * input_state, SDL_Keycode key) {
	// changement de type de vue
	if (key== SDLK_v) {
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
	}

	if (_type== FREE_VIEW) {
		// affichage repere
		if (key== SDLK_r) {
			_repere->_is_repere= !_repere->_is_repere;
			_repere->_is_box= !_repere->_is_box;
			return true;
		}
	}
	else if (_type== THIRD_PERSON_FREE) {

	}
	else if (_type== THIRD_PERSON_BEHIND) {

	}
	return false;
}


bool ViewSystem::key_up(InputState * input_state, SDL_Keycode key) {

	return false;
}


// a appeler des qu'un param est modifie
void ViewSystem::update() {
	// https://en.wikipedia.org/wiki/Spherical_coordinate_system
	_dir= -pt_type_3d(cos(_phi)* sin(_theta), sin(_phi)* sin(_theta), cos(_theta));
	_eye= _target- _rho* _dir;
	_up= -pt_type_3d(cos(_phi)* cos(_theta), sin(_phi)* cos(_theta), -sin(_theta));
	_right= glm::cross(_dir, _up);
	_world2camera= glm::lookAt(_eye, _target, _up);

	/*cout << "-------------------" << endl;
	cout << "target=" << glm::to_string(_target) << endl;
	cout << "eye=" << glm::to_string(_eye) << endl;
	cout << "up=" << glm::to_string(_up) << endl;
	cout << "world2camera=" << glm::to_string(_world2camera) << endl;*/

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


void ViewSystem::set(const pt_type_3d & target, number phi, number theta, number rho) {
	_target= target;
	_phi= phi;
	_theta= theta;
	_rho= rho;

	update();
}


void ViewSystem::set_2d(number rho) {
	_target= pt_type_3d(0.0);
	_phi= -1.0 * M_PI_2;
	_theta= 0.0;
	_rho= rho;

	update();
}

// les move sont des modif delta, contrairement a set ou anim qui font des modifs absolues
/*void ViewSystem::move_target(const pt_type_3d & v) {
	_target.x+= v.x* sin(_phi)- v.y* cos(_phi);
	_target.y+= -v.x* cos(_phi)- v.y* sin(_phi);
	_target.z+= v.z;
	
	update();
}*/

void ViewSystem::move_target(int screen_delta_x, int screen_delta_y, number z) {
	/*
	Avant j'utilisais move_target(pt_type_3d) avec LEFT_MOUSE_SENSIVITY mais du coup la translation ne dépendait pas de l'altitude
	et du coup à basse alti ça bougeait vite, haute alti lentement.
	Ici il y a une meilleure corrélation entre mouvement curseur et translation terrain
	A terme fournir en argument optionnel un Terrain afin de s'ajuster à un relief plus complexe que z = 0
	*/
	pt_type v= screen2world(_screengl->_screen_width* 0.5- screen_delta_x, _screengl->_screen_height* 0.5- screen_delta_y, z);
	_target.x= v.x;
	_target.y= v.y;
	update();
}


// phi est l'angle horizontal compris entre 0 et 2PI
void ViewSystem::move_phi(number x) {
	_phi+= x;
	while (_phi< 0.0) {
		_phi+= M_PI* 2.0;
	}
	while (_phi> M_PI* 2.0) {
		_phi-= M_PI* 2.0;
	}
	
	update();
}


// theta est l'angle vertical, compris entre 0 et PI
void ViewSystem::move_theta(number x) {
	if ((_theta+ x> 0.0) && (_theta+ x< M_PI)) {
		_theta+= x;
	}
	
	update();
}


void ViewSystem::move_rho(number x) {
	_rho+= x;
	if (_rho< 0.0) {
		_rho= 0.0;
	}
	
	update();
}


void ViewSystem::draw() {
	_repere->draw(_world2clip);
	_rect_select->draw();

	// TODO : afficher des infos relatives à ViewSystem
	/*std::vector<Text> texts;
	texts.push_back(Text("hello", pt_type(0.0, 0.0), 0.01, pt_type_4d(0.7f, 0.6f, 0.5f, 1.0)));
	_font->set_text(texts);
	_font->draw();*/
}


void ViewSystem::anim(const pt_type_3d & target, const quat & rotation) {
	if (_type== FREE_VIEW) {
		// legère inertie aux translations
		number tresh= 1e-9;
		number decay_factor= 0.95;
		if (abs(_free_view_x)> tresh) { _free_view_x*= decay_factor; }
		if (abs(_free_view_y)> tresh) { _free_view_y*= decay_factor; }
		if ((abs(_free_view_x)> tresh) || (abs(_free_view_y)> tresh)) {
			move_target(int(_free_view_x), int(_free_view_y), 0.0);
		}
	}
	else if (_type== THIRD_PERSON_FREE) {
		// ajuste les params de ViewSystem en fonction d'un but
		_target= target;
		update();
	}
	else if (_type== THIRD_PERSON_BEHIND) {
		// ajuste les params de ViewSystem en fonction d'un but et d'une rotation
		_target= target;
		_phi= glm::roll(rotation);
		update();
	}
}


// cf http://antongerdelan.net/opengl/raycasting.html
pt_type ViewSystem::screen2world(uint x, uint y, number z) {
	pt_type gl_coords= screen2gl(x, y);
	return screen2world(gl_coords, z);
}


pt_type ViewSystem::screen2world(pt_type gl_coords, number z) {
	pt_type_4d ray_clip= pt_type_4d(gl_coords.x, gl_coords.y, -1.0, 1.0);
	//cout << "ray_clip=" << glm::to_string(ray_clip) << "\n";
	
	pt_type_4d ray_eye= glm::inverse(_camera2clip)* ray_clip;
	ray_eye= pt_type_4d(ray_eye.x, ray_eye.y, -1.0, 0.0);
	//cout << "_camera2clip=" << glm::to_string(_camera2clip) << "\n";
	//cout << "inverse(_camera2clip)=" << glm::to_string(glm::inverse(_camera2clip)) << "\n";
	//cout << "ray_eye=" << glm::to_string(ray_eye) << "\n";
	
	pt_type_3d ray_world= pt_type_3d(glm::inverse(_world2camera)* ray_eye);
	ray_world= glm::normalize(ray_world);
	//cout << "ray_world=" << glm::to_string(ray_world) << "\n";
	
	number lambda= (z- _eye.z)/ ray_world.z;
	//cout << "lambda=" << lambda << "\n";
	
	pt_type_3d v= _eye+ lambda* ray_world;
	//cout << "result=" << glm::to_string(v) << "\n";

	return pt_type(v.x, v.y);
}


pt_type ViewSystem::screen2gl(uint x, uint y) {
	// les coords OpenGL sont [-1, 1] x [-1, 1]
	number x_gl= 2.0* (number)(x)/ (number)(_screengl->_screen_width)- 1.0;
	number y_gl= 1.0- 2.0* (number)(y)/ (number)(_screengl->_screen_height);
	return pt_type(x_gl, y_gl);
}


glm::uvec2 ViewSystem::gl2screen(pt_type gl_coords) {
	uint x= (uint)((gl_coords.x+ 1.0)* (number)(_screengl->_screen_width)* 0.5);
	uint y= (uint)((1.0- gl_coords.y)* (number)(_screengl->_screen_height)* 0.5);
	return pt_type(x, y);
}


number ViewSystem::depthbuffer2world(number depth) {
	return _frustum_near* _frustum_far* 2.0/ ((2.0* depth- 1.0)* (_frustum_far- _frustum_near) - (_frustum_far+ _frustum_near));
}


// cf http://cgvr.cs.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
/*bool ViewSystem::contains_point(const pt_type_3d & pos) {
	pt_type_3d pos2= pt_type_3d(_world2camera* pt_type_4d(pos.x, pos.y, pos.z, 1.0));

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


/*bool ViewSystem::intersects_aabb_2d(AABB_2D * aabb, bool selection=false) {
	std::vector<pt_type_3d> pts;
	return intersects_pts(pts, 8, selection);
}*/


bool ViewSystem::intersects_aabb(AABB * aabb, const mat_4d & model2world_matrix, bool selection) {
	std::vector<pt_type_3d> pts;
	for (uint i=0; i<8; ++i) {
		pts.push_back(pt_type_3d(model2world_matrix* pt_type_4d(pt_type_3d(aabb->_pts[i]), 1.0)));
	}

	return intersects_pts(pts, selection);
}


bool ViewSystem::intersects_pts(std::vector<pt_type_3d> pts, bool selection) {
	pt_type_3d * add = pts.data();
	return intersects_pts(add, pts.size(), selection);
}


// cf https://zeux.io/2009/01/31/view-frustum-culling-optimization-introduction
bool ViewSystem::intersects_pts(pt_type_3d * pts, uint n_pts, bool selection) {
	bool b= false;
	pt_type_3d norms[4];
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
	/*for (uint i=0; i<n_pts; ++i) {
		std::cout << glm::to_string(pts[i]) << " ; ";
	}
	std::cout << "\n";
	for (uint i=0; i<4; ++i) {
		std::cout << glm::to_string(norms[i]) << " ; ";
	}
	std::cout << "\n";*/

	for (uint i=0; i<4; ++i) {
		b= false;
		for (uint j=0; j<n_pts; ++j) {
			//std::cout << "i=" << i << " ; j=" << j << " ; pts[j]=" << glm::to_string(pts[j]) << " ; center_near" << glm::to_string(_center_near) << " ; norms[i]" << glm::to_string(norms[i]) << " ; dot=" << glm::dot(pts[j]- _center_near, norms[i]) << "\n";
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

/*
bool ViewSystem::selection_contains_point(const pt_type_3d & pt) {
	float xmin= min(_rect_select->_gl_origin.x, _rect_select->_gl_moving.x);
	float ymin= min(_rect_select->_gl_origin.y, _rect_select->_gl_moving.y);
	float xmax= max(_rect_select->_gl_origin.x, _rect_select->_gl_moving.x);
	float ymax= max(_rect_select->_gl_origin.y, _rect_select->_gl_moving.y);
	
	pt_type v1= screen2world(pt_type(xmin, ymin), 0.0);
	pt_type v2= screen2world(pt_type(xmax, ymin), 0.0);
	pt_type v3= screen2world(pt_type(xmax, ymax), 0.0);
	pt_type v4= screen2world(pt_type(xmin, ymax), 0.0);

	pt_type_3d dir1= pt_type_3d(v1, 0.0)- _eye;
	pt_type_3d dir2= pt_type_3d(v2, 0.0)- _eye;
	pt_type_3d dir3= pt_type_3d(v3, 0.0)- _eye;
	pt_type_3d dir4= pt_type_3d(v4, 0.0)- _eye;

	pt_type_3d n1= glm::cross(dir2, dir1);
	pt_type_3d n2= glm::cross(dir3, dir2);
	pt_type_3d n3= glm::cross(dir4, dir3);
	pt_type_3d n4= glm::cross(dir1, dir4);

	pt_type_3d dir_pt= pt- _eye;

	cout << "-----------------\n";
	cout << "xmin=" << xmin << " ; " << " ; xmax=" << xmax << " ; " << "ymin=" << ymin << " ; " << " ; ymax=" << ymax << "\n";
	cout << "v1=" << glm::to_string(v1) << " ; v2=" << glm::to_string(v2) << " ; v3=" << glm::to_string(v3) << " ; v4=" << glm::to_string(v4) << "\n";
	cout << "dir1=" << glm::to_string(dir1) << " ; dir2=" << glm::to_string(dir2) << " ; dir3=" << glm::to_string(dir3) << " ; dir4=" << glm::to_string(dir4) << "\n";
	cout << "n1=" << glm::to_string(n1) << " ; n2=" << glm::to_string(n2) << " ; n3=" << glm::to_string(n3) << " ; n4=" << glm::to_string(n4) << "\n";
	cout << "pt=" << glm::to_string(pt) << " ; " << "eye=" << glm::to_string(_eye) << "\n";
	cout << glm::dot(n1, pt- _eye) << " ; " << glm::dot(n2, pt- _eye) << " ; " << glm::dot(n3, pt- _eye) << " ; " << glm::dot(n4, pt- _eye) << "\n";
	cout << "-----------------\n";

	if ((glm::dot(n1, dir_pt)< 0.0) || (glm::dot(n2, dir_pt)< 0.0) || (glm::dot(n3, dir_pt)< 0.0) || (glm::dot(n4, dir_pt)< 0.0)) {
		return false;
	}

	return true;
}
*/


void ViewSystem::update_selection_norms() {
	number xmin= min(_rect_select->_gl_origin.x, _rect_select->_gl_moving.x);
	number ymin= min(_rect_select->_gl_origin.y, _rect_select->_gl_moving.y);
	number xmax= max(_rect_select->_gl_origin.x, _rect_select->_gl_moving.x);
	number ymax= max(_rect_select->_gl_origin.y, _rect_select->_gl_moving.y);
	
	pt_type v1= screen2world(pt_type(xmin, ymin), 0.0);
	pt_type v2= screen2world(pt_type(xmax, ymin), 0.0);
	pt_type v3= screen2world(pt_type(xmax, ymax), 0.0);
	pt_type v4= screen2world(pt_type(xmin, ymax), 0.0);
	//std::cout << "v1=" << glm::to_string(v1) << " ; v2=" << glm::to_string(v2) << " ; v3=" << glm::to_string(v3) << " ; v4=" << glm::to_string(v4) << "\n";
	//std::cout << "center_near=" << glm::to_string(_center_near) << " ; eye=" << glm::to_string(_eye) << "\n";

	pt_type_3d dir1= pt_type_3d(v1, 0.0)- _eye;
	pt_type_3d dir2= pt_type_3d(v2, 0.0)- _eye;
	pt_type_3d dir3= pt_type_3d(v3, 0.0)- _eye;
	pt_type_3d dir4= pt_type_3d(v4, 0.0)- _eye;

	_rect_select->_norms[0]= glm::cross(dir2, dir1);
	_rect_select->_norms[1]= glm::cross(dir3, dir2);
	_rect_select->_norms[2]= glm::cross(dir4, dir3);
	_rect_select->_norms[3]= glm::cross(dir1, dir4);
}


bool ViewSystem::single_selection_intersects_aabb(AABB * aabb, bool check_depth) {
	pt_type click_world= screen2world(_rect_select->_gl_origin, 0.0);
	number t_hit;
	bool intersect= ray_intersects_aabb(_eye, pt_type_3d(click_world, 0.0)- _eye, aabb, t_hit);
	if (!intersect) {
		return false;
	}

	if (!check_depth) {
		return true;
	}

	glm::uvec2 screen_coords= gl2screen(_rect_select->_gl_origin);
	number buffer_depth;
	// attention au height- y
	glReadPixels(screen_coords.x, _screengl->_screen_height- screen_coords.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &buffer_depth);
	number world_depth= depthbuffer2world(buffer_depth);
	if (abs(abs(world_depth)- t_hit)< 40.0) {
		return true;
	}

	return false;
}


bool ViewSystem::rect_selection_intersects_bbox(BBox * bbox, bool check_depth) {
	if (!intersects_bbox(bbox, true)) {
		return false;
	}

	if (!check_depth) {
		return true;
	}

	pt_type_3d center= 0.5* (bbox->_aabb->_vmin+ bbox->_aabb->_vmax);
	pt_type_4d v= _world2clip* pt_type_4d(center, 1.0);
	glm::uvec2 screen_coords= gl2screen(pt_type(v.x/ v.w, v.y/ v.w));
	number buffer_depth;
	// attention au height- y
	glReadPixels(screen_coords.x, _screengl->_screen_height- screen_coords.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &buffer_depth);
	number world_depth= depthbuffer2world(buffer_depth);
	number t_hit= glm::distance(center, _eye);
	if (abs(abs(world_depth)- t_hit)< 40.0) {
		return true;
	}

	return false;
}
