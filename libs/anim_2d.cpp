#include "anim_2d.h"

using namespace std;
using namespace rapidxml;


void Action::print() {
    cout << "_name=" << _name << " ; _first_idx=" << _first_idx << " ; _n_idx=" << _n_idx << " ; _n_ms=" << _n_ms << " ; _move=" << _move << "\n";
    for (auto png : _pngs) {
        cout << png << "\n";
    }
}


Model::Model() {

}


Model::Model(string path) {
    string root_pngs= path+ "/pngs";
    vector<string> l_dirs= list_files(root_pngs);
    unsigned int compt= 0;
    for (auto dir : l_dirs) {
        if (dir[0]!= '.') {
            //cout << dir << "\n";
            Action action= {};
            action._name= dir;
            action._first_idx= compt;
            action._n_idx= 0;
            vector<string> l_files= list_files(root_pngs+ "/"+ dir);
            sort(l_files.begin(), l_files.end());
            for (auto f : l_files) {
                if (f[0]!= '.') {
                    //cout << f << "\n";
                    action._pngs.push_back(root_pngs+ "/"+ dir+ "/"+ f);
                    action._n_idx++;
                    compt++;
                }
            }
            _actions.push_back(action);
        }
    }

 	string anim_xml= path+ "/anim.xml";

	ifstream xml_file(anim_xml);
	stringstream buffer;
	buffer << xml_file.rdbuf();
	xml_file.close();
	string xml_content(buffer.str());
	xml_document<> doc;
	doc.parse<0>(&xml_content[0]);
	xml_node<> * root_node= doc.first_node();

    xml_node<> * anims_node= root_node->first_node("anims");
	for (xml_node<> * anim_node=anims_node->first_node("anim"); anim_node; anim_node=anim_node->next_sibling()) {
		xml_node<> * name_node= anim_node->first_node("name");
	    string name= name_node->value();
		xml_node<> * n_ms_node= anim_node->first_node("n_ms");
	    unsigned int n_ms= stoi(n_ms_node->value());
		xml_node<> * move_node= anim_node->first_node("move");
	    float move= stof(move_node->value());
        int idx_action_ok= -1;
        for (int idx_action=0; idx_action<_actions.size(); ++idx_action) {
            if (_actions[idx_action]._name== name) {
                idx_action_ok= idx_action;
                break;
            }
        }
        if (idx_action_ok< 0) {
            cout << "action non trouvee\n";
            return;
        }
        _actions[idx_action_ok]._n_ms= n_ms;
        _actions[idx_action_ok]._move= move;
    }

    xml_node<> * footprint_node= root_node->first_node("footprint");
    float xmin= stof(footprint_node->first_node("xmin")->value())/ (float)(MODEL_SIZE);
    float ymin= stof(footprint_node->first_node("ymin")->value())/ (float)(MODEL_SIZE);
    float xmax= stof(footprint_node->first_node("xmax")->value())/ (float)(MODEL_SIZE);
    float ymax= stof(footprint_node->first_node("ymax")->value())/ (float)(MODEL_SIZE);
    _footprint= new AABB_2D(glm::vec2(xmin, ymin), glm::vec2(xmax, ymax));

	glGenTextures(1, &_texture_id);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);
    
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, MODEL_SIZE, MODEL_SIZE, compt, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    for (auto action : _actions) {
        //action.print();
        for (unsigned int i=0; i<action._n_idx; ++i) {
            SDL_Surface * surface= IMG_Load(action._pngs[i].c_str());
            if (!surface) {
                cout << "IMG_Load error :" << IMG_GetError() << endl;
                return;
            }

            // sais pas pourquoi mais GL_BGRA fonctionne mieux que GL_RGBA
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                            0,                          // mipmap number
                            0, 0, action._first_idx+ i, // xoffset, yoffset, zoffset
                            MODEL_SIZE, MODEL_SIZE, 1,  // width, height, depth
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
}


Model::~Model() {
    delete _footprint;
}


AnimTexture::AnimTexture() {

}


AnimTexture::AnimTexture(GLuint prog_draw, GLuint prog_draw_footprint, ScreenGL * screengl, Model * model) :
    _prog_draw(prog_draw), _prog_draw_footprint(prog_draw_footprint), _screengl(screengl), _current_anim(0), _next_anim(1),
    _interpol_anim(0.0f), _first_ms(0), _position(glm::vec2(0.0f, -4.0f)), _current_action_idx(0), _model(model), _z(0.0f),
	_go_right(false), _go_left(false), _go_up(false), _go_down(false), _falling(false), _n_ms_start_falling(0)
{
	_camera2clip= glm::ortho(-_screengl->_gl_width* 0.5f, _screengl->_gl_width* 0.5f, -_screengl->_gl_height* 0.5f, _screengl->_gl_height* 0.5f, Z_NEAR, Z_FAR);
    update_model2world();
    set_size(1.0f);

	glUseProgram(_prog_draw);
	_camera2clip_loc= glGetUniformLocation(_prog_draw, "camera2clip_matrix");
    _model2world_loc= glGetUniformLocation(_prog_draw, "model2world_matrix");
    _texture_array_loc= glGetUniformLocation(_prog_draw, "texture_array");
    _current_layer_loc= glGetUniformLocation(_prog_draw, "current_layer");
    _next_layer_loc= glGetUniformLocation(_prog_draw, "next_layer");
    _interpol_layer_loc= glGetUniformLocation(_prog_draw, "interpol_layer");
    _z_loc= glGetUniformLocation(_prog_draw, "z");
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
    _tex_coord_loc= glGetAttribLocation(_prog_draw, "tex_coord_in");
	glUseProgram(0);

    glGenBuffers(1, &_vbo);
	
    float xpos= 0.0f;
    float ypos= 0.0f;
    float w= 1.0f;
    float h= 1.0f;
    float vertices[24]= {
        xpos,     ypos + h, 0.0f, 0.0f,
        xpos,     ypos,     0.0f, 1.0f,
        xpos + w, ypos,     1.0f, 1.0f,
        xpos,     ypos + h, 0.0f, 0.0f,
        xpos + w, ypos,     1.0f, 1.0f,
        xpos + w, ypos + h, 1.0f, 0.0f
    };
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, 24* sizeof(float), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

    // footprint ------------------------------------------------
	glUseProgram(_prog_draw_footprint);
	_camera2clip_fp_loc= glGetUniformLocation(_prog_draw_footprint, "camera2clip_matrix");
    _model2world_fp_loc= glGetUniformLocation(_prog_draw_footprint, "model2world_matrix");
	_z_fp_loc= glGetUniformLocation(_prog_draw_footprint, "z");
    _position_fp_loc= glGetAttribLocation(_prog_draw_footprint, "position_in");
    _color_fp_loc= glGetAttribLocation(_prog_draw_footprint, "color_in");
	glUseProgram(0);

    glGenBuffers(1, &_vbo_footprint);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo_footprint);
	glBufferData(GL_ARRAY_BUFFER, 80* sizeof(float), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void AnimTexture::draw() {
    glActiveTexture(GL_TEXTURE0);

    glUseProgram(_prog_draw);
   	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _model->_texture_id);

    glUniform1i(_texture_array_loc, 0); //Sampler refers to texture unit 0
    glUniform1i(_current_layer_loc, _model->_actions[_current_action_idx]._first_idx+ _current_anim);
    glUniform1i(_next_layer_loc, _model->_actions[_current_action_idx]._first_idx+ _next_anim);
    glUniform1f(_interpol_layer_loc, _interpol_anim);
    glUniform1f(_z_loc, _z);
    glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, glm::value_ptr(_camera2clip));
    glUniformMatrix4fv(_model2world_loc, 1, GL_FALSE, glm::value_ptr(_model2world));
    
    glEnableVertexAttribArray(_position_loc);
    glEnableVertexAttribArray(_tex_coord_loc);

    glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 4* sizeof(float), (void*)0);
    glVertexAttribPointer(_tex_coord_loc, 2, GL_FLOAT, GL_FALSE, 4* sizeof(float), (void*)(2* sizeof(float)));

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(_position_loc);
    glDisableVertexAttribArray(_tex_coord_loc);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);

    // footprint ----------------------
    glUseProgram(_prog_draw_footprint);
   	glBindBuffer(GL_ARRAY_BUFFER, _vbo_footprint);

    glUniformMatrix4fv(_camera2clip_fp_loc, 1, GL_FALSE, glm::value_ptr(_camera2clip));
    glUniformMatrix4fv(_model2world_fp_loc, 1, GL_FALSE, glm::value_ptr(_model2world));
	glUniform1f(_z_fp_loc, Z_FAR- 0.01f); // on affiche le footprint par dessus tout
    
    glEnableVertexAttribArray(_position_fp_loc);
    glEnableVertexAttribArray(_color_fp_loc);

    glVertexAttribPointer(_position_fp_loc, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)0);
    glVertexAttribPointer(_color_fp_loc, 3, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(2* sizeof(float)));

    glDrawArrays(GL_LINES, 0, 16);

    glDisableVertexAttribArray(_position_fp_loc);
    glDisableVertexAttribArray(_color_fp_loc);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}


