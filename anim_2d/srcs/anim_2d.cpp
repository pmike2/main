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
	else if (s== "CHECKPOINT_SOLID_TOP") {
		return CHECKPOINT_SOLID_TOP;
	}
	cout << "physics non trouve\n";
	return STATIC_DESTRUCTIBLE;
}


CharacterType str2character_type(string s) {
	if (s== "CHARACTER_2D") {
		return CHARACTER_2D;
	}
	else if (s== "ANIM_CHARACTER_2D") {
		return ANIM_CHARACTER_2D;
	}
	else if (s== "PERSON_2D") {
		return PERSON_2D;
	}
	cout << "character_type non trouve\n";
	return CHARACTER_2D;
}


// CheckPoint ------------------------------------------------------------------------------------------
CheckPoint::CheckPoint() {

}


CheckPoint::CheckPoint(pt_2d pos, number velocity) : _pos(pos), _velocity(velocity) {

}


CheckPoint::~CheckPoint() {

}


// Object2D --------------------------------------------------------------------------------------------
Object2D::Object2D() {

}


Object2D::Object2D(AABB_2D * aabb, AABB_2D * footprint, ObjectPhysics physics, vector<CheckPoint> checkpoints) :
	_velocity(pt_2d(0.0)), _physics(physics), _idx_checkpoint(0), _referential(nullptr)
{
	_aabb= new AABB_2D(*aabb);
	_footprint= new AABB_2D(pt_2d(0.0), pt_2d(_aabb->_size.x* footprint->_size.x, _aabb->_size.y* footprint->_size.y));
	_footprint_offset= pt_2d(_aabb->_size.x* footprint->_pos.x, _aabb->_size.y* footprint->_pos.y);
	update_footprint_pos();

	for (auto checkpoint : checkpoints) {
		_checkpoints.push_back(new CheckPoint(checkpoint._pos, checkpoint._velocity));
	}
	if ( (((_physics== CHECKPOINT_SOLID) || (_physics== CHECKPOINT_UNSOLID) || (_physics== CHECKPOINT_SOLID_TOP)) && (!_checkpoints.size())) || ((_physics!= CHECKPOINT_SOLID) && (_physics!= CHECKPOINT_UNSOLID) && (_physics!= CHECKPOINT_SOLID_TOP) && (_checkpoints.size())) ) {
		cout << "erreur object checkpoint : _physics=" << _physics << " ; _checkpoints.size()=" << _checkpoints.size() << "\n";
	}
}


