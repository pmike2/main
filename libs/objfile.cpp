
#include "objfile.h"

using namespace std;
using namespace rapidxml;



void parse_static_config_file(string ch_config_file, GLuint * vertex_buffers, GLuint * index_buffers, uint * n_faces, uint & n_precisions, glm::mat4 * materials_array, AABB * aabb) {
	string line;
	glm::vec3 vmin= glm::vec3(1e8, 1e8, 1e8);
	glm::vec3 vmax= glm::vec3(-1e8, -1e8, -1e8);

	ifstream config_file(ch_config_file);
	if (!config_file.is_open()) {
		cout << "Impossible d'ouvrir le fichier de config : " << ch_config_file << endl;
		return;
	}

	stringstream buffer;
	buffer << config_file.rdbuf();
	config_file.close();
	string config_content(buffer.str());

	xml_document<> doc;
	doc.parse<0>(&config_content[0]);
	
	xml_node<> * root_node= doc.first_node();

	// fichier mtl -------------------------------------------------
	xml_node<> * mat_node= root_node->first_node("mat");
	string ch_mat= mat_node->value();

	ifstream mat_file(ch_mat.c_str());
	if (!mat_file.is_open()) {
		cout << "Impossible d'ouvrir le fichier mat : " << ch_mat << endl;
		return;
	}

	std::vector<Material *> materials;
	string id;
	glm::vec3 ambient, diffuse;
	float shininess;

	mat_file.seekg(0, ios::beg);
	while (!mat_file.eof()) {
		getline(mat_file, line);
		
		istringstream iss(line);
		string s;
		iss >> s;

		if (s== "newmtl") {
			if (materials.size()>= N_MAX_MATERIALS) {
				cout << "Trop de materials" << endl;
				return;
			}

			if (id!= "") {
				Material * mat= new Material();
				mat->_id= id;
				mat->_ambient= ambient;
				mat->_diffuse= diffuse;
				mat->_shininess= shininess;
				materials.push_back(mat);
			}

			iss >> id;
		}
		else if (s== "Ka") {
			iss >> s; ambient.x= stof(s);
			iss >> s; ambient.y= stof(s);
			iss >> s; ambient.z= stof(s);
		}
		else if (s== "Kd") {
			iss >> s; diffuse.x= stof(s);
			iss >> s; diffuse.y= stof(s);
			iss >> s; diffuse.z= stof(s);
		}
		else if (s== "Ns") {
			iss >> s; shininess= stof(s);
		}
		
	}

	Material * mat= new Material();
	mat->_id= id;
	mat->_ambient= ambient;
	mat->_diffuse= diffuse;
	mat->_shininess= shininess;
	materials.push_back(mat);

	mat_file.close();

	for (uint i=0; i<N_MAX_MATERIALS; ++i) {
		materials_array[i]= glm::mat4(0.0f);
	}
	for (uint i=0; i<materials.size(); ++i) {
		if (i>= N_MAX_MATERIALS) {
			cout << "Trop de materials" << endl;
			return;
		}

		materials_array[i][0]= glm::vec4(materials[i]->_ambient.x, materials[i]->_ambient.y, materials[i]->_ambient.z, 0.0f);
		materials_array[i][1]= glm::vec4(materials[i]->_diffuse.x, materials[i]->_diffuse.y, materials[i]->_diffuse.z, 0.0f);
		materials_array[i][2]= glm::vec4(materials[i]->_shininess, 0.0f, 0.0f, 0.0f);
		materials_array[i][3]= glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	// fichier obj -------------------------------------------------
	xml_node<> * objs_node= root_node->first_node("objs");

	// calcul du nombre de niveaux de précisions
	n_precisions= 0;
	for (xml_node<> * obj_node=objs_node->first_node("obj"); obj_node; obj_node=obj_node->next_sibling()) {
		n_precisions++;
	}

	// un buffer par précision pour les vertices et les indices
	glGenBuffers(n_precisions, vertex_buffers);
	glGenBuffers(n_precisions, index_buffers);

	for (xml_node<> * obj_node=objs_node->first_node("obj"); obj_node; obj_node=obj_node->next_sibling()) {
		float * vertices;
		uint * faces;
		uint vertices_compt= 0;
		uint faces_compt= 0;
		uint face_tmp[3]= {0, 0, 0};
		uint n_vertices_tmp= 0;
		float * vertices_tmp;

		int precision= stoi(obj_node->first_attribute("precision")->value());
		
		string ch_obj= obj_node->value();
		ifstream obj_file(ch_obj.c_str());
		if (!obj_file.is_open()) {
			cout << "Impossible d'ouvrir le fichier obj : " << ch_obj << endl;
			return;
		}

		n_faces[precision]= 0;
		obj_file.seekg(0, ios::beg);
		while (!obj_file.eof()) {
			getline(obj_file, line);
			
			if ( (line.c_str()[0]== 'v') && (line.c_str()[1]!= 't') )
				n_vertices_tmp++;
			else if (line.c_str()[0]== 'f')
				n_faces[precision]++;
		}
		
		// pour chaque face, 3 sommets, et chaque sommet est (x, y, z, nx, ny, nz, idx_material)
		vertices= new float[(3+ 3+ 1)* 3* n_faces[precision]];
		faces= new uint[3* n_faces[precision]];
		vertices_tmp= new float[3* n_vertices_tmp];
		
		uint current_idx_mat= 0;
		obj_file.clear();
		obj_file.seekg(0, ios::beg);
		while (!obj_file.eof()) {		
			getline(obj_file, line);

			istringstream iss(line);
			string s;
			iss >> s;

			if (s== "v") {
				iss >> s; vertices_tmp[3* vertices_compt]= stof(s);
				iss >> s; vertices_tmp[3* vertices_compt+ 1]= stof(s);
				iss >> s; vertices_tmp[3* vertices_compt+ 2]= stof(s);
				vertices_compt++;
			}
			else if (s== "f") {
				iss >> s; face_tmp[0]= stoi(s);
				iss >> s; face_tmp[1]= stoi(s);
				iss >> s; face_tmp[2]= stoi(s);
				
				// OBJ file starts counting from 1
				face_tmp[0]-= 1;
				face_tmp[1]-= 1;
				face_tmp[2]-= 1;
				
				float coord1[3]= {vertices_tmp[3* face_tmp[0]], vertices_tmp[3* face_tmp[0]+ 1], vertices_tmp[3* face_tmp[0]+ 2]};
				float coord2[3]= {vertices_tmp[3* face_tmp[1]], vertices_tmp[3* face_tmp[1]+ 1], vertices_tmp[3* face_tmp[1]+ 2]};
				float coord3[3]= {vertices_tmp[3* face_tmp[2]], vertices_tmp[3* face_tmp[2]+ 1], vertices_tmp[3* face_tmp[2]+ 2]};
				float norm[3];
				calculate_normal(coord1, coord2, coord3, norm);
				
				//cout << current_idx_mat << endl;
				vertices[21* faces_compt+  0]= coord1[0];
				vertices[21* faces_compt+  1]= coord1[1];
				vertices[21* faces_compt+  2]= coord1[2];
				vertices[21* faces_compt+  3]= norm[0];
				vertices[21* faces_compt+  4]= norm[1];
				vertices[21* faces_compt+  5]= norm[2];
				vertices[21* faces_compt+  6]= (float)(current_idx_mat);

				vertices[21* faces_compt+ 7]= coord2[0];
				vertices[21* faces_compt+ 8]= coord2[1];
				vertices[21* faces_compt+ 9]= coord2[2];
				vertices[21* faces_compt+ 10]= norm[0];
				vertices[21* faces_compt+ 11]= norm[1];
				vertices[21* faces_compt+ 12]= norm[2];
				vertices[21* faces_compt+ 13]= (float)(current_idx_mat);

				vertices[21* faces_compt+ 14]= coord3[0];
				vertices[21* faces_compt+ 15]= coord3[1];
				vertices[21* faces_compt+ 16]= coord3[2];
				vertices[21* faces_compt+ 17]= norm[0];
				vertices[21* faces_compt+ 18]= norm[1];
				vertices[21* faces_compt+ 19]= norm[2];
				vertices[21* faces_compt+ 20]= (float)(current_idx_mat);
				
				faces_compt++;
			}
			else if (s== "usemtl") {
				iss >> s;
				bool ok= false;
				for (uint idx_mat=0; idx_mat<materials.size(); ++idx_mat) {
					if (materials[idx_mat]->_id== s) {
						current_idx_mat= idx_mat;
						ok= true;
						break;
					}
				}
				if (!ok) {
					cout << "material non trouve : " << s << endl;
					return;
				}
			}
		}
		obj_file.close();
		
		// ----------------------------------------------------------------------------------------------
		// Buffer d'indices : puisque l'on duplique tous les sommets pour ne pas avoir de normale partag�e, 
		// faces = { 0,1,2,3,4,5,6,7,8,9,10,... }
		for (uint i=0; i<3* n_faces[precision]; ++i) {
			faces[i]= i;
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[precision]);
		glBufferData(GL_ARRAY_BUFFER, (3+ 3+ 1)* 3* n_faces[precision]* sizeof(float), vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[precision]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3* n_faces[precision]* sizeof(uint), faces, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// pour AABB
		for (uint i=0; i<3* n_faces[precision]; ++i) {
			if (vertices[7* i+ 0]< vmin.x) vmin.x= vertices[7* i+ 0];
			if (vertices[7* i+ 0]> vmax.x) vmax.x= vertices[7* i+ 0];
			if (vertices[7* i+ 1]< vmin.y) vmin.y= vertices[7* i+ 1];
			if (vertices[7* i+ 1]> vmax.y) vmax.y= vertices[7* i+ 1];
			if (vertices[7* i+ 2]< vmin.z) vmin.z= vertices[7* i+ 2];
			if (vertices[7* i+ 2]> vmax.z) vmax.z= vertices[7* i+ 2];
		}

		delete[] faces;
		delete[] vertices;
		delete[] vertices_tmp;
	}

	for (auto it_mat : materials) {
		delete it_mat;
	}
	materials.clear();

	// boite AABB englobante
	aabb->set_vmin_vmax(vmin, vmax);
}


// ----------------------------------------------------------------------------------------
Material::Material() {
	
}


Material::~Material() {
	
}


void Material::print() {
	cout << "id=" << _id << " ; ambient=" << glm::to_string(_ambient) << " ; diffuse=" << glm::to_string(_diffuse) << " ; shininess=" << _shininess << endl;
}


// ----------------------------------------------------------------------------------------
StaticModel::StaticModel() {
	
}


StaticModel::StaticModel(string ch_config_file, GLuint prog_draw) :
	_ch_config_file(ch_config_file), _prog_draw(prog_draw)
{
	_aabb= new AABB();
	parse_static_config_file(_ch_config_file, _vertex_buffers, _index_buffers, _n_faces, _n_precisions, _materials_array, _aabb);

	glUseProgram(_prog_draw);

	_position_loc     = glGetAttribLocation(_prog_draw, "position_in");
	_normal_loc       = glGetAttribLocation(_prog_draw, "normal_in");
	_idx_material_loc  = glGetAttribLocation(_prog_draw, "idx_material_in");

	_model2clip_loc  = glGetUniformLocation(_prog_draw, "model2clip_matrix");
	_model2camera_loc= glGetUniformLocation(_prog_draw, "model2camera_matrix");
	_normal_mat_loc  = glGetUniformLocation(_prog_draw, "normal_matrix");
	_materials_loc   = glGetUniformLocation(_prog_draw, "materials");
	
	_alpha_loc= glGetUniformLocation(_prog_draw, "alpha");

	glUseProgram(0);
}


StaticModel::~StaticModel() {
	glDeleteBuffers(_n_precisions, _vertex_buffers);
	glDeleteBuffers(_n_precisions, _index_buffers);
	delete _aabb;
}


void StaticModel::print() {
}


// ----------------------------------------------------------------------------------------
StaticInstance::StaticInstance() {

}


StaticInstance::StaticInstance(StaticModel * model, const glm::vec3 & scale) :
	_model(model), _alpha(1.0f), _draw_mesh(true), _precision(0),
	_model2camera(glm::mat4(1.0f)), _model2clip(glm::mat4(1.0f)), _normal(glm::mat3(1.0f))
{
	_pos_rot= new InstancePosRot(glm::vec3(0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f), _model->_aabb);
	set_pos_rot_scale(glm::vec3(0.0f), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), scale);
}


StaticInstance::~StaticInstance() {
	delete _pos_rot;
}


void StaticInstance::draw() {
	if (!_pos_rot->_active) {
		return;
	}

	if (_draw_mesh) {
		glUseProgram(_model->_prog_draw);
		// On precise les donnees que l'on souhaite utiliser
		glBindBuffer(GL_ARRAY_BUFFER, _model->_vertex_buffers[_precision]);
		// On precise le tableau d'indices de triangle a utiliser
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _model->_index_buffers[_precision]);
		
		glUniformMatrix4fv(_model->_model2clip_loc  , 1, GL_FALSE, glm::value_ptr(_model2clip));
		glUniformMatrix4fv(_model->_model2camera_loc, 1, GL_FALSE, glm::value_ptr(_model2camera));
		glUniformMatrix3fv(_model->_normal_mat_loc  , 1, GL_FALSE, glm::value_ptr(_normal));
		glUniformMatrix4fv(_model->_materials_loc, N_MAX_MATERIALS, GL_FALSE, glm::value_ptr(_model->_materials_array[0]));
		glUniform1f(_model->_alpha_loc, _alpha);

		// Enables the attribute indices
		glEnableVertexAttribArray(_model->_position_loc);
		glEnableVertexAttribArray(_model->_normal_loc);
		glEnableVertexAttribArray(_model->_idx_material_loc);

		// Modifie les tableaux associes au buffer en cours d'utilisation
		glVertexAttribPointer(_model->_position_loc, 3, GL_FLOAT, GL_FALSE, (3+ 3+ 1)* sizeof(float), 0);
		glVertexAttribPointer(_model->_normal_loc  , 3, GL_FLOAT, GL_FALSE, (3+ 3+ 1)* sizeof(float), (void *)(3* sizeof(float)));
		glVertexAttribPointer(_model->_idx_material_loc, 1, GL_FLOAT, GL_FALSE, (3+ 3+ 1)* sizeof(float), (void *)((3+ 3)* sizeof(float)));
		
		// Rendu de notre geometrie
		glDrawElements(GL_TRIANGLES, _model->_n_faces[_precision]* 3, GL_UNSIGNED_INT, 0);

		// Disables the attribute indices
		glDisableVertexAttribArray(_model->_position_loc);
		glDisableVertexAttribArray(_model->_normal_loc);
		glDisableVertexAttribArray(_model->_idx_material_loc);
		
		// on reinit a 0
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}
}


