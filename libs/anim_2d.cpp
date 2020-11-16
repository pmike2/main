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


ObjectPhysics str2physics(string s) {
	if (s== "STATIC_DESTRUCTIBLE") {
		return STATIC_DESTRUCTIBLE;
	}
	else if (s== "STATIC_INDESTRUCTIBLE") {
		return STATIC_INDESTRUCTIBLE;
	}
	else if (s== "STATIC_UNSOLID") {
		return STATIC_UNSOLID;
	}
	else if (s== "FALLING") {
		return FALLING;
	}
	else if (s== "CHECKPOINT_SOLID") {
		return CHECKPOINT_SOLID;
	}
	else if (s== "CHECKPOINT_UNSOLID") {
		return CHECKPOINT_UNSOLID;
	}
	cout << "physics non trouvee\n";
	return STATIC_DESTRUCTIBLE;
}

// CheckPoint ------------------------------------------------------------------------------------------
CheckPoint::CheckPoint() {

}


CheckPoint::CheckPoint(glm::vec2 pos, float velocity) : _pos(pos), _velocity(velocity) {

}


CheckPoint::~CheckPoint() {

}


// Object2D --------------------------------------------------------------------------------------------
Object2D::Object2D() {

}


Object2D::Object2D(glm::vec2 pos, glm::vec2 size, AABB_2D * footprint, ObjectPhysics physics, vector<CheckPoint> checkpoints) :
	_velocity(glm::vec2(0.0f)), _physics(physics), _idx_checkpoint(0), _referential(nullptr)
{
	_aabb= new AABB_2D(pos, size);
	_footprint= new AABB_2D(glm::vec2(0.0f), glm::vec2(_aabb->_size.x* footprint->_size.x, _aabb->_size.y* footprint->_size.y));
	_footprint_offset= glm::vec2(_aabb->_size.x* footprint->_pos.x, _aabb->_size.y* footprint->_pos.y);
	update_footprint_pos();

	for (auto checkpoint : checkpoints) {
		_checkpoints.push_back(new CheckPoint(checkpoint._pos, checkpoint._velocity));
	}
	if ( (((_physics== CHECKPOINT_SOLID) || (_physics== CHECKPOINT_UNSOLID)) && (!_checkpoints.size())) || ((_physics!= CHECKPOINT_SOLID) && (_physics!= CHECKPOINT_UNSOLID) && (_checkpoints.size())) ) {
		cout << "erreur object checkpoint\n";
	}
}


Object2D::Object2D(const Object2D & obj) {
	_aabb= new AABB_2D(*obj._aabb);
	_footprint= new AABB_2D(*obj._footprint);
	_footprint_offset= glm::vec2(obj._footprint_offset);
	_velocity= glm::vec2(obj._velocity);
	_physics= obj._physics;
	_idx_checkpoint= obj._idx_checkpoint;
	for (auto checkpoint : obj._checkpoints) {
		_checkpoints.push_back(new CheckPoint(checkpoint->_pos, checkpoint->_velocity));
	}
}


Object2D::~Object2D() {
	delete _aabb;
	delete _footprint;
	for (auto checkpoint : _checkpoints) {
		delete checkpoint;
	}
	_checkpoints.clear();
}


void Object2D::update_pos(float elapsed_time) {
	if ((_physics!= STATIC_DESTRUCTIBLE) && (_physics!= STATIC_INDESTRUCTIBLE) && (_physics!= STATIC_UNSOLID)) {
		_aabb->_pos+= _velocity* elapsed_time;
		update_footprint_pos();
	}
	if ((_physics== CHECKPOINT_SOLID) || (_physics== CHECKPOINT_UNSOLID)) {
		if (glm::distance2(_checkpoints[_idx_checkpoint]->_pos, _aabb->_pos)< 0.1f) {
			_idx_checkpoint++;
			if (_idx_checkpoint>= _checkpoints.size()) {
				_idx_checkpoint= 0;
			}
		}
	}
}


void Object2D::update_velocity() {
	if (_physics== FALLING) {
		_velocity.y-= 1.0f;
		if (_velocity.y< -20.0f) {
			_velocity.y= -20.0f;
		}
	}
	else if ((_physics== CHECKPOINT_SOLID) || (_physics== CHECKPOINT_UNSOLID)) {
		if (glm::distance2(_checkpoints[_idx_checkpoint]->_pos, _aabb->_pos)> 0.1f) {
			glm::vec2 direction= _checkpoints[_idx_checkpoint]->_pos- _aabb->_pos;
			_velocity= glm::normalize(direction)* _checkpoints[_idx_checkpoint]->_velocity;
		}
	}
}


void Object2D::update_footprint_pos() {
	_footprint->_pos= _aabb->_pos+ _footprint_offset;
}


void Object2D::set_aabb_pos(glm::vec2 pos) {
	_aabb->_pos= pos;
	update_footprint_pos();
}


void Object2D::set_footprint(AABB_2D * footprint) {
	_footprint_offset= glm::vec2(_aabb->_size.x* footprint->_pos.x, _aabb->_size.y* footprint->_pos.y);
	_footprint->_size= glm::vec2(_aabb->_size.x* footprint->_size.x, _aabb->_size.y* footprint->_size.y);
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

	if (ray_intersects_aabb(anim_obj->_footprint->center(), time_step* anim_obj->_velocity, &expanded, contact_pt, contact_normal, contact_time)) {
		// le = du >= est important
		return ((contact_time>= 0.0f) && (contact_time< 1.0f));
		//return ((contact_time> -1e-3f) && (contact_time< 1.0f));
		//return ( ((contact_normal.y== 1.0f) && (contact_time> -1e-3f) && (contact_time< 1.0f)) || ((contact_time>= 0.0f) && (contact_time< 1.0f)) );
	}
	
	return false;
}