Object2D::Object2D(const Object2D & obj) {
	_aabb= new AABB_2D(*obj._aabb);
	_footprint= new AABB_2D(*obj._footprint);
	_footprint_offset= pt_2d(obj._footprint_offset);
	_velocity= pt_2d(obj._velocity);
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


void Object2D::update_pos(number elapsed_time) {
	if ((_physics!= STATIC_DESTRUCTIBLE) && (_physics!= STATIC_INDESTRUCTIBLE) && (_physics!= STATIC_UNSOLID)) {
		_aabb->_pos+= _velocity* elapsed_time;
		update_footprint_pos();
	}
	if ((_physics== CHECKPOINT_SOLID) || (_physics== CHECKPOINT_UNSOLID) || (_physics== CHECKPOINT_SOLID_TOP)) {
		if (glm::distance2(_checkpoints[_idx_checkpoint]->_pos, _aabb->_pos)< CHECKPOINT_TRESH) {
			_idx_checkpoint++;
			if (_idx_checkpoint>= _checkpoints.size()) {
				_idx_checkpoint= 0;
			}
		}
	}
}


void Object2D::update_velocity() {
	if (_physics== FALLING) {
		_velocity.y-= GRAVITY_INC;
		if (_velocity.y< -1.0* GRAVITY_MAX) {
			_velocity.y= -1.0* GRAVITY_MAX;
		}
	}
	else if ((_physics== CHECKPOINT_SOLID) || (_physics== CHECKPOINT_UNSOLID) || (_physics== CHECKPOINT_SOLID_TOP)) {
		if (glm::distance2(_checkpoints[_idx_checkpoint]->_pos, _aabb->_pos)> CHECKPOINT_TRESH) {
			pt_2d direction= _checkpoints[_idx_checkpoint]->_pos- _aabb->_pos;
			_velocity= glm::normalize(direction)* _checkpoints[_idx_checkpoint]->_velocity;
		}
	}
}


void Object2D::update_footprint_pos() {
	_footprint->_pos= _aabb->_pos+ _footprint_offset;
}


void Object2D::set_aabb_pos(pt_2d pos) {
	_aabb->_pos= pos;
	update_footprint_pos();
}


void Object2D::set_footprint(AABB_2D * footprint) {
	_footprint_offset= pt_2d(_aabb->_size.x* footprint->_pos.x, _aabb->_size.y* footprint->_pos.y);
	_footprint->_size= pt_2d(_aabb->_size.x* footprint->_size.x, _aabb->_size.y* footprint->_size.y);
	update_footprint_pos();
}


ostream & operator << (ostream & os, const Object2D & obj) {
	os << "aabb : " << *obj._aabb << "\n";
	os << "footprint : " << *obj._footprint << "\n";
	os << "velocity : " << glm::to_string(obj._velocity) << "\n";
	return os;
}


// -------------------------------------------------------------------------------------------
// voir refs dans bbox_2d.h pour algo
bool anim_intersect_static(const Object2D * anim_obj, const Object2D * static_obj, const number time_step, pt_2d & contact_pt, pt_2d & contact_normal, number & contact_time) {
	//if (glm::length2(anim_obj->_velocity)< 1e-9f) {
	if ((anim_obj->_velocity.x== 0.0) && (anim_obj->_velocity.y== 0.0)) {
		return false;
	}

	AABB_2D expanded;
	expanded._pos= static_obj->_footprint->_pos- 0.5* anim_obj->_footprint->_size;
	expanded._size= static_obj->_footprint->_size+ anim_obj->_footprint->_size;

	if (ray_intersects_aabb(anim_obj->_footprint->center(), time_step* anim_obj->_velocity, &expanded, contact_pt, contact_normal, contact_time)) {
		// le = du >= est important
		return ((contact_time>= 0.0) && (contact_time< 1.0));
	}
	
	return false;
}


// Action ---------------------------------------------------------------------------
Action::Action() : _name(""), _first_idx(0), _n_idx(0), _anim_time(0.0) {
	// par défaut footprint prend toute l'emprise
	_footprint= new AABB_2D(pt_2d(0.0, 0.0), pt_2d(1.0, 1.0));
}


Action::~Action() {
	delete _footprint;
}


ostream & operator << (ostream & os, const Action & action) {
	os << "_name=" << action._name << " ; _first_idx=" << action._first_idx << " ; _n_idx=" << action._n_idx << "\n";
	for (auto png : action._pngs) {
		os << png << "\n";
	}
	return os;
}


// Texture2D ----------------------------------------------------------------------------------------------------
Texture2D::Texture2D() {

}


Texture2D::Texture2D(GLuint prog_draw, string path, ScreenGL * screengl, ObjectPhysics physics, CharacterType character_type) :
	_prog_draw(prog_draw), _screengl(screengl), _n_aabbs(0), _physics(physics), _character_type(character_type), _texture_id(0)
{
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


StaticTexture::StaticTexture(GLuint prog_draw, string path, ScreenGL * screengl, ObjectPhysics physics, CharacterType character_type) :
	Texture2D(prog_draw, path, screengl, physics, character_type), _alpha(1.0) 
{
	glGenTextures(1, &_texture_id);
	glBindTexture(GL_TEXTURE_2D, _texture_id);
	
	SDL_Surface * surface= IMG_Load(path.c_str());
	if (!surface) {
		cout << "IMG_Load error :" << IMG_GetError() << endl;
		return;
	}
	// sais pas pourquoi mais GL_BGRA fonctionne mieux que GL_RGBA
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);

	SDL_FreeSurface(surface);

	glActiveTexture(GL_TEXTURE0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glActiveTexture(0);
	
	glBindTexture(GL_TEXTURE_2D, 0);

	_camera2clip= glm::ortho(-0.5f* float(_screengl->_gl_width), 0.5f* float(_screengl->_gl_width), -0.5f* float(_screengl->_gl_height), 0.5f* float(_screengl->_gl_height), Z_NEAR, Z_FAR);
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

	// 1 seule action pour StaticTexture
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

	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(number), (void*)0);
	glVertexAttribPointer(_tex_coord_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(number), (void*)(3* sizeof(number)));

	glDrawArrays(GL_TRIANGLES, 0, 6* _n_aabbs);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_tex_coord_loc);

	glBindTexture(GL_TEXTURE, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glActiveTexture(0);
}


void StaticTexture::update() {
	_n_aabbs= _characters.size();
	number vertices[30* _n_aabbs];
	for (unsigned int idx=0; idx<_n_aabbs; ++idx) {
		vertices[30* idx+ 0]= _characters[idx]->_obj->_aabb->_pos.x;
		vertices[30* idx+ 1]= _characters[idx]->_obj->_aabb->_pos.y+ _characters[idx]->_obj->_aabb->_size.y;
		vertices[30* idx+ 2]= _characters[idx]->_z;
		vertices[30* idx+ 3]= 0.0;
		vertices[30* idx+ 4]= 0.0;

		vertices[30* idx+ 5]= _characters[idx]->_obj->_aabb->_pos.x;
		vertices[30* idx+ 6]= _characters[idx]->_obj->_aabb->_pos.y;
		vertices[30* idx+ 7]= _characters[idx]->_z;
		vertices[30* idx+ 8]= 0.0;
		vertices[30* idx+ 9]= 1.0;

		vertices[30* idx+ 10]= _characters[idx]->_obj->_aabb->_pos.x+ _characters[idx]->_obj->_aabb->_size.x;
		vertices[30* idx+ 11]= _characters[idx]->_obj->_aabb->_pos.y;
		vertices[30* idx+ 12]= _characters[idx]->_z;
		vertices[30* idx+ 13]= 1.0;
		vertices[30* idx+ 14]= 1.0;

		vertices[30* idx+ 15]= _characters[idx]->_obj->_aabb->_pos.x;
		vertices[30* idx+ 16]= _characters[idx]->_obj->_aabb->_pos.y+ _characters[idx]->_obj->_aabb->_size.y;
		vertices[30* idx+ 17]= _characters[idx]->_z;
		vertices[30* idx+ 18]= 0.0;
		vertices[30* idx+ 19]= 0.0;

		vertices[30* idx+ 20]= _characters[idx]->_obj->_aabb->_pos.x+ _characters[idx]->_obj->_aabb->_size.x;
		vertices[30* idx+ 21]= _characters[idx]->_obj->_aabb->_pos.y;
		vertices[30* idx+ 22]= _characters[idx]->_z;
		vertices[30* idx+ 23]= 1.0;
		vertices[30* idx+ 24]= 1.0;

		vertices[30* idx+ 25]= _characters[idx]->_obj->_aabb->_pos.x+ _characters[idx]->_obj->_aabb->_size.x;
		vertices[30* idx+ 26]= _characters[idx]->_obj->_aabb->_pos.y+ _characters[idx]->_obj->_aabb->_size.y;
		vertices[30* idx+ 27]= _characters[idx]->_z;
		vertices[30* idx+ 28]= 1.0;
		vertices[30* idx+ 29]= 0.0;
	}
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, 30* _n_aabbs* sizeof(number), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// AnimTexture ---------------------------------------------------------------------------
AnimTexture::AnimTexture() : Texture2D() {

}


AnimTexture::AnimTexture(GLuint prog_draw, string path, ScreenGL * screengl, ObjectPhysics physics, CharacterType character_type) :
	Texture2D(prog_draw, path, screengl, physics, character_type) 
{
	string root_pngs= path+ "/pngs";
	vector<string> l_dirs= list_files(root_pngs);
	unsigned int compt= 0; // compteur courant permettant l'init de _first_idx
	for (auto dir : l_dirs) {
		Action * action= new Action();
		action->_name= basename(dir);
		action->_first_idx= compt;

		vector<string> l_files= list_files(dir);
		sort(l_files.begin(), l_files.end());
		for (auto f : l_files) {
			action->_pngs.push_back(f);
			action->_n_idx++;
			compt++;
		}
		_actions.push_back(action);
	}

	// utilisation de GL_TEXTURE_2D_ARRAY : on stocke dans un tableau de textures toutes les images de toutes les actions
	glGenTextures(1, &_texture_id);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);
	

	// on utilise la 1ere image de la 1ere action pour déterminer la taille de toutes les images
	SDL_Surface * surface_0= IMG_Load(_actions[0]->_pngs[0].c_str());
	glm::ivec2 model_size(surface_0->w, surface_0->h);
	SDL_FreeSurface(surface_0);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, model_size.x, model_size.y, compt, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	for (auto action : _actions) {
		for (unsigned int i=0; i<action->_n_idx; ++i) {
			SDL_Surface * surface= IMG_Load(action->_pngs[i].c_str());
			if (!surface) {
				cout << "IMG_Load error :" << IMG_GetError() << endl;
				return;
			}

			// sais pas pourquoi mais GL_BGRA fonctionne mieux que GL_RGBA
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
							0,                             // mipmap number
							0, 0, action->_first_idx+ i,   // xoffset, yoffset, zoffset
							model_size.x, model_size.y, 1, // width, height, depth
							GL_BGRA,                       // format
							GL_UNSIGNED_BYTE,              // type
							surface->pixels);              // pointer to data

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

	_camera2clip= glm::ortho(-0.5f* float(_screengl->_gl_width), 0.5f* float(_screengl->_gl_width), -0.5f* float(_screengl->_gl_height), 0.5f* float(_screengl->_gl_height), Z_NEAR, Z_FAR);
	_model2world= glm::mat4(1.0);

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

	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(number), (void*)0);
	glVertexAttribPointer(_tex_coord_loc, 2, GL_FLOAT, GL_FALSE, 6* sizeof(number), (void*)(3* sizeof(number)));
	glVertexAttribPointer(_current_layer_loc, 1, GL_FLOAT, GL_FALSE, 6* sizeof(number), (void*)(5* sizeof(number)));

	glDrawArrays(GL_TRIANGLES, 0, 6* _n_aabbs);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_tex_coord_loc);
	glDisableVertexAttribArray(_current_layer_loc);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glActiveTexture(0);
}


