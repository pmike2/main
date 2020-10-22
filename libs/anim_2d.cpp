#include "anim_2d.h"

using namespace std;
using namespace rapidxml;

// Action ---------------------------------------------------------------------------
void Action::print() {
	cout << "_name=" << _name << " ; _first_idx=" << _first_idx << " ; _n_idx=" << _n_idx << "\n";
	for (auto png : _pngs) {
		cout << png << "\n";
	}
}


// StaticObj --------------------------------------------------------------------------------------------
StaticObj::StaticObj() {

}


StaticObj::StaticObj(glm::vec2 pos, glm::vec2 size, glm::vec2 footprint_offset, glm::vec2 footprint_size) {
	_aabb= new AABB_2D(pos, size);
	_footprint= new AABB_2D(glm::vec2(0.0f), glm::vec2(size.x* footprint_size.x, size.y* footprint_size.y));
	_footprint_offset= glm::vec2(size.x* footprint_offset.x, size.y* footprint_offset.y);
	update_footprint_pos();
}


StaticObj::~StaticObj() {
	delete _aabb;
}


void StaticObj::update_footprint_pos() {
	_footprint->_pos= _aabb->_pos+ _footprint_offset;
}


// AnimObj --------------------------------------------------------------------------------------------
AnimObj::AnimObj() {

}


AnimObj::AnimObj(glm::vec2 pos, glm::vec2 size, glm::vec2 footprint_offset, glm::vec2 footprint_size) : _velocity(glm::vec2(0.0f)) {
	_aabb= new AABB_2D(pos, size);
	_footprint= new AABB_2D(glm::vec2(0.0f), glm::vec2(size.x* footprint_size.x, size.y* footprint_size.y));
	_footprint_offset= glm::vec2(size.x* footprint_offset.x, size.y* footprint_offset.y);
	update_footprint_pos();
}


AnimObj::~AnimObj() {
	delete _aabb;
}


void AnimObj::anim(float elapsed_time) {
	_aabb->_pos+= _velocity* elapsed_time;
	update_footprint_pos();
}


void AnimObj::update_footprint_pos() {
	_footprint->_pos= _aabb->_pos+ _footprint_offset;
}


// -------------------------------------------------------------------------------------------
bool anim_intersect_static(const AnimObj * anim_obj, const StaticObj * static_obj, const float time_step, glm::vec2 & contact_pt, glm::vec2 & contact_normal, float & contact_time) {
	if (glm::length2(anim_obj->_velocity)< 1e-6f) {
		return false;
	}

	AABB_2D expanded;
	expanded._pos= static_obj->_footprint->_pos- 0.5f* anim_obj->_footprint->_size;
	expanded._size= static_obj->_footprint->_size+ anim_obj->_footprint->_size;

	if (ray_intersects_aabb(anim_obj->_footprint->_pos+ 0.5f* anim_obj->_footprint->_size, time_step* anim_obj->_velocity, &expanded, contact_pt, contact_normal, contact_time)) {
		return ((contact_time> 0.0f) && (contact_time< 1.0f));
	}
	
	return false;
}


// StaticTexture ----------------------------------------------------------------------------------------------------
StaticTexture::StaticTexture() {

}


