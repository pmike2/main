
#include "objfile.h"

using namespace std;



ModelObj::ModelObj(){
}


ModelObj::ModelObj(GLuint prog_draw_, struct timeval t_){
	n_faces= 0;

	xmin= 1.e8; xmax= -1.e8;
	ymin= 1.e8; ymax= -1.e8;
	zmin= 1.e8; zmax= -1.e8;
	
	prog_draw= prog_draw_;
	
	is_active= true;
	t= t_;
}


void ModelObj::load(string ch_obj, string ch_mat)
{
	ifstream obj_file(ch_obj.c_str());
	ifstream mat_file(ch_mat.c_str());
	string line;

	unsigned int vertices_compt= 0;
	unsigned int faces_compt= 0;
	unsigned int face_tmp[3]= {0, 0, 0};
	float diffuse_tmp[3]= {0.0, 0.0, 0.0};
	unsigned int n_vertices_tmp= 0;
	float * vertices_tmp;

	// fichier mtl -------------------------------------------------
	if (mat_file.is_open())
	{
		mat_file.seekg(0, ios::beg);
		while (!mat_file.eof())
		{		
			getline(mat_file, line);
			
			istringstream iss(line);
			string sub, sub2;
			iss >> sub;
			
			// A faire Žvoluer; j'utilise pour l'instant le fichier de config pour faire passer d'un matŽriau a un autre...
			
			
			/*if (sub== "Ka"){
				iss >> sub2; ambient[0]= atof(sub2.c_str());
				iss >> sub2; ambient[1]= atof(sub2.c_str());
				iss >> sub2; ambient[2]= atof(sub2.c_str());
			}*/
			if (sub== "Kd"){
				iss >> sub2; diffuse_tmp[0]= atof(sub2.c_str());
				iss >> sub2; diffuse_tmp[1]= atof(sub2.c_str());
				iss >> sub2; diffuse_tmp[2]= atof(sub2.c_str());
			}
			/*if (sub== "Ns"){
				iss >> sub2; shininess= atof(sub2.c_str());
			}*/
			
		}
	}
	else 
	{
		cout << "Impossible d'ouvrir le fichier mat : " << ch_mat << endl;
	}
	mat_file.close();
	
	// fichier obj -------------------------------------------------
	if (obj_file.is_open())
	{
		obj_file.seekg(0, ios::beg);
		while (!obj_file.eof())
		{		
			getline(obj_file, line);
			
			if ( (line.c_str()[0]== 'v') && (line.c_str()[1]!= 't') )
				n_vertices_tmp++;
			else if (line.c_str()[0]== 'f')
				n_faces++;
		}
		
		// pour chaque face, 3 sommets, et chaque sommet est (x, y, z, nx, ny, nz, r, g, b)
		vertices= (float*) malloc((3+ 3+ 3)* 3* n_faces* sizeof(float));
		vertices_tmp= (float*) malloc(3* n_vertices_tmp* sizeof(float));
		faces= (unsigned int*) malloc(3* n_faces* sizeof(unsigned int));
		
		obj_file.clear();
		obj_file.seekg(0, ios::beg);
		while (!obj_file.eof())
		{		
			getline(obj_file, line);
			
			if (line.c_str()[0]== 'v') {
				// Set first character to 0. This will allow us to use sscanf
				line[0]= ' ';
				sscanf(line.c_str(), "%f%f%f ", &vertices_tmp[3* vertices_compt], &vertices_tmp[3* vertices_compt+ 1], &vertices_tmp[3* vertices_compt+ 2]);

				if (xmin> vertices_tmp[3* vertices_compt]) xmin= vertices_tmp[3* vertices_compt];
				if (xmax< vertices_tmp[3* vertices_compt]) xmax= vertices_tmp[3* vertices_compt];
				if (ymin> vertices_tmp[3* vertices_compt+ 1]) ymin= vertices_tmp[3* vertices_compt+ 1];
				if (ymax< vertices_tmp[3* vertices_compt+ 1]) ymax= vertices_tmp[3* vertices_compt+ 1];
				if (zmin> vertices_tmp[3* vertices_compt+ 2]) zmin= vertices_tmp[3* vertices_compt+ 2];
				if (zmax< vertices_tmp[3* vertices_compt+ 2]) zmax= vertices_tmp[3* vertices_compt+ 2];

				vertices_compt++;
			}
			else if (line.c_str()[0]== 'f') {
				// Set first character to 0. This will allow us to use sscanf
		    	line[0]= ' ';
				
                sscanf(line.c_str(),"%i %i %i", &face_tmp[0], &face_tmp[1], &face_tmp[2]);
				// OBJ file starts counting from 1
				face_tmp[0]-= 1;
				face_tmp[1]-= 1;
				face_tmp[2]-= 1;
				
				float coord1[3]= {vertices_tmp[3* face_tmp[0]], vertices_tmp[3* face_tmp[0]+ 1], vertices_tmp[3* face_tmp[0]+ 2]};
				float coord2[3]= {vertices_tmp[3* face_tmp[1]], vertices_tmp[3* face_tmp[1]+ 1], vertices_tmp[3* face_tmp[1]+ 2]};
				float coord3[3]= {vertices_tmp[3* face_tmp[2]], vertices_tmp[3* face_tmp[2]+ 1], vertices_tmp[3* face_tmp[2]+ 2]};
				float norm[3];
				calculate_normal(coord1, coord2, coord3, norm);
				
				// a terme gŽrer un diffuse diffŽrent par sommet dans Blender
				
				vertices[27* faces_compt+  0]= coord1[0];
				vertices[27* faces_compt+  1]= coord1[1];
				vertices[27* faces_compt+  2]= coord1[2];
				vertices[27* faces_compt+  3]= norm[0];
				vertices[27* faces_compt+  4]= norm[1];
				vertices[27* faces_compt+  5]= norm[2];
				vertices[27* faces_compt+  6]= diffuse_tmp[0];
				vertices[27* faces_compt+  7]= diffuse_tmp[1];
				vertices[27* faces_compt+  8]= diffuse_tmp[2];

				vertices[27* faces_compt+  9]= coord2[0];
				vertices[27* faces_compt+ 10]= coord2[1];
				vertices[27* faces_compt+ 11]= coord2[2];
				vertices[27* faces_compt+ 12]= norm[0];
				vertices[27* faces_compt+ 13]= norm[1];
				vertices[27* faces_compt+ 14]= norm[2];
				vertices[27* faces_compt+ 15]= diffuse_tmp[0];
				vertices[27* faces_compt+ 16]= diffuse_tmp[1];
				vertices[27* faces_compt+ 17]= diffuse_tmp[2];

				vertices[27* faces_compt+ 18]= coord3[0];
				vertices[27* faces_compt+ 19]= coord3[1];
				vertices[27* faces_compt+ 20]= coord3[2];
				vertices[27* faces_compt+ 21]= norm[0];
				vertices[27* faces_compt+ 22]= norm[1];
				vertices[27* faces_compt+ 23]= norm[2];
				vertices[27* faces_compt+ 24]= diffuse_tmp[0];
				vertices[27* faces_compt+ 25]= diffuse_tmp[1];
				vertices[27* faces_compt+ 26]= diffuse_tmp[2];
				
				faces_compt++;
			}	
		}
		obj_file.close();

		free(vertices_tmp);
		
		// ----------------------------------------------------------------------------------------------
		// Buffer d'indices : puisque l'on duplique tous les sommets pour ne pas avoir de normale partagŽe, 
		// faces = { 0,1,2,3,4,5,6,7,8,9,10,... }
		for (unsigned int i=0; i<3* n_faces; i++){
			faces[i]= i;
		}
		
		/* buffers est un tableau de 2 indices qui nous permettra de rappeler le tableau de donnŽes
			(sommets, couleurs, normales, ...) et le tableau d'indices des triangles */
		glGenBuffers(2, buffers);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, (3+ 3+ 3)* 3* n_faces* sizeof(float), vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3* n_faces* sizeof(unsigned int), faces, GL_STATIC_DRAW);

		glUseProgram(prog_draw);

		position_loc     = glGetAttribLocation(prog_draw, "position_in");
		normal_loc       = glGetAttribLocation(prog_draw, "normal_in");
		diffuse_color_loc= glGetAttribLocation(prog_draw, "color_in");

		ambient_color_loc= glGetUniformLocation(prog_draw, "ambient_color");
		shininess_loc    = glGetUniformLocation(prog_draw, "shininess");
		
		model2clip_loc  = glGetUniformLocation(prog_draw, "model2clip_matrix");
		model2camera_loc= glGetUniformLocation(prog_draw, "model2camera_matrix");
		normal_mat_loc  = glGetUniformLocation(prog_draw, "normal_matrix");
		
		alpha_loc= glGetUniformLocation(prog_draw, "alpha");

		glUseProgram(0);
		
	}
	else{
		cout << "Impossible d'ouvrir le fichier obj : " << ch_obj << endl;
	}
}