// Action ---------------------------------------------------------------------------
Action::Action() : _name(""), _first_idx(0), _n_idx(0), _anim_time(0.0f) {
	_footprint= new AABB_2D();
}


Action::~Action() {
	delete _footprint;
}


void Action::print() {
	cout << "_name=" << _name << " ; _first_idx=" << _first_idx << " ; _n_idx=" << _n_idx << "\n";
	for (auto png : _pngs) {
		cout << png << "\n";
	}
}


// Texture2D ----------------------------------------------------------------------------------------------------
Texture2D::Texture2D() {

}


Texture2D::Texture2D(GLuint prog_draw, string path, ScreenGL * screengl, ObjectPhysics physics) : _prog_draw(prog_draw), _screengl(screengl), _n_aabbs(0), _physics(physics) {
	_name= basename(path);
}


Texture2D::~Texture2D() {
	for (auto action : _actions) {
		delete action;
	}
	_actions.clear();
}


void Texture2D::set_model2world(glm::mat4 model2world) {
	_model2world= model2world;
}


Action * Texture2D::get_action(string action_name) {
	for (auto action : _actions) {
		if (action->_name== action_name) {
			return action;
		}
	}
	cout << "action non trouvee : " << action_name << "\n";
	return nullptr;
}


// StaticTexture ----------------------------------------------------------------------------------------------------
StaticTexture::StaticTexture() : Texture2D() {

}


