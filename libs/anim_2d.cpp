#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <map>
#include <sstream>
#include <iomanip>
#include <dirent.h>

#include <OpenGL/gl3.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

// cf bug : https://stackoverflow.com/questions/14113923/rapidxml-print-header-has-undefined-methods
#include "rapidxml_ext.h"

#include "anim_2d.h"
#include "utile.h"


using namespace std;
using namespace rapidxml;


// Action ---------------------------------------------------------------------------
string Action::direction() {
	return _name.substr(0, _name.find("_"));
}


string Action::type() {
	return _name.substr(_name.find("_")+ 1);
}


void Action::print() {
	cout << "_name=" << _name << " ; _first_idx=" << _first_idx << " ; _n_idx=" << _n_idx << "\n";
	for (auto png : _pngs) {
		cout << png << "\n";
	}
}


// Object2D --------------------------------------------------------------------------------------------
Object2D::Object2D() {

}


Object2D::Object2D(glm::vec2 pos, glm::vec2 size, glm::vec2 footprint_offset, glm::vec2 footprint_size, bool is_static) : _velocity(glm::vec2(0.0f)), _is_static(is_static) {
	_aabb= new AABB_2D(pos, size);
	_footprint= new AABB_2D(glm::vec2(0.0f), glm::vec2(_aabb->_size.x* footprint_size.x, _aabb->_size.y* footprint_size.y));
	_footprint_offset= glm::vec2(_aabb->_size.x* footprint_offset.x, _aabb->_size.y* footprint_offset.y);
	update_footprint_pos();
}


Object2D::~Object2D() {
	delete _aabb;
}


void Object2D::anim(float elapsed_time) {
	if (!_is_static) {
		_aabb->_pos+= _velocity* elapsed_time;
		update_footprint_pos();
	}
}


void Object2D::update_footprint_pos() {
	_footprint->_pos= _aabb->_pos+ _footprint_offset;
}


void Object2D::set_aabb_pos(glm::vec2 pos) {
	_aabb->_pos= pos;
	update_footprint_pos();
}


void Object2D::set_footprint(glm::vec2 footprint_offset, glm::vec2 footprint_size) {
	_footprint_offset= glm::vec2(_aabb->_size.x* footprint_offset.x, _aabb->_size.y* footprint_offset.y);
	_footprint->_size= glm::vec2(_aabb->_size.x* footprint_size.x, _aabb->_size.y* footprint_size.y);
	update_footprint_pos();
}


// -------------------------------------------------------------------------------------------
bool anim_intersect_static(const Object2D * anim_obj, const Object2D * static_obj, const float time_step, glm::vec2 & contact_pt, glm::vec2 & contact_normal, float & contact_time) {
	//if (glm::length2(anim_obj->_velocity)< 1e-9f) {
	if ((anim_obj->_velocity.x== 0.0f) && (anim_obj->_velocity.y== 0.0f)) {
		return false;
	}

	AABB_2D expanded;
	expanded._pos= static_obj->_footprint->_pos- 0.5f* anim_obj->_footprint->_size;
	expanded._size= static_obj->_footprint->_size+ anim_obj->_footprint->_size;

	if (ray_intersects_aabb(anim_obj->_footprint->_pos+ 0.5f* anim_obj->_footprint->_size, time_step* anim_obj->_velocity, &expanded, contact_pt, contact_normal, contact_time)) {
		// le = du >= est important
		return ((contact_time>= 0.0f) && (contact_time< 1.0f));
		//return ((contact_time> -1e-3f) && (contact_time< 1.0f));
		//return ( ((contact_normal.y== 1.0f) && (contact_time> -1e-3f) && (contact_time< 1.0f)) || ((contact_time>= 0.0f) && (contact_time< 1.0f)) );
	}
	
	return false;
}


// Texture2D ----------------------------------------------------------------------------------------------------
Texture2D::Texture2D() {

}


Texture2D::Texture2D(GLuint prog_draw, ScreenGL * screengl) : _prog_draw(prog_draw), _screengl(screengl), _n_aabbs(0) {

}


Texture2D::~Texture2D() {

}


// StaticTexture ----------------------------------------------------------------------------------------------------
StaticTexture::StaticTexture() : Texture2D() {

}


