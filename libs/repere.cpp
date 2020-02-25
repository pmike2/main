

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
	for (i=0; i<36; i++)
		_data_repere[i]= data_repere[i];
	
	float EPS= 0.01f;
	float data_ground[]= {
		-REPERE_GROUND, -REPERE_GROUND, -EPS, 0.0f, 0.0f, 0.0f,
		 REPERE_GROUND, -REPERE_GROUND, -EPS, 0.0f, 0.0f, 0.0f,
		 REPERE_GROUND,  REPERE_GROUND, -EPS, 0.0f, 0.0f, 0.0f,
		-REPERE_GROUND, -REPERE_GROUND, -EPS, 0.0f, 0.0f, 0.0f,
		 REPERE_GROUND,  REPERE_GROUND, -EPS, 0.0f, 0.0f, 0.0f,
		-REPERE_GROUND,  REPERE_GROUND, -EPS, 0.0f, 0.0f, 0.0f
	};
	for (i=0; i<36; i++)
		_data_ground[i]= data_ground[i];

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
	for (i=0; i<144; i++)
		_data_box[i]= data_box[i];

	glGenBuffers(1, &_buffer_repere);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer_repere);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_data_repere), _data_repere, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &_buffer_ground);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer_ground);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_data_ground), _data_ground, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &_buffer_box);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer_box);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_data_box), _data_box, GL_STATIC_DRAW);
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
		glBindBuffer(GL_ARRAY_BUFFER, _buffer_repere);
		
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
		glBindBuffer(GL_ARRAY_BUFFER, _buffer_ground);

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
		glBindBuffer(GL_ARRAY_BUFFER, _buffer_box);

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
ViewSystem::ViewSystem() {

}


ViewSystem::ViewSystem(GLuint prog_repere, unsigned int screen_width, unsigned int screen_height) :
	_target(glm::vec3(0.0f, 0.0f, 0.0f)), _eye(glm::vec3(0.0f, 0.0f, 0.0f)), _up(glm::vec3(0.0f, 0.0f, 0.0f)), 
	_phi(0.0f), _theta(0.0f), _rho(1.0f), _screen_width(screen_width), _screen_height(screen_height),
	_type(FREE_VIEW), _frustum_halfsize(FRUSTUM_HALFSIZE), _frustum_near(FRUSTUM_NEAR), _frustum_far(FRUSTUM_FAR)
{
	_camera2clip= glm::frustum(-_frustum_halfsize* (float)(_screen_width)/ (float)(_screen_height), _frustum_halfsize* (float)(_screen_width)/ (float)(_screen_height), -_frustum_halfsize, _frustum_halfsize, _frustum_near, _frustum_far);

	update();

	_repere= new Repere(prog_repere);
}


ViewSystem::~ViewSystem() {
	delete _repere;
}


