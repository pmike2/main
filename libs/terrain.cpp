
#include "terrain.h"

using namespace std;



glm::vec3 alti2color(float z) {
	for (auto & it_color : LEVEL_COLORS) {
		if (z<= it_color[0]) {
			return glm::vec3(it_color[1], it_color[2], it_color[3]);
		}
	}
	return glm::vec3(0.0);
}


// --------------------------------------------------------------------------------------------
TerrainConfig::TerrainConfig() {

}


TerrainConfig::TerrainConfig(const glm::vec2 & origin, float width_sub, float height_sub, float width_step, float height_step, unsigned int n_subs_x, unsigned int n_subs_y) :
	_origin(origin), _width_sub(width_sub), _height_sub(height_sub), _width_step(width_step), _height_step(height_step), _n_subs_x(n_subs_x), _n_subs_y(n_subs_y)
{
	_width= _width_sub* (float)(_n_subs_x);
	_height= _height_sub* (float)(_n_subs_y);
	_width_n= (unsigned int)(_width/ _width_step);
	_height_n= (unsigned int)(_height/ _height_step);
	_width_sub_n= (unsigned int)(_width_sub/ _width_step);
	_height_sub_n= (unsigned int)(_height_sub/ _height_step);
	_n_subs= _n_subs_x* _n_subs_y;

	// on divise la resolution du terrain par des puissances de 2 ; d'ou l'interet d'avoir des dimensions en puissance de 2 a la base
	// le buffer des vertices est constant, ce qui change est le buffer des indices
	for (unsigned int k=0; k<N_DEGRADATIONS; ++k) {
		unsigned int step= pow(2, DEGRADATIONS[k]._step);
		unsigned int width_n_sub= _width_sub_n/ step;
		unsigned int height_n_sub= _height_sub_n/ step;

		unsigned int * idx= new unsigned int[width_n_sub* height_n_sub* 2* 3];
		for (unsigned i=0; i<width_n_sub; ++i) {
			for (unsigned j=0; j<height_n_sub; ++j) {
				unsigned int ii= 6* (i* height_n_sub+ j);
				
				idx[ii+ 0]= (i* (_height_sub_n+ 1)+ j)* step;
				idx[ii+ 1]= ((i+ 1)* (_height_sub_n+ 1)+ j)* step;
				idx[ii+ 2]= ((i+ 1)* (_height_sub_n+ 1)+ j+ 1)* step;
				idx[ii+ 3]= (i* (_height_sub_n+ 1)+ j)* step;
				idx[ii+ 4]= ((i+ 1)* (_height_sub_n+ 1)+ j+ 1)* step;
				idx[ii+ 5]= (i* (_height_sub_n+ 1)+ j+ 1)* step;
			}
		}

		_idxs[k]= idx;
	}
}


TerrainConfig::~TerrainConfig() {
	for (unsigned int k=0; k<N_DEGRADATIONS; ++k) {
		delete _idxs[k];
	}
}


void TerrainConfig::print() {
	cout << "_origin=" << glm::to_string(_origin) << endl;
	cout << "_width_step=" << _width_step << " ; _height_step=" << _height_step << endl;
	cout << "_width=" << _width << " ; _height=" << _height << endl;
	cout << "_width_n=" << _width_n << " ; _height_n=" << _height_n << endl;
	cout << "_n_subs_x=" << _n_subs_x << " ; _n_subs_y=" << _n_subs_y << endl;
	cout << "_width_sub=" << _width_sub << " ; _height_sub=" << _height_sub << endl;
	cout << "_width_sub_n=" << _width_sub_n << " ; _height_sub_n=" << _height_sub_n << endl;
}


// --------------------------------------------------------------------------------------------
TerrainMesh::TerrainMesh() {

}