void AnimTexture::anim(unsigned int n_ms) {
    float xmin= _model->_footprint->_pt_min.x;
    float ymin= _model->_footprint->_pt_min.y;
    float xmax= _model->_footprint->_pt_max.x;
    float ymax= _model->_footprint->_pt_max.y;
    glm::vec3 color_left, color_right, color_up, color_down;
	glm::vec3 color_go(0.0f, 1.0f, 0.0f);
	glm::vec3 color_nogo(1.0f, 0.0f, 0.0f);
	glm::vec3 color_total(0.9f, 0.9f, 0.9f);
	if (_go_left) { color_left= color_go; } else { color_left= color_nogo; }
	if (_go_right) { color_right= color_go; } else { color_right= color_nogo; }
	if (_go_up) { color_up= color_go; } else { color_up= color_nogo; }
	if (_go_down) { color_down= color_go; } else { color_down= color_nogo; }
    float vertices_fp[16* 5]= {
		0.0f, 0.0f, color_total.x, color_total.x, color_total.x,
		1.0f, 0.0f, color_total.x, color_total.x, color_total.x,
		
		1.0f, 0.0f, color_total.x, color_total.x, color_total.x,
		1.0f, 1.0f, color_total.x, color_total.x, color_total.x,

		1.0f, 1.0f, color_total.x, color_total.x, color_total.x,
		0.0f, 1.0f, color_total.x, color_total.x, color_total.x,

		0.0f, 1.0f, color_total.x, color_total.x, color_total.x,
		0.0f, 0.0f, color_total.x, color_total.x, color_total.x,

        xmin, ymin, color_down.x, color_down.y, color_down.z,
        xmax, ymin, color_down.x, color_down.y, color_down.z,

		xmax, ymin, color_right.x, color_right.y, color_right.z,
        xmax, ymax, color_right.x, color_right.y, color_right.z,

		xmax, ymax, color_up.x, color_up.y, color_up.z,
        xmin, ymax, color_up.x, color_up.y, color_up.z,

		xmin, ymax, color_left.x, color_left.y, color_left.z,
		xmin, ymin, color_left.x, color_left.y, color_left.z
    };

    glBindBuffer(GL_ARRAY_BUFFER, _vbo_footprint);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 80* sizeof(float), vertices_fp);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (n_ms- _first_ms>= _model->_actions[_current_action_idx]._n_ms) {
        _first_ms+= _model->_actions[_current_action_idx]._n_ms;

        _current_anim++;
        if (_current_anim>= _model->_actions[_current_action_idx]._n_idx) {
            _current_anim= 0;
        }
        _next_anim= _current_anim+ 1;
        if (_next_anim>= _model->_actions[_current_action_idx]._n_idx) {
            _next_anim= 0;
        }
    }

    _interpol_anim= (float)(n_ms- _first_ms)/ (float)(_model->_actions[_current_action_idx]._n_ms);

	if ((_model->_actions[_current_action_idx]._move> 0.0f) && (_go_right)) {
    	_position.x+= _model->_actions[_current_action_idx]._move;
	}
	else if ((_model->_actions[_current_action_idx]._move< 0.0f) && (_go_left)) {
    	_position.x+= _model->_actions[_current_action_idx]._move;
	}
	if (_go_down) {
		if (_falling) {
			_position.y-= (float)(n_ms- _n_ms_start_falling)* float(n_ms- _n_ms_start_falling)*  0.000001f;
		}
		else {
			_falling= true;
			_n_ms_start_falling= n_ms;
		}
	}
	else {
		if (_falling) {
			_falling= false;
			_n_ms_start_falling= 0;
		}
	}
    
    update_model2world();
}


