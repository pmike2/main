

#include "repere.h"

using namespace std;



// ------------------------------------------------------------------------------------------------------------------------
Repere::Repere() {
	
}


Repere::Repere(GLuint prog_draw) : _prog_draw(prog_draw), _is_repere(true), _is_ground(true), _is_box(true) {
	unsigned int i;
	
	float data_repere[]= {
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		REPERE_AXIS, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, REPERE_AXIS, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, REPERE_AXIS, 0.0f, 0.0f, 1.0f
	};
	
	float EPS= 0.01f;
	float data_ground[]= {
		-REPERE_GROUND, -REPERE_GROUND, -EPS, 0.0f, 0.0f, 0.0f,
		 REPERE_GROUND, -REPERE_GROUND, -EPS, 0.0f, 0.0f, 0.0f,
		 REPERE_GROUND,  REPERE_GROUND, -EPS, 0.0f, 0.0f, 0.0f,
		-REPERE_GROUND, -REPERE_GROUND, -EPS, 0.0f, 0.0f, 0.0f,
		 REPERE_GROUND,  REPERE_GROUND, -EPS, 0.0f, 0.0f, 0.0f,
		-REPERE_GROUND,  REPERE_GROUND, -EPS, 0.0f, 0.0f, 0.0f
	};

	float data_box[]= {
		-REPERE_BOX, -REPERE_BOX, -REPERE_BOX, 0.0f, 0.0f, 0.0f,  REPERE_BOX, -REPERE_BOX, -REPERE_BOX , 0.0f, 0.0f, 0.0f,
		REPERE_BOX, -REPERE_BOX, -REPERE_BOX, 0.0f, 0.0f, 0.0f,  REPERE_BOX, REPERE_BOX, -REPERE_BOX , 0.0f, 0.0f, 0.0f,
		REPERE_BOX, REPERE_BOX, -REPERE_BOX, 0.0f, 0.0f, 0.0f,  -REPERE_BOX, REPERE_BOX, -REPERE_BOX, 0.0f, 0.0f, 0.0f,  
		-REPERE_BOX, REPERE_BOX, -REPERE_BOX, 0.0f, 0.0f, 0.0f,  -REPERE_BOX, -REPERE_BOX, -REPERE_BOX, 0.0f, 0.0f, 0.0f,

		-REPERE_BOX, -REPERE_BOX, REPERE_BOX, 0.0f, 0.0f, 0.0f,  REPERE_BOX, -REPERE_BOX, REPERE_BOX , 0.0f, 0.0f, 0.0f,
		REPERE_BOX, -REPERE_BOX, REPERE_BOX, 0.0f, 0.0f, 0.0f,  REPERE_BOX, REPERE_BOX, REPERE_BOX , 0.0f, 0.0f, 0.0f,
		REPERE_BOX, REPERE_BOX, REPERE_BOX, 0.0f, 0.0f, 0.0f,  -REPERE_BOX, REPERE_BOX, REPERE_BOX, 0.0f, 0.0f, 0.0f,  
		-REPERE_BOX, REPERE_BOX, REPERE_BOX, 0.0f, 0.0f, 0.0f,  -REPERE_BOX, -REPERE_BOX, REPERE_BOX, 0.0f, 0.0f, 0.0f,
		
		-REPERE_BOX, -REPERE_BOX, -REPERE_BOX, 0.0f, 0.0f, 0.0f,  -REPERE_BOX, -REPERE_BOX, REPERE_BOX , 0.0f, 0.0f, 0.0f,
		REPERE_BOX, -REPERE_BOX, -REPERE_BOX, 0.0f, 0.0f, 0.0f,  REPERE_BOX, -REPERE_BOX, REPERE_BOX , 0.0f, 0.0f, 0.0f,
		-REPERE_BOX, REPERE_BOX, -REPERE_BOX, 0.0f, 0.0f, 0.0f,  -REPERE_BOX, REPERE_BOX, REPERE_BOX , 0.0f, 0.0f, 0.0f,
		REPERE_BOX, REPERE_BOX, -REPERE_BOX, 0.0f, 0.0f, 0.0f,  REPERE_BOX, REPERE_BOX, REPERE_BOX , 0.0f, 0.0f, 0.0f
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


void Repere::draw(const glm::mat4 & world2clip) {
	if (_is_repere) {
		glUseProgram(_prog_draw);
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
		
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_diffuse_color_loc);

		glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(world2clip));
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
		
		glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(world2clip));
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
		
		glUniformMatrix4fv(_world2clip_loc, 1, GL_FALSE, glm::value_ptr(world2clip));
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
	_prog_draw(prog_draw), _is_active(false), _gl_origin(glm::vec2(0.0f)), _gl_moving(glm::vec2(0.0f)),	_color(glm::vec3(1.0f, 1.0f, 0.0f))
{
	glGenBuffers(1, &_buffer);
	
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_color_loc= glGetAttribLocation(_prog_draw, "color_in");

	for (unsigned int i=0; i<4; ++i) {
		_norms[i]= glm::vec3(0.0f);
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


void RectSelect::set_origin(glm::vec2 gl_v) {
	_gl_origin= gl_v;
	_gl_moving= _gl_origin;
	update_draw();
}


void RectSelect::set_moving(glm::vec2 gl_v) {
	_gl_moving= gl_v;
	update_draw();
}


void RectSelect::set_active(bool is_active) {
	_is_active= is_active;
}


void RectSelect::update_draw() {
	float data_selection[]= {
		_gl_origin.x, _gl_origin.y, _color.x, _color.y, _color.z,
		_gl_moving.x, _gl_origin.y, _color.x, _color.y, _color.z,
		
		_gl_moving.x, _gl_origin.y, _color.x, _color.y, _color.z,
		_gl_moving.x, _gl_moving.y, _color.x, _color.y, _color.z,
		
		_gl_moving.x, _gl_moving.y, _color.x, _color.y, _color.z,
		_gl_origin.x, _gl_moving.y, _color.x, _color.y, _color.z,

		_gl_origin.x, _gl_moving.y, _color.x, _color.y, _color.z,
		_gl_origin.x, _gl_origin.y, _color.x, _color.y, _color.z,
	};

	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data_selection), data_selection, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// ------------------------------------------------------------------------------------------------------------------------
ViewSystem::ViewSystem() {

}


ViewSystem::ViewSystem(GLuint prog_repere, GLuint prog_select, unsigned int screen_width, unsigned int screen_height) :
	_target(glm::vec3(0.0f, 0.0f, 0.0f)), _eye(glm::vec3(0.0f, 0.0f, 0.0f)), _up(glm::vec3(0.0f, 0.0f, 0.0f)), 
	_phi(0.0f), _theta(0.0f), _rho(1.0f), _screen_width(screen_width), _screen_height(screen_height),
	_type(FREE_VIEW), _frustum_halfsize(FRUSTUM_HALFSIZE), _frustum_near(FRUSTUM_NEAR), _frustum_far(FRUSTUM_FAR),
	_new_single_selection(false), _new_rect_selection(false), _new_destination(false), _free_view_x(0), _free_view_y(0)
{
	_camera2clip= glm::frustum(-_frustum_halfsize* (float)(_screen_width)/ (float)(_screen_height), _frustum_halfsize* (float)(_screen_width)/ (float)(_screen_height), -_frustum_halfsize, _frustum_halfsize, _frustum_near, _frustum_far);

	update();

	_repere= new Repere(prog_repere);
	_rect_select= new RectSelect(prog_select);
}


ViewSystem::~ViewSystem() {
	delete _repere;
	delete _rect_select;
}


bool ViewSystem::mouse_button_down(InputState * input_state) {
	_free_view_x= 0.0;
	_free_view_y= 0.0;

	if (input_state->_left_mouse) {
		if (input_state->_keys[SDLK_m]) {
			_new_destination= true;
			_destination= glm::vec3(screen2world(input_state->_x, input_state->_y, 0.0f), 0.0f);
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
	_free_view_x= input_state->_xrel;
	_free_view_y= input_state->_yrel;

	if (_rect_select->_is_active) {
		_rect_select->set_active(false);
		if (glm::distance2(_rect_select->_gl_origin, _rect_select->_gl_moving)< 0.0001f) {
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
				move_target(input_state->_xrel, input_state->_yrel, 0.0f);
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
			move_rho(-(float)(input_state->_yrel)* MIDDLE_MOUSE_SENSIVITY);
			return true;
		}
		// rotations
		else if (input_state->_right_mouse) {
			move_theta((float)(-input_state->_yrel)* RIGHT_MOUSE_SENSIVITY);
			move_phi((float)(-input_state->_xrel)* RIGHT_MOUSE_SENSIVITY);
			return true;
		}
	}

	else if (_type== THIRD_PERSON_FREE) {
		if (input_state->_left_mouse) {
		}
		// zoom
		else if ((input_state->_middle_mouse) || (input_state->_keys[SDLK_LCTRL])) {
			move_rho(-(float)(input_state->_yrel)* MIDDLE_MOUSE_SENSIVITY);
			return true;
		}
		// rotations
		else if (input_state->_right_mouse) {
			move_theta((float)(-input_state->_yrel)* RIGHT_MOUSE_SENSIVITY);
			move_phi((float)(-input_state->_xrel)* RIGHT_MOUSE_SENSIVITY);
			return true;
		}
	}

	else if (_type== THIRD_PERSON_BEHIND) {
		if (input_state->_left_mouse) {
		}
		// zoom
		else if ((input_state->_middle_mouse) || (input_state->_keys[SDLK_LCTRL])) {
			move_rho(-(float)(input_state->_yrel)* MIDDLE_MOUSE_SENSIVITY);
			return true;
		}
		else if (input_state->_right_mouse) {
			
		}
		else {
			// rotation haut - bas ; la rotation gauche - droite est assurée par world.mouse_motion et view_system.anim
			move_theta((float)(-input_state->_yrel)* RIGHT_MOUSE_SENSIVITY);
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
	_dir= -glm::vec3(cos(_phi)* sin(_theta), sin(_phi)* sin(_theta), cos(_theta));
	_eye= _target- _rho* _dir;
	_up= -glm::vec3(cos(_phi)* cos(_theta), sin(_phi)* cos(_theta), -sin(_theta));
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
	_norm_right= glm::cross(_up, glm::normalize(_center_near- _eye+ _right* _frustum_halfsize* (float)(_screen_width)/ (float)(_screen_height)));
	_norm_left = glm::cross(glm::normalize(_center_near- _eye- _right* _frustum_halfsize* (float)(_screen_width)/ (float)(_screen_height)), _up);
	_norm_top= glm::cross(glm::normalize(_center_near- _eye+ _up* _frustum_halfsize), _right);
	_norm_bottom= glm::cross(_right, glm::normalize(_center_near- _eye- _up* _frustum_halfsize));
}


void ViewSystem::set(const glm::vec3 & target, float phi, float theta, float rho) {
	_target= target;
	_phi= phi;
	_theta= theta;
	_rho= rho;

	update();
}


// les move sont des modif delta, contrairement a set ou anim qui font des modifs absolues
/*void ViewSystem::move_target(const glm::vec3 & v) {
	_target.x+= v.x* sin(_phi)- v.y* cos(_phi);
	_target.y+= -v.x* cos(_phi)- v.y* sin(_phi);
	_target.z+= v.z;
	
	update();
}*/

void ViewSystem::move_target(int screen_delta_x, int screen_delta_y, float z) {
	/*
	Avant j'utilisais move_target(glm::vec3) avec LEFT_MOUSE_SENSIVITY mais du coup la translation ne dépendait pas de l'altitude
	et du coup à basse alti ça bougeait vite, haute alti lentement.
	Ici il y a une meilleure corrélation entre mouvement curseur et translation terrain
	A terme fournir en argument optionnel un Terrain afin de s'ajuster à un relief plus complexe que z = 0
	*/
	glm::vec2 v= screen2world(_screen_width*0.5- screen_delta_x, _screen_height*0.5- screen_delta_y, z);
	_target.x= v.x;
	_target.y= v.y;
	update();
}


// phi est l'angle horizontal compris entre 0 et 2PI
void ViewSystem::move_phi(float x) {
	_phi+= x;
	while (_phi< 0.0f) {
		_phi+= M_PI* 2.0f;
	}
	while (_phi> M_PI* 2.0f) {
		_phi-= M_PI* 2.0f;
	}
	
	update();
}


// theta est l'angle vertical, compris entre 0 et PI
void ViewSystem::move_theta(float x) {
	if ((_theta+ x> 0.0f) && (_theta+ x< M_PI)) {
		_theta+= x;
	}
	
	update();
}


void ViewSystem::move_rho(float x) {
	_rho+= x;
	if (_rho< 0.0f) {
		_rho= 0.0f;
	}
	
	update();
}


void ViewSystem::draw() {
	_repere->draw(_world2clip);
	_rect_select->draw();
}


// permet a certaines vues d'ajuster les params de ViewSystem en fonction d'un but
void ViewSystem::anim(const glm::vec3 & target, const glm::quat & rotation) {
	if (_type== FREE_VIEW) {
		// legère inertie aux translations
		int tresh= 1e-7;
		int decay_factor= 0.9;
		if (abs(_free_view_x)> tresh) { _free_view_x*= decay_factor; }
		if (abs(_free_view_y)> tresh) { _free_view_y*= decay_factor; }
		if ((abs(_free_view_x)> tresh) || (abs(_free_view_y)> tresh)) {
			move_target(_free_view_x, _free_view_y, 0.0f);
		}
	}
	else if (_type== THIRD_PERSON_FREE) {
		_target= target;
		update();
	}
	else if (_type== THIRD_PERSON_BEHIND) {
		_target= target;
		_phi= glm::roll(rotation);
		update();
	}
}


// cf http://antongerdelan.net/opengl/raycasting.html
glm::vec2 ViewSystem::screen2world(unsigned int x, unsigned int y, float z) {
	glm::vec2 gl_coords= screen2gl(x, y);
	return screen2world(gl_coords, z);
}


glm::vec2 ViewSystem::screen2world(glm::vec2 gl_coords, float z) {
	glm::vec4 ray_clip= glm::vec4(gl_coords.x, gl_coords.y, -1.0f, 1.0f);
	//cout << "ray_clip=" << glm::to_string(ray_clip) << "\n";
	
	glm::vec4 ray_eye= glm::inverse(_camera2clip)* ray_clip;
	ray_eye= glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
	//cout << "_camera2clip=" << glm::to_string(_camera2clip) << "\n";
	//cout << "inverse(_camera2clip)=" << glm::to_string(glm::inverse(_camera2clip)) << "\n";
	//cout << "ray_eye=" << glm::to_string(ray_eye) << "\n";
	
	glm::vec3 ray_world= glm::vec3(glm::inverse(_world2camera)* ray_eye);
	ray_world= glm::normalize(ray_world);
	//cout << "ray_world=" << glm::to_string(ray_world) << "\n";
	
	float lambda= (z- _eye.z)/ ray_world.z;
	//cout << "lambda=" << lambda << "\n";
	
	glm::vec3 v= _eye+ lambda* ray_world;
	//cout << "result=" << glm::to_string(v) << "\n";

	return glm::vec2(v.x, v.y);
}


glm::vec2 ViewSystem::screen2gl(unsigned int x, unsigned int y) {
	// les coords OpenGL sont [-1, 1] x [-1, 1]
	float x_gl= 2.0f* (float)(x)/ (float)(_screen_width)- 1.0f;
	float y_gl= 1.0f- 2.0f* (float)(y)/ (float)(_screen_height);
	return glm::vec2(x_gl, y_gl);
}


glm::uvec2 ViewSystem::gl2screen(glm::vec2 gl_coords) {
	unsigned int x= (unsigned int)((gl_coords.x+ 1.0f)* (float)(_screen_width)* 0.5f);
	unsigned int y= (unsigned int)((1.0f- gl_coords.y)* (float)(_screen_height)* 0.5f);
	return glm::vec2(x, y);
}


float ViewSystem::depthbuffer2world(float depth) {
	return _frustum_near* _frustum_far* 2.0f/ ((2.0f* depth- 1.0f)* (_frustum_far- _frustum_near) - (_frustum_far+ _frustum_near));
}


// cf http://cgvr.cs.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
/*bool ViewSystem::contains_point(const glm::vec3 & pos) {
	glm::vec3 pos2= glm::vec3(_world2camera* glm::vec4(pos.x, pos.y, pos.z, 1.0f));

	// z pointe vers l'extérieur il faut donc l'inverser
	float y= -pos2.z* _frustum_halfsize/ _frustum_near;
	float x= y* (float)(_screen_width)/ (float)(_screen_height);

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


bool ViewSystem::intersects_aabb(AABB * aabb, const glm::mat4 & model2world_matrix, bool selection) {
	glm::vec3 pts[8];
	for (unsigned int i=0; i<8; ++i) {
		pts[i]= glm::vec3(model2world_matrix* glm::vec4(aabb->_pts[i], 1.0f));
	}

	return intersects_pts(pts, 8, selection);
}


// cf https://zeux.io/2009/01/31/view-frustum-culling-optimization-introduction
bool ViewSystem::intersects_pts(glm::vec3 * pts, unsigned int n_pts, bool selection) {
	bool b= false;
	glm::vec3 norms[4];
	if (selection) {
		for (unsigned int i=0; i<4; ++i) {
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

	for (unsigned int i=0; i<4; ++i) {
		b= false;
		for (unsigned int j=0; j<n_pts; ++j) {
			if (glm::dot(pts[j]- _center_near, norms[i])> 0) {
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
bool ViewSystem::selection_contains_point(const glm::vec3 & pt) {
	float xmin= min(_rect_select->_gl_origin.x, _rect_select->_gl_moving.x);
	float ymin= min(_rect_select->_gl_origin.y, _rect_select->_gl_moving.y);
	float xmax= max(_rect_select->_gl_origin.x, _rect_select->_gl_moving.x);
	float ymax= max(_rect_select->_gl_origin.y, _rect_select->_gl_moving.y);
	
	glm::vec2 v1= screen2world(glm::vec2(xmin, ymin), 0.0f);
	glm::vec2 v2= screen2world(glm::vec2(xmax, ymin), 0.0f);
	glm::vec2 v3= screen2world(glm::vec2(xmax, ymax), 0.0f);
	glm::vec2 v4= screen2world(glm::vec2(xmin, ymax), 0.0f);

	glm::vec3 dir1= glm::vec3(v1, 0.0f)- _eye;
	glm::vec3 dir2= glm::vec3(v2, 0.0f)- _eye;
	glm::vec3 dir3= glm::vec3(v3, 0.0f)- _eye;
	glm::vec3 dir4= glm::vec3(v4, 0.0f)- _eye;

	glm::vec3 n1= glm::cross(dir2, dir1);
	glm::vec3 n2= glm::cross(dir3, dir2);
	glm::vec3 n3= glm::cross(dir4, dir3);
	glm::vec3 n4= glm::cross(dir1, dir4);

	glm::vec3 dir_pt= pt- _eye;

	cout << "-----------------\n";
	cout << "xmin=" << xmin << " ; " << " ; xmax=" << xmax << " ; " << "ymin=" << ymin << " ; " << " ; ymax=" << ymax << "\n";
	cout << "v1=" << glm::to_string(v1) << " ; v2=" << glm::to_string(v2) << " ; v3=" << glm::to_string(v3) << " ; v4=" << glm::to_string(v4) << "\n";
	cout << "dir1=" << glm::to_string(dir1) << " ; dir2=" << glm::to_string(dir2) << " ; dir3=" << glm::to_string(dir3) << " ; dir4=" << glm::to_string(dir4) << "\n";
	cout << "n1=" << glm::to_string(n1) << " ; n2=" << glm::to_string(n2) << " ; n3=" << glm::to_string(n3) << " ; n4=" << glm::to_string(n4) << "\n";
	cout << "pt=" << glm::to_string(pt) << " ; " << "eye=" << glm::to_string(_eye) << "\n";
	cout << glm::dot(n1, pt- _eye) << " ; " << glm::dot(n2, pt- _eye) << " ; " << glm::dot(n3, pt- _eye) << " ; " << glm::dot(n4, pt- _eye) << "\n";
	cout << "-----------------\n";

	if ((glm::dot(n1, dir_pt)< 0.0f) || (glm::dot(n2, dir_pt)< 0.0f) || (glm::dot(n3, dir_pt)< 0.0f) || (glm::dot(n4, dir_pt)< 0.0f)) {
		return false;
	}

	return true;
}
*/


void ViewSystem::update_selection_norms() {
	float xmin= min(_rect_select->_gl_origin.x, _rect_select->_gl_moving.x);
	float ymin= min(_rect_select->_gl_origin.y, _rect_select->_gl_moving.y);
	float xmax= max(_rect_select->_gl_origin.x, _rect_select->_gl_moving.x);
	float ymax= max(_rect_select->_gl_origin.y, _rect_select->_gl_moving.y);
	
	glm::vec2 v1= screen2world(glm::vec2(xmin, ymin), 0.0f);
	glm::vec2 v2= screen2world(glm::vec2(xmax, ymin), 0.0f);
	glm::vec2 v3= screen2world(glm::vec2(xmax, ymax), 0.0f);
	glm::vec2 v4= screen2world(glm::vec2(xmin, ymax), 0.0f);

	glm::vec3 dir1= glm::vec3(v1, 0.0f)- _eye;
	glm::vec3 dir2= glm::vec3(v2, 0.0f)- _eye;
	glm::vec3 dir3= glm::vec3(v3, 0.0f)- _eye;
	glm::vec3 dir4= glm::vec3(v4, 0.0f)- _eye;

	_rect_select->_norms[0]= glm::cross(dir2, dir1);
	_rect_select->_norms[1]= glm::cross(dir3, dir2);
	_rect_select->_norms[2]= glm::cross(dir4, dir3);
	_rect_select->_norms[3]= glm::cross(dir1, dir4);
}


bool ViewSystem::single_selection_intersects_aabb(AABB * aabb) {
	glm::vec2 click_world= screen2world(_rect_select->_gl_origin, 0.0f);
	float t_hit;
	bool intersect= ray_intersects_aabb(_eye, glm::vec3(click_world, 0.0f)- _eye, aabb, t_hit);
	if (!intersect) {
		return false;
	}

	glm::uvec2 screen_coords= gl2screen(_rect_select->_gl_origin);
	float buffer_depth;
	// attention au height- y
	glReadPixels(screen_coords.x, _screen_height- screen_coords.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &buffer_depth);
	float world_depth= depthbuffer2world(buffer_depth);
	if (abs(abs(world_depth)- t_hit)< 40.0f) {
		return true;
	}

	return false;
}


bool ViewSystem::rect_selection_intersects_bbox(BBox * bbox) {
	if (!intersects_bbox(bbox, true)) {
		return false;
	}

	glm::vec3 center= 0.5f* (bbox->_aabb->_vmin+ bbox->_aabb->_vmax);
	glm::vec4 v= _world2clip* glm::vec4(center, 1.0f);
	glm::uvec2 screen_coords= gl2screen(glm::vec2(v.x/ v.w, v.y/ v.w));
	float buffer_depth;
	// attention au height- y
	glReadPixels(screen_coords.x, _screen_height- screen_coords.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &buffer_depth);
	float world_depth= depthbuffer2world(buffer_depth);
	float t_hit= glm::distance(center, _eye);
	if (abs(abs(world_depth)- t_hit)< 40.0f) {
		return true;
	}

	return false;
}