TerrainMesh::TerrainMesh(GLuint prog_draw_3d) :
	_prog_draw(prog_draw_3d), _alpha(1.0f), _shininess(LEVEL_SHININESS), _tide(0.0f), _current_index_buffer_idx(N_DEGRADATIONS- 1), _ambient(LEVEL_AMBIENT_COLOR),
	_model2world(glm::mat4(1.0f)), _model2camera(glm::mat4(1.0f)), _model2clip(glm::mat4(1.0f)), _normal(glm::mat3(1.0f))
{
	glUseProgram(_prog_draw);

	_position_loc     = glGetAttribLocation(_prog_draw, "position_in");
	_normal_loc       = glGetAttribLocation(_prog_draw, "normal_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");

	_ambient_color_loc= glGetUniformLocation(_prog_draw, "ambient_color");
	_shininess_loc    = glGetUniformLocation(_prog_draw, "shininess");
	
	_model2clip_loc  = glGetUniformLocation(_prog_draw, "model2clip_matrix");
	_model2camera_loc= glGetUniformLocation(_prog_draw, "model2camera_matrix");
	_normal_mat_loc  = glGetUniformLocation(_prog_draw, "normal_matrix");
	
	_alpha_loc= glGetUniformLocation(_prog_draw, "alpha");

	_tide_loc= glGetUniformLocation(_prog_draw, "tide");
	
	glUseProgram(0);

	glGenBuffers(1, &_vertex_buffer);
	glGenBuffers(N_DEGRADATIONS, _index_buffers);
}


TerrainMesh::~TerrainMesh() {

}


void TerrainMesh::draw() {
	glUseProgram(_prog_draw);
	// On précise les données que l'on souhaite utiliser
	glBindBuffer(GL_ARRAY_BUFFER, _vertex_buffer);
	// On précise le tableau d'indices de triangle à utiliser
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _index_buffers[_current_index_buffer_idx]);
	
	glUniformMatrix4fv(_model2clip_loc  , 1, GL_FALSE, glm::value_ptr(_model2clip));
	glUniformMatrix4fv(_model2camera_loc, 1, GL_FALSE, glm::value_ptr(_model2camera));
	glUniformMatrix3fv(_normal_mat_loc  , 1, GL_FALSE, glm::value_ptr(_normal));
	glUniform3fv(_ambient_color_loc, 1, glm::value_ptr(_ambient));
	glUniform1f(_shininess_loc, _shininess);
	glUniform1f(_alpha_loc, _alpha);
	glUniform1f(_tide_loc, _tide);

	// Enables the attribute indices
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_normal_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	// Modifie les tableaux associés au buffer en cours d'utilisation
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3)* sizeof(float), 0);
	glVertexAttribPointer(_normal_loc  , 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3)* sizeof(float), (void *)(3* sizeof(float)));
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3)* sizeof(float), (void *)((3+ 3)* sizeof(float)));
	
	// Rendu de notre geometrie
	glDrawElements(GL_TRIANGLES, 3* _n_faces[_current_index_buffer_idx], GL_UNSIGNED_INT, 0);

	// Disables the attribute indices
	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_normal_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);
	
	// on réinit à 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
	
}


void TerrainMesh::anim(const glm::mat4 & world2camera, const glm::mat4 & camera2clip) {
	_model2camera= world2camera* _model2world;
	_model2clip= camera2clip* _model2camera;
	// theoriquement il faudrait prendre la transposee de l'inverse mais si model2camera est 
	// une matrice orthogonale, TRANS(INV(M)) == M, ce qui est le cas lorsqu'elle ne comprend que 
	// des translations et rotations
	_normal= glm::mat3(_model2camera);
	
	_tide+= TIDE_ANIM_SPEED;
	while (_tide> M_PI* 2.0f) {
		_tide-= M_PI* 2.0f;
	}
}


// -----------------------------------------------------------------------------
SubTerrain::SubTerrain() {

}