bool ViewSystem::mouse_motion(InputState * input_state) {
	if (_type== FREE_VIEW) {
		// translation
		if (input_state->_left_mouse) {
			move_target(glm::vec3((float)(input_state->_xrel)* LEFT_MOUSE_SENSIVITY, (float)(input_state->_yrel)* LEFT_MOUSE_SENSIVITY, 0.0f));
			return true;
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
void ViewSystem::move_target(const glm::vec3 & v) {
	_target.x+= v.x* sin(_phi)- v.y* cos(_phi);
	_target.y+= -v.x* cos(_phi)- v.y* sin(_phi);
	_target.z+= v.z;
	
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
}


// permet a certaines vues d'ajuster les params de ViewSystem en fonction d'un but
void ViewSystem::anim(const glm::vec3 & target, const glm::quat & rotation) {
	if (_type== FREE_VIEW) {

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
glm::vec2 ViewSystem::click2world(unsigned int x, unsigned int y, float z) {
	
	glm::vec4 ray_clip= glm::vec4(2.0f* (float)(x)/ (float)(_screen_width)- 1.0f, -2.0f* (float)(y)/ (float)(_screen_height)+ 1.0f, -1.0f, 1.0f);
	
	glm::vec4 ray_eye= glm::inverse(_camera2clip)* ray_clip;
	ray_eye= glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
	
	glm::vec3 ray_world= glm::vec3(glm::inverse(_world2camera)* ray_eye);
	ray_world= glm::normalize(ray_world);
	
	float lambda= (z- _eye.z)/ ray_world.z;
	glm::vec3 v= _eye+ lambda* ray_world;

	return glm::vec2(v.x, v.y);
}


// cf http://cgvr.cs.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
bool ViewSystem::contains_point(const glm::vec3 & pos) {
	glm::vec3 pos2= glm::vec3(_world2camera* glm::vec4(pos.x, pos.y, pos.z, 1.0f));

	// z pointe vers l'extérieur il faut donc l'inverser
	float y= -pos2.z* _frustum_halfsize/ _frustum_near;
	float x= y* (float)(_screen_width)/ (float)(_screen_height);

	x+= CONTAINS_POINT_TOLERANCE;
	y+= CONTAINS_POINT_TOLERANCE;

	/*cout << glm::to_string(pos2) << " ; " << _frustum_near << " ; " << _frustum_far << " ; " << x << " ; " << y << endl;
	cout << (-pos2.z> _frustum_near) << " : " << (-pos2.z< _frustum_far) << " : " << (pos2.x> -x) << " : " << (pos2.x< x) << " : " << (pos2.y> -y) << " : " << (pos2.y< y) << endl;*/

	return ((-pos2.z> _frustum_near) && (-pos2.z< _frustum_far) && (pos2.x> -x) && (pos2.x< x) && (pos2.y> -y) && (pos2.y< y));
}



// plusieurs versions de test d'intersection
bool ViewSystem::intersects_bbox(BBox * bbox) {
	return intersects_pts(bbox->_pts);
}


bool ViewSystem::intersects_aabb(AABB * aabb) {
	return intersects_pts(aabb->_pts);
}


bool ViewSystem::intersects_aabb(AABB * aabb, const glm::mat4 & model2world_matrix) {
	glm::vec3 pts[8];
	for (unsigned int i=0; i<8; ++i) {
		pts[i]= glm::vec3(model2world_matrix* glm::vec4(aabb->_pts[i], 1.0f));
	}

	return intersects_pts(pts);
}


// cf https://zeux.io/2009/01/31/view-frustum-culling-optimization-introduction
bool ViewSystem::intersects_pts(glm::vec3 * pts) {
	bool verbose= false;
	
	bool b= false;
	
	// near
	b= false;
	for (unsigned int i=0; i<8; ++i) {
		if (glm::dot(pts[i]- _center_near, _norm_near)> 0) {
			b= true;
			break;
		}
	}
	if (!b) {
		if (verbose) {
			cout << "near" << endl;
		}
		return false;
	}

	// far
	b= false;
	for (unsigned int i=0; i<8; ++i) {
		if (glm::dot(pts[i]- _center_far, _norm_far)> 0) {
			b= true;
			break;
		}
	}
	if (!b) {
		if (verbose) {
			cout << "far" << endl;
		}
		return false;
	}

	// right
	b= false;
	for (unsigned int i=0; i<8; ++i) {
		if (glm::dot(pts[i]- _eye, _norm_right)> 0) {
			b= true;
			break;
		}
	}
	if (!b) {
		if (verbose) {
			cout << "right" << endl;
		}
		return false;
	}

	// left
	b= false;
	for (unsigned int i=0; i<8; ++i) {
		if (glm::dot(pts[i]- _eye, _norm_left)> 0) {
			b= true;
			break;
		}
	}
	if (!b) {
		if (verbose) {
			cout << "left" << endl;
		}
		return false;
	}

	// top
	b= false;
	for (unsigned int i=0; i<8; ++i) {
		if (glm::dot(pts[i]- _eye, _norm_top)> 0) {
			b= true;
			break;
		}
	}
	if (!b) {
		if (verbose) {
			cout << "top" << endl;
		}
		return false;
	}

	// bottom
	b= false;
	for (unsigned int i=0; i<8; ++i) {
		if (glm::dot(pts[i]- _eye, _norm_bottom)> 0) {
			b= true;
			break;
		}
	}
	if (!b) {
		if (verbose) {
			cout << "bottom" << endl;
		}
		return false;
	}

	return true;
}