void StaticInstance::anim(ViewSystem * view_system) {
	if (!_pos_rot->_active) {
		return;
	}
	
	_model2camera= view_system->_world2camera* _pos_rot->_model2world;
	_model2clip= view_system->_camera2clip* _model2camera;
	// theoriquement il faudrait prendre la transposee de l'inverse mais si model2camera est 
	// une matrice orthogonale, TRANS(INV(M)) == M, ce qui est le cas lorsqu'elle ne comprend que 
	// des translations et rotations
	_normal= glm::mat3(_model2camera);
}


void StaticInstance::set_pos_rot_scale(const glm::vec3 & position, const glm::quat & rotation, const glm::vec3 & scale) {
	_pos_rot->set_pos_rot_scale(position, rotation, scale);
}


// lent, mieux vaut utiliser l'autre
void StaticInstance::set_pos_rot_scale(const glm::mat4 & mat) {
	_pos_rot->set_pos_rot_scale(mat);
}


void StaticInstance::set_precision(uint precision) {
	if (precision< _model->_n_precisions) {
		_precision= precision;
	}
	else {
		//cout << "Precision non supportée : " << _model->_ch_config_file << " ; " << precision << endl;
		_precision= _model->_n_precisions- 1;
	}
}


void StaticInstance::print() {
	_model->print();
}