void AnimTexture::update() {
	_n_aabbs= _characters.size();
	float vertices[36* _n_aabbs];
	for (unsigned int idx=0; idx<_n_aabbs; ++idx) {
		// cast en AnimatedCharacter2D *
		AnimatedCharacter2D * anim_character= dynamic_cast<AnimatedCharacter2D *>(_characters[idx]);
		// on ajoute l'indice de la 1ere image liée à l'action courante + indice de l'anim courante au sein de cette action
		float current_layer= (float)(anim_character->_current_action->_first_idx+ anim_character->_current_anim);

		vertices[36* idx+ 0]= float(anim_character->_obj->_aabb->_pos.x);
		vertices[36* idx+ 1]= float(anim_character->_obj->_aabb->_pos.y+ anim_character->_obj->_aabb->_size.y);
		vertices[36* idx+ 2]= float(anim_character->_z);
		vertices[36* idx+ 3]= 0.0f;
		vertices[36* idx+ 4]= 0.0f;
		vertices[36* idx+ 5]= current_layer;

		vertices[36* idx+ 6]= float(anim_character->_obj->_aabb->_pos.x);
		vertices[36* idx+ 7]= float(anim_character->_obj->_aabb->_pos.y);
		vertices[36* idx+ 8]= float(anim_character->_z);
		vertices[36* idx+ 9]= 0.0f;
		vertices[36* idx+ 10]= 1.0f;
		vertices[36* idx+ 11]= current_layer;

		vertices[36* idx+ 12]= float(anim_character->_obj->_aabb->_pos.x+ anim_character->_obj->_aabb->_size.x);
		vertices[36* idx+ 13]= float(anim_character->_obj->_aabb->_pos.y);
		vertices[36* idx+ 14]= float(anim_character->_z);
		vertices[36* idx+ 15]= 1.0f;
		vertices[36* idx+ 16]= 1.0f;
		vertices[36* idx+ 17]= current_layer;

		vertices[36* idx+ 18]= float(anim_character->_obj->_aabb->_pos.x);
		vertices[36* idx+ 19]= float(anim_character->_obj->_aabb->_pos.y+ anim_character->_obj->_aabb->_size.y);
		vertices[36* idx+ 20]= float(anim_character->_z);
		vertices[36* idx+ 21]= 0.0f;
		vertices[36* idx+ 22]= 0.0f;
		vertices[36* idx+ 23]= current_layer;

		vertices[36* idx+ 24]= float(anim_character->_obj->_aabb->_pos.x+ anim_character->_obj->_aabb->_size.x);
		vertices[36* idx+ 25]= float(anim_character->_obj->_aabb->_pos.y);
		vertices[36* idx+ 26]= float(anim_character->_z);
		vertices[36* idx+ 27]= 1.0f;
		vertices[36* idx+ 28]= 1.0f;
		vertices[36* idx+ 29]= current_layer;

		vertices[36* idx+ 30]= float(anim_character->_obj->_aabb->_pos.x+ anim_character->_obj->_aabb->_size.x);
		vertices[36* idx+ 31]= float(anim_character->_obj->_aabb->_pos.y+ anim_character->_obj->_aabb->_size.y);
		vertices[36* idx+ 32]= float(anim_character->_z);
		vertices[36* idx+ 33]= 1.0f;
		vertices[36* idx+ 34]= 0.0f;
		vertices[36* idx+ 35]= current_layer;
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, 36* _n_aabbs* sizeof(GL_FLOAT), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// -------------------------------------------------------------------------------------------------------
Character2D::Character2D() {

}


Character2D::Character2D(Object2D * obj, Texture2D * texture, number z) :
	_obj(obj), _texture(texture), _z(z)
{
	
}


Character2D::~Character2D() {
	delete _obj;
}


// AnimatedCharacter2D -----------------------------------------------
AnimatedCharacter2D::AnimatedCharacter2D() : Character2D() {

}


AnimatedCharacter2D::AnimatedCharacter2D(Object2D * obj, Texture2D * texture, number z) :
	Character2D(obj, texture, z), _current_anim(0), _accumulated_time(0.0)
{
	_current_action= _texture->_actions[0];
}


AnimatedCharacter2D::~AnimatedCharacter2D() {

}


void AnimatedCharacter2D::anim(number elapsed_time) {
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
	if (idx_action>= _texture->_actions.size()) {
		cout << "set_action " << idx_action << " trop grand\n";
		return;
	}
	_current_action= _texture->_actions[idx_action];
	_current_anim= 0;
	// l'objet recupere le footprint de l'action
	_obj->set_footprint(_current_action->_footprint);
}


void AnimatedCharacter2D::set_action(string action_name) {
	int idx_action_ok= -1;
	for (int idx_action=0; idx_action<_texture->_actions.size(); ++idx_action) {
		if (_texture->_actions[idx_action]->_name== action_name) {
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


Person2D::Person2D(Object2D * obj, Texture2D * texture, number z) : 
	AnimatedCharacter2D(obj, texture, z), _left_pressed(false), _right_pressed(false), _down_pressed(false), _up_pressed(false),
	_lshift_pressed(false), _jump(false)
{
	set_action("left_wait");
}


Person2D::~Person2D() {

}


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
	pt_2d v(0.0);
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
	
	if (v.y< 0.0) {
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


SVGParser::SVGParser(string svg_path, ScreenGL * screengl) : _screengl(screengl) {
	ifstream xml_file(svg_path);
	stringstream buffer;
	buffer << xml_file.rdbuf();
	xml_file.close();
	string xml_content(buffer.str());
	xml_document<> doc;
	doc.parse<0>(&xml_content[0]);
	xml_node<> * root_node= doc.first_node();
	
	// id pour debug
	vector<string> tags= {"id", "x", "y", "width", "height", "xlink:href", "image_name", "action_name", "physics", "anim_time", "velocity_walk", "velocity_run", "velocity_roll", "velocity_jump_walk", "velocity_jump_run", "d", "velocity", "hero"};
	
	for (xml_node<> * g_node=root_node->first_node("g"); g_node; g_node=g_node->next_sibling()) {
		string layer_label= g_node->first_attribute("inkscape:label")->value();

		for (xml_node<> * obj_node=g_node->first_node(); obj_node; obj_node=obj_node->next_sibling()) {

			map<string, string> obj;
			obj["type"]= string(obj_node->name());
			for (xml_attribute<> * attr=obj_node->first_attribute(); attr; attr=attr->next_attribute()) {
				for (auto tag : tags) {
					if (string(attr->name())== tag) {
						obj[tag]= string(attr->value());
						break;
					}
				}
			}

			if (obj["type"]== "image") {
				if (obj["xlink:href"].find("static_textures")!= string::npos) {
					obj["image_type"]= "static";
					obj["image_path"]= obj["xlink:href"].replace(0, 2, "../data");
					obj["image_name"]= basename(obj["image_path"]);
					obj["character_type"]= "CHARACTER_2D";
				}
				else if (obj["xlink:href"].find("anim_textures")!= string::npos) {
					obj["image_type"]= "anim";
					string image_path= obj["xlink:href"].replace(0, 2, "../data");
					obj["image_path"]= image_path.substr(0, image_path.find("/pngs"));
					obj["image_name"]= basename(obj["image_path"]);
					if (obj["image_path"].find("persons")!= string::npos) {
						obj["character_type"]= "PERSON_2D";
					}
					else {
						obj["character_type"]= "ANIM_CHARACTER_2D";
					}
					string action_name= obj["xlink:href"].substr(obj["xlink:href"].find("pngs/")+ 5);
					obj["action_name"]= action_name.substr(0, action_name.find("/"));
				}
				else {
					cout << "image ni static ni anim\n";
					continue;
				}
			}

			// dans le calque Models on met des images qui ne feront pas parties du level, mais servent à initialiser les textures
			if (layer_label== "Models") {
				_models.push_back(obj);
			}
			// calque View contient un rectangle d'init du pt de vue au chargement du level
			else if (layer_label== "View") {
				_view= new AABB_2D(pt_2d(stof(obj["x"]), stof(obj["y"])), pt_2d(stof(obj["width"]), stof(obj["height"])));
			}
			else {
				// tous les objs d'un meme calque ont le meme z
				obj["z"]= g_node->first_attribute("z")->value();
				_objs.push_back(obj);
			}
		}
	}
}


SVGParser::~SVGParser() {
	delete _view;
}


// SVG : origine = pt haut gauche ; y positif pointe vers le bas
AABB_2D SVGParser::svg2screen(AABB_2D aabb) {
	number x= (aabb._pos.x- (_view->_pos.x+ _view->_size.x* 0.5f))* _screengl->_gl_width / _view->_size.x;
	number y= ((_view->_pos.y+ _view->_size.y* 0.5f)- aabb._pos.y)* _screengl->_gl_height/ _view->_size.y;
	number w= aabb._size.x* _screengl->_gl_width / _view->_size.x;
	number h= aabb._size.y* _screengl->_gl_height/ _view->_size.y;

	number snap= 0.2f;
	x= round(x/ snap)* snap;
	y= round(y/ snap)* snap;
	w= round(w/ snap)* snap;
	h= round(h/ snap)* snap;

	return AABB_2D(pt_2d(x, y- h), pt_2d(w, h));
}


// Level -------------------------------------------------------------------------------------------------
Level::Level() {

}


void Level::gen_textures(GLuint prog_draw_anim, GLuint prog_draw_static, ScreenGL * screengl, SVGParser * svg_parser, bool verbose) {
	for (auto model : svg_parser->_models) {
		if (model["type"]!= "image") {
			continue;
		}
		if (!model.count("physics")) {
			continue;
		}
		ObjectPhysics physics= str2physics(model["physics"]);
		CharacterType character_type= str2character_type(model["character_type"]);
		if ((model["image_type"]== "static") && (get_texture(model["image_name"], false)== nullptr)) {
			_textures.push_back(new StaticTexture(prog_draw_static, model["image_path"], screengl, physics, character_type));
			if (verbose) {
				cout << "add static texture " << model["image_name"] << "\n";
			}
		}
		else if ((model["image_type"]== "anim") && (get_texture(model["image_name"], false)== nullptr)) {
			_textures.push_back(new AnimTexture(prog_draw_anim, model["image_path"], screengl, physics, character_type));
			if (verbose) {
				cout << "add anim texture " << model["image_name"] << "\n";
			}
		}
	}
}


void Level::update_static_textures(SVGParser * svg_parser) {
	for (auto static_texture : _textures) {
		if (!dynamic_cast<StaticTexture *>(static_texture)) {
			continue;
		}
		
		for (auto model : svg_parser->_models) {
			if (model["type"]!= "image") {
				continue;
			}
			if (model["image_type"]!= "static") {
				continue;
			}
			if (model["image_name"]!= static_texture->_name) {
				continue;
			}
			
			AABB_2D aabb_svg(pt_2d(stof(model["x"]), stof(model["y"])), pt_2d(stof(model["width"]), stof(model["height"])));
			AABB_2D aabb_gl= svg_parser->svg2screen(aabb_svg);

			// si un rect correspond au model on modifie le footprint de l'unique action de la texture statique
			for (auto rect : svg_parser->_models) {
				if (rect["type"]!= "rect") {
					continue;
				}
				if (rect["image_name"]!= static_texture->_name) {
					continue;
				}
				
				AABB_2D footprint_svg(pt_2d(stof(rect["x"]), stof(rect["y"])), pt_2d(stof(rect["width"]), stof(rect["height"])));
				AABB_2D footprint_gl= svg_parser->svg2screen(footprint_svg);
				// Action._footprint entre 0 et 1 pour pos et size
				static_texture->_actions[0]->_footprint->_pos= (footprint_gl._pos- aabb_gl._pos)/ aabb_gl._size;
				static_texture->_actions[0]->_footprint->_size= footprint_gl._size/ aabb_gl._size;
				break;
			}
			break;
		}
	}
}


void Level::update_anim_textures(SVGParser * svg_parser) {
	for (auto anim_texture : _textures) {
		if (!dynamic_cast<AnimTexture *>(anim_texture)) {
			continue;
		}

		// 1ere étape : on init toutes les actions avec la version du modele qui a physics en paramètre
		// et avec pour footprint l'emprise du rect dont action_name == default
		for (auto model : svg_parser->_models) {
			if (model["type"]!= "image") {
				continue;
			}
			if (model["image_type"]!= "anim") {
				continue;
			}
			if (!model.count("physics")) {
				continue;
			}
			if (model["image_name"]!= anim_texture->_name) {
				continue;
			}

			// récup d'éventuelles vitesses
			vector<string> velocity_tags= {"velocity_walk", "velocity_run", "velocity_roll", "velocity_jump_walk", "velocity_jump_run"};
			for (auto tag : velocity_tags) {
				if (model.count(tag)) {
					dynamic_cast<AnimTexture *>(anim_texture)->_velocities[tag.substr(tag.find("_")+ 1)]= stof(model[tag]);
				}
			}
			
			AABB_2D aabb_svg(pt_2d(stof(model["x"]), stof(model["y"])), pt_2d(stof(model["width"]), stof(model["height"])));
			AABB_2D aabb_gl= svg_parser->svg2screen(aabb_svg);

			for (auto action : anim_texture->_actions) {
				// récup éventuelle d'un temps d'anim
				if (model.count("anim_time")) {
					action->_anim_time= stof(model["anim_time"]);
				}

				for (auto rect : svg_parser->_models) {
					if (rect["type"]!= "rect") {
						continue;
					}
					if (rect["image_name"]!= anim_texture->_name) {
						continue;
					}
					if (rect["action_name"]!= "default") {
						continue;
					}
					
					// footprint du rect action_name == default
					AABB_2D footprint_svg(pt_2d(stof(rect["x"]), stof(rect["y"])), pt_2d(stof(rect["width"]), stof(rect["height"])));
					AABB_2D footprint_gl= svg_parser->svg2screen(footprint_svg);
					action->_footprint->_pos= (footprint_gl._pos- aabb_gl._pos)/ aabb_gl._size;
					action->_footprint->_size= footprint_gl._size/ aabb_gl._size;
					// faut-il forcer cela ?
					//action->_footprint->_pos.y= 0.0;
					break;
				}
			}
		}

		// 2e étape : spécifications potentielles des params ou footprint d'autres versions du modele
		for (auto model : svg_parser->_models) {
			if (model["type"]!= "image") {
				continue;
			}
			if (model["image_type"]!= "anim") {
				continue;
			}
			if (model["image_name"]!= anim_texture->_name) {
				continue;
			}
			
			// action spécifique
			Action * action= anim_texture->get_action(model["action_name"]);

			AABB_2D aabb_svg(pt_2d(stof(model["x"]), stof(model["y"])), pt_2d(stof(model["width"]), stof(model["height"])));
			AABB_2D aabb_gl= svg_parser->svg2screen(aabb_svg);

			if (model.count("anim_time")) {
				action->_anim_time= stof(model["anim_time"]);
			}

			for (auto rect : svg_parser->_models) {
				if (rect["type"]!= "rect") {
					continue;
				}
				if (rect["image_name"]!= anim_texture->_name) {
					continue;
				}
				// rect lié à l'action considérée
				if (rect["action_name"]!= action->_name) {
					continue;
				}
				
				AABB_2D footprint_svg(pt_2d(stof(rect["x"]), stof(rect["y"])), pt_2d(stof(rect["width"]), stof(rect["height"])));
				AABB_2D footprint_gl= svg_parser->svg2screen(footprint_svg);
				action->_footprint->_pos= (footprint_gl._pos- aabb_gl._pos)/ aabb_gl._size;
				action->_footprint->_size= footprint_gl._size/ aabb_gl._size;
				// faut-il forcer cela ?
				//action->_footprint->_pos.y= 0.0;
				break;
			}
		}
	}
}


void Level::add_characters(SVGParser * svg_parser, bool verbose) {
	for (auto obj : svg_parser->_objs) {
		if (obj["type"]!= "image") {
			continue;
		}
		AABB_2D aabb_svg= AABB_2D(pt_2d(stof(obj["x"]), stof(obj["y"])), pt_2d(stof(obj["width"]), stof(obj["height"])));
		AABB_2D aabb_gl= svg_parser->svg2screen(aabb_svg);

		vector<CheckPoint> checkpoints;
		ObjectPhysics physics= get_texture(obj["image_name"])->_physics;
		if ((physics== CHECKPOINT_SOLID) || (physics== CHECKPOINT_UNSOLID) || (physics== CHECKPOINT_SOLID_TOP)) {
			for (auto path : svg_parser->_objs) {
				if (path["type"]!= "path") {
					continue;
				}
				number velocity= stof(path["velocity"]);

				istringstream iss(path["d"]);
				string token;
				string instruction= "";
				// dans les preferences de inkscape aller à Entrée/Sortie / Sortie SVG / Format de la chaine du chemin
				// et mettre Absolu
				while (getline(iss, token, ' ')) {
					if (token== "M") { // M = move en absolu
						instruction= "M";
					}
					else if (token== "H") { // H = deplacement horizontal en absolu
						instruction= "H";
					}
					else {
						if (instruction== "M") {
							number x= stof(token.substr(0, token.find(",")));
							number y= stof(token.substr(token.find(",")+ 1));
							pt_2d pt= pt_2d(x, y);
							// on cherche le path dont le 1er point est contenu dans l'emprise de l'objet
							if (!point_in_aabb(pt, &aabb_svg)) {
								break;
							}
							checkpoints.push_back({pt, velocity});
						}
						else if (instruction== "H") {
							number x= stof(token);
							number y= checkpoints[checkpoints.size()- 1]._pos.y;
							pt_2d pt= pt_2d(x, y);
							checkpoints.push_back({pt, velocity});
						}
					}
				}
			}
		}

		if (checkpoints.size()) {
			// conversion dans l'espace GL
			for (unsigned int i=0; i<checkpoints.size(); ++i) {
				// aabb reduite a un pt -> size == 0
				AABB_2D pt_gl= svg_parser->svg2screen(AABB_2D(checkpoints[i]._pos, pt_2d(0.0)));
				checkpoints[i]._pos= pt_gl._pos;
			}
			pt_2d v= aabb_gl._pos- checkpoints[0]._pos;
			for (unsigned int i=0; i<checkpoints.size(); ++i) {
				checkpoints[i]._pos+= v;
			}
		}

		// on ne fait de texture->update() a chaque fois, mais une fois que tout est chargé
		add_character(obj["image_name"], &aabb_gl, stof(obj["z"]), checkpoints, false);
		if (verbose) {
			cout << "add character " << obj["image_name"] << " ; " << obj["id"] << "\n";
		}

		// si le tag hero est présent (quelque soit sa valeur)
		if (obj.count("hero")) {
			_hero= dynamic_cast<Person2D *>(_characters[_characters.size()- 1]);
		}
	}
}


Level::Level(GLuint prog_draw_anim, GLuint prog_draw_static, GLuint prog_draw_aabb, string path, ScreenGL * screengl, bool verbose) :
	_screengl(screengl), _viewpoint(pt_2d(0.0)), _draw(true)
{
	SVGParser * svg_parser= new SVGParser(path, _screengl);
	gen_textures(prog_draw_anim, prog_draw_static, screengl, svg_parser, verbose);
	update_static_textures(svg_parser);
	update_anim_textures(svg_parser);
	add_characters(svg_parser, verbose);
	delete svg_parser;

	update_textures();

	if (verbose) {
		cout << "fin chargement level\n";
	}
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


Texture2D * Level::get_texture(string texture_name, bool verbose) {
	for (auto texture : _textures) {
		if (texture->_name== texture_name) {
			return texture;
		}
	}
	if (verbose) {
		cout << "texture non trouvee : " << texture_name << "\n";
	}
	return nullptr;
}


void Level::add_character(string texture_name, AABB_2D * aabb, number z, vector<CheckPoint> checkpoints, bool update_texture) {
	Texture2D * texture= get_texture(texture_name);
	Object2D * obj= new Object2D(aabb, texture->_actions[0]->_footprint, texture->_physics, checkpoints);
	Character2D * character;
	if (texture->_character_type== CHARACTER_2D) {
		character= new Character2D(obj, texture, z);
	}
	else if (texture->_character_type== ANIM_CHARACTER_2D) {
		character= new AnimatedCharacter2D(obj, texture, z);
	}
	else if (texture->_character_type== PERSON_2D) {
		character= new Person2D(obj, texture, z);
	}
	_characters.push_back(character);
	texture->_characters.push_back(character);

	if (update_texture) {
		texture->update();
	}
}


void Level::delete_character(Character2D * character) {
	Texture2D * texture= character->_texture;
	// https://en.wikipedia.org/wiki/Erase-remove_idiom
	texture->_characters.erase(remove(texture->_characters.begin(), texture->_characters.end(), character), texture->_characters.end());
	delete character;
	_characters.erase(remove(_characters.begin(), _characters.end(), character), _characters.end());
}


void Level::update_velocities() {
	// si referential, vx = ref.vx
	for (auto character : _characters) {
		Object2D * obj= character->_obj;
		if (obj->_physics!= FALLING) {
			continue;
		}
		obj->_velocity.x= 0.0;
		if (obj->_referential!= nullptr) {
			obj->_velocity.x= obj->_referential->_velocity.x;
		}
	}

	// gravité + checkpoints
	for (auto character : _characters) {
		Object2D * obj= character->_obj;
		obj->update_velocity();
	}

	// init velocity en fonction de l'action courante
	for (auto character : _characters) {
		Person2D * person= dynamic_cast<Person2D *>(character);
		if (person) {
			if (person!= _hero) {
				person->ia();
			}
			person->update_velocity();
		}
	}
}


void Level::intersections(number elapsed_time) {
	pt_2d contact_pt(0.0);
	pt_2d contact_normal(0.0);
	number contact_time= 0.0;

	for (auto character : _characters) {
		Object2D * obj= character->_obj;
		if (obj->_physics!= FALLING) {
			continue;
		}
		
		obj->_bottom.clear();
		obj->_top.clear();
		obj->_referential= nullptr;
	}

	// pour chaque objet FALLING (perso par ex), si intersecte la prochaine fois un objet solide, correction
	// + renseignement _bottom, _top, _referential
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

			if ((obj2->_physics!= STATIC_DESTRUCTIBLE) && (obj2->_physics!= STATIC_INDESTRUCTIBLE) && (obj2->_physics!= CHECKPOINT_SOLID) && (obj2->_physics!= CHECKPOINT_SOLID_TOP)) {
				continue;
			}

			if (anim_intersect_static(obj1, obj2, elapsed_time, contact_pt, contact_normal, contact_time)) {
				// les CHECKPOINT_SOLID_TOP ne font du contact que lorsqu'on les approche par dessus
				if ((obj2->_physics!= CHECKPOINT_SOLID_TOP) || (contact_normal.y> 0.0)) {
					// cf refs dans bbox_2d.h
					pt_2d correction= (1.0- contact_time)* pt_2d(abs(obj1->_velocity.x)* contact_normal.x, abs(obj1->_velocity.y)* contact_normal.y);
					// malheureusement ca ne marche pas nickel, il faut * par 1.xxx a cause de l'approx number, et encore ca foire parfois. Que faire ?
					obj1->_velocity+= correction* CORRECT_FACTOR;
				}

				if (contact_normal.y> 0.0) {
					obj1->_bottom.push_back(obj2);
					if ((obj2->_physics== CHECKPOINT_SOLID) || (obj2->_physics== CHECKPOINT_SOLID_TOP)) {
						// on est sur une plateforme
						obj1->_referential= obj2;
					}
				}
				else if (contact_normal.y< 0.0) {
					obj1->_top.push_back(obj2);
				}
			}
		}
	}

	// pour chaque objet CHECKPOINT_SOLID, si intersecte un FALLING, on corrige la vélocité du FALLING
	// ex d'une plateforme qui rentre dans un perso
	for (auto character1 : _characters) {
		Object2D * obj1= character1->_obj;
		if ((obj1->_physics!= CHECKPOINT_SOLID) || (obj1->_physics!= CHECKPOINT_SOLID_TOP)) {
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

			// on considère le FALLING après update position
			Object2D * obj_tmp= new Object2D(*obj2);
			obj_tmp->update_pos(elapsed_time);
			if (anim_intersect_static(obj1, obj_tmp, elapsed_time, contact_pt, contact_normal, contact_time)) {
				if ((obj1->_physics!= CHECKPOINT_SOLID_TOP) || (contact_normal.y> 0.0)) {
					pt_2d correction= (1.0- contact_time)* pt_2d(abs(obj1->_velocity.x)* contact_normal.x, abs(obj1->_velocity.y)* contact_normal.y);
					obj2->_velocity-= correction* CORRECT_FACTOR;
				}

				if (contact_normal.y< 0.0) {
					// obj2 sur platform obj1
					obj2->_bottom.push_back(obj1);
					obj2->_referential= obj1;
				}
				else if (contact_normal.y> 0.0) {
					obj2->_top.push_back(obj1);
				}
			}
			delete obj_tmp;
		}
	}
}


void Level::deletes() {
	// suppression des characters destructibles touchés par en dessous
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

	// si coincé entre 2 objets, mort
	for (auto character : _characters) {
		Object2D * obj= character->_obj;
		if (obj->_physics!= FALLING) {
			continue;
		}
		if ((obj->_bottom.size()) && (obj->_top.size())) {
			cout << "death\n";
		}
	}
}


void Level::update_positions(number elapsed_time) {
	// pos = pos + k * velocity
	for (auto character : _characters) {
		Object2D * obj= character->_obj;
		obj->update_pos(elapsed_time);
	}
}


void Level::update_actions() {
	// passage d'une action à une autre en fonction de la vitesse
	for (auto character : _characters) {
		Person2D * person= dynamic_cast<Person2D *>(character);
		if (person) {
			person->update_action();
		}
	}
}


void Level::anim_characters(number elapsed_time) {
	// animation chars animables
	for (auto character : _characters) {
		AnimatedCharacter2D * anim_character= dynamic_cast<AnimatedCharacter2D *>(character);
		if (anim_character) {
			anim_character->anim(elapsed_time);
		}
	}
}


void Level::update_textures() {
	for (auto texture : _textures) {
		texture->update();
	}
}


void Level::follow_hero() {
	// la caméra suit le héros
	pt_2d hero= _hero->_obj->_aabb->center();
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

	for (auto texture: _textures) {
		texture->set_model2world(glm::translate(glm::mat4(1.0), glm::vec3(-_viewpoint.x, -_viewpoint.y, 0.0)));
	}
}


void Level::anim(number elapsed_time) {
	update_velocities();
	intersections(elapsed_time);
	deletes();
	update_positions(elapsed_time);
	update_actions();
	anim_characters(elapsed_time);
	update_textures();
	follow_hero();
}


void Level::draw() {
	if (_draw) {
		for (auto texture : _textures) {
			texture->draw();
		}
	}
}


bool Level::key_down(InputState * input_state, SDL_Keycode key) {
	if ((key== SDLK_DOWN) || (key== SDLK_UP) || (key== SDLK_LEFT) || (key== SDLK_RIGHT) || (key== SDLK_a)) {
		_hero->key_down(key);
		return true;
	}
	else if (key== SDLK_SPACE) {
		cout << _hero->current_action() << "\n" << *(_hero->_obj) << "\n";
		return true;
	}
	else if (key== SDLK_e) {
		_draw= !_draw;
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
	_camera2clip= glm::ortho(-0.5f* float(_screengl->_gl_width), 0.5f* float(_screengl->_gl_width), -0.5f* float(_screengl->_gl_height), 0.5f* float(_screengl->_gl_height), Z_NEAR, Z_FAR);
	_model2world= glm::mat4(1.0);

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

	glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(number), (void*)0);
	glVertexAttribPointer(_color_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(number), (void*)(2* sizeof(number)));

	glDrawArrays(GL_LINES, 0, 8* _n_aabbs);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void LevelDebug::update() {
	glm::vec3 aabb_color(1.0, 0.0, 0.0);
	glm::vec3 footprint_color(0.0, 1.0, 0.0);

	unsigned int n_chars= _level->_characters.size();

	_n_aabbs= n_chars* 2;
	float vertices[_n_aabbs* 40];
	for (unsigned int i=0; i<_n_aabbs* 40; ++i) {
		vertices[i]= 0.0;
	}

	if (_draw_aabb) {
		for (unsigned int idx=0; idx<n_chars; ++idx) {
			AABB_2D * aabb= _level->_characters[idx]->_obj->_aabb;
			vertices[40* idx+ 0]= float(aabb->_pos.x);
			vertices[40* idx+ 1]= float(aabb->_pos.y);
			vertices[40* idx+ 5]= float(aabb->_pos.x+ aabb->_size.x);
			vertices[40* idx+ 6]= float(aabb->_pos.y);

			vertices[40* idx+ 10]= float(aabb->_pos.x+ aabb->_size.x);
			vertices[40* idx+ 11]= float(aabb->_pos.y);
			vertices[40* idx+ 15]= float(aabb->_pos.x+ aabb->_size.x);
			vertices[40* idx+ 16]= float(aabb->_pos.y+ aabb->_size.y);

			vertices[40* idx+ 20]= float(aabb->_pos.x+ aabb->_size.x);
			vertices[40* idx+ 21]= float(aabb->_pos.y+ aabb->_size.y);
			vertices[40* idx+ 25]= float(aabb->_pos.x);
			vertices[40* idx+ 26]= float(aabb->_pos.y+ aabb->_size.y);

			vertices[40* idx+ 30]= float(aabb->_pos.x);
			vertices[40* idx+ 31]= float(aabb->_pos.y+ aabb->_size.y);
			vertices[40* idx+ 35]= float(aabb->_pos.x);
			vertices[40* idx+ 36]= float(aabb->_pos.y);

			for (unsigned int i=0; i<8; ++i) {
				vertices[40* idx+ 5* i+ 2]= aabb_color.x;
				vertices[40* idx+ 5* i+ 3]= aabb_color.y;
				vertices[40* idx+ 5* i+ 4]= aabb_color.z;
			}
		}
	}
	
	if (_draw_footprint) {
		for (unsigned int idx=0; idx<n_chars; ++idx) {
			AABB_2D * footprint= _level->_characters[idx]->_obj->_footprint;
			vertices[n_chars* 40+ 40* idx+ 0]= float(footprint->_pos.x);
			vertices[n_chars* 40+ 40* idx+ 1]= float(footprint->_pos.y);
			vertices[n_chars* 40+ 40* idx+ 5]= float(footprint->_pos.x+ footprint->_size.x);
			vertices[n_chars* 40+ 40* idx+ 6]= float(footprint->_pos.y);

			vertices[n_chars* 40+ 40* idx+ 10]= float(footprint->_pos.x+ footprint->_size.x);
			vertices[n_chars* 40+ 40* idx+ 11]= float(footprint->_pos.y);
			vertices[n_chars* 40+ 40* idx+ 15]= float(footprint->_pos.x+ footprint->_size.x);
			vertices[n_chars* 40+ 40* idx+ 16]= float(footprint->_pos.y+ footprint->_size.y);

			vertices[n_chars* 40+ 40* idx+ 20]= float(footprint->_pos.x+ footprint->_size.x);
			vertices[n_chars* 40+ 40* idx+ 21]= float(footprint->_pos.y+ footprint->_size.y);
			vertices[n_chars* 40+ 40* idx+ 25]= float(footprint->_pos.x);
			vertices[n_chars* 40+ 40* idx+ 26]= float(footprint->_pos.y+ footprint->_size.y);

			vertices[n_chars* 40+ 40* idx+ 30]= float(footprint->_pos.x);
			vertices[n_chars* 40+ 40* idx+ 31]= float(footprint->_pos.y+ footprint->_size.y);
			vertices[n_chars* 40+ 40* idx+ 35]= float(footprint->_pos.x);
			vertices[n_chars* 40+ 40* idx+ 36]= float(footprint->_pos.y);

			for (unsigned int i=0; i<8; ++i) {
				vertices[n_chars* 40+ 40* idx+ 5* i+ 2]= footprint_color.x;
				vertices[n_chars* 40+ 40* idx+ 5* i+ 3]= footprint_color.y;
				vertices[n_chars* 40+ 40* idx+ 5* i+ 4]= footprint_color.z;
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, 40* _n_aabbs* sizeof(GL_FLOAT), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// suivre le mouvement de caméra
	_model2world= glm::translate(glm::mat4(1.0), glm::vec3(-_level->_viewpoint.x, -_level->_viewpoint.y, 0.0));
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