void ModelObj::release()
{
	free(faces);
	free(vertices);
}


void ModelObj::draw()
{
	if (!is_active)
		return;

	glUseProgram(prog_draw);
	
	glUniformMatrix4fv(model2clip_loc  , 1, GL_FALSE, model2clip);
	glUniformMatrix4fv(model2camera_loc, 1, GL_FALSE, model2camera);
	glUniformMatrix3fv(normal_mat_loc  , 1, GL_FALSE, normal);
	glUniform3fv(ambient_color_loc, 1, current_values.ambient);
	glUniform1f(shininess_loc     , current_values.shininess);
	glUniform1f(alpha_loc, alpha);

	// On prŽcise les donnŽes que l'on souhaite utiliser
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);

	// Enables the attribute indices
	glEnableVertexAttribArray(position_loc);
	glEnableVertexAttribArray(normal_loc);
	glEnableVertexAttribArray(diffuse_color_loc);

	// Modifie les tableaux associŽs au buffer en cours d'utilisation
	glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3)* sizeof(float), 0);
	glVertexAttribPointer(normal_loc  , 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3)* sizeof(float), (void *)(3* sizeof(float)));
	glVertexAttribPointer(diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3)* sizeof(float), (void *)((3+ 3)* sizeof(float)));
	
	// On prŽcise le tableau d'indices de triangle ˆ utiliser
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);

	// Rendu de notre geometrie
	glDrawElements(GL_TRIANGLES, n_faces* 3, GL_UNSIGNED_INT, 0);

	// Disables the attribute indices
	glDisableVertexAttribArray(position_loc);
	glDisableVertexAttribArray(normal_loc);
	glDisableVertexAttribArray(diffuse_color_loc);

	glUseProgram(0);

}