SubTerrain::SubTerrain(GLuint prog_draw_3d, float * altis, const TerrainConfig * config, unsigned int idx_x, unsigned int idx_y) :
	_draw_mesh(true), _active(true), _loaded(false), _buffers_filled(false), _idx_x(idx_x), _idx_y(idx_y)
{
	_config= (TerrainConfig * )(config);
	_mesh= new TerrainMesh(prog_draw_3d);

	glm::vec3 vmin= glm::vec3(_config->_origin.x+ _config->_width_sub* (float)(idx_x), _config->_origin.y+ _config->_height_sub* (float)(idx_y), 0.0f);
	glm::vec3 vmax= vmin+ glm::vec3(_config->_width_sub, _config->_height_sub, 0.0f);
	_aabb= new AABB(vmin, vmax);
}


SubTerrain::~SubTerrain() {
	delete _mesh;
	delete _aabb;
}


void SubTerrain::load(float * altis) {
	// extraction des vertices du terrain global

	//cout << "loading " << idx_x << " ; " << idx_y << endl;
	_vertices= new float[(3+ 3+ 3)* (_config->_width_sub_n+ 1)* (_config->_height_sub_n+ 1)];
	
	unsigned int idx_alti_origin= (_config->_width_n+ 1)* _config->_height_sub_n* _idx_y+ _config->_width_sub_n* _idx_x;
	glm::vec3 vmin= glm::vec3(_config->_origin.x+ _config->_width_sub* (float)(_idx_x), _config->_origin.y+ _config->_height_sub* (float)(_idx_y), 0.0f);
	//glm::vec3 vmax= vmin+ glm::vec3(_config->_width_sub, _config->_height_sub, 0.0f);
	for (int i=0; i<_config->_width_sub_n+ 1; ++i) {
		for (int j=0; j<_config->_height_sub_n+ 1; ++j) {

			glm::vec3 pos= glm::vec3(vmin.x+ (float)(i)* _config->_width_step, vmin.y+ (float)(j)* _config->_height_step, altis[idx_alti_origin+ i+ (_config->_width_n+ 1)* j]);

			// maj de bbox.z
			if (pos.z< _aabb->_vmin.z) {
				_aabb->_vmin.z= pos.z;
			}
			if (pos.z> _aabb->_vmax.z) {
				_aabb->_vmax.z= pos.z;
			}
			
			glm::vec3 color= alti2color(pos.z);
			
			// calcul de la normale
			glm::vec3 norm(0.0f, 0.0f, 0.0f);
			for (int ii=0; ii<6; ++ii) {
				bool out_of_bound= false;
				for (int jj=0; jj<3; ++jj) {
					if ((i+ NEIGHBORS[ii][jj][0]< 0) || (i+ NEIGHBORS[ii][jj][0]> _config->_width_n) || (j+ NEIGHBORS[ii][jj][1]< 0) || (j+ NEIGHBORS[ii][jj][1]> _config->_height_n)) {
						out_of_bound= true;
						break;
					}
				}
				if (out_of_bound) {
					continue;
				}

				// push_back SUPER LENT !!! et glm::triangleNormal un peu lent aussi -> on utilise du basique
				float pts[4][3];
				for (int jj=0; jj<3; ++jj) {
					pts[jj][0]= pos.x+ (float)(NEIGHBORS[ii][jj][0])* _config->_width_step;
					pts[jj][1]= pos.y+ (float)(NEIGHBORS[ii][jj][1])* _config->_height_step;
					pts[jj][2]= altis[idx_alti_origin+ (i+ NEIGHBORS[ii][jj][0])+ (_config->_width_n+ 1)* (j+ NEIGHBORS[ii][jj][1])];
				}

				calculate_normal(pts[0], pts[1], pts[2], pts[3]);
				norm.x+= pts[3][0];
				norm.y+= pts[3][1];
				norm.z+= pts[3][2];
			}
			norm= normalize(norm);
			
			unsigned int ii= 9* (i* (_config->_height_sub_n+ 1)+ j);
			_vertices[ii+ 0]= pos.x;
			_vertices[ii+ 1]= pos.y;
			_vertices[ii+ 2]= pos.z;
			_vertices[ii+ 3]= norm.x;
			_vertices[ii+ 4]= norm.y;
			_vertices[ii+ 5]= norm.z;
			_vertices[ii+ 6]= color.x;
			_vertices[ii+ 7]= color.y;
			_vertices[ii+ 8]= color.z;
		}
	}

	// calcul du nombre de faces en fonction de la degradation
	for (unsigned int k=0; k<N_DEGRADATIONS; ++k) {
		unsigned int step= pow(2, DEGRADATIONS[k]._step);
		unsigned int width_n_sub= _config->_width_sub_n/ step;
		unsigned int height_n_sub= _config->_height_sub_n/ step;

		_mesh->_n_faces[k]= width_n_sub* height_n_sub* 2;
	}

	// est-ce necessaire ?
 	//_mtx.lock();
	 //unique_lock<mutex> lock(_mtx);
	_loaded= true;
	//_mtx.unlock();
}