// ---------------------------------------------------------------------------------------------------------------------
StaticGroup::StaticGroup() {

}


StaticGroup::StaticGroup(string ch_config_file, GLuint prog_draw, vector<InstancePosRot *> pos_rots, vector<float> distances) :
	_ch_config_file(ch_config_file), _prog_draw(prog_draw), _alpha(1.0f), _world2camera(glm::mat4(1.0f)), _world2clip(glm::mat4(1.0f))
{
	AABB * aabb_model= new AABB();
	parse_static_config_file(ch_config_file, _vertex_buffers, _index_buffers, _n_faces, _n_precisions, _materials_array, aabb_model);

	// on crée par classe de précision un buffer qui contiendra les 
	glGenBuffers(_n_precisions, _instance_buffers);

	glUseProgram(_prog_draw);

	_position_loc        = glGetAttribLocation(_prog_draw, "position_in");
	_normal_loc          = glGetAttribLocation(_prog_draw, "normal_in");
	_idx_material_loc    = glGetAttribLocation(_prog_draw, "idx_material_in");
	_instanced_matrix_loc= glGetAttribLocation(_prog_draw, "instanced_matrix");

	_world2clip_loc  = glGetUniformLocation(_prog_draw, "world2clip_matrix");
	_world2camera_loc= glGetUniformLocation(_prog_draw, "world2camera_matrix");
	_materials_loc   = glGetUniformLocation(_prog_draw, "materials");
	
	_alpha_loc= glGetUniformLocation(_prog_draw, "alpha");

	glUseProgram(0);

	// chaque pos correspond a un objet qui sera dessine a travers ce group
	for (auto pos : pos_rots) {
		_pos_rots.push_back(new InstancePosRot(pos->_position, pos->_rotation, pos->_scale, aabb_model));
	}

	for (uint i=0; i<_n_precisions; ++i) {
		// on prévoit que dans le cas extreme une classe de precision peut contenir tous les objets, et les autres classes aucun
		_mats[i]= new glm::mat4[_pos_rots.size()]; 
		_mats_sizes[i]= 0;
	}

	// _distances est un tableau de floats qui sont les seuils correspondants aux précisions
	for (auto d : distances) {
		_distances.push_back(d);
	}

	delete aabb_model;
}


