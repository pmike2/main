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

	for (xml_node<> * anim_node=root_node->first_node("anim"); anim_node; anim_node=anim_node->next_sibling()) {
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

	glGenTextures(1, &_texture_id);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture_id);
    
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 512, 512, compt, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

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
                            0,                     //Mipmap number
                            0, 0, action._first_idx+ i, //xoffset, yoffset, zoffset
                            512, 512, 1,           //width, height, depth
                            GL_BGRA,               //format
                            GL_UNSIGNED_BYTE,      //type
                            surface->pixels);      //pointer to data

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

}


Test::Test() {

}


Test::Test(GLuint prog_draw, ScreenGL * screengl, Model * model) :
    _prog_draw(prog_draw), _screengl(screengl), _current_anim(0), _next_anim(1),
    _interpol_anim(0.0f), _first_ms(0), _position(glm::vec2(0.0f, -4.0f)), _current_action_idx(0), _model(model)
{
	_camera2clip= glm::ortho(-_screengl->_gl_width* 0.5f, _screengl->_gl_width* 0.5f, -_screengl->_gl_height* 0.5f, _screengl->_gl_height* 0.5f, -1.0f, 1.0f);
    update_model2world();
    set_size(1.0f);

	glUseProgram(_prog_draw);
	_camera2clip_loc= glGetUniformLocation(_prog_draw, "camera2clip_matrix");
    _model2world_loc= glGetUniformLocation(_prog_draw, "model2world_matrix");
    _texture_array_loc= glGetUniformLocation(_prog_draw, "texture_array");
    _current_layer_loc= glGetUniformLocation(_prog_draw, "current_layer");
    _next_layer_loc= glGetUniformLocation(_prog_draw, "next_layer");
    _interpol_layer_loc= glGetUniformLocation(_prog_draw, "interpol_layer");
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

}


void Test::draw() {
    glActiveTexture(GL_TEXTURE0);

    glUseProgram(_prog_draw);
   	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _model->_texture_id);

    glUniform1i(_texture_array_loc, 0); //Sampler refers to texture unit 0
    glUniform1i(_current_layer_loc, _model->_actions[_current_action_idx]._first_idx+ _current_anim);
    glUniform1i(_next_layer_loc, _model->_actions[_current_action_idx]._first_idx+ _next_anim);
    glUniform1f(_interpol_layer_loc, _interpol_anim);
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
}


void Test::anim(unsigned int n_ms) {
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

    _position.x+= _model->_actions[_current_action_idx]._move;
    if (_position.x> _screengl->_gl_width* 0.5f- 6.0f) {
        set_action("left_walk");
    }
    if (_position.x< -_screengl->_gl_width* 0.5f) {
        set_action("right_walk");
    }
    
    update_model2world();
}


void Test::set_action(string action_name) {
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


void Test::update_model2world() {
    //_model2world= glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(_size, _size, 1.0f)), glm::vec3(_position.x, _position.y, 0.0f));
    _model2world= glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(_position.x, _position.y, 0.0f)), glm::vec3(_size, _size, 1.0f));
}


void Test::set_size(float size) {
    _size= size;
    update_model2world();
}


StaticTexture::StaticTexture() {

}


StaticTexture::StaticTexture(GLuint prog_draw, ScreenGL * screengl, string path) {

}


StaticTexture::~StaticTexture() {

}