void ModelObj::print(bool verbose){
	cout << "ModelObj -----------------------------" << endl;
	cout << "n_faces= " << n_faces << endl;
	cout << "n_vertices=" << sizeof(vertices)/ sizeof(float) << endl;
	if (verbose) {
		cout << "vertices= " << endl;
		for (unsigned int i=0; i< 3* n_faces; i++) {
			for (unsigned int k=0; k< 9; k++)
				cout << vertices[9* i+ k] << " ; ";
			cout << endl;
		}
		cout << "faces= " << endl;
		for (unsigned int i=0; i< n_faces; i++)
			cout << faces[3* i] << " ; " << faces[3* i+ 1] << " ; " << faces[3* i+ 2] << " ; " << endl;
	}
	cout << "is_active=" << is_active << endl;
	cout << "config->lifetime=" << config->lifetime << endl;
	cout << "-----------------------------" << endl;
}


void ModelObj::anim(float * world2camera, float * camera2clip){
	if (!is_active)
		return;
	
	unsigned int i;
	unsigned int diff_ms= diff_time_ms_from_now(&t);
	float morph= (float)(diff_ms)/ (float)(config->lifetime); // entre 0 (initial) et 1 (final)
	float morph_alpha= morph;
	if (morph> 1.0) {
		is_active= false;
		//cout << "Fin OBJ" << endl;
		return;
	}
	
	// modif de morph en fonction des morph_pos
	for (i=1; i<config->morph_pos.size(); i++)
		if (config->morph_pos[i].x> morph)
			break;
	morph= config->morph_pos[i- 1].y+ (morph- config->morph_pos[i- 1].x)* 
		(config->morph_pos[i].y- config->morph_pos[i- 1].y)/ (config->morph_pos[i].x- config->morph_pos[i- 1].x);

	for (i=1; i<config->alpha_pos.size(); i++)
		if (config->alpha_pos[i].x> morph_alpha)
			break;
	alpha= config->alpha_pos[i- 1].y+ (morph_alpha- config->alpha_pos[i- 1].x)* 
		(config->alpha_pos[i].y- config->alpha_pos[i- 1].y)/ (config->alpha_pos[i].x- config->alpha_pos[i- 1].x);

	for (i=0; i<16; i++)
		current_values.model2world[i]= (1.0- morph)* config->init_values.model2world[i]+ morph* config->final_values.model2world[i];
	for (i=0; i<3; i++)
		current_values.ambient[i]= (1.0- morph)* config->init_values.ambient[i]+ morph* config->final_values.ambient[i];

	// a faire Žvoluer
	/*for (i=0; i<3; i++)
		current_values.diffuse[i]= (1.0- morph)* config->init_values.diffuse[i]+ morph* config->final_values.diffuse[i];*/

	current_values.shininess= (1.0- morph)* config->init_values.shininess + morph* config->final_values.shininess;
	
	glm::mat4 glm_model2world= glm::make_mat4(current_values.model2world);
	glm::mat4 glm_world2camera= glm::make_mat4(world2camera);
	glm::mat4 glm_camera2clip= glm::make_mat4(camera2clip);
	glm::mat4 glm_model2camera= glm_world2camera* glm_model2world;
	glm::mat4 glm_model2clip= glm_camera2clip* glm_model2camera;
	// theoriquement il faudrait prendre la transposee de l'inverse mais si model2camera est 
	// une matrice orthogonale, TRANS(INV(M)) == M, ce qui est le cas lorsqu'elle ne comprend que 
	// des translations et rotations
	glm::mat3 glm_normal= glm::mat3(glm_model2camera);
	
	memcpy(model2clip, glm::value_ptr(glm_model2clip), sizeof(float) * 16);
	memcpy(model2camera, glm::value_ptr(glm_model2camera), sizeof(float) * 16);
	memcpy(normal, glm::value_ptr(glm_normal), sizeof(float) * 9);
}


void ModelObj::set_config(ModelObjConfig * config_) {
	config= config_;
	load(config->ch_obj, config->ch_mat);
}