StaticTexture::StaticTexture(GLuint prog_draw, string path, ScreenGL * screengl) : _prog_draw(prog_draw), _screengl(screengl), _alpha(1.0f), _n_aabbs(0) {
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
	_footprint_offset= glm::vec2(0.2f, 0.2f);
	_footprint_size= glm::vec2(0.5f, 0.5f);
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


void StaticTexture::update(vector<StaticCharacter *> static_chars) {
	_n_aabbs= static_chars.size();
	float vertices[30* _n_aabbs];
	for (unsigned int idx=0; idx<_n_aabbs; ++idx) {
		//glm::vec2 tex_coord= static_chars[idx]->_static_obj->_aabb->_pt_max- static_chars[idx]->_static_obj->_aabb->_pt_min;
		glm::vec2 tex_coord(1.0f, 1.0f);

		vertices[30* idx+ 0]= static_chars[idx]->_static_obj->_aabb->_pos.x;
		vertices[30* idx+ 1]= static_chars[idx]->_static_obj->_aabb->_pos.y+ static_chars[idx]->_static_obj->_aabb->_size.y;
		vertices[30* idx+ 2]= static_chars[idx]->_z;
		vertices[30* idx+ 3]= 0.0f;
		vertices[30* idx+ 4]= 0.0f;

		vertices[30* idx+ 5]= static_chars[idx]->_static_obj->_aabb->_pos.x;
		vertices[30* idx+ 6]= static_chars[idx]->_static_obj->_aabb->_pos.y;
		vertices[30* idx+ 7]= static_chars[idx]->_z;
		vertices[30* idx+ 8]= 0.0f;
		vertices[30* idx+ 9]= tex_coord.y;

		vertices[30* idx+ 10]= static_chars[idx]->_static_obj->_aabb->_pos.x+ static_chars[idx]->_static_obj->_aabb->_size.x;
		vertices[30* idx+ 11]= static_chars[idx]->_static_obj->_aabb->_pos.y;
		vertices[30* idx+ 12]= static_chars[idx]->_z;
		vertices[30* idx+ 13]= tex_coord.x;
		vertices[30* idx+ 14]= tex_coord.y;

		vertices[30* idx+ 15]= static_chars[idx]->_static_obj->_aabb->_pos.x;
		vertices[30* idx+ 16]= static_chars[idx]->_static_obj->_aabb->_pos.y+ static_chars[idx]->_static_obj->_aabb->_size.y;
		vertices[30* idx+ 17]= static_chars[idx]->_z;
		vertices[30* idx+ 18]= 0.0f;
		vertices[30* idx+ 19]= 0.0f;

		vertices[30* idx+ 20]= static_chars[idx]->_static_obj->_aabb->_pos.x+ static_chars[idx]->_static_obj->_aabb->_size.x;
		vertices[30* idx+ 21]= static_chars[idx]->_static_obj->_aabb->_pos.y;
		vertices[30* idx+ 22]= static_chars[idx]->_z;
		vertices[30* idx+ 23]= tex_coord.x;
		vertices[30* idx+ 24]= tex_coord.y;

		vertices[30* idx+ 25]= static_chars[idx]->_static_obj->_aabb->_pos.x+ static_chars[idx]->_static_obj->_aabb->_size.x;
		vertices[30* idx+ 26]= static_chars[idx]->_static_obj->_aabb->_pos.y+ static_chars[idx]->_static_obj->_aabb->_size.y;
		vertices[30* idx+ 27]= static_chars[idx]->_z;
		vertices[30* idx+ 28]= tex_coord.x;
		vertices[30* idx+ 29]= 0.0f;
	}
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, 30* _n_aabbs* sizeof(float), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// StaticCharacter -----------------------------------------------------------------------
StaticCharacter::StaticCharacter() {

}


StaticCharacter::StaticCharacter(StaticObj * static_obj, StaticTexture * static_texture, float z) : _static_obj(static_obj), _static_texture(static_texture), _z(z) {

}


StaticCharacter::~StaticCharacter() {

}


// AnimTexture ---------------------------------------------------------------------------
AnimTexture::AnimTexture() {

}


AnimTexture::AnimTexture(GLuint prog_draw, std::string path, ScreenGL * screengl) : _prog_draw(prog_draw), _screengl(screengl), _n_aabbs(0) {
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

	// A FAIRE EVOLUER !
	_footprint_offset= glm::vec2(0.2f, 0.2f);
	_footprint_size= glm::vec2(0.5f, 0.5f);
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


void AnimTexture::update(vector<AnimCharacter *> anim_chars) {
	_n_aabbs= anim_chars.size();
	float vertices[36* _n_aabbs];
	for (unsigned int idx=0; idx<_n_aabbs; ++idx) {
		//glm::vec2 tex_coord= anim_chars[idx]->_anim_obj->_aabb->_pt_max- anim_chars[idx]->_anim_obj->_aabb->_pt_min;
		glm::vec2 tex_coord(1.0f, 1.0f);
		float current_layer= (float)(anim_chars[idx]->_current_action->_first_idx+ anim_chars[idx]->_current_anim);

		vertices[36* idx+ 0]= anim_chars[idx]->_anim_obj->_aabb->_pos.x;
		vertices[36* idx+ 1]= anim_chars[idx]->_anim_obj->_aabb->_pos.y+ anim_chars[idx]->_anim_obj->_aabb->_size.y;
		vertices[36* idx+ 2]= anim_chars[idx]->_z;
		vertices[36* idx+ 3]= 0.0f;
		vertices[36* idx+ 4]= 0.0f;
		vertices[36* idx+ 5]= current_layer;

		vertices[36* idx+ 6]= anim_chars[idx]->_anim_obj->_aabb->_pos.x;
		vertices[36* idx+ 7]= anim_chars[idx]->_anim_obj->_aabb->_pos.y;
		vertices[36* idx+ 8]= anim_chars[idx]->_z;
		vertices[36* idx+ 9]= 0.0f;
		vertices[36* idx+ 10]= tex_coord.y;
		vertices[36* idx+ 11]= current_layer;

		vertices[36* idx+ 12]= anim_chars[idx]->_anim_obj->_aabb->_pos.x+ anim_chars[idx]->_anim_obj->_aabb->_size.x;
		vertices[36* idx+ 13]= anim_chars[idx]->_anim_obj->_aabb->_pos.y;
		vertices[36* idx+ 14]= anim_chars[idx]->_z;
		vertices[36* idx+ 15]= tex_coord.x;
		vertices[36* idx+ 16]= tex_coord.y;
		vertices[36* idx+ 17]= current_layer;

		vertices[36* idx+ 18]= anim_chars[idx]->_anim_obj->_aabb->_pos.x;
		vertices[36* idx+ 19]= anim_chars[idx]->_anim_obj->_aabb->_pos.y+ anim_chars[idx]->_anim_obj->_aabb->_size.y;
		vertices[36* idx+ 20]= anim_chars[idx]->_z;
		vertices[36* idx+ 21]= 0.0f;
		vertices[36* idx+ 22]= 0.0f;
		vertices[36* idx+ 23]= current_layer;

		vertices[36* idx+ 24]= anim_chars[idx]->_anim_obj->_aabb->_pos.x+ anim_chars[idx]->_anim_obj->_aabb->_size.x;
		vertices[36* idx+ 25]= anim_chars[idx]->_anim_obj->_aabb->_pos.y;
		vertices[36* idx+ 26]= anim_chars[idx]->_z;
		vertices[36* idx+ 27]= tex_coord.x;
		vertices[36* idx+ 28]= tex_coord.y;
		vertices[36* idx+ 29]= current_layer;

		vertices[36* idx+ 30]= anim_chars[idx]->_anim_obj->_aabb->_pos.x+ anim_chars[idx]->_anim_obj->_aabb->_size.x;
		vertices[36* idx+ 31]= anim_chars[idx]->_anim_obj->_aabb->_pos.y+ anim_chars[idx]->_anim_obj->_aabb->_size.y;
		vertices[36* idx+ 32]= anim_chars[idx]->_z;
		vertices[36* idx+ 33]= tex_coord.x;
		vertices[36* idx+ 34]= 0.0f;
		vertices[36* idx+ 35]= current_layer;
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, 36* _n_aabbs* sizeof(float), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// -------------------------------------------------------------------------------------------------------
AnimCharacter::AnimCharacter() {

}


AnimCharacter::AnimCharacter(AnimObj * anim_obj, AnimTexture * anim_texture, float z) : _anim_obj(anim_obj), _anim_texture(anim_texture), _z(z), _current_anim(0), _accumulated_time(0.0f) {
	_current_action= _anim_texture->_actions[0];
}


AnimCharacter::~AnimCharacter() {

}


void AnimCharacter::anim(float elapsed_time) {
	_accumulated_time+= elapsed_time;
	if (_accumulated_time>= ANIM_TIME) {
		_accumulated_time-= ANIM_TIME;
		_current_anim++;
		if (_current_anim>= _current_action->_n_idx) {
			_current_anim= 0;
		}
	}
}


void AnimCharacter::set_action(unsigned int idx_action) {
	if (idx_action>= _anim_texture->_actions.size()) {
		cout << "set_action " << idx_action << " trop grand\n";
		return;
	}
	_current_action= _anim_texture->_actions[idx_action];
	_current_anim= 0;
}


void AnimCharacter::set_action(string action_name) {
	bool found= false;
	for (auto action : _anim_texture->_actions) {
		if (action->_name== action_name) {
			found= true;
			_current_action= action;
			break;
		}
	}
	if (!found) {
		cout << "action non trouvee : " << action_name << "\n";
		return;
	}

	_current_anim= 0;
}


// Level -------------------------------------------------------------------------------------------------
Level::Level() {

}


Level::Level(GLuint prog_draw_anim, GLuint prog_draw_static, GLuint prog_draw_aabb, ScreenGL * screengl) :
	_w(LEVEL_WIDTH) ,_h(LEVEL_HEIGHT), _screengl(screengl), _left_pressed(false), _right_pressed(false), _down_pressed(false), _up_pressed(false)
{
	_block_w= _screengl->_gl_width/ (float)(_w);
	_block_h= _screengl->_gl_height/ (float)(_h);

	// static -------------
	_static_textures.push_back(new StaticTexture(prog_draw_static, "./static_textures/brick.png", screengl));
	_static_textures.push_back(new StaticTexture(prog_draw_static, "./static_textures/grass.png", screengl));
	_static_textures.push_back(new StaticTexture(prog_draw_static, "./static_textures/tree.png", screengl));
	_static_textures.push_back(new StaticTexture(prog_draw_static, "./static_textures/hill.png", screengl));
	_static_textures.push_back(new StaticTexture(prog_draw_static, "./static_textures/sky.png", screengl));

	for (unsigned int col=0; col<_w; ++col) {
		for (unsigned int row=0; row<_h; ++row) {
			if ((row== 4) && (((col> 4) && (col< 10)))) {
				glm::vec2 pos(-0.5f* _screengl->_gl_width+ (float)(col)* _block_w, -0.5f* _screengl->_gl_height+ (float)(row)* _block_h);
				glm::vec2 size= glm::vec2(_block_w, _block_h);
				_static_objs.push_back(new StaticObj(pos, size, _static_textures[0]->_footprint_offset, _static_textures[0]->_footprint_size));
				_static_characters.push_back(new StaticCharacter(_static_objs[_static_objs.size()- 1], _static_textures[0], 0.0f));
			}
		}
	}

	/*for (unsigned int i=0; i<10000; ++i) {
		glm::vec2 pt_min(rand_float(-5.0f, 5.0f), rand_float(-5.0f, 5.0f));
		glm::vec2 pt_max= pt_min+ glm::vec2(1.0f, 1.0f);
		_static_objs.push_back(new StaticObj(pt_min, pt_max));
		_static_characters.push_back(new StaticCharacter(_static_objs[_static_objs.size()- 1], _static_textures[rand_int(0, 1)], 0.0f));
	}*/

	for (auto st : _static_textures) {
		vector<StaticCharacter *> static_chars;
		for (auto sc : _static_characters) {
			if (sc->_static_texture== st) {
				static_chars.push_back(sc);
			}
		}
		st->update(static_chars);
	}

	// anim -------------------
	_anim_textures.push_back(new AnimTexture(prog_draw_anim, "./anim_textures/modele_1", screengl));
	_anim_textures.push_back(new AnimTexture(prog_draw_anim, "./anim_textures/modele_2", screengl));
	
	_anim_objs.push_back(new AnimObj(glm::vec2(0.0f, 0.0f), glm::vec2(2.0f, 2.0f), _anim_textures[0]->_footprint_offset, _anim_textures[0]->_footprint_size));
	_anim_characters.push_back(new AnimCharacter(_anim_objs[0], _anim_textures[0], 1.0f));

	/*for (unsigned int i=0; i<100; ++i) {
		glm::vec2 pt_min(rand_float(-5.0f, 5.0f), rand_float(-5.0f, 5.0f));
		glm::vec2 pt_max= pt_min+ glm::vec2(1.0f, 1.0f);
		_anim_objs.push_back(new AnimObj(pt_min, pt_max));
		_anim_characters.push_back(new AnimCharacter(_anim_objs[_anim_objs.size()- 1], _anim_textures[rand_int(0, 1)], 1.0f));
	}*/

	for (auto at : _anim_textures) {
		vector<AnimCharacter *> anim_chars;
		for (auto ac : _anim_characters) {
			if (ac->_anim_texture== at) {
				anim_chars.push_back(ac);
			}
		}
		at->update(anim_chars);
	}
}


Level::~Level() {
	for (auto static_obj : _static_objs) {
		delete static_obj;
	}
	_static_objs.clear();
	
	for (auto anim_char : _anim_characters) {
		delete anim_char;
	}
	_anim_characters.clear();

	for (auto anim_obj : _anim_objs) {
		delete anim_obj;
	}
	_anim_objs.clear();
	
	for (auto static_texture : _static_textures) {
		delete static_texture;
	}
	_static_textures.clear();
	
	for (auto anim_texture : _anim_textures) {
		delete anim_texture;
	}
	_anim_textures.clear();
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
	/*for (auto anim_char : _anim_characters) {
		if (!rand_int(0, 100)) {
			anim_char->set_action(rand_int(0, anim_char->_anim_texture->_actions.size()- 1));
		}
	}*/

	float vel= 3.0f;

	if (_left_pressed) {
		_anim_objs[0]->_velocity.x= -vel;
	}
	else if (_right_pressed) {
		_anim_objs[0]->_velocity.x= vel;
	}
	else {
		_anim_objs[0]->_velocity.x= 0.0f;
	}

	if (_up_pressed) {
		_anim_objs[0]->_velocity.y= vel;
	}
	else if (_down_pressed) {
		_anim_objs[0]->_velocity.y= -vel;
	}
	else {
		_anim_objs[0]->_velocity.y= 0.0f;
	}

	glm::vec2 contact_pt(0.0f);
	glm::vec2 contact_normal(0.0f);
	float contact_time= 0.0f;
	for (unsigned int i=0; i<_static_objs.size(); ++i) {
		if (anim_intersect_static(_anim_objs[0], _static_objs[i], elapsed_time, contact_pt, contact_normal, contact_time)) {
			cout << "inter " << i << "\n";
			cout << glm::to_string(_anim_objs[0]->_velocity) << "\n";
			cout << glm::to_string((1.0f- contact_time)* glm::vec2(abs(_anim_objs[0]->_velocity.x)* contact_normal.x, abs(_anim_objs[0]->_velocity.y)* contact_normal.y)) << "\n";
			_anim_objs[0]->_velocity+= (1.0f- contact_time)* glm::vec2(abs(_anim_objs[0]->_velocity.x)* contact_normal.x, abs(_anim_objs[0]->_velocity.y)* contact_normal.y);
			cout << glm::to_string(_anim_objs[0]->_velocity) << "\n";
		}
	}

	for (auto anim_obj : _anim_objs) {
		anim_obj->anim(elapsed_time);
	}
	
	for (auto anim_char : _anim_characters) {
		anim_char->anim(elapsed_time);
	}

	for (auto at : _anim_textures) {
		vector<AnimCharacter *> anim_chars;
		for (auto ac : _anim_characters) {
			if (ac->_anim_texture== at) {
				anim_chars.push_back(ac);
			}
		}
		at->update(anim_chars);
	}
}


bool Level::key_down(InputState * input_state, SDL_Keycode key) {
	if ((key== SDLK_LEFT) && (!_left_pressed)) {
		_left_pressed= true;
		return true;
	}
	else if ((key== SDLK_RIGHT) && (!_right_pressed)) {
		_right_pressed= true;
		return true;
	}
	else if ((key== SDLK_UP) && (!_up_pressed)) {
		_up_pressed= true;
		return true;
	}
	else if ((key== SDLK_DOWN) && (!_down_pressed)) {
		_down_pressed= true;
		return true;
	}

	return false;
}


bool Level::key_up(InputState * input_state, SDL_Keycode key) {
	if (key== SDLK_LEFT) {
		_left_pressed= false;
		return true;
	}
	else if (key== SDLK_RIGHT) {
		_right_pressed= false;
		return true;
	}
	else if (key== SDLK_UP) {
		_up_pressed= false;
		return true;
	}
	else if (key== SDLK_DOWN) {
		_down_pressed= false;
		return true;
	}

	return false;
}


// LevelDebug ---------------------------------------------------------------------------------------------
LevelDebug::LevelDebug() {

}


LevelDebug::LevelDebug(GLuint prog_draw_aabb, Level * level, ScreenGL * screengl) : _prog_draw(prog_draw_aabb), _level(level), _screengl(screengl), _n_aabbs(0) {
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
	glm::vec3 static_footprint_color(1.0f, 0.5f, 0.5f);
	glm::vec3 anim_aabb_color(0.0f, 1.0f, 0.0f);
	glm::vec3 anim_footprint_color(0.5f, 1.0f, 0.5f);

	unsigned int n_static= _level->_static_objs.size();
	unsigned int n_anim= _level->_anim_objs.size();

	_n_aabbs= (n_static+ n_anim)* 2;
	float vertices[_n_aabbs* 40];

	for (unsigned int idx=0; idx<n_static; ++idx) {
		vertices[40* idx+ 0]= _level->_static_objs[idx]->_aabb->_pos.x;
		vertices[40* idx+ 1]= _level->_static_objs[idx]->_aabb->_pos.y;
		vertices[40* idx+ 5]= _level->_static_objs[idx]->_aabb->_pos.x+ _level->_static_objs[idx]->_aabb->_size.x;
		vertices[40* idx+ 6]= _level->_static_objs[idx]->_aabb->_pos.y;

		vertices[40* idx+ 10]= _level->_static_objs[idx]->_aabb->_pos.x+ _level->_static_objs[idx]->_aabb->_size.x;
		vertices[40* idx+ 11]= _level->_static_objs[idx]->_aabb->_pos.y;
		vertices[40* idx+ 15]= _level->_static_objs[idx]->_aabb->_pos.x+ _level->_static_objs[idx]->_aabb->_size.x;
		vertices[40* idx+ 16]= _level->_static_objs[idx]->_aabb->_pos.y+ _level->_static_objs[idx]->_aabb->_size.y;

		vertices[40* idx+ 20]= _level->_static_objs[idx]->_aabb->_pos.x+ _level->_static_objs[idx]->_aabb->_size.x;
		vertices[40* idx+ 21]= _level->_static_objs[idx]->_aabb->_pos.y+ _level->_static_objs[idx]->_aabb->_size.y;
		vertices[40* idx+ 25]= _level->_static_objs[idx]->_aabb->_pos.x;
		vertices[40* idx+ 26]= _level->_static_objs[idx]->_aabb->_pos.y+ _level->_static_objs[idx]->_aabb->_size.y;

		vertices[40* idx+ 30]= _level->_static_objs[idx]->_aabb->_pos.x;
		vertices[40* idx+ 31]= _level->_static_objs[idx]->_aabb->_pos.y+ _level->_static_objs[idx]->_aabb->_size.y;
		vertices[40* idx+ 35]= _level->_static_objs[idx]->_aabb->_pos.x;
		vertices[40* idx+ 36]= _level->_static_objs[idx]->_aabb->_pos.y;

		for (unsigned int i=0; i<8; ++i) {
			vertices[40* idx+ 5* i+ 2]= static_aabb_color.x;
			vertices[40* idx+ 5* i+ 3]= static_aabb_color.y;
			vertices[40* idx+ 5* i+ 4]= static_aabb_color.z;
		}
	}

	for (unsigned int idx=0; idx<n_static; ++idx) {
		vertices[n_static* 40+ 40* idx+ 0]= _level->_static_objs[idx]->_footprint->_pos.x;
		vertices[n_static* 40+ 40* idx+ 1]= _level->_static_objs[idx]->_footprint->_pos.y;
		vertices[n_static* 40+ 40* idx+ 5]= _level->_static_objs[idx]->_footprint->_pos.x+ _level->_static_objs[idx]->_footprint->_size.x;
		vertices[n_static* 40+ 40* idx+ 6]= _level->_static_objs[idx]->_footprint->_pos.y;

		vertices[n_static* 40+ 40* idx+ 10]= _level->_static_objs[idx]->_footprint->_pos.x+ _level->_static_objs[idx]->_footprint->_size.x;
		vertices[n_static* 40+ 40* idx+ 11]= _level->_static_objs[idx]->_footprint->_pos.y;
		vertices[n_static* 40+ 40* idx+ 15]= _level->_static_objs[idx]->_footprint->_pos.x+ _level->_static_objs[idx]->_footprint->_size.x;
		vertices[n_static* 40+ 40* idx+ 16]= _level->_static_objs[idx]->_footprint->_pos.y+ _level->_static_objs[idx]->_footprint->_size.y;

		vertices[n_static* 40+ 40* idx+ 20]= _level->_static_objs[idx]->_footprint->_pos.x+ _level->_static_objs[idx]->_footprint->_size.x;
		vertices[n_static* 40+ 40* idx+ 21]= _level->_static_objs[idx]->_footprint->_pos.y+ _level->_static_objs[idx]->_footprint->_size.y;
		vertices[n_static* 40+ 40* idx+ 25]= _level->_static_objs[idx]->_footprint->_pos.x;
		vertices[n_static* 40+ 40* idx+ 26]= _level->_static_objs[idx]->_footprint->_pos.y+ _level->_static_objs[idx]->_footprint->_size.y;

		vertices[n_static* 40+ 40* idx+ 30]= _level->_static_objs[idx]->_footprint->_pos.x;
		vertices[n_static* 40+ 40* idx+ 31]= _level->_static_objs[idx]->_footprint->_pos.y+ _level->_static_objs[idx]->_footprint->_size.y;
		vertices[n_static* 40+ 40* idx+ 35]= _level->_static_objs[idx]->_footprint->_pos.x;
		vertices[n_static* 40+ 40* idx+ 36]= _level->_static_objs[idx]->_footprint->_pos.y;

		for (unsigned int i=0; i<8; ++i) {
			vertices[n_static* 40+ 40* idx+ 5* i+ 2]= static_footprint_color.x;
			vertices[n_static* 40+ 40* idx+ 5* i+ 3]= static_footprint_color.y;
			vertices[n_static* 40+ 40* idx+ 5* i+ 4]= static_footprint_color.z;
		}
	}

	for (unsigned int idx=0; idx<n_anim; ++idx) {
		vertices[n_static* 80+ 40* idx+ 0]= _level->_anim_objs[idx]->_aabb->_pos.x;
		vertices[n_static* 80+ 40* idx+ 1]= _level->_anim_objs[idx]->_aabb->_pos.y;
		vertices[n_static* 80+ 40* idx+ 5]= _level->_anim_objs[idx]->_aabb->_pos.x+ _level->_anim_objs[idx]->_aabb->_size.x;
		vertices[n_static* 80+ 40* idx+ 6]= _level->_anim_objs[idx]->_aabb->_pos.y;

		vertices[n_static* 80+ 40* idx+ 10]= _level->_anim_objs[idx]->_aabb->_pos.x+ _level->_anim_objs[idx]->_aabb->_size.x;
		vertices[n_static* 80+ 40* idx+ 11]= _level->_anim_objs[idx]->_aabb->_pos.y;
		vertices[n_static* 80+ 40* idx+ 15]= _level->_anim_objs[idx]->_aabb->_pos.x+ _level->_anim_objs[idx]->_aabb->_size.x;
		vertices[n_static* 80+ 40* idx+ 16]= _level->_anim_objs[idx]->_aabb->_pos.y+ _level->_anim_objs[idx]->_aabb->_size.y;

		vertices[n_static* 80+ 40* idx+ 20]= _level->_anim_objs[idx]->_aabb->_pos.x+ _level->_anim_objs[idx]->_aabb->_size.x;
		vertices[n_static* 80+ 40* idx+ 21]= _level->_anim_objs[idx]->_aabb->_pos.y+ _level->_anim_objs[idx]->_aabb->_size.y;
		vertices[n_static* 80+ 40* idx+ 25]= _level->_anim_objs[idx]->_aabb->_pos.x;
		vertices[n_static* 80+ 40* idx+ 26]= _level->_anim_objs[idx]->_aabb->_pos.y+ _level->_anim_objs[idx]->_aabb->_size.y;

		vertices[n_static* 80+ 40* idx+ 30]= _level->_anim_objs[idx]->_aabb->_pos.x;
		vertices[n_static* 80+ 40* idx+ 31]= _level->_anim_objs[idx]->_aabb->_pos.y+ _level->_anim_objs[idx]->_aabb->_size.y;
		vertices[n_static* 80+ 40* idx+ 35]= _level->_anim_objs[idx]->_aabb->_pos.x;
		vertices[n_static* 80+ 40* idx+ 36]= _level->_anim_objs[idx]->_aabb->_pos.y;

		for (unsigned int i=0; i<8; ++i) {
			vertices[n_static* 80+ 40* idx+ 5* i+ 2]= anim_aabb_color.x;
			vertices[n_static* 80+ 40* idx+ 5* i+ 3]= anim_aabb_color.y;
			vertices[n_static* 80+ 40* idx+ 5* i+ 4]= anim_aabb_color.z;
		}
	}

	for (unsigned int idx=0; idx<n_anim; ++idx) {
		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 0]= _level->_anim_objs[idx]->_footprint->_pos.x;
		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 1]= _level->_anim_objs[idx]->_footprint->_pos.y;
		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 5]= _level->_anim_objs[idx]->_footprint->_pos.x+ _level->_anim_objs[idx]->_footprint->_size.x;
		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 6]= _level->_anim_objs[idx]->_footprint->_pos.y;

		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 10]= _level->_anim_objs[idx]->_footprint->_pos.x+ _level->_anim_objs[idx]->_footprint->_size.x;
		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 11]= _level->_anim_objs[idx]->_footprint->_pos.y;
		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 15]= _level->_anim_objs[idx]->_footprint->_pos.x+ _level->_anim_objs[idx]->_footprint->_size.x;
		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 16]= _level->_anim_objs[idx]->_footprint->_pos.y+ _level->_anim_objs[idx]->_footprint->_size.y;

		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 20]= _level->_anim_objs[idx]->_footprint->_pos.x+ _level->_anim_objs[idx]->_footprint->_size.x;
		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 21]= _level->_anim_objs[idx]->_footprint->_pos.y+ _level->_anim_objs[idx]->_footprint->_size.y;
		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 25]= _level->_anim_objs[idx]->_footprint->_pos.x;
		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 26]= _level->_anim_objs[idx]->_footprint->_pos.y+ _level->_anim_objs[idx]->_footprint->_size.y;

		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 30]= _level->_anim_objs[idx]->_footprint->_pos.x;
		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 31]= _level->_anim_objs[idx]->_footprint->_pos.y+ _level->_anim_objs[idx]->_footprint->_size.y;
		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 35]= _level->_anim_objs[idx]->_footprint->_pos.x;
		vertices[n_static* 80+ n_anim* 40+ 40* idx+ 36]= _level->_anim_objs[idx]->_footprint->_pos.y;

		for (unsigned int i=0; i<8; ++i) {
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 5* i+ 2]= anim_footprint_color.x;
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 5* i+ 3]= anim_footprint_color.y;
			vertices[n_static* 80+ n_anim* 40+ 40* idx+ 5* i+ 4]= anim_footprint_color.z;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, 40* _n_aabbs* sizeof(float), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