void AnimTexture::set_action(string action_name) {
    int idx_action_ok= -1;
    for (int idx_action=0; idx_action<_model->_actions.size(); ++idx_action) {
        if (_model->_actions[idx_action]._name== action_name) {
            idx_action_ok= idx_action;
            break;
        }
    }
    if (idx_action_ok< 0) {
        cout << "action non trouvee\n";
        return;
    }

    _current_action_idx= idx_action_ok;
    _current_anim= 0;
    _next_anim= 1;
    _interpol_anim= 0.0f;
}


void AnimTexture::update_model2world() {
    _model2world= glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(_position.x, _position.y, 0.0f)), glm::vec3(_size, _size, 1.0f));
}


void AnimTexture::set_size(float size) {
    _size= size;
    update_model2world();
}


StaticTexture::StaticTexture() {

}


StaticTexture::StaticTexture(GLuint prog_draw, ScreenGL * screengl, string path) : _prog_draw(prog_draw), _screengl(screengl), _alpha(1.0f), _n_blocs(0), _size(1.0f), _z(1.0f) {
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
    _z_loc= glGetUniformLocation(_prog_draw, "z");
	_position_loc= glGetAttribLocation(_prog_draw, "position_in");
    _tex_coord_loc= glGetAttribLocation(_prog_draw, "tex_coord_in");
	glUseProgram(0);

    glGenBuffers(1, &_vbo);	
}