StaticTexture::StaticTexture(GLuint prog_draw, string path, ScreenGL * screengl) : Texture2D(prog_draw, screengl), _alpha(1.0f) {
	glGenTextures(1, &_texture_id);
	glBindTexture(GL_TEXTURE_2D, _texture_id);
	
	SDL_Surface * surface= IMG_Load(path.c_str());
	if (!surface) {
		cout << "IMG_Load error :" << IMG_GetError() << endl;
		return;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);

	SDL_FreeSurface(surface);

	glActiveTexture(GL_TEXTURE0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S    , GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T    , GL_REPEAT);
	glActiveTexture(0);
	
	glBindTexture(GL_TEXTURE_2D, 0);

	_camera2clip= glm::ortho(-_screengl->_gl_width* 0.5f, _screengl->_gl_width* 0.5f, -_screengl->_gl_height* 0.5f, _screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
	_model2world= glm::mat4(1.0f);

	glUseProgram(_prog_draw);
	_camera2clip_loc= glGetUniformLocation(_prog_draw, "camera2clip_matrix");
	_model2world_loc= glGetUniformLocation(_prog_draw, "model2world_matrix");
	_tex_loc= glGetUniformLocation(_prog_draw, "tex");
	_alpha_loc= glGetUniformLocation(_prog_draw, "alpha");
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_tex_coord_loc= glGetAttribLocation(_prog_draw, "tex_coord_in");
	glUseProgram(0);

	glGenBuffers(1, &_vbo);

	// A FAIRE EVOLUER !
	_footprint_offset= glm::vec2(0.0f, 0.0f);
	_footprint_size= glm::vec2(1.0f, 1.0f);
	if (path.find("brick")!= string::npos) {
		_is_solid= true;
	}
	else {
		_is_solid= false;
	}
}


StaticTexture::~StaticTexture() {

}


void StaticTexture::draw() {
	if (_n_aabbs== 0) {
		return;
	}
	
	glActiveTexture(GL_TEXTURE0);

	glUseProgram(_prog_draw);
   	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBindTexture(GL_TEXTURE_2D, _texture_id);

	glUniform1i(_tex_loc, 0); //Sampler refers to texture unit 0
	glUniform1f(_alpha_loc, _alpha);
	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(_model2world_loc, 1, GL_FALSE, glm::value_ptr(_model2world));
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_tex_coord_loc);

	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
	glVertexAttribPointer(_tex_coord_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(3* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, 6* _n_aabbs);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_tex_coord_loc);

	glBindTexture(GL_TEXTURE, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void StaticTexture::update() {
	_n_aabbs= _static_characters.size();
	float vertices[30* _n_aabbs];
	for (unsigned int idx=0; idx<_n_aabbs; ++idx) {
		//glm::vec2 tex_coord= _static_characters[idx]->_static_obj->_aabb->_pt_max- _static_characters[idx]->_static_obj->_aabb->_pt_min;
		glm::vec2 tex_coord(1.0f, 1.0f);

		vertices[30* idx+ 0]= _static_characters[idx]->_static_obj->_aabb->_pos.x;
		vertices[30* idx+ 1]= _static_characters[idx]->_static_obj->_aabb->_pos.y+ _static_characters[idx]->_static_obj->_aabb->_size.y;
		vertices[30* idx+ 2]= _static_characters[idx]->_z;
		vertices[30* idx+ 3]= 0.0f;
		vertices[30* idx+ 4]= 0.0f;

		vertices[30* idx+ 5]= _static_characters[idx]->_static_obj->_aabb->_pos.x;
		vertices[30* idx+ 6]= _static_characters[idx]->_static_obj->_aabb->_pos.y;
		vertices[30* idx+ 7]= _static_characters[idx]->_z;
		vertices[30* idx+ 8]= 0.0f;
		vertices[30* idx+ 9]= tex_coord.y;

		vertices[30* idx+ 10]= _static_characters[idx]->_static_obj->_aabb->_pos.x+ _static_characters[idx]->_static_obj->_aabb->_size.x;
		vertices[30* idx+ 11]= _static_characters[idx]->_static_obj->_aabb->_pos.y;
		vertices[30* idx+ 12]= _static_characters[idx]->_z;
		vertices[30* idx+ 13]= tex_coord.x;
		vertices[30* idx+ 14]= tex_coord.y;

		vertices[30* idx+ 15]= _static_characters[idx]->_static_obj->_aabb->_pos.x;
		vertices[30* idx+ 16]= _static_characters[idx]->_static_obj->_aabb->_pos.y+ _static_characters[idx]->_static_obj->_aabb->_size.y;
		vertices[30* idx+ 17]= _static_characters[idx]->_z;
		vertices[30* idx+ 18]= 0.0f;
		vertices[30* idx+ 19]= 0.0f;

		vertices[30* idx+ 20]= _static_characters[idx]->_static_obj->_aabb->_pos.x+ _static_characters[idx]->_static_obj->_aabb->_size.x;
		vertices[30* idx+ 21]= _static_characters[idx]->_static_obj->_aabb->_pos.y;
		vertices[30* idx+ 22]= _static_characters[idx]->_z;
		vertices[30* idx+ 23]= tex_coord.x;
		vertices[30* idx+ 24]= tex_coord.y;

		vertices[30* idx+ 25]= _static_characters[idx]->_static_obj->_aabb->_pos.x+ _static_characters[idx]->_static_obj->_aabb->_size.x;
		vertices[30* idx+ 26]= _static_characters[idx]->_static_obj->_aabb->_pos.y+ _static_characters[idx]->_static_obj->_aabb->_size.y;
		vertices[30* idx+ 27]= _static_characters[idx]->_z;
		vertices[30* idx+ 28]= tex_coord.x;
		vertices[30* idx+ 29]= 0.0f;
	}
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, 30* _n_aabbs* sizeof(float), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// AnimTexture ---------------------------------------------------------------------------
AnimTexture::AnimTexture() : Texture2D() {

}


AnimTexture::AnimTexture(GLuint prog_draw, string path, ScreenGL * screengl) : Texture2D(prog_draw, screengl) {
	string root_pngs= path+ "/pngs";
	vector<string> l_dirs= list_files(root_pngs);
	unsigned int compt= 0;
	for (auto dir : l_dirs) {
		if (dir[0]!= '.') {
			//cout << dir << "\n";
			Action * action= new Action();
			action->_name= dir;
			action->_first_idx= compt;
			action->_n_idx= 0;

			// FAIRE EVOLUER -> XML
			if (dir.find("roll")!= string::npos) {
				action->_anim_time= 0.08f;
			}
			else {
				action->_anim_time= 0.13f;
			}

			// FAIRE EVOLUER -> XML
			if ((dir.find("crouch")!= string::npos) || (dir.find("roll")!= string::npos)) {
				action->_footprint_offset= glm::vec2(0.3f, 0.05f);
				action->_footprint_size= glm::vec2(0.4f, 0.5f);
			}
			else {
				action->_footprint_offset= glm::vec2(0.3f, 0.05f);
				action->_footprint_size= glm::vec2(0.4f, 0.9f);
			}

			vector<string> l_files= list_files(root_pngs+ "/"+ dir);
			sort(l_files.begin(), l_files.end());
			for (auto f : l_files) {
				if (f[0]!= '.') {
					//cout << f << "\n";
					action->_pngs.push_back(root_pngs+ "/"+ dir+ "/"+ f);
					action->_n_idx++;
					compt++;
				}
			}
			_actions.push_back(action);
		}
	}

	glGenTextures(1, &_texture_id);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);
	
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, ANIM_MODEL_SIZE, ANIM_MODEL_SIZE, compt, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	for (auto action : _actions) {
		//action.print();
		for (unsigned int i=0; i<action->_n_idx; ++i) {
			SDL_Surface * surface= IMG_Load(action->_pngs[i].c_str());
			if (!surface) {
				cout << "IMG_Load error :" << IMG_GetError() << endl;
				return;
			}

			// sais pas pourquoi mais GL_BGRA fonctionne mieux que GL_RGBA
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
							0,                          // mipmap number
							0, 0, action->_first_idx+ i, // xoffset, yoffset, zoffset
							ANIM_MODEL_SIZE, ANIM_MODEL_SIZE, 1,  // width, height, depth
							GL_BGRA,                    // format
							GL_UNSIGNED_BYTE,           // type
							surface->pixels);           // pointer to data

			SDL_FreeSurface(surface);
		}
	}

	glActiveTexture(GL_TEXTURE0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);
	glActiveTexture(0);
	
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	_camera2clip= glm::ortho(-_screengl->_gl_width* 0.5f, _screengl->_gl_width* 0.5f, -_screengl->_gl_height* 0.5f, _screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
	_model2world= glm::mat4(1.0f);

	glUseProgram(_prog_draw);
	_camera2clip_loc= glGetUniformLocation(_prog_draw, "camera2clip_matrix");
	_model2world_loc= glGetUniformLocation(_prog_draw, "model2world_matrix");
	_texture_array_loc= glGetUniformLocation(_prog_draw, "texture_array");
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_tex_coord_loc= glGetAttribLocation(_prog_draw, "tex_coord_in");
	_current_layer_loc= glGetAttribLocation(_prog_draw, "current_layer_in");
	glUseProgram(0);

	glGenBuffers(1, &_vbo);
}


AnimTexture::~AnimTexture() {
	for (auto action : _actions) {
		delete action;
	}
	_actions.clear();
}


void AnimTexture::draw() {
	if (_n_aabbs== 0) {
		return;
	}

	glActiveTexture(GL_TEXTURE0);

	glUseProgram(_prog_draw);
   	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);

	glUniform1i(_texture_array_loc, 0); //Sampler refers to texture unit 0
	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(_model2world_loc, 1, GL_FALSE, glm::value_ptr(_model2world));
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_tex_coord_loc);
	glEnableVertexAttribArray(_current_layer_loc);

	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_tex_coord_loc, 2, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));
	glVertexAttribPointer(_current_layer_loc, 1, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(5* sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, 6* _n_aabbs);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_tex_coord_loc);
	glDisableVertexAttribArray(_current_layer_loc);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void AnimTexture::update() {
	_n_aabbs= _anim_characters.size();
	float vertices[36* _n_aabbs];
	for (unsigned int idx=0; idx<_n_aabbs; ++idx) {
		//glm::vec2 tex_coord= _anim_characters[idx]->_anim_obj->_aabb->_pt_max- _anim_characters[idx]->_anim_obj->_aabb->_pt_min;
		glm::vec2 tex_coord(1.0f, 1.0f);
		float current_layer= (float)(_anim_characters[idx]->_current_action->_first_idx+ _anim_characters[idx]->_current_anim);

		vertices[36* idx+ 0]= _anim_characters[idx]->_anim_obj->_aabb->_pos.x;
		vertices[36* idx+ 1]= _anim_characters[idx]->_anim_obj->_aabb->_pos.y+ _anim_characters[idx]->_anim_obj->_aabb->_size.y;
		vertices[36* idx+ 2]= _anim_characters[idx]->_z;
		vertices[36* idx+ 3]= 0.0f;
		vertices[36* idx+ 4]= 0.0f;
		vertices[36* idx+ 5]= current_layer;

		vertices[36* idx+ 6]= _anim_characters[idx]->_anim_obj->_aabb->_pos.x;
		vertices[36* idx+ 7]= _anim_characters[idx]->_anim_obj->_aabb->_pos.y;
		vertices[36* idx+ 8]= _anim_characters[idx]->_z;
		vertices[36* idx+ 9]= 0.0f;
		vertices[36* idx+ 10]= tex_coord.y;
		vertices[36* idx+ 11]= current_layer;

		vertices[36* idx+ 12]= _anim_characters[idx]->_anim_obj->_aabb->_pos.x+ _anim_characters[idx]->_anim_obj->_aabb->_size.x;
		vertices[36* idx+ 13]= _anim_characters[idx]->_anim_obj->_aabb->_pos.y;
		vertices[36* idx+ 14]= _anim_characters[idx]->_z;
		vertices[36* idx+ 15]= tex_coord.x;
		vertices[36* idx+ 16]= tex_coord.y;
		vertices[36* idx+ 17]= current_layer;

		vertices[36* idx+ 18]= _anim_characters[idx]->_anim_obj->_aabb->_pos.x;
		vertices[36* idx+ 19]= _anim_characters[idx]->_anim_obj->_aabb->_pos.y+ _anim_characters[idx]->_anim_obj->_aabb->_size.y;
		vertices[36* idx+ 20]= _anim_characters[idx]->_z;
		vertices[36* idx+ 21]= 0.0f;
		vertices[36* idx+ 22]= 0.0f;
		vertices[36* idx+ 23]= current_layer;

		vertices[36* idx+ 24]= _anim_characters[idx]->_anim_obj->_aabb->_pos.x+ _anim_characters[idx]->_anim_obj->_aabb->_size.x;
		vertices[36* idx+ 25]= _anim_characters[idx]->_anim_obj->_aabb->_pos.y;
		vertices[36* idx+ 26]= _anim_characters[idx]->_z;
		vertices[36* idx+ 27]= tex_coord.x;
		vertices[36* idx+ 28]= tex_coord.y;
		vertices[36* idx+ 29]= current_layer;

		vertices[36* idx+ 30]= _anim_characters[idx]->_anim_obj->_aabb->_pos.x+ _anim_characters[idx]->_anim_obj->_aabb->_size.x;
		vertices[36* idx+ 31]= _anim_characters[idx]->_anim_obj->_aabb->_pos.y+ _anim_characters[idx]->_anim_obj->_aabb->_size.y;
		vertices[36* idx+ 32]= _anim_characters[idx]->_z;
		vertices[36* idx+ 33]= tex_coord.x;
		vertices[36* idx+ 34]= 0.0f;
		vertices[36* idx+ 35]= current_layer;
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, 36* _n_aabbs* sizeof(float), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// -------------------------------------------------------------------------------------------------------
Character2D::Character2D() {

}


Character2D::Character2D(Object2D * obj, Texture2D * texture, float z) :
	_obj(obj), _texture(texture), _z(z), _current_anim(0), _accumulated_time(0.0f),
	_left_pressed(false), _right_pressed(false), _down_pressed(false), _up_pressed(false), _lshift_pressed(false), _jump(false)
{
	_current_action= _anim_texture->_actions[0];
	//set_action("left_wait");
}


Character2D::~Character2D() {
	delete _anim_obj;
}


void Character2D::anim(float elapsed_time) {
	_accumulated_time+= elapsed_time;
	if (_accumulated_time>= _current_action->_anim_time) {
		_accumulated_time-= _current_action->_anim_time;
		_current_anim++;
		if (_current_anim>= _current_action->_n_idx) {
			_current_anim= 0;
		}
	}
}


void Character2D::update_velocity() {
	float vel_run= 9.0f;
	float vel_walk= 5.0f;
	float vel_roll= 9.0f;

	if (_left_pressed) {
		if (current_action()== "left_run") {
			_anim_obj->_velocity.x= -vel_run;
		}
		else if (current_action()== "left_walk") {
			_anim_obj->_velocity.x= -vel_walk;
		}
		else if ((current_action()== "left_jump") || (current_action()== "left_fall")) {
			if (_lshift_pressed) {
				_anim_obj->_velocity.x= -vel_run;
			}
			else {
				_anim_obj->_velocity.x= -vel_walk;
			}
		}
		else if (current_action()== "left_roll") {
			_anim_obj->_velocity.x= -vel_roll;
		}
		else {
			_anim_obj->_velocity.x*= 0.9f;
		}
	}
	else if (_right_pressed) {
		if (current_action()== "right_run") {
			_anim_obj->_velocity.x= vel_run;
		}
		else if (current_action()== "right_walk") {
			_anim_obj->_velocity.x= vel_walk;
		}
		else if ((current_action()== "right_jump") || (current_action()== "right_fall")) {
			if (_lshift_pressed) {
				_anim_obj->_velocity.x= vel_run;
			}
			else {
				_anim_obj->_velocity.x= vel_walk;
			}
		}
		else if (current_action()== "right_roll") {
			_anim_obj->_velocity.x= vel_roll;
		}
		else {
			_anim_obj->_velocity.x*= 0.9f;
		}
	}
	else {
		_anim_obj->_velocity.x*= 0.9f;
	}

	if (_jump) {
		_jump= false;
		if (_lshift_pressed) {
			_anim_obj->_velocity.y+= 25.0f;
		}
		else {
			_anim_obj->_velocity.y+= 20.0f;
		}
	}

	_anim_obj->_velocity.y-= 1.0f;
	if (_anim_obj->_velocity.y< -20.0f) {
		_anim_obj->_velocity.y= -20.0f;
	}
}


void Character2D::update_action() {
	if ((abs(_anim_obj->_velocity.x)< 0.1f) && (abs(_anim_obj->_velocity.y)< 0.1f)) {
		if ((current_action()== "left_walk") || (current_action()== "left_run") || (current_action()== "left_fall")) {
			set_action("left_wait");
		}
		else if ((current_action()== "right_walk") || (current_action()== "right_run") || (current_action()== "right_fall")) {
			set_action("right_wait");
		}
	}
	
	if (_anim_obj->_velocity.y< 0.0f) {
		if (current_action()== "left_jump") {
			set_action("left_fall");
		}
		else if (current_action()== "right_jump") {
			set_action("right_fall");
		}
	}
	else if (abs(_anim_obj->_velocity.y)< 0.1f) {
		if (current_action()== "left_fall") {
			if (_lshift_pressed) {
				set_action("left_run");
			}
			else {
				set_action("left_walk");
			}
		}
		else if (current_action()== "right_fall") {
			if (_lshift_pressed) {
				set_action("right_run");
			}
			else {
				set_action("right_walk");
			}
		}
	}
}


void Character2D::key_down(SDL_Keycode key) {
	if ((key== SDLK_LEFT) && (!_left_pressed)) {
		//cout << "left_pressed\n";
		_left_pressed= true;
		_right_pressed= false;
		if ((current_action()== "right_walk") || (current_action()== "right_run") || (current_type()== "wait") || (current_type()== "crouch")) {
			if (_lshift_pressed) {
				set_action("left_run");
			}
			else {
				set_action("left_walk");
			}
		}
		else if (current_action()== "right_jump") {
			set_action("left_jump");
		}
		else if (current_action()== "right_fall") {
			set_action("left_fall");
		}
	}
	else if ((key== SDLK_RIGHT) && (!_right_pressed)) {
		//cout << "right_pressed\n";
		_right_pressed= true;
		_left_pressed= false;
		if ((current_action()== "left_walk") || (current_action()== "left_run") || (current_type()== "wait") || (current_type()== "crouch")) {
			if (_lshift_pressed) {
				set_action("right_run");
			}
			else {
				set_action("right_walk");
			}
		}
		else if (current_action()== "left_jump") {
			set_action("right_jump");
		}
		else if (current_action()== "left_fall") {
			set_action("right_fall");
		}
	}
	else if ((key== SDLK_UP) && (!_up_pressed)) {
		//cout << "up_pressed\n";
		_up_pressed= true;
		if ((current_type()!= "jump") && (current_type()!= "fall")) {
			_jump= true;
			if (current_direction()== "left") {
				set_action("left_jump");
			}
			else {
				set_action("right_jump");
			}
		}
	}
	else if ((key== SDLK_DOWN) && (!_down_pressed)) {
		//cout << "down_pressed\n";
		_down_pressed= true;
		if ((current_action()== "left_walk") || (current_action()== "left_wait")) {
			set_action("left_crouch");
		}
		else if ((current_action()== "right_walk") || (current_action()== "right_wait")) {
			set_action("right_crouch");
		}
		else if (current_action()== "left_run") {
			set_action("left_roll");
		}
		else if (current_action()== "right_run") {
			set_action("right_roll");
		}
	}
	// BUG cherry keyboard
	//else if ((key== SDLK_LSHIFT) && (!_lshift_pressed)) {
	else if ((key== SDLK_a) && (!_lshift_pressed)) {
		//cout << "shift_pressed\n";
		_lshift_pressed= true;
		if (current_action()== "left_walk") {
			set_action("left_run");
		}
		else if (current_action()== "right_walk") {
			set_action("right_run");
		}
	}
}


void Character2D::key_up(SDL_Keycode key) {
	if (key== SDLK_LEFT) {
		_left_pressed= false;
	}
	else if (key== SDLK_RIGHT) {
		_right_pressed= false;
	}
	else if (key== SDLK_UP) {
		_up_pressed= false;
	}
	else if (key== SDLK_DOWN) {
		_down_pressed= false;
		if (current_direction()== "left") {
			if (_left_pressed) {
				if (_lshift_pressed) {
					set_action("left_run");
				}
				else {
					set_action("left_walk");
				}
			}
			else {
				set_action("left_wait");
			}
		}
		else if (current_direction()== "right") {
			if (_right_pressed) {
				if (_lshift_pressed) {
					set_action("right_run");
				}
				else {
					set_action("right_walk");
				}
			}
			else {
				set_action("right_wait");
			}
		}
	}
	// BUG cherry keyboard
	//else if (key== SDLK_LSHIFT) {
	else if (key== SDLK_a) {
		_lshift_pressed= false;
		if (current_action()== "left_run") {
			set_action("left_walk");
		}
		else if (current_action()== "right_run") {
			set_action("right_walk");
		}
	}
}


void Character2D::ia() {
	if (rand_int(0, 100)== 0) {
		if (current_direction()== "right") {
			key_up(SDLK_RIGHT);
			key_down(SDLK_LEFT);
		}
		else {
			key_up(SDLK_LEFT);
			key_down(SDLK_RIGHT);
		}
	}
	if (rand_int(0, 30)== 0) {
		key_up(SDLK_UP);
		key_down(SDLK_UP);
	}
}


void Character2D::set_action(unsigned int idx_action) {
	if (idx_action>= _anim_texture->_actions.size()) {
		cout << "set_action " << idx_action << " trop grand\n";
		return;
	}
	_current_action= _anim_texture->_actions[idx_action];
	_current_anim= 0;
	_anim_obj->set_footprint(_current_action->_footprint_offset, _current_action->_footprint_size);
}


void Character2D::set_action(string action_name) {
	int idx_action_ok= -1;
	for (int idx_action=0; idx_action<_anim_texture->_actions.size(); ++idx_action) {
		if (_anim_texture->_actions[idx_action]->_name== action_name) {
			idx_action_ok= idx_action;
			break;
		}
	}
	if (idx_action_ok< 0) {
		cout << "action non trouvee : " << action_name << "\n";
		return;
	}

	set_action(idx_action_ok);
}


string Character2D::current_action() {
	return _current_action->_name;
}


string Character2D::current_direction() {
	return _current_action->direction();
}


string Character2D::current_type() {
	return _current_action->type();
}


// Level -------------------------------------------------------------------------------------------------
Level::Level() {

}


Level::Level(GLuint prog_draw_anim, GLuint prog_draw_static, GLuint prog_draw_aabb, string path, ScreenGL * screengl) :
	_screengl(screengl)
{
	ifstream xml_file(path+ "/levels/level_01.xml");
	stringstream buffer;
	buffer << xml_file.rdbuf();
	xml_file.close();
	string xml_content(buffer.str());
	xml_document<> doc;
	doc.parse<0>(&xml_content[0]);
	xml_node<> * root_node= doc.first_node();

	xml_node<> * w_node= root_node->first_node("w");
	_w= stoi(w_node->value());
	xml_node<> * h_node= root_node->first_node("h");
	_h= stoi(h_node->value());
	xml_node<> * data_node= root_node->first_node("data");
	string data= data_node->value();
	trim(data);

	stringstream ss(data);
	string line;
	char level_data[_w* _h];
	unsigned int idx= 0;
	while (getline(ss, line, '\n')) {
		trim(line);
		for(char & c : line) {
			level_data[idx++]= c;
		}
	}

	_block_w= _screengl->_gl_width/ (float)(_w);
	_block_h= _screengl->_gl_height/ (float)(_h);

	_static_textures.push_back(new StaticTexture(prog_draw_static, path+ "/static_textures/brick.png", screengl));
	_static_textures.push_back(new StaticTexture(prog_draw_static, path+ "/static_textures/grass.png", screengl));
	_static_textures.push_back(new StaticTexture(prog_draw_static, path+ "/static_textures/tree.png", screengl));
	_static_textures.push_back(new StaticTexture(prog_draw_static, path+ "/static_textures/hill.png", screengl));
	_static_textures.push_back(new StaticTexture(prog_draw_static, path+ "/static_textures/sky.png", screengl));

	_anim_textures.push_back(new AnimTexture(prog_draw_anim, path+ "/anim_textures/modele_1", screengl));
	//_anim_textures.push_back(new AnimTexture(prog_draw_anim, path+ "/anim_textures/modele_2", screengl));

	add_static_character(2, glm::vec2(-5.0f, -5.0f), glm::vec2(5.0f, 5.0f), 0.3f);
	add_static_character(3, glm::vec2(-0.5f* _screengl->_gl_width, -0.5f* _screengl->_gl_height), glm::vec2(_screengl->_gl_width, _screengl->_gl_height), 0.2f);
	add_static_character(4, glm::vec2(-0.5f* _screengl->_gl_width, -0.5f* _screengl->_gl_height), glm::vec2(_screengl->_gl_width, _screengl->_gl_height), 0.1f);

	for (unsigned int col=0; col<_w; ++col) {
		for (unsigned int row=0; row<_h; ++row) {
			glm::vec2 pos(-0.5f* _screengl->_gl_width+ (float)(col)* _block_w, -0.5f* _screengl->_gl_height+ (float)(row)* _block_h);
			glm::vec2 size= glm::vec2(_block_w, _block_h);
			if (level_data[col+ _w* (_h- row- 1)]== '*') {
				add_static_character(0, pos, size, 1.0f);
				if ((row!= _h- 1) && (level_data[col+ _w* (_h- row- 2)]!= '*')) {
					add_static_character(1, pos+ glm::vec2(0.0f, size.y), size, 1.5f);
				}
			}
			else if (level_data[col+ _w* (_h- row- 1)]== 'x') {
				add_anim_character(0, pos, size* 2.0f, 0.5f);
			}
		}
	}
}


Level::~Level() {	
	for (auto static_character : _static_characters) {
		delete static_character;
	}
	_static_characters.clear();

	for (auto anim_character : _anim_characters) {
		delete anim_character;
	}
	_anim_characters.clear();

	for (auto static_texture : _static_textures) {
		delete static_texture;
	}
	_static_textures.clear();
	
	for (auto anim_texture : _anim_textures) {
		delete anim_texture;
	}
	_anim_textures.clear();
}


void Level::add_static_character(unsigned int idx_texture, glm::vec2 pos, glm::vec2 size, float z) {
	StaticObj * static_obj= new StaticObj(pos, size, _static_textures[idx_texture]->_footprint_offset, _static_textures[idx_texture]->_footprint_size);
	StaticCharacter * static_character= new StaticCharacter(static_obj, _static_textures[idx_texture], z);
	_static_characters.push_back(static_character);
	_static_textures[idx_texture]->_static_characters.push_back(static_character);
	_static_textures[idx_texture]->update();
}


void Level::add_anim_character(unsigned int idx_texture, glm::vec2 pos, glm::vec2 size, float z) {
	AnimObj* anim_obj= new AnimObj(pos, size, _anim_textures[idx_texture]->_actions[0]->_footprint_offset, _anim_textures[idx_texture]->_actions[0]->_footprint_size);
	Character2D * anim_character= new Character2D(anim_obj, _anim_textures[idx_texture], z);
	_anim_characters.push_back(anim_character);
	_anim_textures[idx_texture]->_anim_characters.push_back(anim_character);
	_anim_textures[idx_texture]->update();
}


void Level::delete_static_character(unsigned int idx_static_character) {
	StaticTexture * static_texture= _static_characters[idx_static_character]->_static_texture;
	static_texture->_static_characters.erase(remove(static_texture->_static_characters.begin(), static_texture->_static_characters.end(), _static_characters[idx_static_character]), static_texture->_static_characters.end());
	delete _static_characters[idx_static_character];
	_static_characters.erase(_static_characters.begin()+ idx_static_character);
}


void Level::draw() {
	for (auto static_texture : _static_textures) {
		static_texture->draw();
	}

	for (auto anim_texture : _anim_textures) {
		anim_texture->draw();
	}
}


void Level::anim(float elapsed_time) {
	glm::vec2 contact_pt(0.0f);
	glm::vec2 contact_normal(0.0f);
	float contact_time= 0.0f;

	for (unsigned int idx_anim=0; idx_anim<_anim_characters.size(); ++idx_anim) {
		Character2D * anim_character= _anim_characters[idx_anim];
		AnimObj * anim_obj= anim_character->_anim_obj;
		
		if (idx_anim!= 0) {
			anim_character->ia();
		}

		anim_character->update_velocity();

		vector<unsigned int> delete_statics;
		for (unsigned int idx_static=0; idx_static<_static_characters.size(); ++idx_static) {
			StaticCharacter * static_character= _static_characters[idx_static];

			if (!static_character->_static_texture->_is_solid) {
				continue;
			}

			if (anim_intersect_static(anim_obj, static_character->_static_obj, elapsed_time, contact_pt, contact_normal, contact_time)) {
				glm::vec2 correction= (1.0f- contact_time)* glm::vec2(abs(anim_obj->_velocity.x)* contact_normal.x, abs(anim_obj->_velocity.y)* contact_normal.y);
				anim_obj->_velocity+= correction* 1.1f;

				if (contact_normal.y< 0.0f) {
					delete_statics.push_back(idx_static);
				}
			}
		}
		reverse(delete_statics.begin(), delete_statics.end());
		for (auto idx_delete : delete_statics) {
			delete_static_character(idx_delete);
		}

		anim_character->update_action();
		anim_obj->anim(elapsed_time);
		anim_character->anim(elapsed_time);
	}

	for (auto static_texture : _static_textures) {
		static_texture->update();
	}

	for (auto anim_texture : _anim_textures) {
		anim_texture->update();
	}
}


bool Level::key_down(InputState * input_state, SDL_Keycode key) {
	if ((key== SDLK_DOWN) || (key== SDLK_UP) || (key== SDLK_LEFT) || (key== SDLK_RIGHT) || (key== SDLK_a)) {
		_anim_characters[0]->key_down(key);
		return true;
	}

	return false;
}


bool Level::key_up(InputState * input_state, SDL_Keycode key) {
	if ((key== SDLK_DOWN) || (key== SDLK_UP) || (key== SDLK_LEFT) || (key== SDLK_RIGHT) || (key== SDLK_a)) {
		_anim_characters[0]->key_up(key);
		return true;
	}

	return false;
}


// LevelDebug ---------------------------------------------------------------------------------------------
LevelDebug::LevelDebug() {

}


LevelDebug::LevelDebug(GLuint prog_draw_aabb, Level * level, ScreenGL * screengl) :
	_prog_draw(prog_draw_aabb), _level(level), _screengl(screengl), _n_aabbs(0), _draw_aabb(false), _draw_footprint(false)
{
	_camera2clip= glm::ortho(-_screengl->_gl_width* 0.5f, _screengl->_gl_width* 0.5f, -_screengl->_gl_height* 0.5f, _screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
	_model2world= glm::mat4(1.0f);

	glUseProgram(_prog_draw);
	_camera2clip_loc= glGetUniformLocation(_prog_draw, "camera2clip_matrix");
	_model2world_loc= glGetUniformLocation(_prog_draw, "model2world_matrix");
	_z_loc= glGetUniformLocation(_prog_draw, "z");
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
	_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	glUseProgram(0);

	glGenBuffers(1, &_vbo);

}


LevelDebug::~LevelDebug() {

}


void LevelDebug::draw() {
	if (_n_aabbs== 0) {
		return;
	}

	glUseProgram(_prog_draw);
   	glBindBuffer(GL_ARRAY_BUFFER, _vbo);

	glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, glm::value_ptr(_camera2clip));
	glUniformMatrix4fv(_model2world_loc, 1, GL_FALSE, glm::value_ptr(_model2world));
	glUniform1f(_z_loc, Z_FAR- 0.1f); // devant tout le reste
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_color_loc);

	glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
	glVertexAttribPointer(_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

	glDrawArrays(GL_LINES, 0, 8* _n_aabbs);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void LevelDebug::update() {
	glm::vec3 static_aabb_color(1.0f, 0.0f, 0.0f);
	glm::vec3 static_footprint_color(0.5f, 0.2f, 0.2f);
	glm::vec3 anim_aabb_color(0.0f, 1.0f, 0.0f);
	glm::vec3 anim_footprint_color(0.5f, 1.0f, 0.5f);

	unsigned int n_static= _level->_static_characters.size();
	unsigned int n_anim= _level->_anim_characters.size();

	_n_aabbs= (n_static+ n_anim)* 2;
	float vertices[_n_aabbs* 40];
	for (unsigned int i=0; i<_n_aabbs* 40; ++i) {
		vertices[i]= 0.0f;
	}

	if (_draw_aabb) {
		for (unsigned int idx=0; idx<n_static; ++idx) {
			vertices[40* idx+ 0]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.x;
			vertices[40* idx+ 1]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.y;
			vertices[40* idx+ 5]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.x+ _level->_static_characters[idx]->_static_obj->_aabb->_size.x;
			vertices[40* idx+ 6]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.y;

			vertices[40* idx+ 10]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.x+ _level->_static_characters[idx]->_static_obj->_aabb->_size.x;
			vertices[40* idx+ 11]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.y;
			vertices[40* idx+ 15]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.x+ _level->_static_characters[idx]->_static_obj->_aabb->_size.x;
			vertices[40* idx+ 16]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.y+ _level->_static_characters[idx]->_static_obj->_aabb->_size.y;

			vertices[40* idx+ 20]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.x+ _level->_static_characters[idx]->_static_obj->_aabb->_size.x;
			vertices[40* idx+ 21]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.y+ _level->_static_characters[idx]->_static_obj->_aabb->_size.y;
			vertices[40* idx+ 25]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.x;
			vertices[40* idx+ 26]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.y+ _level->_static_characters[idx]->_static_obj->_aabb->_size.y;

			vertices[40* idx+ 30]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.x;
			vertices[40* idx+ 31]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.y+ _level->_static_characters[idx]->_static_obj->_aabb->_size.y;
			vertices[40* idx+ 35]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.x;
			vertices[40* idx+ 36]= _level->_static_characters[idx]->_static_obj->_aabb->_pos.y;

			for (unsigned int i=0; i<8; ++i) {
				vertices[40* idx+ 5* i+ 2]= static_aabb_color.x;
				vertices[40* idx+ 5* i+ 3]= static_aabb_color.y;
				vertices[40* idx+ 5* i+ 4]= static_aabb_color.z;
			}
		}
	}
	
	if (_draw_footprint) {
		for (unsigned int idx=0; idx<n_static; ++idx) {
			vertices[n_static* 40+ 40* idx+ 0]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.x;
			vertices[n_static* 40+ 40* idx+ 1]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.y;
			vertices[n_static* 40+ 40* idx+ 5]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.x+ _level->_static_characters[idx]->_static_obj->_footprint->_size.x;
			vertices[n_static* 40+ 40* idx+ 6]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.y;

			vertices[n_static* 40+ 40* idx+ 10]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.x+ _level->_static_characters[idx]->_static_obj->_footprint->_size.x;
			vertices[n_static* 40+ 40* idx+ 11]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.y;
			vertices[n_static* 40+ 40* idx+ 15]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.x+ _level->_static_characters[idx]->_static_obj->_footprint->_size.x;
			vertices[n_static* 40+ 40* idx+ 16]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.y+ _level->_static_characters[idx]->_static_obj->_footprint->_size.y;

			vertices[n_static* 40+ 40* idx+ 20]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.x+ _level->_static_characters[idx]->_static_obj->_footprint->_size.x;
			vertices[n_static* 40+ 40* idx+ 21]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.y+ _level->_static_characters[idx]->_static_obj->_footprint->_size.y;
			vertices[n_static* 40+ 40* idx+ 25]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.x;
			vertices[n_static* 40+ 40* idx+ 26]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.y+ _level->_static_characters[idx]->_static_obj->_footprint->_size.y;

			vertices[n_static* 40+ 40* idx+ 30]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.x;
			vertices[n_static* 40+ 40* idx+ 31]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.y+ _level->_static_characters[idx]->_static_obj->_footprint->_size.y;
			vertices[n_static* 40+ 40* idx+ 35]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.x;
			vertices[n_static* 40+ 40* idx+ 36]= _level->_static_characters[idx]->_static_obj->_footprint->_pos.y;

			for (unsigned int i=0; i<8; ++i) {
				vertices[n_static* 40+ 40* idx+ 5* i+ 2]= static_footprint_color.x;
				vertices[n_static* 40+ 40* idx+ 5* i+ 3]= static_footprint_color.y;
				vertices[n_static* 40+ 40* idx+ 5* i+ 4]= static_footprint_color.z;
			}
		}
	}

	if (_draw_aabb) {
		for (unsigned int idx=0; idx<n_anim; ++idx) {
			vertices[n_static* 80+ 40* idx+ 0]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.x;
			vertices[n_static* 80+ 40* idx+ 1]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.y;
			vertices[n_static* 80+ 40* idx+ 5]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.x+ _level->_anim_characters[idx]->_anim_obj->_aabb->_size.x;
			vertices[n_static* 80+ 40* idx+ 6]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.y;

			vertices[n_static* 80+ 40* idx+ 10]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.x+ _level->_anim_characters[idx]->_anim_obj->_aabb->_size.x;
			vertices[n_static* 80+ 40* idx+ 11]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.y;
			vertices[n_static* 80+ 40* idx+ 15]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.x+ _level->_anim_characters[idx]->_anim_obj->_aabb->_size.x;
			vertices[n_static* 80+ 40* idx+ 16]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.y+ _level->_anim_characters[idx]->_anim_obj->_aabb->_size.y;

			vertices[n_static* 80+ 40* idx+ 20]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.x+ _level->_anim_characters[idx]->_anim_obj->_aabb->_size.x;
			vertices[n_static* 80+ 40* idx+ 21]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.y+ _level->_anim_characters[idx]->_anim_obj->_aabb->_size.y;
			vertices[n_static* 80+ 40* idx+ 25]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.x;
			vertices[n_static* 80+ 40* idx+ 26]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.y+ _level->_anim_characters[idx]->_anim_obj->_aabb->_size.y;

			vertices[n_static* 80+ 40* idx+ 30]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.x;
			vertices[n_static* 80+ 40* idx+ 31]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.y+ _level->_anim_characters[idx]->_anim_obj->_aabb->_size.y;
			vertices[n_static* 80+ 40* idx+ 35]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.x;
			vertices[n_static* 80+ 40* idx+ 36]= _level->_anim_characters[idx]->_anim_obj->_aabb->_pos.y;

			for (unsigned int i=0; i<8; ++i) {
				vertices[n_static* 80+ 40* idx+ 5* i+ 2]= anim_aabb_color.x;
				vertices[n_static* 80+ 40* idx+ 5* i+ 3]= anim_aabb_color.y;
				vertices[n_static* 80+ 40* idx+ 5* i+ 4]= anim_aabb_color.z;
			}
		}
	}

	if (_draw_footprint) {
		for (unsigned int idx=0; idx<n_anim; ++idx) {
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 0]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.x;
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 1]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.y;
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 5]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.x+ _level->_anim_characters[idx]->_anim_obj->_footprint->_size.x;
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 6]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.y;

			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 10]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.x+ _level->_anim_characters[idx]->_anim_obj->_footprint->_size.x;
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 11]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.y;
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 15]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.x+ _level->_anim_characters[idx]->_anim_obj->_footprint->_size.x;
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 16]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.y+ _level->_anim_characters[idx]->_anim_obj->_footprint->_size.y;

			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 20]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.x+ _level->_anim_characters[idx]->_anim_obj->_footprint->_size.x;
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 21]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.y+ _level->_anim_characters[idx]->_anim_obj->_footprint->_size.y;
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 25]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.x;
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 26]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.y+ _level->_anim_characters[idx]->_anim_obj->_footprint->_size.y;

			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 30]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.x;
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 31]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.y+ _level->_anim_characters[idx]->_anim_obj->_footprint->_size.y;
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 35]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.x;
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 36]= _level->_anim_characters[idx]->_anim_obj->_footprint->_pos.y;

			for (unsigned int i=0; i<8; ++i) {
				vertices[n_static* 80+ n_anim* 40+ 40* idx+ 5* i+ 2]= anim_footprint_color.x;
				vertices[n_static* 80+ n_anim* 40+ 40* idx+ 5* i+ 3]= anim_footprint_color.y;
				vertices[n_static* 80+ n_anim* 40+ 40* idx+ 5* i+ 4]= anim_footprint_color.z;
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, 40* _n_aabbs* sizeof(float), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool LevelDebug::key_down(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_d) {
		_draw_aabb= !_draw_aabb;
		return true;
	}
	else if (key== SDLK_f) {
		_draw_footprint= !_draw_footprint;
		return true;
	}

	return false;
}