void SubTerrain::set_degradation(unsigned int degradation) {
	if (degradation>= N_DEGRADATIONS) {
		cout << "Terrain::set_degradation erreur : " << degradation << endl;
		return;
	}
	_mesh->_current_index_buffer_idx= degradation;
}


void SubTerrain::draw() {
	if ((!_active) || (!_loaded) || (!_buffers_filled)) {
		return;
	}

	if (_draw_mesh) {
		_mesh->draw();
	}

}


void SubTerrain::anim(ViewSystem * view_system) {
	if (!_loaded) {
		return;
	}

	if (!_buffers_filled) {
		// on rentre 1 fois dans ce bloc, lorsque _loaded vaut true mais pas encore _buffers_filled
		// remplissage des buffers et nettoyage
		glBindBuffer(GL_ARRAY_BUFFER, _mesh->_vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, (3+ 3+ 3)* (_config->_width_sub_n+ 1)* (_config->_height_sub_n+ 1)* sizeof(float), _vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		for (unsigned int i=0; i<N_DEGRADATIONS; ++i) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _mesh->_index_buffers[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh->_n_faces[i]* 3* sizeof(unsigned int), _config->_idxs[i], GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		delete[] _vertices;

		_buffers_filled= true;

		//cout << "loaded subterrain " << _idx_x << " ; " << _idx_y << endl;
	}

	if (!_active) {
		return;
	}

	_mesh->anim(view_system->_world2camera, view_system->_camera2clip);
}


// -----------------------------------------------------------------------------
GroupTerrain::GroupTerrain() : _loaded(false) {

}


GroupTerrain::~GroupTerrain() {

}


void GroupTerrain::load(float * altis) {
	// lancement du thread de chargement des subterrains associés
	_thr= thread(& GroupTerrain::load_thread, this,  altis);
	//load_thread(altis);
}


void GroupTerrain::load_thread(float * altis) {
	for (auto subterrain : _subterrains) {
		subterrain->load(altis);
	}
 }


// -----------------------------------------------------------------------------
Terrain::Terrain() {

}


Terrain::Terrain(GLuint prog_draw_3d, const TerrainConfig * config, const TerrainRandomConfig * random_config, std::string ch_alti_file) {
	_config= (TerrainConfig * )(config);
	//_config->print();

	_altis= new float[(_config->_width_n+ 1)* (_config->_height_n+ 1)];
	for (unsigned int i=0; i<(_config->_width_n+ 1)* (_config->_height_n+ 1); ++i) {
		_altis[i]= 0.0f;
	}

	// ref : https://www.redblobgames.com/maps/terrain-from-noise
	if (random_config) {
		/* // test
		for (unsigned i=0; i<_config->_width_n+ 1; ++i)
			for (unsigned j=0; j<_config->_height_n+ 1; ++j) {
				_altis[i+ (_config->_width_n+ 1)* j]= (float)(i)* 2.0f;
			}*/

		for (unsigned i=0; i<_config->_width_n+ 1; ++i)
			for (unsigned j=0; j<_config->_height_n+ 1; ++j) {
				_altis[i+ (_config->_width_n+ 1)* j]= random_config->_alti_offset;
			}
		
		for (unsigned int level=0; level<random_config->_n_levels; ++level) {
			unsigned int gradient_w= random_config->_gradient_base_size* (level+ 1);
			unsigned int gradient_h= random_config->_gradient_base_size* (level+ 1);
			float gradient[gradient_w* gradient_h* 2];
			float factor= random_config->_max_factor* pow(2.0f, -float(level));
		
			for (unsigned i=0; i<gradient_w; ++i)
				for (unsigned j=0; j<gradient_h; ++j) {
					float rand_angle= rand_float(0.0f, 2.0f* M_PI);
					gradient[2*(i+ j* gradient_w)]   = cos(rand_angle);
					gradient[2*(i+ j* gradient_w)+ 1]= sin(rand_angle);
				}
		
			for (unsigned i=0; i<_config->_width_n+ 1; ++i)
				for (unsigned j=0; j<_config->_height_n+ 1; ++j) {
					float ii= float(i)* (gradient_w- 1)/ (_config->_width_n+ 1);
					float jj= float(j)* (gradient_h- 1)/ (_config->_height_n+ 1);
					
					_altis[i+ (_config->_width_n+ 1)* j]+= factor* perlin(ii, jj, gradient, gradient_w, gradient_h);
				}
		}

		for (unsigned i=0; i<_config->_width_n+ 1; ++i) {
			for (unsigned j=0; j<_config->_height_n+ 1; ++j) {
				if (_altis[i+ (_config->_width_n+ 1)* j]< 0.0f) {
					_altis[i+ (_config->_width_n+ 1)* j]= 0.0f;
				}
				else {
					_altis[i+ (_config->_width_n+ 1)* j]= pow(_altis[i+ (_config->_width_n+ 1)* j], random_config->_redistribution_power);
				}
			}
		}
	}

	// chargement a partir d'un TIF
	else if (ch_alti_file!= "") {
		unsigned int height, width;

		TIFFSetWarningHandler(NULL);
		TIFF * alti_tif= TIFFOpen(ch_alti_file.c_str(), "r");
		TIFFGetField(alti_tif, TIFFTAG_IMAGELENGTH, & height);
		TIFFGetField(alti_tif, TIFFTAG_IMAGEWIDTH, & width);
		if ((width!= _config->_width_n+ 1) || (height!= _config->_height_n+ 1)) {
			cout << "TIF mauvaises dimensions : theorique = (" << _config->_width_n+ 1 << " , " << _config->_height_n+ 1 << ") ; pratique = (" << width << " , " << height << ")" << endl;
			return;
		}
		
		for (unsigned int row=0; row<height; ++row) {
			TIFFReadScanline(alti_tif, _altis+ width* row, row, 0);
		}
		
		TIFFClose(alti_tif);
	}

	for (unsigned int i=0; i<N_GROUPS; ++i) {
		_group_terrains[i]= new GroupTerrain();
	}

 	_subterrains= new SubTerrain * [_config->_n_subs];
	for (unsigned int idx_x=0; idx_x<_config->_n_subs_x; ++idx_x) {
		for (unsigned int idx_y=0; idx_y<_config->_n_subs_y; ++idx_y) {
			_subterrains[idx_x* _config->_n_subs_y+ idx_y]= new SubTerrain(prog_draw_3d, _altis, config, idx_x, idx_y);
			// a terme faire un ajout + intelligent de sorte qu'apparaisse a l'ecran les 1ers subterrains chargés
			_group_terrains[(idx_x* _config->_n_subs_y+ idx_y) % N_GROUPS]->_subterrains.push_back(_subterrains[idx_x* _config->_n_subs_y+ idx_y]);
		}
	}

	for (unsigned int i=0; i<N_GROUPS; ++i) {
		_group_terrains[i]->load(_altis);
	}

	/*_horizon_n_width= HORIZON_N_WIDTH;
	_horizon_n_height= HORIZON_N_HEIGHT;
	_horizons= new float[(_horizon_n_width+ 1)* (_horizon_n_height+ 1)];*/
}


Terrain::~Terrain() {
	delete[] _altis;
	delete[] _subterrains;
	for (unsigned int i=0; i<N_GROUPS; ++i) {
		//delete _group_terrains[i]; // je ne sais pas pourquoi genere une erreur !
	}
	//delete[] _horizons;
}


void Terrain::draw() {
	for (unsigned int idx=0; idx<_config->_n_subs; ++idx) {
		_subterrains[idx]->draw();
	}
}


void Terrain::anim(ViewSystem * view_system) {
	// on fait le join des threads group_terrain quand tous les subterrains associés sont chargés
	for (unsigned int i=0; i<N_GROUPS; ++i) {
		if (_group_terrains[i]->_loaded) {
			continue;
		}
		bool done= true;
		for (auto subterrain : _group_terrains[i]->_subterrains) {
			if (!subterrain->_buffers_filled) {
				done= false;
				break;
			}
		}
		if (done) {
			_group_terrains[i]->_loaded= true;
			_group_terrains[i]->_thr.join();
		}
	}

	for (unsigned int idx_x=0; idx_x<_config->_n_subs_x; ++idx_x) {
		for (unsigned int idx_y=0; idx_y<_config->_n_subs_y; ++idx_y) {
			unsigned int idx= idx_x* _config->_n_subs_y+ idx_y;
			
			// est-ce que cela vaudrait le coup de faire du quadtree ?
			if (!view_system->intersects_aabb(_subterrains[idx]->_aabb)) {
				_subterrains[idx]->_active= false;
				continue;
			}
			else {
				_subterrains[idx]->_active= true;
			}
			
			// on calcule la distance entre eye et le milieu du subterrain
			// distance2 est lent !
			//float dist2= glm::distance2(view_system->_eye, glm::vec3(((float)(idx_x)+ 0.5f)* _config->_width_sub, ((float)(idx_y)+ 0.5f)* _config->_height_sub, 0.0f));
			float x= ((float)(idx_x)+ 0.5f)* _config->_width_sub;
			float y= ((float)(idx_y)+ 0.5f)* _config->_height_sub;
			float z= 0.0f;
			float dist2= (view_system->_eye.x- x)* (view_system->_eye.x- x)+ (view_system->_eye.y- y)* (view_system->_eye.y- y)+ (view_system->_eye.z- z)* (view_system->_eye.z- z);

			// choix de la degradation en fonction de la distance
			_subterrains[idx]->set_degradation(DEGRADATIONS[N_DEGRADATIONS- 1]._idx);
			for (unsigned int k=0; k<N_DEGRADATIONS; ++k) {
				if (dist2< DEGRADATIONS[k]._dist* DEGRADATIONS[k]._dist) {
					_subterrains[idx]->set_degradation(DEGRADATIONS[k]._idx);
					break;
				}
			}

			_subterrains[idx]->anim(view_system);
		}
	}

	/*float horizon_step_height= (view_system->_frustum_far- view_system->_frustum_near)/ (float)(_horizon_n_height);
	glm::vec2 eye= glm::vec2(view_system->_eye);
	glm::vec2 dir= glm::vec2(view_system->_dir);
	glm::vec2 right= glm::vec2(view_system->_right);
	for (unsigned int j=0; j<_horizon_n_height+ 1; ++j) {
		float height= view_system->_frustum_near+ (float)(j)* horizon_step_height;
		float halfsize= (height/ view_system->_frustum_near)* view_system->_frustum_halfsize;
		float horizon_step_width= 2.0f* halfsize/ (float)(_horizon_n_width);
		glm::vec2 center= eye+ height* dir;
		glm::vec2 left= center- halfsize* right;
		for (unsigned int i=0; i<_horizon_n_width+ 1; ++i) {
			glm::vec2 pt= left+ horizon_step_width* (float)(i);
			_horizons[i+ (_horizon_n_width+ 1)* j]= get_alti(pt, false); // pas de msg si on sort du terrain, valeur 0.0f OK
		}
	}

	for (unsigned int i=0; i<_horizon_n_width+ 1; ++i) {
		for (unsigned int j=1; j<_horizon_n_height+ 1; ++j) {
			if (_horizons[i+ (_horizon_n_width+ 1)* j]< _horizons[i+ (_horizon_n_width+ 1)* (j- 1)]) {
				_horizons[i+ (_horizon_n_width+ 1)* j]= _horizons[i+ (_horizon_n_width+ 1)* (j- 1)];
			}
		}
	}*/

}


// renvoie l'alti interpolée
float Terrain::get_alti(const glm::vec2 & pos, bool cout_err) {
	int i, j;
	float xx, yy;

	if ((pos.x< _config->_origin.x) || (pos.x- _config->_origin.x> _config->_width) || (pos.y< _config->_origin.y) || (pos.y- _config->_origin.y> _config->_height)) {
		if (cout_err) {
			cout << "Terrain::get_alti ERREUR : pos=" << glm::to_string(pos) << " width=" << _config->_width << " height=" << _config->_height << " origin=" << glm::to_string(_config->_origin) << endl;
		}
		return 0.0f;
	}
	
	i= (int)((pos.x- _config->_origin.x)/ _config->_width_step);
	j= (int)((pos.y- _config->_origin.y)/ _config->_height_step);

	xx= (pos.x- _config->_origin.x)/ _config->_width_step- (float)(i);
	yy= (pos.y- _config->_origin.y)/ _config->_height_step- (float)(j);

	// gestion du ca-tombe-pile-poil-sur-le-bord droit ou haut
	if (i== _config->_width_n) {
		i= _config->_width_n- 1;
		xx= 1.0f;
	}

	if (j== _config->_height_n) {
		j= _config->_height_n- 1;
		yy= 1.0f;
	}

	float alti_1= (1.0f- xx)* _altis[i+ (_config->_width_n+ 1)* j]+ xx* _altis[(i+ 1)+ (_config->_width_n+ 1)* j];
	float alti_2= (1.0f- xx)* _altis[i+ (_config->_width_n+ 1)* (j+ 1)]+ xx* _altis[(i+ 1)+ (_config->_width_n+ 1)* (j+ 1)];

	float alti= (1.0f- yy)* alti_1+ yy* alti_2;
	//if ((alti> 10000.0f) || (alti< -1000.0f)) cout << "get : " << glm::to_string(pos) << " ; " << i << " ; " << j << " ; " << xx << " ; " << yy << " ; " << alti << endl;
	
	return alti;
}


void Terrain::set_draw_mesh(bool b) {
	for (unsigned int idx=0; idx<_config->_n_subs; ++idx) {
		_subterrains[idx]->_draw_mesh= true;
	}
}



void Terrain::save(string ch_tif) {
	TIFF * alti_tif= TIFFOpen(ch_tif.c_str(), "w");
	TIFFSetField(alti_tif, TIFFTAG_IMAGEWIDTH, _config->_width_n+ 1);
	TIFFSetField(alti_tif, TIFFTAG_IMAGELENGTH, _config->_height_n+ 1);
	TIFFSetField(alti_tif, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(alti_tif, TIFFTAG_BITSPERSAMPLE, 32);
	TIFFSetField(alti_tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(alti_tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(alti_tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
	TIFFSetField(alti_tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	//TIFFSetField(alti_tif, TIFFTAG_ROWSPERSTRIP, 1);
	TIFFSetField(alti_tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(alti_tif, _config->_width_n+ 1));

	for (unsigned int row=0; row<_config->_height_n+ 1; ++row) {
		TIFFWriteScanline(alti_tif, (unsigned char *)(_altis+ (_config->_width_n+ 1)* row), row, 0);
	}

	TIFFClose(alti_tif);
}

/*
bool Terrain::aabb_show_horizon(AABB * aabb) {
	for (unsigned int i=0; i<8; ++i) {
		aabb->_pts[i]
	}
}*/


// est-ce qu'une bounding box translatée et tournée intersecte le level
// a revoir ...
/*bool Terrain::intersects_bbox(glm::vec3 &translation, glm::mat3 &rotation_matrix, BBox &bbox) {
	// on construit les 8 sommets transformés
	 vector<glm::vec3> bbox_vertices;
	for (int i=0; i<2; ++i)
		for (int j=0; j<2; ++j)
			for (int k=0; k<2; ++k)
				bbox_vertices.push_back(translation+ rotation_matrix* glm::vec3(bbox._xmin+ i* (bbox._xmax- bbox._xmin), bbox._ymin+ j* (bbox._ymax- bbox._ymin), bbox._zmin+ k* (bbox._zmax- bbox._zmin)));
	
	// on cherche l'ensemble des indices des sommets qui sont sous l'emprise large (utilisation de radius) de la bbox
	int imin= int((translation.x- bbox._radius+ _width * 0.5f)/ _width_step);
	int imax= int((translation.x+ bbox._radius+ _width * 0.5f)/ _width_step);
	int jmin= int((translation.y- bbox._radius+ _height* 0.5f)/ _height_step);
	int jmax= int((translation.y+ bbox._radius+ _height* 0.5f)/ _height_step);
	
	if (imin< 0) imin= 0; if (imin>=_width_n)  imin= _width_n - 1;
	if (imax< 0) imax= 0; if (imax>=_width_n)  imax= _width_n - 1;
	if (jmin< 0) jmin= 0; if (jmin>=_height_n) jmin= _height_n- 1;
	if (jmax< 0) jmax= 0; if (jmax>=_height_n) jmax= _height_n- 1;
	
	vector<int> idx_faces;
	for (int i=imin; i<=imax; ++i)
		for (int j=jmin; j<=jmax; ++j) {
			idx_faces.push_back(i+ _width_n* j);
		}
	
	// pour chaque face du level (2 triangles) on regarde si au - 1 des sommets de la bbox est du mauvais coté
	glm::vec3 v0, norm0, norm1; // v0 est commun aux 2 faces d'un carré
	float scal0, scal1;
	for (auto it_face : idx_faces) {
		v0   = glm::vec3(_mesh->_vertices[27* (2* it_face+ 0)+ 0], _mesh->_vertices[27* (2* it_face+ 0)+ 1], _mesh->_vertices[27* (2* it_face+ 0)+ 2]);
		norm0= glm::vec3(_mesh->_vertices[27* (2* it_face+ 0)+ 3], _mesh->_vertices[27* (2* it_face+ 0)+ 4], _mesh->_vertices[27* (2* it_face+ 0)+ 5]);
		norm1= glm::vec3(_mesh->_vertices[27* (2* it_face+ 1)+ 3], _mesh->_vertices[27* (2* it_face+ 1)+ 4], _mesh->_vertices[27* (2* it_face+ 1)+ 5]);
		
		for (auto &it_vertex : bbox_vertices) {
			scal0= glm::dot(norm0, it_vertex- v0);
			scal1= glm::dot(norm1, it_vertex- v0);
			if ((scal0< 0.0f) || (scal1< 0.0f)) {
				return true;
			}
		}
	}
	return false;
}*/


/*
// lecture a partir d'un fichier binaire ; a conserver ?
void Terrain::read(string ch_alti_data) {
	ifstream alti_data(ch_alti_data, ios::binary);
	alti_data.seekg(0, ios::end);
	unsigned int n_altis= alti_data.tellg()/ sizeof(float);
	alti_data.seekg(0, ios::beg);

	for (unsigned int i=0; i<(_width_n+ 1)* (_height_n+ 1); ++i) {
		_altis[i]= 0.0f;
	}

	unsigned int compt= 0;
	while ((compt< (_width_n+ 1)* (_height_n+ 1)) && (alti_data.read(reinterpret_cast<char *>(_altis+ compt), sizeof(float)))) {
		compt++;
	}

	sync_mesh();
}

*/