StaticTexture::~StaticTexture() {

}


void StaticTexture::draw() {
    glActiveTexture(GL_TEXTURE0);

    glUseProgram(_prog_draw);
   	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindTexture(GL_TEXTURE_2D, _texture_id);

    glUniform1i(_tex_loc, 0); //Sampler refers to texture unit 0
    glUniform1f(_alpha_loc, _alpha);
    glUniform1f(_z_loc, _z);
    glUniformMatrix4fv(_camera2clip_loc, 1, GL_FALSE, glm::value_ptr(_camera2clip));
    glUniformMatrix4fv(_model2world_loc, 1, GL_FALSE, glm::value_ptr(_model2world));
    
    glEnableVertexAttribArray(_position_loc);
    glEnableVertexAttribArray(_tex_coord_loc);

    glVertexAttribPointer(_position_loc, 2, GL_FLOAT, GL_FALSE, 4* sizeof(float), (void*)0);
    glVertexAttribPointer(_tex_coord_loc, 2, GL_FLOAT, GL_FALSE, 4* sizeof(float), (void*)(2* sizeof(float)));

    glDrawArrays(GL_TRIANGLES, 0, 6* _n_blocs);

    glDisableVertexAttribArray(_position_loc);
    glDisableVertexAttribArray(_tex_coord_loc);

    glBindTexture(GL_TEXTURE, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}


void StaticTexture::set_blocs(vector<AABB_2D *> blocs) {
    _n_blocs= blocs.size();
    float vertices[24* _n_blocs];
    for (unsigned int idx_bloc=0; idx_bloc<_n_blocs; ++idx_bloc) {
        glm::vec2 tex_coord((blocs[idx_bloc]->_pt_max.x- blocs[idx_bloc]->_pt_min.x)/ _size, (blocs[idx_bloc]->_pt_max.y- blocs[idx_bloc]->_pt_min.y)/ _size);
        //glm::vec2 tex_coord(2.0f, 2.0f);

        vertices[24* idx_bloc+ 0]= blocs[idx_bloc]->_pt_min.x;
        vertices[24* idx_bloc+ 1]= blocs[idx_bloc]->_pt_max.y;
        vertices[24* idx_bloc+ 2]= 0.0f;
        vertices[24* idx_bloc+ 3]= 0.0f;

        vertices[24* idx_bloc+ 4]= blocs[idx_bloc]->_pt_min.x;
        vertices[24* idx_bloc+ 5]= blocs[idx_bloc]->_pt_min.y;
        vertices[24* idx_bloc+ 6]= 0.0f;
        vertices[24* idx_bloc+ 7]= tex_coord.y;

        vertices[24* idx_bloc+ 8]= blocs[idx_bloc]->_pt_max.x;
        vertices[24* idx_bloc+ 9]= blocs[idx_bloc]->_pt_min.y;
        vertices[24* idx_bloc+ 10]= tex_coord.x;
        vertices[24* idx_bloc+ 11]= tex_coord.y;

        vertices[24* idx_bloc+ 12]= blocs[idx_bloc]->_pt_min.x;
        vertices[24* idx_bloc+ 13]= blocs[idx_bloc]->_pt_max.y;
        vertices[24* idx_bloc+ 14]= 0.0f;
        vertices[24* idx_bloc+ 15]= 0.0f;

        vertices[24* idx_bloc+ 16]= blocs[idx_bloc]->_pt_max.x;
        vertices[24* idx_bloc+ 17]= blocs[idx_bloc]->_pt_min.y;
        vertices[24* idx_bloc+ 18]= tex_coord.x;
        vertices[24* idx_bloc+ 19]= tex_coord.y;

        vertices[24* idx_bloc+ 20]= blocs[idx_bloc]->_pt_max.x;
        vertices[24* idx_bloc+ 21]= blocs[idx_bloc]->_pt_max.y;
        vertices[24* idx_bloc+ 22]= tex_coord.x;
        vertices[24* idx_bloc+ 23]= 0.0f;
    };
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, 24* _n_blocs* sizeof(float), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


Level::Level() {

}


Level::Level(GLuint prog_draw_anim, GLuint prog_draw_footprint, GLuint prog_draw_static, ScreenGL * screengl, unsigned int w, unsigned int h) :
    _w(w) ,_h(h), _screengl(screengl)
{
	_block_w= _screengl->_gl_width/ (float)(_w);
	_block_h= _screengl->_gl_height/ (float)(_h);

	_models.push_back(new Model("./modeles/modele_1"));
	_models.push_back(new Model("./modeles/modele_2"));
	
    for (unsigned int i=0; i<1; ++i) {
		AnimTexture * anim_texture= new AnimTexture(prog_draw_anim, prog_draw_footprint, screengl, _models[i%2]);
		anim_texture->_z= rand_float(0.0f, 0.9f);
		//anim_texture->_z= (float)(i % 2)* 0.1f;
		//anim_texture->_z= 2.0f;
		//anim_texture->_position= glm::vec2(rand_float(-5.0f, 5.0f), -7.0f);
		anim_texture->_position= glm::vec2(0.0f, 7.0f);
		//anim_texture->set_size(rand_float(1.0f, 5.0f));
		anim_texture->set_size(3.0f);
		_anim_textures.push_back(anim_texture);
	}

	StaticTexture * static_texture= new StaticTexture(prog_draw_static, screengl, "./static_textures/brick.png");
	static_texture->_z= 1.5f;
	/*vector<AABB_2D *> blocs;
	blocs.push_back(new AABB_2D(glm::vec2(-8.0f, -8.0f), glm::vec2(8.0f, -7.0f)));
	blocs.push_back(new AABB_2D(glm::vec2(-8.0f, -8.0f), glm::vec2(-7.0f, 5.0f)));
	blocs.push_back(new AABB_2D(glm::vec2(7.0f, -8.0f), glm::vec2(8.0f, 5.0f)));
	static_texture->set_blocs(blocs);*/
	_static_textures.push_back(static_texture);

	StaticTexture * static_texture_2= new StaticTexture(prog_draw_static, screengl, "./static_textures/grass.png");
	static_texture_2->_z= 1.1f;
	/*vector<AABB_2D *> blocs_2;
	blocs_2.push_back(new AABB_2D(glm::vec2(-8.0f, -7.0f), glm::vec2(8.0f, -6.0f)));
	static_texture_2->set_blocs(blocs_2);*/
	_static_textures.push_back(static_texture_2);
/*
	StaticTexture * static_texture_3= new StaticTexture(prog_draw_static, screengl, "./static_textures/tree.png");
	static_texture_3->_z= -1.0f;
	static_texture_3->_size= 8.0f;
	vector<AABB_2D *> blocs_3;
	blocs_3.push_back(new AABB_2D(glm::vec2(-4.0f, -7.0f), glm::vec2(4.0f, 1.0f)));
	static_texture_3->set_blocs(blocs_3);
	_static_textures.push_back(static_texture_3);

	StaticTexture * static_texture_4= new StaticTexture(prog_draw_static, screengl, "./static_textures/hill.png");
	static_texture_4->_z= -2.0f;
	static_texture_4->_size= 16.0f;
	vector<AABB_2D *> blocs_4;
	blocs_4.push_back(new AABB_2D(glm::vec2(-8.0f, -8.0f), glm::vec2(8.0f, 8.0f)));
	static_texture_4->set_blocs(blocs_4);
	_static_textures.push_back(static_texture_4);

	StaticTexture * static_texture_5= new StaticTexture(prog_draw_static, screengl, "./static_textures/sky.png");
	static_texture_5->_z= -3.0f;
	static_texture_5->_size= 16.0f;
	vector<AABB_2D *> blocs_5;
	blocs_5.push_back(new AABB_2D(glm::vec2(-8.0f, -8.0f), glm::vec2(8.0f, 8.0f)));
	static_texture_5->set_blocs(blocs_5);
	_static_textures.push_back(static_texture_5);
*/
	_obstacles.clear();
	for (unsigned int i=0; i<_w* _h; ++i) {
		_obstacles.push_back(AIR);
	}

    for (unsigned int col=0; col<_w; ++col) {
	    for (unsigned int row=0; row<_h; ++row) {
			unsigned int idx= _w* row+ col;
			if ((col== 0) || (col== _w- 1) || (row== 0) || (row== _h- 1)) {
				_obstacles[idx]= SOLIDE;
			}
		}
    }
	/*_obstacles[_w* 5+ 5]= SOLIDE;
	_obstacles[_w* 5+ 6]= SOLIDE;
	_obstacles[_w* 5+ 7]= SOLIDE;
	_obstacles[_w* 5+ 8]= SOLIDE;*/

	_obstacles[_w* 2+ 1]= SOLIDE;
	_obstacles[_w* 2+ 2]= SOLIDE;
	_obstacles[_w* 2+ 3]= SOLIDE;

	_obstacles[_w* 2+ 21]= SOLIDE;
	_obstacles[_w* 2+ 22]= SOLIDE;
	_obstacles[_w* 2+ 23]= SOLIDE;

	//_obstacles[_w* 16+ 16]= SOLIDE;

	sync_obstacles_bricks();
}


Level::~Level() {
	for (auto anim_texture : _anim_textures) {
		delete anim_texture;
	}
	_anim_textures.clear();
	for (auto model : _models) {
		delete model;
	}
	_models.clear();
	for (auto static_texture : _static_textures) {
		delete static_texture;
	}
	_static_textures.clear();
}


void Level::randomize() {
    for (unsigned int i=0; i<_w* _h; ++i) {
        if (rand_int(0, 1)) {
            _obstacles[i]= AIR;
        }
        else {
            _obstacles[i]= SOLIDE;
        }
    }
	sync_obstacles_bricks();
}


void Level::draw() {
	for (auto static_texture : _static_textures) {
		static_texture->draw();
	}

	for (auto anim_texture : _anim_textures) {
		anim_texture->draw();
	}
}


void Level::anim(unsigned int n_ms) {
	update_gos();

	for (auto anim_texture : _anim_textures) {
		int x= rand_int(0, 1000);
		//int x= 0;
		if (x== 0) {
			anim_texture->set_action("right_wait");
		}
		else if (x== 1) {
			anim_texture->set_action("left_wait");
		}
		else if (x== 2) {
			anim_texture->set_action("right_walk");
		}
		else if (x== 3) {
			anim_texture->set_action("left_walk");
		}
		else if (x== 4) {
			anim_texture->set_action("right_run");
		}
		else if (x== 5) {
			anim_texture->set_action("left_run");
		}

		/*if (anim_texture->_position.x> anim_texture->_screengl->_gl_width* 0.5f- 5.0f) {
        	anim_texture->set_action("left_walk");
    	}
		if (anim_texture->_position.x< -anim_texture->_screengl->_gl_width* 0.5f+ 3.0f) {
			anim_texture->set_action("right_walk");
		}*/

		anim_texture->anim(n_ms);
	}
}


void Level::sync_obstacles_bricks() {
	vector<AABB_2D *> blocs_air;
	vector<AABB_2D *> blocs_solide;
	for (unsigned int idx_obst=0; idx_obst<_w* _h; ++idx_obst) {
		unsigned int col= idx_obst% _w;
		unsigned int row= idx_obst/ _w;
		float xmin= -0.5f* _screengl->_gl_width+ (float)(col)* _block_w;
		float xmax= xmin+ _block_w;
		float ymin= -0.5f* _screengl->_gl_height+ (float)(row)* _block_h;
		float ymax= ymin+ _block_h;
		if (_obstacles[idx_obst]== SOLIDE) {
			blocs_solide.push_back(new AABB_2D(glm::vec2(xmin, ymin), glm::vec2(xmax, ymax)));
		}
		else {
			blocs_air.push_back(new AABB_2D(glm::vec2(xmin, ymin), glm::vec2(xmax, ymax)));
		}
	}
	_static_textures[0]->set_blocs(blocs_solide);
	_static_textures[1]->set_blocs(blocs_air);
}


void Level::update_gos() {
	for (auto anim_texture : _anim_textures) {
		glm::vec2 pos= anim_texture->_position;
		glm::vec2 offset_min= anim_texture->_model->_footprint->_pt_min* anim_texture->_size;
		glm::vec2 offset_max= anim_texture->_model->_footprint->_pt_max* anim_texture->_size;
		glm::vec2 pt_min= pos+ offset_min;
		glm::vec2 pt_max= pos+ offset_max;
		unsigned int col_min= (unsigned int)(floor((pt_min.x+ 0.5f* _screengl->_gl_width)/ _block_w));
		unsigned int col_max= (unsigned int)(floor((pt_max.x+ 0.5f* _screengl->_gl_width)/ _block_w));
		unsigned int row_min= (unsigned int)(floor((pt_min.y+ 0.5f* _screengl->_gl_height)/ _block_h));
		unsigned int row_max= (unsigned int)(floor((pt_max.y+ 0.5f* _screengl->_gl_height)/ _block_h));

		//cout << glm::to_string(pos) << " ; " << glm::to_string(offset_min) << " ; " << glm::to_string(offset_max) << "\n";
		//cout << row_min << " ; " << row_max << " ; " << col_min << " ; " << col_max << "\n";

		anim_texture->_go_down= true;
		anim_texture->_go_up= true;
		anim_texture->_go_left= true;
		anim_texture->_go_right= true;
		for (unsigned int row=row_min; row<=row_max; ++row) {
			for (unsigned int col=col_min; col<=col_max; ++col) {
				unsigned int idx_obst= row* _w+ col;

				//cout << idx_obst << " ; " << row << " ; " << col << " ; " << _obstacles[idx_obst] << "\n";
				//cout << "----\n";

				if ((col== col_min) && (_obstacles[idx_obst]== SOLIDE)) {
					anim_texture->_go_left= false;
				}
				if ((col== col_max) && (_obstacles[idx_obst]== SOLIDE)) {
					anim_texture->_go_right= false;
				}
				if ((row== row_min) && (_obstacles[idx_obst]== SOLIDE)) {
					anim_texture->_go_down= false;
				}
				if ((row== row_max) && (_obstacles[idx_obst]== SOLIDE)) {
					anim_texture->_go_up= false;
				}
			}
		}
	}
}