StaticGroup::~StaticGroup() {
	for (auto pr : _pos_rots) {
		delete pr;
	}
	_pos_rots.clear();

	for (uint i=0; i<_n_precisions; ++i) {
		delete[] _mats[i];
	}
}


void StaticGroup::draw() {
	glUseProgram(_prog_draw);
	
	for (uint precision=0; precision<_n_precisions; ++precision) {
		glBindBuffer(GL_ARRAY_BUFFER, _vertex_buffers[precision]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _index_buffers[precision]);
		
		glUniformMatrix4fv(_world2clip_loc  , 1, GL_FALSE, glm::value_ptr(_world2clip));
		glUniformMatrix4fv(_world2camera_loc, 1, GL_FALSE, glm::value_ptr(_world2camera));
		glUniformMatrix4fv(_materials_loc, N_MAX_MATERIALS, GL_FALSE, glm::value_ptr(_materials_array[0]));
		glUniform1f(_alpha_loc, _alpha);

		// Enables the attribute indices
		glEnableVertexAttribArray(_position_loc);
		glEnableVertexAttribArray(_normal_loc);
		glEnableVertexAttribArray(_idx_material_loc);

		// Modifie les tableaux associés au buffer en cours d'utilisation
		glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, (3+ 3+ 1)* sizeof(float), 0);
		glVertexAttribPointer(_normal_loc  , 3, GL_FLOAT, GL_FALSE, (3+ 3+ 1)* sizeof(float), (void *)(3* sizeof(float)));
		glVertexAttribPointer(_idx_material_loc, 1, GL_FLOAT, GL_FALSE, (3+ 3+ 1)* sizeof(float), (void *)((3+ 3)* sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, _instance_buffers[precision]);
		for (uint j=0; j<4 ; ++j) {
			glEnableVertexAttribArray(_instanced_matrix_loc+ j);
			glVertexAttribPointer(_instanced_matrix_loc+ j, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const GLvoid*)(sizeof(glm::vec4)* j));
			glVertexAttribDivisor(_instanced_matrix_loc+ j, 1); // pour faire de l'instanced
		}

		// Rendu de notre geometrie
		glDrawElementsInstanced(GL_TRIANGLES, _n_faces[precision]* 3, GL_UNSIGNED_INT, 0, _mats_sizes[precision]);

		// Disables the attribute indices
		glDisableVertexAttribArray(_position_loc);
		glDisableVertexAttribArray(_normal_loc);
		glDisableVertexAttribArray(_idx_material_loc);
		for (uint j=0; j<4 ; ++j) {
			glVertexAttribDivisor(_instanced_matrix_loc+ j, 0); // necessaire de reinit !
			glDisableVertexAttribArray(_instanced_matrix_loc+ j);
		}
	}

	// on réinit à 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void StaticGroup::anim(ViewSystem * view_system) {
	_world2camera= view_system->_world2camera;
	_world2clip= view_system->_world2clip;

	for (uint i=0; i<_n_precisions; ++i) {
		_mats_sizes[i]= 0;
	}

	for (auto pr : _pos_rots) {
		if (!pr->_active) {
			continue;
		}

		// a quelle classe de precision va appartenir cet objet
		// ATTENTION : _dist2 doit etre calculé dans le programme appelant cette classe ; cela n'est pas fait de facon systematique ici pour des raisons de performance
		for (uint i=0; i<_distances.size(); ++i) {
			// cout << i << " ; " << _n_precisions- 1 << " ; " << pr->_dist2 << " ; " << _distances[i]* _distances[i] << endl;
			if ((i== _n_precisions- 1) || (pr->_dist2< _distances[i]* _distances[i])) {
				_mats[i][_mats_sizes[i]]= pr->_model2world;
				_mats_sizes[i]++;
				break;
			}
		}
	}

	// on renseigne les _instance_buffers
	for (uint precision=0; precision<_n_precisions; ++precision) {
		glBindBuffer(GL_ARRAY_BUFFER, _instance_buffers[precision]);
		glBufferData(GL_ARRAY_BUFFER, _mats_sizes[precision]* sizeof(glm::mat4), glm::value_ptr(_mats[precision][0]), GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