StaticTexture::StaticTexture(GLuint prog_draw, string path, ScreenGL * screengl, ObjectPhysics physics) : Texture2D(prog_draw, path, screengl, physics), _alpha(1.0f) {
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

	Action * action= new Action();
	action->_name= "static_action";
	_actions.push_back(action);
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
	_n_aabbs= _characters.size();
	float vertices[30* _n_aabbs];
	for (unsigned int idx=0; idx<_n_aabbs; ++idx) {
		glm::vec2 tex_coord(1.0f, 1.0f);

		vertices[30* idx+ 0]= _characters[idx]->_obj->_aabb->_pos.x;
		vertices[30* idx+ 1]= _characters[idx]->_obj->_aabb->_pos.y+ _characters[idx]->_obj->_aabb->_size.y;
		vertices[30* idx+ 2]= _characters[idx]->_z;
		vertices[30* idx+ 3]= 0.0f;
		vertices[30* idx+ 4]= 0.0f;

		vertices[30* idx+ 5]= _characters[idx]->_obj->_aabb->_pos.x;
		vertices[30* idx+ 6]= _characters[idx]->_obj->_aabb->_pos.y;
		vertices[30* idx+ 7]= _characters[idx]->_z;
		vertices[30* idx+ 8]= 0.0f;
		vertices[30* idx+ 9]= tex_coord.y;

		vertices[30* idx+ 10]= _characters[idx]->_obj->_aabb->_pos.x+ _characters[idx]->_obj->_aabb->_size.x;
		vertices[30* idx+ 11]= _characters[idx]->_obj->_aabb->_pos.y;
		vertices[30* idx+ 12]= _characters[idx]->_z;
		vertices[30* idx+ 13]= tex_coord.x;
		vertices[30* idx+ 14]= tex_coord.y;

		vertices[30* idx+ 15]= _characters[idx]->_obj->_aabb->_pos.x;
		vertices[30* idx+ 16]= _characters[idx]->_obj->_aabb->_pos.y+ _characters[idx]->_obj->_aabb->_size.y;
		vertices[30* idx+ 17]= _characters[idx]->_z;
		vertices[30* idx+ 18]= 0.0f;
		vertices[30* idx+ 19]= 0.0f;

		vertices[30* idx+ 20]= _characters[idx]->_obj->_aabb->_pos.x+ _characters[idx]->_obj->_aabb->_size.x;
		vertices[30* idx+ 21]= _characters[idx]->_obj->_aabb->_pos.y;
		vertices[30* idx+ 22]= _characters[idx]->_z;
		vertices[30* idx+ 23]= tex_coord.x;
		vertices[30* idx+ 24]= tex_coord.y;

		vertices[30* idx+ 25]= _characters[idx]->_obj->_aabb->_pos.x+ _characters[idx]->_obj->_aabb->_size.x;
		vertices[30* idx+ 26]= _characters[idx]->_obj->_aabb->_pos.y+ _characters[idx]->_obj->_aabb->_size.y;
		vertices[30* idx+ 27]= _characters[idx]->_z;
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


AnimTexture::AnimTexture(GLuint prog_draw, string path, ScreenGL * screengl, ObjectPhysics physics) : Texture2D(prog_draw, path, screengl, physics) {
	string root_pngs= path+ "/pngs";
	vector<string> l_dirs= list_files(root_pngs);
	unsigned int compt= 0;
	for (auto dir : l_dirs) {
		if (dir[0]!= '.') {
			//cout << dir << "\n";
			Action * action= new Action();
			action->_name= dir;
			action->_first_idx= compt;
			//action->_n_idx= 0;

			// FAIRE EVOLUER -> XML
			/*if (dir.find("roll")!= string::npos) {
				action->_anim_time= 0.08f;
			}
			else {
				action->_anim_time= 0.13f;
			}*/

			// FAIRE EVOLUER -> XML
			/*if ((dir.find("crouch")!= string::npos) || (dir.find("roll")!= string::npos)) {
				action->_footprint_offset= glm::vec2(0.3f, 0.05f);
				action->_footprint_size= glm::vec2(0.4f, 0.5f);
			}
			else {
				action->_footprint_offset= glm::vec2(0.3f, 0.05f);
				action->_footprint_size= glm::vec2(0.4f, 0.9f);
			}*/

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
	_n_aabbs= _characters.size();
	float vertices[36* _n_aabbs];
	for (unsigned int idx=0; idx<_n_aabbs; ++idx) {
		glm::vec2 tex_coord(1.0f, 1.0f);
		AnimatedCharacter2D * anim_character= dynamic_cast<AnimatedCharacter2D *>(_characters[idx]);
		float current_layer= (float)(anim_character->_current_action->_first_idx+ anim_character->_current_anim);

		vertices[36* idx+ 0]= anim_character->_obj->_aabb->_pos.x;
		vertices[36* idx+ 1]= anim_character->_obj->_aabb->_pos.y+ anim_character->_obj->_aabb->_size.y;
		vertices[36* idx+ 2]= anim_character->_z;
		vertices[36* idx+ 3]= 0.0f;
		vertices[36* idx+ 4]= 0.0f;
		vertices[36* idx+ 5]= current_layer;

		vertices[36* idx+ 6]= anim_character->_obj->_aabb->_pos.x;
		vertices[36* idx+ 7]= anim_character->_obj->_aabb->_pos.y;
		vertices[36* idx+ 8]= anim_character->_z;
		vertices[36* idx+ 9]= 0.0f;
		vertices[36* idx+ 10]= tex_coord.y;
		vertices[36* idx+ 11]= current_layer;

		vertices[36* idx+ 12]= anim_character->_obj->_aabb->_pos.x+ anim_character->_obj->_aabb->_size.x;
		vertices[36* idx+ 13]= anim_character->_obj->_aabb->_pos.y;
		vertices[36* idx+ 14]= anim_character->_z;
		vertices[36* idx+ 15]= tex_coord.x;
		vertices[36* idx+ 16]= tex_coord.y;
		vertices[36* idx+ 17]= current_layer;

		vertices[36* idx+ 18]= anim_character->_obj->_aabb->_pos.x;
		vertices[36* idx+ 19]= anim_character->_obj->_aabb->_pos.y+ anim_character->_obj->_aabb->_size.y;
		vertices[36* idx+ 20]= anim_character->_z;
		vertices[36* idx+ 21]= 0.0f;
		vertices[36* idx+ 22]= 0.0f;
		vertices[36* idx+ 23]= current_layer;

		vertices[36* idx+ 24]= anim_character->_obj->_aabb->_pos.x+ anim_character->_obj->_aabb->_size.x;
		vertices[36* idx+ 25]= anim_character->_obj->_aabb->_pos.y;
		vertices[36* idx+ 26]= anim_character->_z;
		vertices[36* idx+ 27]= tex_coord.x;
		vertices[36* idx+ 28]= tex_coord.y;
		vertices[36* idx+ 29]= current_layer;

		vertices[36* idx+ 30]= anim_character->_obj->_aabb->_pos.x+ anim_character->_obj->_aabb->_size.x;
		vertices[36* idx+ 31]= anim_character->_obj->_aabb->_pos.y+ anim_character->_obj->_aabb->_size.y;
		vertices[36* idx+ 32]= anim_character->_z;
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
	_obj(obj), _texture(texture), _z(z)
{
	
}


Character2D::~Character2D() {
	delete _obj;
}


// AnimatedCharacter2D -----------------------------------------------
AnimatedCharacter2D::AnimatedCharacter2D() : Character2D() {

}


AnimatedCharacter2D::AnimatedCharacter2D(Object2D * obj, Texture2D * texture, float z) :
	Character2D(obj, texture, z), _current_anim(0), _accumulated_time(0.0f)
{
	AnimTexture * anim_texture= dynamic_cast<AnimTexture *>(_texture);
	_current_action= anim_texture->_actions[0];
}


AnimatedCharacter2D::~AnimatedCharacter2D() {

}


void AnimatedCharacter2D::anim(float elapsed_time) {
	_accumulated_time+= elapsed_time;
	if (_accumulated_time>= _current_action->_anim_time) {
		_accumulated_time-= _current_action->_anim_time;
		_current_anim++;
		if (_current_anim>= _current_action->_n_idx) {
			_current_anim= 0;
		}
	}
}


void AnimatedCharacter2D::set_action(unsigned int idx_action) {
	AnimTexture * anim_texture= dynamic_cast<AnimTexture *>(_texture);
	if (idx_action>= anim_texture->_actions.size()) {
		cout << "set_action " << idx_action << " trop grand\n";
		return;
	}
	_current_action= anim_texture->_actions[idx_action];
	_current_anim= 0;
	_obj->set_footprint(_current_action->_footprint);
}


void AnimatedCharacter2D::set_action(string action_name) {
	int idx_action_ok= -1;
	AnimTexture * anim_texture= dynamic_cast<AnimTexture *>(_texture);
	for (int idx_action=0; idx_action<anim_texture->_actions.size(); ++idx_action) {
		if (anim_texture->_actions[idx_action]->_name== action_name) {
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


string AnimatedCharacter2D::current_action() {
	return _current_action->_name;
}


// Person2D -------------------------------------------
Person2D::Person2D() : AnimatedCharacter2D() {

}


Person2D::Person2D(Object2D * obj, Texture2D * texture, float z) : 
	AnimatedCharacter2D(obj, texture, z), _left_pressed(false), _right_pressed(false), _down_pressed(false), _up_pressed(false), _lshift_pressed(false), _jump(false)
{
	set_action("left_wait");
}


Person2D::~Person2D() {

}


/*
	float vel_run= 9.0f;
	float vel_walk= 5.0f;
	float vel_roll= 9.0f;
	float vel_jump_run= 25.0f;
	float vel_jump_wait= 20.0f;
*/
void Person2D::update_velocity() {
	AnimTexture * anim_texture= dynamic_cast<AnimTexture *>(_texture);

	if (_left_pressed) {
		if (current_action()== "left_run") {
			_obj->_velocity.x-= anim_texture->_velocities["run"];
		}
		else if (current_action()== "left_walk") {
			_obj->_velocity.x-= anim_texture->_velocities["walk"];
		}
		else if ((current_action()== "left_jump") || (current_action()== "left_fall")) {
			if (_lshift_pressed) {
				_obj->_velocity.x-= anim_texture->_velocities["run"];
			}
			else {
				_obj->_velocity.x-= anim_texture->_velocities["walk"];
			}
		}
		else if (current_action()== "left_roll") {
			_obj->_velocity.x-= anim_texture->_velocities["roll"];
		}
		else {
			//_obj->_velocity.x*= 0.9f;
		}
	}
	else if (_right_pressed) {
		if (current_action()== "right_run") {
			_obj->_velocity.x+= anim_texture->_velocities["run"];
		}
		else if (current_action()== "right_walk") {
			_obj->_velocity.x+= anim_texture->_velocities["walk"];
		}
		else if ((current_action()== "right_jump") || (current_action()== "right_fall")) {
			if (_lshift_pressed) {
				_obj->_velocity.x+= anim_texture->_velocities["run"];
			}
			else {
				_obj->_velocity.x+= anim_texture->_velocities["walk"];
			}
		}
		else if (current_action()== "right_roll") {
			_obj->_velocity.x+= anim_texture->_velocities["roll"];
		}
		else {
			//_obj->_velocity.x*= 0.9f;
		}
	}
	else {
		//_obj->_velocity.x*= 0.9f;
	}

	if (_jump) {
		_jump= false;
		if (_lshift_pressed) {
			_obj->_velocity.y+= anim_texture->_velocities["jump_run"];
		}
		else {
			_obj->_velocity.y+= anim_texture->_velocities["jump_walk"];
		}
	}
}


void Person2D::update_action() {
	glm::vec2 v(0.0f);
	if (_obj->_referential!= nullptr) {
		v= _obj->_velocity- _obj->_referential->_velocity;
	}
	else {
		v= _obj->_velocity;
	}

	if ((abs(v.x)< UPDATE_ACTION_TRESH) && (abs(v.y)< UPDATE_ACTION_TRESH)) {
		if ((current_action()== "left_walk") || (current_action()== "left_run") || (current_action()== "left_fall")) {
			set_action("left_wait");
		}
		else if ((current_action()== "right_walk") || (current_action()== "right_run") || (current_action()== "right_fall")) {
			set_action("right_wait");
		}
	}
	
	if (v.y< 0.0f) {
		if (current_action()== "left_jump") {
			set_action("left_fall");
		}
		else if (current_action()== "right_jump") {
			set_action("right_fall");
		}
	}
	else if (abs(v.y)< UPDATE_ACTION_TRESH) {
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


void Person2D::key_down(SDL_Keycode key) {
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


void Person2D::key_up(SDL_Keycode key) {
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


void Person2D::ia() {
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


string Person2D::current_direction() {
	return _current_action->_name.substr(0, _current_action->_name.find("_"));
}


string Person2D::current_type() {
	return _current_action->_name.substr(_current_action->_name.find("_")+ 1);
}


// SVGParser ----------------------------------------------------------------
SVGParser::SVGParser() {

}


SVGParser::SVGParser(string svg_path) {
	ifstream xml_file(svg_path);
	stringstream buffer;
	buffer << xml_file.rdbuf();
	xml_file.close();
	string xml_content(buffer.str());
	xml_document<> doc;
	doc.parse<0>(&xml_content[0]);
	xml_node<> * root_node= doc.first_node();
	
	_viewbox= root_node->first_attribute("viewBox")->value();
	vector<string> tags= {"x", "y", "width", "height", "xlink:href", "id", "anim_time", "velocity_walk", "velocity_run", "velocity_roll", "velocity_jump_walk", "velocity_jump_run", "d", "velocity"};
	
	for (xml_node<> * g_node=root_node->first_node("g"); g_node; g_node=g_node->next_sibling()) {
		string layer_label= g_node->first_attribute("inkscape:label")->value();

		for (xml_node<> * obj_node=g_node->first_node(); obj_node; obj_node=obj_node->next_sibling()) {
			/*if (string(image_node->name())!= "image") {
				continue;
			}*/

			std::map<std::string, std::string> obj;
			obj["type"]= string(obj_node->name());
			for (xml_attribute<> * attr=obj_node->first_attribute(); attr; attr=attr->next_attribute()) {
				for (auto tag : tags) {
					if (string(attr->name())== tag) {
						obj[tag]= string(attr->value());
						break;
					}
				}
			}
			if (layer_label== "Models") {
				_models.push_back(obj);
			}
			else {
				unsigned int label_idx= stoi(layer_label.substr(layer_label.find("layer"+ 1)));
				obj["z"]= (float)(label_idx)* 0.1f;
				_objs.push_back(obj);
			}
		}
	}
}


SVGParser::~SVGParser() {

}


// Level -------------------------------------------------------------------------------------------------
Level::Level() {

}


Level::Level(GLuint prog_draw_anim, GLuint prog_draw_static, GLuint prog_draw_aabb, string path, ScreenGL * screengl) :
	_screengl(screengl), _viewpoint(glm::vec2(0.0f))
{
	/*ifstream xml_file(path+ "/levels/level_01.xml");
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
	xml_node<> * block_w_node= root_node->first_node("block_w");
	_block_w= stof(block_w_node->value());
	xml_node<> * block_h_node= root_node->first_node("block_h");
	_block_h= stof(block_h_node->value());
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

	_textures.push_back(new StaticTexture(prog_draw_static, path+ "/static_textures/brick.png", screengl));
	_textures.push_back(new StaticTexture(prog_draw_static, path+ "/static_textures/grass.png", screengl));
	_textures.push_back(new StaticTexture(prog_draw_static, path+ "/static_textures/tree.png", screengl));
	_textures.push_back(new StaticTexture(prog_draw_static, path+ "/static_textures/hill.png", screengl));
	_textures.push_back(new StaticTexture(prog_draw_static, path+ "/static_textures/sky.png", screengl));
	_textures.push_back(new StaticTexture(prog_draw_static, path+ "/static_textures/cloud.png", screengl));
	_textures.push_back(new StaticTexture(prog_draw_static, path+ "/static_textures/platform.png", screengl));

	_textures.push_back(new AnimTexture(prog_draw_anim, path+ "/anim_textures/modele_1", screengl));
	_textures.push_back(new AnimTexture(prog_draw_anim, path+ "/anim_textures/modele_2", screengl));
	_textures.push_back(new AnimTexture(prog_draw_anim, path+ "/anim_textures/flower", screengl));

	add_character("tree", glm::vec2(-5.0f, -5.0f), glm::vec2(5.0f, 5.0f), 0.3f, STATIC_UNSOLID, "Character2D");
	add_character("hill", glm::vec2(-0.5f* _screengl->_gl_width, -0.5f* _screengl->_gl_height), glm::vec2(_screengl->_gl_width, _screengl->_gl_height), 0.2f, STATIC_UNSOLID, "Character2D");
	add_character("sky", glm::vec2(-0.5f* _screengl->_gl_width, -0.5f* _screengl->_gl_height), glm::vec2(_screengl->_gl_width, _screengl->_gl_height), 0.1f, STATIC_UNSOLID, "Character2D");

	for (unsigned int col=0; col<_w; ++col) {
		for (unsigned int row=0; row<_h; ++row) {
			glm::vec2 pos(-0.5f* _screengl->_gl_width+ (float)(col)* _block_w, -0.5f* _screengl->_gl_height+ (float)(row)* _block_h);
			glm::vec2 size= glm::vec2(_block_w, _block_h);
			if (level_data[col+ _w* (_h- row- 1)]== '*') {
				add_character("brick", pos, size, 1.0f, STATIC_DESTRUCTIBLE, "Character2D");
				if ((row!= _h- 1) && (level_data[col+ _w* (_h- row- 2)]!= '*')) {
					add_character("grass", pos+ glm::vec2(0.0f, size.y), size, 1.5f, STATIC_UNSOLID, "Character2D");
				}
			}
			else if (level_data[col+ _w* (_h- row- 1)]== 'x') {
				add_character("modele_1", pos, size* 2.0f, 0.5f, FALLING, "Person2D");
			}
			else if (level_data[col+ _w* (_h- row- 1)]== '+') {
				add_character("modele_1", pos, size* 2.0f, 0.5f, FALLING, "Person2D");
				_hero= dynamic_cast<Person2D *>(_characters[_characters.size()- 1]);
			}
		}
	}

	add_character("flower", glm::vec2(0.0f, 0.0f), glm::vec2(4.0f, 4.0f), 2.0f, STATIC_UNSOLID, "AnimatedCharacter2D");
	add_character("brick", glm::vec2(0.0f, 0.0f), glm::vec2(4.0f, 1.0f), 3.0f, CHECKPOINT_SOLID, "Character2D", {{glm::vec2(-2.0f, -3.0f), 2.0f}, {glm::vec2(-2.0f, 3.0f), 2.0f}});
	add_character("cloud", glm::vec2(-4.0f, 5.0f), glm::vec2(5.0f, 5.0f), 1.4f, CHECKPOINT_UNSOLID, "Character2D", {{glm::vec2(-4.0f, 5.0f), 2.0f}, {glm::vec2(4.0f, 5.0f), 2.0f}});
	add_character("platform", glm::vec2(-2.0f, -3.0f), glm::vec2(2.0f, 2.0f), 2.0f, STATIC_UNSOLID, "Character2D");*/

	SVGParser * svg_parser= new SVGParser(path);

	for (auto model : svg_parser->_models) {
		if (model["type"]!= "image") {
			continue;
		}
		if (!model.count("physics")) {
			continue;
		}
		ObjectPhysics physics= str2physics(model["physics"]);
		if (model["xlink:href"].find("static_textures")!= string::npos) {
			if (get_texture(basename(model["xlink:href"]))== nullptr) {
				_textures.push_back(new StaticTexture(prog_draw_static, model["xlink:href"], screengl, physics));
			}
		}
		else if (model["xlink:href"].find("anim_textures")!= string::npos) {
			string anim_path= model["xlink:href"].substr(0, model["xlink:href"].find("/pngs"));
			if (get_texture(basename(anim_path))== nullptr) {
				_textures.push_back(new AnimTexture(prog_draw_anim, anim_path, screengl, physics));
			}
		}
	}

	for (auto static_texture : _textures) {
		if (!dynamic_cast<StaticTexture *>(static_texture)) {
			continue;
		}
		glm::vec2 pos;
		glm::vec2 size;
		glm::vec2 footprint_pos;
		glm::vec2 footprint_size;
		
		for (auto model : svg_parser->_models) {
			if (model["type"]!= "image") {
				continue;
			}
			if (model["id"].substr(model["id"].find("_")+ 1)== static_texture->_name) {
				pos= glm::vec2(stof(model["x"]), stof(model["y"]));
				size= glm::vec2(stof(model["width"]), stof(model["height"]));
				break;
			}
		}
		
		for (auto model : svg_parser->_models) {
			if (model["type"]!= "rect") {
				continue;
			}
			if (model["id"].substr(model["id"].find("_")+ 1)== static_texture->_name) {
				footprint_pos= glm::vec2(stof(model["x"]), stof(model["y"]));
				footprint_size= glm::vec2(stof(model["width"]), stof(model["height"]));
				static_texture->_actions[0]->_footprint->_pos= footprint_pos- pos;
				static_texture->_actions[0]->_footprint->_size= footprint_size/ size;
				break;
			}
		}
	}

	for (auto anim_texture : _textures) {
		if (!dynamic_cast<AnimTexture *>(anim_texture)) {
			continue;
		}
		glm::vec2 pos;
		glm::vec2 size;
		glm::vec2 footprint_pos;
		glm::vec2 footprint_size;
		string anim_name;
		string action_name;
		Action * action= nullptr;

		for (auto model : svg_parser->_models) {
			if (model["type"]!= "image") {
				continue;
			}
			anim_name= model["xlink:href"].substr(0, model["xlink:href"].find("/pngs"));
			anim_name= anim_name.substr(anim_name.find_last_of("/")+ 1);
			action_name= model["xlink:href"].substr(model["xlink:href"].find("pngs/")+ 1);
			action_name= anim_name.substr(0, anim_name.find("/"));
			if (anim_name== anim_texture->_name) {
				pos= glm::vec2(stof(model["x"]), stof(model["y"]));
				size= glm::vec2(stof(model["width"]), stof(model["height"]));
				action= anim_texture->get_action(action_name);
				if (model.count("anim_time")) {
					action->_anim_time= stof(model["anim_time"]);
				}
				vector<string> velocity_tags= {"velocity_walk", "velocity_run", "velocity_roll", "velocity_jump_walk", "velocity_jump_run"};
				for (auto tag : velocity_tags) {
					if (model.count(tag)) {
						dynamic_cast<AnimTexture *>(anim_texture)->_velocities[tag]= stof(model[tag]);
					}
				}
				break;
			}
		}
		
		for (auto model : svg_parser->_models) {
			if (model["type"]!= "rect") {
				continue;
			}
			if (model["id"].substr(model["id"].find("_")+ 1)== anim_texture->_name) {
				footprint_pos= glm::vec2(stof(model["x"]), stof(model["y"]));
				footprint_size= glm::vec2(stof(model["width"]), stof(model["height"]));
				action->_footprint->_pos= footprint_pos- pos;
				action->_footprint->_size= footprint_size/ size;
				break;
			}
		}
	}

	for (auto model : svg_parser->_objs) {
		if (model["type"]!= "image") {
			continue;
		}
		if (model["xlink:href"].find("static_textures")!= string::npos) {
			add_character(basename(model["xlink:href"]), glm::vec2(stof(model["x"]), stof(model["y"])), glm::vec2(stof(model["width"]), stof(model["height"])), model["z"], STATIC_UNSOLID, "Character2D");
		}
	}

	delete svg_parser;
}


Level::~Level() {	
	for (auto character : _characters) {
		delete character;
	}
	_characters.clear();

	for (auto texture : _textures) {
		delete texture;
	}
	_textures.clear();	
}


Texture2D * Level::get_texture(string texture_name) {
	for (auto texture : _textures) {
		if (texture->_name== texture_name) {
			return texture;
		}
	}
	cout << "texture non trouvee\n";
	return nullptr;
}


void Level::add_character(string texture_name, glm::vec2 pos, glm::vec2 size, float z, ObjectPhysics physics, string character_type, vector<CheckPoint> checkpoints) {
	Texture2D * texture= get_texture(texture_name);
	Object2D * obj= new Object2D(pos, size, texture->_actions[0]->_footprint, physics, checkpoints);
	Character2D * character;
	if (character_type== "Character2D") {
		character= new Character2D(obj, texture, z);
	}
	else if (character_type== "AnimatedCharacter2D") {
		character= new AnimatedCharacter2D(obj, texture, z);
	}
	else if (character_type== "Person2D") {
		character= new Person2D(obj, texture, z);
	}
	_characters.push_back(character);
	texture->_characters.push_back(character);
	texture->update();
}


void Level::delete_character(Character2D * character) {
	Texture2D * texture= character->_texture;
	texture->_characters.erase(remove(texture->_characters.begin(), texture->_characters.end(), character), texture->_characters.end());
	delete character;
	_characters.erase(remove(_characters.begin(), _characters.end(), character), _characters.end());
}


void Level::update_model2worlds() {
	for (auto texture: _textures) {
		// pourquoi des - ici ?
		texture->set_model2world(glm::translate(glm::mat4(1.0f), glm::vec3(-_viewpoint.x, -_viewpoint.y, 0.0f)));
	}
}


void Level::draw() {
	for (auto texture : _textures) {
		texture->draw();
	}
}


void Level::anim(float elapsed_time) {
	glm::vec2 contact_pt(0.0f);
	glm::vec2 contact_normal(0.0f);
	float contact_time= 0.0f;

	for (auto character : _characters) {
		Object2D * obj= character->_obj;
		if (obj->_physics!= FALLING) {
			continue;
		}
		obj->_velocity.x= 0.0f;
		if (obj->_referential!= nullptr) {
			obj->_velocity.x= obj->_referential->_velocity.x;
		}
	}

	for (auto character : _characters) {
		Object2D * obj= character->_obj;
		obj->update_velocity();
	}

	for (auto character : _characters) {
		Person2D * person= dynamic_cast<Person2D *>(character);
		if (person) {
			if (person!= _hero) {
				person->ia();
			}
			person->update_velocity();
		}
	}

	/*for (auto character : _characters) {
		Object2D * obj= character->_obj;
		Person2D * person= dynamic_cast<Person2D *>(character);
		if (person== _hero) {
			//cout << glm::to_string(obj->_velocity) << "\n";
			if (obj->_referential!= nullptr) {
				cout << glm::to_string(obj->_velocity) << " ; " << glm::to_string(obj->_referential->_velocity) << "\n";
				cout << glm::to_string(obj->_velocity- obj->_referential->_velocity) << "\n";
				cout << person->_left_pressed << " ; " << person->_right_pressed << "\n";
			}
		}
	}*/

	for (auto character : _characters) {
		Object2D * obj= character->_obj;
		if (obj->_physics!= FALLING) {
			continue;
		}
		
		obj->_bottom.clear();
		obj->_top.clear();
		obj->_referential= nullptr;
	}

	for (auto character1 : _characters) {
		Object2D * obj1= character1->_obj;
		if (obj1->_physics!= FALLING) {
			continue;
		}

		for (auto character2 : _characters) {
			if (character2== character1) {
				continue;
			}

			Object2D * obj2= character2->_obj;

			if ((obj2->_physics!= STATIC_DESTRUCTIBLE) && (obj2->_physics!= STATIC_INDESTRUCTIBLE) && (obj2->_physics!= CHECKPOINT_SOLID)) {
				continue;
			}

			if (anim_intersect_static(obj1, obj2, elapsed_time, contact_pt, contact_normal, contact_time)) {
				glm::vec2 correction= (1.0f- contact_time)* glm::vec2(abs(obj1->_velocity.x)* contact_normal.x, abs(obj1->_velocity.y)* contact_normal.y);
				obj1->_velocity+= correction* 1.1f;

				if (contact_normal.y> 0.0f) {
					obj1->_bottom.push_back(obj2);
					if (obj2->_physics== CHECKPOINT_SOLID) {
						obj1->_referential= obj2;
					}
				}
				else if (contact_normal.y< 0.0f) {
					obj1->_top.push_back(obj2);
				}
			}
		}
	}

	for (auto character1 : _characters) {
		Object2D * obj1= character1->_obj;
		if (obj1->_physics!= CHECKPOINT_SOLID) {
			continue;
		}
		
		for (auto character2 : _characters) {
			if (character2== character1) {
				continue;
			}

			Object2D * obj2= character2->_obj;

			if (obj2->_physics!= FALLING) {
				continue;
			}

			Object2D * obj_tmp= new Object2D(*obj2);
			obj_tmp->update_pos(elapsed_time);
			if (anim_intersect_static(obj1, obj_tmp, elapsed_time, contact_pt, contact_normal, contact_time)) {
				glm::vec2 correction= (1.0f- contact_time)* glm::vec2(abs(obj1->_velocity.x)* contact_normal.x, abs(obj1->_velocity.y)* contact_normal.y);
				obj2->_velocity-= correction* 1.1f;

				if (contact_normal.y< 0.0f) {
					obj2->_bottom.push_back(obj1);
					if (obj1->_physics== CHECKPOINT_SOLID) {
						obj2->_referential= obj1;
					}
				}
				else if (contact_normal.y> 0.0f) {
					obj2->_top.push_back(obj1);
				}
			}
			delete obj_tmp;
		}
	}

	vector<Object2D *> objects2delete;
	for (auto character : _characters) {
		Object2D * obj= character->_obj;
		for (auto object_top : obj->_top) {
			if ((object_top->_physics== STATIC_DESTRUCTIBLE) && (find(objects2delete.begin(), objects2delete.end(), object_top)== objects2delete.end())) {
				objects2delete.push_back(object_top);
			}
		}
	}
	for (auto obj : objects2delete) {
		for (auto character : _characters) {
			if (character->_obj== obj) {
				delete_character(character);
				break;
			}
		}
	}

	for (auto character : _characters) {
		Object2D * obj= character->_obj;
		obj->update_pos(elapsed_time);
	}
	
	for (auto character : _characters) {
		Object2D * obj= character->_obj;
		if (obj->_physics!= FALLING) {
			continue;
		}
		if ((obj->_bottom.size()) && (obj->_top.size())) {
			cout << "death\n";
		}
	}

	for (auto character : _characters) {
		Person2D * person= dynamic_cast<Person2D *>(character);
		if (person) {
			person->update_action();
		}
	}

	for (auto character : _characters) {
		AnimatedCharacter2D * anim_character= dynamic_cast<AnimatedCharacter2D *>(character);
		if (anim_character) {
			anim_character->anim(elapsed_time);
		}
	}

	for (auto texture : _textures) {
		texture->update();
	}

	glm::vec2 hero= _hero->_obj->_aabb->center();
	//_viewpoint= hero;
	if (hero.x< _viewpoint.x- MOVE_VIEWPOINT.x) {
		_viewpoint.x= hero.x+ MOVE_VIEWPOINT.x;
	}
	else if (hero.x> _viewpoint.x+ MOVE_VIEWPOINT.x) {
		_viewpoint.x= hero.x- MOVE_VIEWPOINT.x;
	}
	if (hero.y< _viewpoint.y- MOVE_VIEWPOINT.y) {
		_viewpoint.y= hero.y+ MOVE_VIEWPOINT.y;
	}
	else if (hero.y> _viewpoint.y+ MOVE_VIEWPOINT.y) {
		_viewpoint.y= hero.y- MOVE_VIEWPOINT.y;
	}

	update_model2worlds();
}


bool Level::key_down(InputState * input_state, SDL_Keycode key) {
	if ((key== SDLK_DOWN) || (key== SDLK_UP) || (key== SDLK_LEFT) || (key== SDLK_RIGHT) || (key== SDLK_a)) {
		_hero->key_down(key);
		return true;
	}

	return false;
}


bool Level::key_up(InputState * input_state, SDL_Keycode key) {
	if ((key== SDLK_DOWN) || (key== SDLK_UP) || (key== SDLK_LEFT) || (key== SDLK_RIGHT) || (key== SDLK_a)) {
		_hero->key_up(key);
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
	glm::vec3 aabb_color(1.0f, 0.0f, 0.0f);
	glm::vec3 footprint_color(0.0f, 1.0f, 0.0f);

	unsigned int n_chars= _level->_characters.size();

	_n_aabbs= n_chars* 2;
	float vertices[_n_aabbs* 40];
	for (unsigned int i=0; i<_n_aabbs* 40; ++i) {
		vertices[i]= 0.0f;
	}

	if (_draw_aabb) {
		for (unsigned int idx=0; idx<n_chars; ++idx) {
			vertices[40* idx+ 0]= _level->_characters[idx]->_obj->_aabb->_pos.x;
			vertices[40* idx+ 1]= _level->_characters[idx]->_obj->_aabb->_pos.y;
			vertices[40* idx+ 5]= _level->_characters[idx]->_obj->_aabb->_pos.x+ _level->_characters[idx]->_obj->_aabb->_size.x;
			vertices[40* idx+ 6]= _level->_characters[idx]->_obj->_aabb->_pos.y;

			vertices[40* idx+ 10]= _level->_characters[idx]->_obj->_aabb->_pos.x+ _level->_characters[idx]->_obj->_aabb->_size.x;
			vertices[40* idx+ 11]= _level->_characters[idx]->_obj->_aabb->_pos.y;
			vertices[40* idx+ 15]= _level->_characters[idx]->_obj->_aabb->_pos.x+ _level->_characters[idx]->_obj->_aabb->_size.x;
			vertices[40* idx+ 16]= _level->_characters[idx]->_obj->_aabb->_pos.y+ _level->_characters[idx]->_obj->_aabb->_size.y;

			vertices[40* idx+ 20]= _level->_characters[idx]->_obj->_aabb->_pos.x+ _level->_characters[idx]->_obj->_aabb->_size.x;
			vertices[40* idx+ 21]= _level->_characters[idx]->_obj->_aabb->_pos.y+ _level->_characters[idx]->_obj->_aabb->_size.y;
			vertices[40* idx+ 25]= _level->_characters[idx]->_obj->_aabb->_pos.x;
			vertices[40* idx+ 26]= _level->_characters[idx]->_obj->_aabb->_pos.y+ _level->_characters[idx]->_obj->_aabb->_size.y;

			vertices[40* idx+ 30]= _level->_characters[idx]->_obj->_aabb->_pos.x;
			vertices[40* idx+ 31]= _level->_characters[idx]->_obj->_aabb->_pos.y+ _level->_characters[idx]->_obj->_aabb->_size.y;
			vertices[40* idx+ 35]= _level->_characters[idx]->_obj->_aabb->_pos.x;
			vertices[40* idx+ 36]= _level->_characters[idx]->_obj->_aabb->_pos.y;

			for (unsigned int i=0; i<8; ++i) {
				vertices[40* idx+ 5* i+ 2]= aabb_color.x;
				vertices[40* idx+ 5* i+ 3]= aabb_color.y;
				vertices[40* idx+ 5* i+ 4]= aabb_color.z;
			}
		}
	}
	
	if (_draw_footprint) {
		for (unsigned int idx=0; idx<n_chars; ++idx) {
			vertices[n_chars* 40+ 40* idx+ 0]= _level->_characters[idx]->_obj->_footprint->_pos.x;
			vertices[n_chars* 40+ 40* idx+ 1]= _level->_characters[idx]->_obj->_footprint->_pos.y;
			vertices[n_chars* 40+ 40* idx+ 5]= _level->_characters[idx]->_obj->_footprint->_pos.x+ _level->_characters[idx]->_obj->_footprint->_size.x;
			vertices[n_chars* 40+ 40* idx+ 6]= _level->_characters[idx]->_obj->_footprint->_pos.y;

			vertices[n_chars* 40+ 40* idx+ 10]= _level->_characters[idx]->_obj->_footprint->_pos.x+ _level->_characters[idx]->_obj->_footprint->_size.x;
			vertices[n_chars* 40+ 40* idx+ 11]= _level->_characters[idx]->_obj->_footprint->_pos.y;
			vertices[n_chars* 40+ 40* idx+ 15]= _level->_characters[idx]->_obj->_footprint->_pos.x+ _level->_characters[idx]->_obj->_footprint->_size.x;
			vertices[n_chars* 40+ 40* idx+ 16]= _level->_characters[idx]->_obj->_footprint->_pos.y+ _level->_characters[idx]->_obj->_footprint->_size.y;

			vertices[n_chars* 40+ 40* idx+ 20]= _level->_characters[idx]->_obj->_footprint->_pos.x+ _level->_characters[idx]->_obj->_footprint->_size.x;
			vertices[n_chars* 40+ 40* idx+ 21]= _level->_characters[idx]->_obj->_footprint->_pos.y+ _level->_characters[idx]->_obj->_footprint->_size.y;
			vertices[n_chars* 40+ 40* idx+ 25]= _level->_characters[idx]->_obj->_footprint->_pos.x;
			vertices[n_chars* 40+ 40* idx+ 26]= _level->_characters[idx]->_obj->_footprint->_pos.y+ _level->_characters[idx]->_obj->_footprint->_size.y;

			vertices[n_chars* 40+ 40* idx+ 30]= _level->_characters[idx]->_obj->_footprint->_pos.x;
			vertices[n_chars* 40+ 40* idx+ 31]= _level->_characters[idx]->_obj->_footprint->_pos.y+ _level->_characters[idx]->_obj->_footprint->_size.y;
			vertices[n_chars* 40+ 40* idx+ 35]= _level->_characters[idx]->_obj->_footprint->_pos.x;
			vertices[n_chars* 40+ 40* idx+ 36]= _level->_characters[idx]->_obj->_footprint->_pos.y;

			for (unsigned int i=0; i<8; ++i) {
				vertices[n_chars* 40+ 40* idx+ 5* i+ 2]= footprint_color.x;
				vertices[n_chars* 40+ 40* idx+ 5* i+ 3]= footprint_color.y;
				vertices[n_chars* 40+ 40* idx+ 5* i+ 4]= footprint_color.z;
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

