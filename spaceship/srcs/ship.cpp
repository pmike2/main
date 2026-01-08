
#include "ship.h"

using namespace std;


AppliedForce::AppliedForce() {
	
}


AppliedForce::AppliedForce(const glm::vec3 & f, const glm::vec3 & pt, std::string name) : _f(f), _pt(pt), _name(name), _is_active(false) {

}


// ---------------------------------------------------------------------------------------
ForcesDraw::ForcesDraw() {
	
}


ForcesDraw::ForcesDraw(GLuint prog_draw_basic) : _prog_draw(prog_draw_basic), _n_forces(0) {
	glm::mat4 glm_identity(1.0f);
	memcpy(_model2world, glm::value_ptr(glm_identity), sizeof(float) * 16);
	memcpy(_model2clip , glm::value_ptr(glm_identity), sizeof(float) * 16);
	
	glGenBuffers(1, &_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	_position_loc     = glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_model2clip_loc   = glGetUniformLocation(_prog_draw, "model2clip_matrix");
}


void ForcesDraw::draw() {
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glUniformMatrix4fv(_model2clip_loc, 1, GL_FALSE, glm::value_ptr(_model2clip));
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));
	
	glDrawArrays(GL_LINES, 0, _n_forces* 2);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void ForcesDraw::anim(const glm::mat4 & world2camera, const glm::mat4 & camera2clip) {
	_model2clip= camera2clip* world2camera* _model2world;
}


void ForcesDraw::sync2appliedforces(std::vector<AppliedForce> & applied_forces) {
	_n_forces= 0; // voir + tard si je veux ajouter d'autres trucs à visualiser

	for (uint i=0; i<applied_forces.size(); ++i) {
		if (!applied_forces[i]._is_active)
			continue;
		
		_data[_n_forces* 12+ 0]= applied_forces[i]._pt.x;
		_data[_n_forces* 12+ 1]= applied_forces[i]._pt.y;
		_data[_n_forces* 12+ 2]= applied_forces[i]._pt.z;
		_data[_n_forces* 12+ 3]= 1.0f;
		_data[_n_forces* 12+ 4]= 1.0f;
		_data[_n_forces* 12+ 5]= 1.0f;
		_data[_n_forces* 12+ 6]= applied_forces[i]._pt.x+ applied_forces[i]._f.x* FORCE_DRAW_MULT_FACTOR;
		_data[_n_forces* 12+ 7]= applied_forces[i]._pt.y+ applied_forces[i]._f.y* FORCE_DRAW_MULT_FACTOR;
		_data[_n_forces* 12+ 8]= applied_forces[i]._pt.z+ applied_forces[i]._f.z* FORCE_DRAW_MULT_FACTOR;
		_data[_n_forces* 12+ 9]= 0.5f;
		_data[_n_forces* 12+ 10]=0.5f;
		_data[_n_forces* 12+ 11]=0.5f;
		
		_n_forces++;
	}
	
	// mise à jour du buffer
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)* 12* _n_forces, _data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}


// ---------------------------------------------------------------------------------------
RigidBody::RigidBody() {
	
}


RigidBody::RigidBody(string model_path, float size_factor) :
	_linear_momentum(glm::vec3(0.0f)), _linear_v(glm::vec3(0.0f)), _position(glm::vec3(0.0f)),
	_angular_momentum(glm::vec3(0.0f)), _angular_v(glm::vec3(0.0f)), _rotation_matrix(glm::mat3(1.0f)), _quaternion(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
	_local_inertia_matrix(glm::mat3(1.0f)), _local_inertia_matrix_inverse(glm::mat3(1.0f)), _world_inertia_matrix_inverse(glm::mat3(1.0f)),
	_xmin(1e8), _xmax(-1e8), _ymin(1e8), _ymax(-1e8), _zmin(1e8), _zmax(-1e8)
{
	from_txt(model_path, size_factor);
	
	/*
	cout << model_path << endl;
	cout << "_mass=" << _mass << endl;
	cout << "_mass_center=" << _mass_center.x << ";"  << _mass_center.y << ";"  << _mass_center.z << endl;
	cout << "_local_inertia_matrix=" << endl;
	for (uint i=0; i<3; ++i) {
		for (uint j=0; j<3; ++j)
			cout << _local_inertia_matrix[i][j] << " ; ";
		cout << endl;
	}
	*/
	
	// on remet dans les coordonnées barycentre
	/*_xmax-= _mass_center.x; _xmin-= _mass_center.x;
	_ymax-= _mass_center.y; _ymin-= _mass_center.y;
	_zmax-= _mass_center.z; _zmin-= _mass_center.z;*/
	
	// calcul rayon ----------------------------------------------------------------------
	_radius= abs(_xmin);
	if (abs(_xmax)> _radius) _radius= abs(_xmax);
	if (abs(_ymin)> _radius) _radius= abs(_ymin);
	if (abs(_ymax)> _radius) _radius= abs(_ymax);
	if (abs(_zmin)> _radius) _radius= abs(_zmin);
	if (abs(_zmax)> _radius) _radius= abs(_zmax);
	
	// inverse matrice d'inertie
	_local_inertia_matrix_inverse= glm::inverse(_local_inertia_matrix);
	// dans repere monde
	_world_inertia_matrix_inverse= _local_inertia_matrix_inverse;
}


// calcul matrice inertie, bary d'un corps défini comme un assemblage de pavés droits
// utilisé au début, plus maintenant. Conservé pour mémoire
/*void RigidBody::as_paves(string model_path, float size_factor) {
	string line;
	float x, y, z;
	float volume, mass;
	vector<float> masses;
	glm::vec3 bary= glm::vec3(0.f);
	glm::mat3 inertia= glm::mat3(0.f);
	
	vector<glm::vec3> barys;
	vector<glm::mat3> inertias;
	
	float xmin, xmax, ymin, ymax, zmin, zmax;
	xmin= 1e10; xmax= -1e10;
	ymin= 1e10; ymax= -1e10;
	zmin= 1e10; zmax= -1e10;
	
	ifstream obj_file(model_path.c_str());
	if (obj_file.is_open()) {
		obj_file.seekg(0, ios::beg);
		while (true) {
			getline(obj_file, line);
			
			if (line.c_str()[0]== 'v') {
				line[0]= ' ';
				sscanf(line.c_str(), "%f%f%f ", &x, &y, &z);
				
				x*= size_factor;
				y*= size_factor;
				z*= size_factor;
				
				if (x< xmin) xmin= x; if (x> xmax) xmax= x;
				if (y< ymin) ymin= y; if (y> ymax) ymax= y;
				if (z< zmin) zmin= z; if (z> zmax) zmax= z;
			}

			if ((line.c_str()[0]== 'o') || (obj_file.eof())) {
				if (xmin> xmax)
					continue;
				
				// ici on suppose que chaque composant du ship est un parallélépipède rectangle
				volume= (xmax- xmin)* (ymax- ymin)* (zmax- zmin);
				mass= volume* SHIP_DENSITY;
				bary= glm::vec3((xmin+ xmax)* 0.5f, (ymin+ ymax)* 0.5f, (zmin+ zmax)* 0.5f);
				inertia= glm::mat3(
					mass* ((ymax- ymin)* (ymax- ymin)+ (zmax- zmin)* (zmax- zmin))/ 12.0f, 0.0f, 0.0f, 0.0f,
					mass* ((xmax- xmin)* (xmax- xmin)+ (zmax- zmin)* (zmax- zmin))/ 12.0f, 0.0f, 0.0f, 0.0f,
					mass* ((xmax- xmin)* (xmax- xmin)+ (ymax- ymin)* (ymax- ymin))/ 12.0f
				);
				
				masses.push_back(mass);
				barys.push_back(bary);
				inertias.push_back(inertia);
				
				if (xmin< _xmin) _xmin= xmin; if (xmax> _xmax) _xmax= xmax;
				if (ymin< _ymin) _ymin= ymin; if (ymax> _ymax) _ymax= ymax;
				if (zmin< _zmin) _zmin= zmin; if (zmax> _zmax) _zmax= zmax;
				
				xmin= 1e10; xmax= -1e10;
				ymin= 1e10; ymax= -1e10;
				zmin= 1e10; zmax= -1e10;
			}
			
			if (obj_file.eof())
				break;
		}
	}
	else {
		cout << "Impossible d'ouvrir le fichier obj : " << model_path << endl;
		return;
	}
	
	// calcul masse et barycentre --------------------------------------------------------
	for (uint i=0; i<masses.size(); ++i) {
		_mass+= masses[i];
		_mass_center.x+= masses[i]* barys[i].x;
		_mass_center.y+= masses[i]* barys[i].y;
		_mass_center.z+= masses[i]* barys[i].z;
	}
	
	_mass_center/= _mass;
	
	// calcul matrice d'inertie -------------------------------------------
	// cf https://en.wikipedia.org/wiki/Parallel_axis_theorem
	for (uint i=0; i<masses.size(); ++i) {
		float bary_norm2= barys[i].x* barys[i].x+ barys[i].y* barys[i].y+ barys[i].z* barys[i].z;
		glm::mat3 mat1= glm::mat3(bary_norm2, 0.f, 0.f, 0.f, bary_norm2, 0.f, 0.f, 0.f, bary_norm2);
		glm::mat3 mat2= glm::outerProduct(barys[i], barys[i]);
		_local_inertia_matrix+= inertias[i]+ masses[i]* (mat1- mat2);
	}
}*/


// lecture matrice d'inertie, bary à partir d'un fichier txt généré par .../spaceship/modeles/compute_rigidbody.py
void RigidBody::from_txt(string model_path, float size_factor) {
	
	string line;
	float x1, y1, z1, x2, y2, z2, x3, y3, z3;
	float volume, mass;
	
	// parcours txtfile ------------------------------------------------------------------
	string txt_path= model_path.substr(0, model_path.length()- 4)+ ".txt";
	ifstream txt_file(txt_path.c_str());
	if (txt_file.is_open()) {
		txt_file.seekg(0, ios::beg);
		while (true) {
			getline(txt_file, line);
			if (line.c_str()[0]== 'I') {
				line[0]= ' ';
				
				sscanf(line.c_str(), "%f%f%f%f%f%f%f%f%f ", &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3);

				_local_inertia_matrix[0][0]= x1;
				_local_inertia_matrix[0][1]= y1;
				_local_inertia_matrix[0][2]= z1;
				_local_inertia_matrix[1][0]= x2;
				_local_inertia_matrix[1][1]= y2;
				_local_inertia_matrix[1][2]= z2;
				_local_inertia_matrix[2][0]= x3;
				_local_inertia_matrix[2][1]= y3;
				_local_inertia_matrix[2][2]= z3;
				
				/*_local_inertia_matrix[0][0]= 1.0;
				_local_inertia_matrix[0][1]= 0.0;
				_local_inertia_matrix[0][2]= 0.0;

				_local_inertia_matrix[1][0]= 0.0;
				_local_inertia_matrix[1][1]= 1.0;
				_local_inertia_matrix[1][2]= 0.0;

				_local_inertia_matrix[2][0]= 0.0;
				_local_inertia_matrix[2][1]= 0.0;
				_local_inertia_matrix[2][2]= 1.0;*/

			}
			else if (line.c_str()[0]== 'V') {
				line[0]= ' ';
				sscanf(line.c_str(), "%f ", &volume);
				
				_mass= volume* size_factor* size_factor* size_factor* SHIP_DENSITY;
			}
			else if (line.c_str()[0]== 'B') {
				line[0]= ' ';
				sscanf(line.c_str(), "%f%f%f ", &x1, &y1, &z1);
				
				/*_mass_center.x= x1* size_factor;
				_mass_center.y= y1* size_factor;
				_mass_center.z= z1* size_factor;*/
				
				// je ne sais pas pourquoi mais sinon le mouvmt fwd/bwd fait nawak
				_mass_center.x= 0.0f;
				_mass_center.y= 0.0f;
				_mass_center.z= 0.0f;
			}

			if (txt_file.eof())
				break;
		}
		
	}
	else {
		cout << "Impossible d'ouvrir le fichier txt : " << txt_path << endl;
		return;
	}

	// parcours objfile ------------------------------------------------------------------
	ifstream obj_file(model_path.c_str());
	if (obj_file.is_open()) {
		obj_file.seekg(0, ios::beg);
		while (true) {
			getline(obj_file, line);
			
			if (line.c_str()[0]== 'v') {
				// Set first character to 0. This will allow us to use sscanf
				line[0]= ' ';
				sscanf(line.c_str(), "%f%f%f ", &x1, &y1, &z1);
				
				x1*= size_factor;
				y1*= size_factor;
				z1*= size_factor;
				
				if (x1< _xmin) _xmin= x1; if (x1> _xmax) _xmax= x1;
				if (y1< _ymin) _ymin= y1; if (y1> _ymax) _ymax= y1;
				if (z1< _zmin) _zmin= z1; if (z1> _zmax) _zmax= z1;
			}
			if (obj_file.eof())
				break;
		}
	}
	else {
		cout << "Impossible d'ouvrir le fichier obj : " << model_path << endl;
		return;
	}
}


void RigidBody::anim(vector<AppliedForce> & applied_forces) {

	// calcul force et torque
	glm::vec3 force(0.0f);
	glm::vec3 torque(0.0f);
	
	force+= _rotation_matrix* glm::vec3(0.0f, NOMINAL_SPEED, 0.0f);
	
	for (uint i=0; i<applied_forces.size(); ++i) {
		if (!applied_forces[i]._is_active)
			continue;
		
		// les _applied_forces sont spécifiées dans le repère ship ; il faut donc leur appliquer la rotation
		force+= _rotation_matrix* applied_forces[i]._f;
		//torque+= glm::cross(_rotation_matrix* applied_forces[i]._pt, _rotation_matrix* applied_forces[i]._f);
		torque+= glm::cross(_rotation_matrix* applied_forces[i]._pt- _mass_center, _rotation_matrix* applied_forces[i]._f);
	}
	
	// drag = résistance à l'air ; proportionnel à v * v ou à v lorsque v est assez faible et qu'il n'y a pas de turbulence ; tester...
	float linear_v_amp= glm::length(_linear_v);
	if (linear_v_amp> LINEAR_DRAG_TRESH) {
		//force-= LINEAR_DRAG_COEFF* linear_v_amp* linear_v_amp* glm::normalize(_linear_v);
		force-= LINEAR_DRAG_COEFF* linear_v_amp* glm::normalize(_linear_v);
	}
	// équivalent du drag linéaire ; est-ce valide ?
	float angular_v_amp= glm::length(_angular_v);
	if (angular_v_amp> ANGULAR_DRAG_TRESH) {
		torque-= ANGULAR_DRAG_COEFF* angular_v_amp* glm::normalize(_angular_v);
	}
	
	// calcul position
	_linear_momentum+= ANIM_STEP* force;
	_linear_v= _linear_momentum/ _mass;
	_position+= ANIM_STEP* _linear_v;
	
	// calcul rotation
	_angular_momentum+= ANIM_STEP* torque;
	_angular_v= _world_inertia_matrix_inverse* _angular_momentum;
	glm::quat q(0.0f, _angular_v);
	_quaternion+= 0.5f* q* _quaternion;
	_quaternion= glm::normalize(_quaternion);
	_rotation_matrix= glm::mat3_cast(_quaternion);
	
	// mise à jour du moment d'inertie dans le repère monde
	_world_inertia_matrix_inverse= _rotation_matrix* _local_inertia_matrix_inverse* glm::transpose(_rotation_matrix);
}


// ---------------------------------------------------------------------------------------
bool out_of_bound(const glm::vec3 & position) {
	if (
		(position.x< -WORLD_SIZE) || (position.x> WORLD_SIZE) ||
		(position.y< -WORLD_SIZE) || (position.y> WORLD_SIZE) ||
		(position.z< -WORLD_SIZE) || (position.z> WORLD_SIZE)
	) {
		return true;
	}
	return false;
}


// ---------------------------------------------------------------------------------------
FollowCamera::FollowCamera() {

}


FollowCamera::FollowCamera(GLuint prog_draw_basic) :
	_prog_draw(prog_draw_basic), _position(glm::vec3(0.0f, -FOLLOW_CAMERA_DISTANCE, 0.0f)),
	_rotation_matrix(glm::mat3(1.0f)), _quaternion(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)), _model2world(glm::mat4(1.0f)), _model2clip(glm::mat4(1.0f))
{
	
	// repere simple RGB
	float data[]= {
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
	};
	for (uint i=0; i<36; i++)
		_data[i]= data[i];
	
	glGenBuffers(1, &_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_data), _data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	_position_loc     = glGetAttribLocation(_prog_draw, "position_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_model2clip_loc   = glGetUniformLocation(_prog_draw, "model2clip_matrix");
}


void FollowCamera::draw() {
	glUseProgram(_prog_draw);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	glUniformMatrix4fv(_model2clip_loc, 1, GL_FALSE, glm::value_ptr(_model2clip));
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)0);
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, 6* sizeof(float), (void*)(3* sizeof(float)));
	
	glDrawArrays(GL_LINES, 0, 6);

	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void FollowCamera::anim(const glm::mat4 & world2camera, const glm::mat4 & camera2clip, RigidBody & rigid_body) {
	// la camera converge vers la position idéale (derrière le ship à distance FOLLOW_CAMERA_DISTANCE), avec l'orientation du ship
	
	glm::vec3 target(0.0f, -FOLLOW_CAMERA_DISTANCE, 0.0f);
	target= rigid_body._rotation_matrix* target;
	target+= rigid_body._position;
	_position+= (target- _position)* FOLLOW_LINEAR_FACTOR;
	
	// cf https://en.wikipedia.org/wiki/Slerp
	_quaternion= glm::slerp(_quaternion, rigid_body._quaternion, FOLLOW_ANGULAR_FACTOR);
	_quaternion= glm::normalize(_quaternion); // nécessaire ?
	_rotation_matrix= glm::mat3_cast(_quaternion);
	
	glm::mat4 rotation= glm::mat4(_rotation_matrix);
	glm::mat4 translation= glm::translate(glm::mat4(1.0f), _position);
	_model2world= translation* rotation;
	_model2clip= camera2clip* world2camera* _model2world;
}


// ---------------------------------------------------------------------------------------
Bullet::Bullet() {

}


Bullet::Bullet(GLuint prog_draw, GLuint prog_draw_basic, StaticModel * model, const glm::vec3 & position, const glm::mat3 & rotation_matrix, float size_factor, const glm::vec3 & color) :
	_position(position), _rotation_matrix(rotation_matrix), _is_active(false)
{
	_instance= new StaticInstance(model, glm::vec3(size_factor));
}


Bullet::~Bullet() {
	_model.release();
}


void Bullet::draw() {
	if (!_is_active)
		return;

	_model.draw();
}


void Bullet::anim(float * world2camera, float * camera2clip) {
	if (!_is_active)
		return;

	glm::vec3 vec_advance= _rotation_matrix* glm::vec3(0.0f, 1.0f, 0.0f);
	
	_position+= BULLET_SPEED* vec_advance;

	_instance->set_pos_rot_scale(_position, );
	
	glm::mat4 rotation= glm::mat4(_rotation_matrix);
	glm::mat4 translation= glm::translate(glm::mat4(1.0f), _position);
	glm::mat4 glm_model2world= translation* rotation;
	memcpy(_model._model2world, glm::value_ptr(glm_model2world), sizeof(float) * 16);
	
	_model.anim(world2camera, camera2clip);
}


// ---------------------------------------------------------------------------------------
void ExplosionTransfo::reinit(float translation) {
	_init_translation= glm::vec3(rand_float(-translation, translation),
		rand_float(-translation, translation), rand_float(-translation, translation));
	_translation= glm::vec3(0.0f);
	_rotation= glm::vec3(rand_float(-1.0f, 1.0f), rand_float(-1.0f, 1.0f), rand_float(-1.0f, 1.0f));
	_angle= 0.0f;
	_scale= 1.0f;
}


// ---------------------------------------------------------------------------------------
Explosion::Explosion() {
	
}


Explosion::Explosion(GLuint prog_draw, GLuint prog_draw_basic, std::string model_path, std::string material_path, glm::vec3 position, const ExplosionParams &ep) :
	_position(position), _n_faces(0), _prog_draw(prog_draw), _alpha(1.0f), _is_active(false), _ep(ep)
{
	ModelObj model= ModelObj(prog_draw, prog_draw_basic);
	model.load(model_path, material_path, _ep._size_factor);
	
	_n_faces= model._n_faces;
	_vertices= (float*) malloc((3+ 3+ 3)* 3* _n_faces* sizeof(float));
	_faces= (uint*) malloc(3* _n_faces* sizeof(uint));
	memcpy(_vertices, model._vertices, (3+ 3+ 3)* 3* _n_faces* sizeof(float));
	memcpy(_faces, model._faces, 3* _n_faces* sizeof(uint));
	
	_shininess= model._shininess;
	memcpy(_ambient, model._ambient, 3* sizeof(float));
	
	_model_matrices= new glm::mat4[_ep._n_particles];
	
	_transfo= new ExplosionTransfo[_ep._n_particles];
	
	for (uint i=0; i<_ep._n_particles; ++i) {
		_transfo[i].reinit(_ep._translation);
	}

	for (uint i=0; i<_ep._n_particles; ++i) {
		_model_matrices[i]= glm::translate(glm::mat4(1.0f), _position);
	}
    
	glGenBuffers(3, _buffers);
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, (3+ 3+ 3)* 3* _n_faces* sizeof(float), _vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3* _n_faces* sizeof(uint), _faces, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, _buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, _ep._n_particles * sizeof(glm::mat4), &_model_matrices[0], GL_STATIC_DRAW);

	glUseProgram(_prog_draw);

	_position_loc     = glGetAttribLocation(_prog_draw, "position_in");
	_normal_loc       = glGetAttribLocation(_prog_draw, "normal_in");
	_diffuse_color_loc= glGetAttribLocation(_prog_draw, "color_in");
	_instanced_matrix_loc= glGetAttribLocation(_prog_draw, "instanced_matrix");

	_ambient_color_loc= glGetUniformLocation(_prog_draw, "ambient_color");
	_shininess_loc    = glGetUniformLocation(_prog_draw, "shininess");
	_alpha_loc        = glGetUniformLocation(_prog_draw, "alpha");
	
	_world2clip_loc  = glGetUniformLocation(_prog_draw, "world2clip_matrix");
	_world2camera_loc= glGetUniformLocation(_prog_draw, "world2camera_matrix");
	
	glUseProgram(0);

}


void Explosion::release() {
	free(_faces);
	free(_vertices);
	delete _model_matrices;
	delete _transfo;
}


void Explosion::draw() {
	if (!_is_active)
		return;
	
	glUseProgram(_prog_draw);
	// On précise les données que l'on souhaite utiliser
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
	// On précise le tableau d'indices de triangle à utiliser
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[1]);
	
	glUniformMatrix4fv(_world2clip_loc  , 1, GL_FALSE, _world2clip);
	glUniformMatrix4fv(_world2camera_loc, 1, GL_FALSE, _world2camera);
	glUniform3fv(_ambient_color_loc, 1, _ambient);
	glUniform1f(_shininess_loc     , _shininess);
	glUniform1f(_alpha_loc, _alpha);

	// Enables the attribute indices
	glEnableVertexAttribArray(_position_loc);
	glEnableVertexAttribArray(_normal_loc);
	glEnableVertexAttribArray(_diffuse_color_loc);

	// Modifie les tableaux associés au buffer en cours d'utilisation
	glVertexAttribPointer(_position_loc, 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3)* sizeof(float), 0);
	glVertexAttribPointer(_normal_loc  , 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3)* sizeof(float), (void *)(3* sizeof(float)));
	glVertexAttribPointer(_diffuse_color_loc, 3, GL_FLOAT, GL_FALSE, (3+ 3+ 3)* sizeof(float), (void *)((3+ 3)* sizeof(float)));
	
	// _buffers[2] contient les matrices de chaque 'particule'
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[2]);
	for (uint j=0; j<4 ; ++j) {
		glEnableVertexAttribArray(_instanced_matrix_loc+ j);
		glVertexAttribPointer(_instanced_matrix_loc+ j, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const GLvoid*)(sizeof(glm::vec4)* j));
		glVertexAttribDivisor(_instanced_matrix_loc+ j, 1); // pour faire de l'instanced
	}

	// Rendu de notre geometrie
	glDrawElementsInstanced(GL_TRIANGLES, _n_faces* 3, GL_UNSIGNED_INT, 0, _ep._n_particles);

	// Disables the attribute indices
	glDisableVertexAttribArray(_position_loc);
	glDisableVertexAttribArray(_normal_loc);
	glDisableVertexAttribArray(_diffuse_color_loc);
	for (uint j=0; j<4 ; ++j) {
		glVertexAttribDivisor(_instanced_matrix_loc+ j, 0);
		glDisableVertexAttribArray(_instanced_matrix_loc+ j);
	}
	
	// on réinit à 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}


void Explosion::anim(float * world2camera, float * camera2clip){
	if (!_is_active)
		return;

	for (uint i=0; i<_ep._n_particles; ++i) {
		_transfo[i]._translation+= _transfo[i]._init_translation;
		_transfo[i]._angle+= _ep._angle;
		_transfo[i]._scale-= _ep._scale;
		if (_transfo[i]._scale< 0.0f) {
			_is_active= false;
		}
	}
	for (uint i=0; i<_ep._n_particles; ++i) {
		_model_matrices[i]= glm::translate(glm::mat4(1.0f), _position+ _transfo[i]._translation)* glm::rotate(glm::mat4(1.0f), _transfo[i]._angle, _transfo[i]._rotation)* glm::scale(glm::mat4(1.0f), glm::vec3(_transfo[i]._scale, _transfo[i]._scale, _transfo[i]._scale));
	}
	glBindBuffer(GL_ARRAY_BUFFER, _buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, _ep._n_particles * sizeof(glm::mat4), &_model_matrices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glm::mat4 glm_world2camera= glm::make_mat4(world2camera);
	glm::mat4 glm_camera2clip= glm::make_mat4(camera2clip);
	glm::mat4 glm_world2clip= glm_camera2clip* glm_world2camera;
	
	memcpy(_world2clip, glm::value_ptr(glm_world2clip), sizeof(float) * 16);
	memcpy(_world2camera, glm::value_ptr(glm_world2camera), sizeof(float) * 16);
}


void Explosion::reinit() {
	for (uint i=0; i<_ep._n_particles; ++i) {
		_transfo[i].reinit(_ep._translation);
	}
}


// ---------------------------------------------------------------------------------------
void new_explosion(std::vector<Explosion *> & explosions, const glm::vec3 & position, const glm::vec3 & color) {
	for (auto it_explosion : explosions) {
		if (!it_explosion->_is_active) {
			it_explosion->_is_active= true;
			it_explosion->reinit();
			it_explosion->_position= position;
			it_explosion->_ambient[0]= color.x;
			it_explosion->_ambient[1]= color.y;
			it_explosion->_ambient[2]= color.z;
			//cout << "explosion !" << endl;
			break;
		}
	}
}


// ---------------------------------------------------------------------------------------
Ship::Ship() {
	
}


Ship::Ship(std::string id, GLuint prog_draw_3d, GLuint prog_draw_basic, StaticModel * model_ship, StaticModel * model_bullet, bool is_paves, float size_factor, const glm::vec3 & color) :
	_id(id), _is_shooting(false), _tik_shooting_1(0), _tik_shooting_2(0), _color(color)
{
	_instance= StaticInstance(model_ship, glm::vec3(1.0f));
	
	_forces_draw= ForcesDraw(prog_draw_basic);
	_rigid_body= RigidBody(model_ship->_ch_config_file, size_factor);
	_follow_camera= FollowCamera(prog_draw_basic);

	init_applied_forces();
	
	_keypresseds.insert(pair<string, bool>(KEY_UP, false));
	_keypresseds.insert(pair<string, bool>(KEY_DOWN, false));
	_keypresseds.insert(pair<string, bool>(KEY_LEFT, false));
	_keypresseds.insert(pair<string, bool>(KEY_RIGHT, false));
	_keypresseds.insert(pair<string, bool>(KEY_FWD, false));
	_keypresseds.insert(pair<string, bool>(KEY_BWD, false));
	
	for (uint i=0; i<MAX_BULLETS; ++i)
		_bullets.push_back(new Bullet(_model._prog_draw, _model._prog_bbox, model_bullet, glm::vec3(0.0f), glm::mat3(1.0f), BULLET_SIZE_FACTOR, color));
}


// init des forces possibles à appliquer au ship
void Ship::init_applied_forces() {
	
	// points d'application des forces dans le repère ship
	// ship doit pointer vers y positif
	const glm::vec3 LEFT_ENGINE_PT = glm::vec3(-_rigid_body._xmax, 0.0f, 0.0f)+ _rigid_body._mass_center;
	const glm::vec3 RIGHT_ENGINE_PT= glm::vec3(_rigid_body._xmax, 0.0f, 0.0f) + _rigid_body._mass_center;
	const glm::vec3 BACK_PT        = glm::vec3(0.0f, _rigid_body._ymin, 0.0f) + _rigid_body._mass_center;
	
	glm::vec3 left_engine_up_pt= LEFT_ENGINE_PT;
	glm::vec3 left_engine_up_force= glm::vec3(0.0f, 0.0f, LEFT_RIGHT_INTENSITY);

	glm::vec3 left_engine_down_pt= LEFT_ENGINE_PT;
	glm::vec3 left_engine_down_force= glm::vec3(0.0f, 0.0f, -LEFT_RIGHT_INTENSITY);

	glm::vec3 left_engine_fwd_pt= LEFT_ENGINE_PT;
	glm::vec3 left_engine_fwd_force= glm::vec3(0.0f, FWD_BWD_INTENSITY, 0.0f);

	glm::vec3 left_engine_bwd_pt= LEFT_ENGINE_PT;
	glm::vec3 left_engine_bwd_force= glm::vec3(0.0f, -FWD_BWD_INTENSITY, 0.0f);
	
	glm::vec3 right_engine_up_pt= RIGHT_ENGINE_PT;
	glm::vec3 right_engine_up_force= glm::vec3(0.0f, 0.0f, LEFT_RIGHT_INTENSITY);

	glm::vec3 right_engine_down_pt= RIGHT_ENGINE_PT;
	glm::vec3 right_engine_down_force= glm::vec3(0.0f, 0.0f, -LEFT_RIGHT_INTENSITY);

	glm::vec3 right_engine_fwd_pt= RIGHT_ENGINE_PT;
	glm::vec3 right_engine_fwd_force= glm::vec3(0.0f, FWD_BWD_INTENSITY, 0.0f);

	glm::vec3 right_engine_bwd_pt= RIGHT_ENGINE_PT;
	glm::vec3 right_engine_bwd_force= glm::vec3(0.0f, -FWD_BWD_INTENSITY, 0.0f);

	glm::vec3 back_up_pt= BACK_PT;
	glm::vec3 back_up_force= glm::vec3(0.0f, 0.0f, UP_DOWN_INTENSITY);

	glm::vec3 back_down_pt= BACK_PT;
	glm::vec3 back_down_force= glm::vec3(0.0f, 0.0f, -UP_DOWN_INTENSITY);

	_applied_forces.push_back(AppliedForce(left_engine_up_force  , left_engine_up_pt  , LEFT_ENGINE_UP));
	_applied_forces.push_back(AppliedForce(left_engine_down_force, left_engine_down_pt, LEFT_ENGINE_DOWN));
	_applied_forces.push_back(AppliedForce(left_engine_fwd_force , left_engine_fwd_pt , LEFT_ENGINE_FWD));
	_applied_forces.push_back(AppliedForce(left_engine_bwd_force , left_engine_bwd_pt , LEFT_ENGINE_BWD));

	_applied_forces.push_back(AppliedForce(right_engine_up_force  , right_engine_up_pt  , RIGHT_ENGINE_UP));
	_applied_forces.push_back(AppliedForce(right_engine_down_force, right_engine_down_pt, RIGHT_ENGINE_DOWN));
	_applied_forces.push_back(AppliedForce(right_engine_fwd_force , right_engine_fwd_pt , RIGHT_ENGINE_FWD));
	_applied_forces.push_back(AppliedForce(right_engine_bwd_force , right_engine_bwd_pt , RIGHT_ENGINE_BWD));

	_applied_forces.push_back(AppliedForce(back_up_force  , back_up_pt  , BACK_UP));
	_applied_forces.push_back(AppliedForce(back_down_force, back_down_pt, BACK_DOWN));
	
	vector<string> vec;
	
	vec.clear();
	vec.push_back(LEFT_ENGINE_DOWN);
	vec.push_back(RIGHT_ENGINE_DOWN);
	vec.push_back(BACK_UP);
	_key2forces.insert(pair<string, vector<string> >(KEY_UP, vec));

	vec.clear();
	vec.push_back(LEFT_ENGINE_UP);
	vec.push_back(RIGHT_ENGINE_UP);
	vec.push_back(BACK_DOWN);
	_key2forces.insert(pair<string, vector<string> >(KEY_DOWN, vec));

	vec.clear();
	vec.push_back(LEFT_ENGINE_DOWN);
	vec.push_back(RIGHT_ENGINE_UP);
	_key2forces.insert(pair<string, vector<string> >(KEY_LEFT, vec));

	vec.clear();
	vec.push_back(LEFT_ENGINE_UP);
	vec.push_back(RIGHT_ENGINE_DOWN);
	_key2forces.insert(pair<string, vector<string> >(KEY_RIGHT, vec));

	vec.clear();
	vec.push_back(LEFT_ENGINE_FWD);
	vec.push_back(RIGHT_ENGINE_FWD);
	_key2forces.insert(pair<string, vector<string> >(KEY_FWD, vec));

	vec.clear();
	vec.push_back(LEFT_ENGINE_BWD);
	vec.push_back(RIGHT_ENGINE_BWD);
	_key2forces.insert(pair<string, vector<string> >(KEY_BWD, vec));
}


Ship::~Ship() {
	_model.release();
	for (auto it_bullet : _bullets)
		delete it_bullet;
}


void Ship::draw(bool force_draw, bool draw_follow_camera) {
	_model.draw();
	
	if (force_draw)
		_forces_draw.draw();
	
	if (draw_follow_camera)
		_follow_camera.draw();

	for (auto it_bullet : _bullets)
		it_bullet->draw();
}


void Ship::anim(float * world2camera, float * camera2clip) {
	// on applique la rotation puis la translation à model2world (du modele et du dessin des forces)
	glm::mat4 rotation= glm::mat4(_rigid_body._rotation_matrix);
	glm::mat4 translation= glm::translate(glm::mat4(1.0f), _rigid_body._position);
	glm::mat4 glm_model2world= translation* rotation;
	memcpy(_model._model2world, glm::value_ptr(glm_model2world), sizeof(float) * 16);
	memcpy(_forces_draw._model2world, glm::value_ptr(glm_model2world), sizeof(float) * 16);
	
	_model.anim(world2camera, camera2clip);
	_forces_draw.anim(world2camera, camera2clip);
	_rigid_body.anim(_applied_forces);
	_follow_camera.anim(world2camera, camera2clip, _rigid_body);
	
	if (_is_shooting) {
		_tik_shooting_2= SDL_GetTicks();
		if (_tik_shooting_2- _tik_shooting_1> BULLET_FREQUENCY) {
			
			for (auto it_bullet : _bullets) {
				if (!it_bullet->_is_active) {
					it_bullet->_is_active= true;
					it_bullet->_position= _rigid_body._position;
					it_bullet->_rotation_matrix= _rigid_body._rotation_matrix;
					break;
				}
			}
			_tik_shooting_1= _tik_shooting_2;
		}
	}
	
	for (auto it_bullet : _bullets)
		it_bullet->anim(world2camera, camera2clip);
}


void Ship::update_keys_pressed() {
	const Uint8 *state= SDL_GetKeyboardState(NULL);
	
	_keypresseds[KEY_LEFT] = state[SDL_SCANCODE_LEFT];
	_keypresseds[KEY_RIGHT]= state[SDL_SCANCODE_RIGHT];
	_keypresseds[KEY_UP]   = state[SDL_SCANCODE_UP];
	_keypresseds[KEY_DOWN] = state[SDL_SCANCODE_DOWN];
	_keypresseds[KEY_FWD]  = state[SDL_SCANCODE_Q]; // scancodes en qwerty ; cf https://wiki.libsdl.org/CategoryKeyboard
	_keypresseds[KEY_BWD]  = state[SDL_SCANCODE_W];
	
	/*cout << "------------\n";
	for (auto &it_key : _keypresseds)
		cout << it_key.first << ";" << it_key.second << endl;*/
	
	sync_keys2forces();
}


void Ship::sync_keys2forces() {

	for (auto &it_force : _applied_forces) {
		it_force._is_active= false;
	}
	
	for (auto &it_key : _keypresseds) {
		if (it_key.second) {
			for (auto &it_nom_force : _key2forces[it_key.first]) {
				for (auto &it_force : _applied_forces) {
					if (it_force._name== it_nom_force) {
						it_force._is_active= true;
						break;
					}
				}
			}
		}
	}
	
	_forces_draw.sync2appliedforces(_applied_forces);
}


void Ship::reinit() {
	_rigid_body._position.x= rand_float(-WORLD_SIZE* 0.8f, WORLD_SIZE* 0.8f);
	_rigid_body._position.y= rand_float(-WORLD_SIZE* 0.8f, WORLD_SIZE* 0.8f);
	_rigid_body._position.z= rand_float(WORLD_SIZE* 0.4f, WORLD_SIZE* 0.7f);
	
	_rigid_body._linear_momentum= glm::vec3(0.0f);
	_rigid_body._linear_v= glm::vec3(0.0f);
	_rigid_body._angular_momentum= glm::vec3(0.0f);
	_rigid_body._angular_v= glm::vec3(0.0f);
	_rigid_body._rotation_matrix= glm::mat3(1.0f);
	_rigid_body._world_inertia_matrix_inverse= glm::mat3(1.0f);
	_rigid_body._quaternion= glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}


void Ship::collision(RandTerrain * level, vector<Ship*> &ships, vector<Explosion*> &little_explosions, vector<Explosion*> &big_explosions, Ranking* ranking, GlobalMsg* global_msg) {
	stringstream ss;
	
	/*tikcollision2= SDL_GetTicks();
	if (tikcollision2- tikcollision1< 1)
		return;

	tikcollision1= SDL_GetTicks();*/
	
	// -----------------------------------------------------------------------------------
	if (level->intersects_bbox(_rigid_body._position, _rigid_body._rotation_matrix, _model._bbox)) {
		new_explosion(big_explosions, _rigid_body._position, _color);
		reinit();
		// en priorité on affiche si le 1er a changé
		if (ranking->loose(_id)) {
			ss << ranking->id_first() << " IS FIRST !";
		}
		else {
			ss << _id << " HIT THE GROUND";
		}
		global_msg->new_msg(ss.str(), _color);
	}

	if (out_of_bound(_rigid_body._position)) {
		new_explosion(big_explosions, _rigid_body._position, _color);
		reinit();
		if (ranking->loose(_id)) {
			ss << ranking->id_first() << " IS FIRST !";
		}
		else {
			ss << _id << " HIT THE LIMIT";
		}
		global_msg->new_msg(ss.str(), _color);
	}

	for (auto it_bullet : _bullets) {
		if (!it_bullet->_is_active)
			continue;
			
		if (level->intersects_bbox(it_bullet->_position, it_bullet->_rotation_matrix, it_bullet->_model._bbox)) {
			//cout << _id << " bullet contact sol" << endl;
			it_bullet->_is_active= false;
			new_explosion(little_explosions, it_bullet->_position, _color);
		}

		if (out_of_bound(it_bullet->_position)) {
			//cout << _id << " bullet contact box" << endl;
			it_bullet->_is_active= false;
			//new_explosion(little_explosions, it_bullet->_position, _color);
		}
	}
	
	// -----------------------------------------------------------------------------------
	for (auto &it_ship : ships) {
		
		if (it_ship->_id== _id)
			continue;

		if (bbox_intersects(& _model._bbox, _rigid_body._rotation_matrix, _rigid_body._position, 
			& it_ship->_model._bbox, it_ship->_rigid_body._rotation_matrix, it_ship->_rigid_body._position)) {
			new_explosion(big_explosions, _rigid_body._position, _color);
			new_explosion(big_explosions, it_ship->_rigid_body._position, it_ship->_color);
			reinit();
			it_ship->reinit();
			bool first_changed_1= ranking->loose(_id);
			bool first_changed_2= ranking->loose(it_ship->_id);
			if ((first_changed_1) || (first_changed_2)) {
				ss << ranking->id_first() << " IS FIRST !";
			}
			else {
				ss << _id << " COLLIDED WITH " << it_ship->_id;
			}
			global_msg->new_msg(ss.str(), _color);
		}
	
		for (auto it_bullet : it_ship->_bullets) {
			if (!it_bullet->_is_active)
				continue;
			
			if (bbox_intersects(& _model._bbox, _rigid_body._rotation_matrix, _rigid_body._position, 
				& it_bullet->_model._bbox, it_bullet->_rotation_matrix, it_bullet->_position)) {
				it_bullet->_is_active= false;
				new_explosion(big_explosions, _rigid_body._position, _color);
				reinit();
				bool first_changed_1= ranking->loose(_id);
				bool first_changed_2= ranking->win(it_ship->_id);
				if ((first_changed_1) || (first_changed_2)) {
					ss << ranking->id_first() << " IS FIRST !";
				}
				else {
					ss << it_ship->_id << " HIT " << _id;
				}
				global_msg->new_msg(ss.str(), it_ship->_color);
			}
		}
	}

}


// ---------------------------------------------------------------------------------------
IA::IA() {
	
}


IA::IA(std::string id, GLuint prog_draw_3d, GLuint prog_draw_basic, std::string model_path, std::string material_path, bool is_paves, float size_factor, glm::vec3 color) :
	_target_direction(glm::vec3(0.0f, 1.0f, 0.0f)), _tik1(0), _tik2(0)
{
	_ship= new Ship(id, prog_draw_3d, prog_draw_basic, model_path, material_path, is_paves, size_factor, color);
}


IA::~IA() {
	delete _ship;
}


// cherche à aligner sa direction forward avec _target_direction qui a été fixée dans think()
bool IA::align2target() {
	bool verbose= false;
	
	// on init toutes les touches à false
	for (auto &it_key : _ship->_keypresseds)
		it_key.second= false;
	
	// ship initialement pointe vers y positif, l'aileron vers z positif, et les ailes le long de x
	glm::vec3 fwd_direction= _ship->_rigid_body._rotation_matrix* glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 up_direction= _ship->_rigid_body._rotation_matrix* glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 right_direction= glm::cross(fwd_direction, up_direction);
	
	if (verbose) {
		cout << "---------------------------------\n";
		cout << "fwd_direction=" << fwd_direction.x << ";" << fwd_direction.y << ";" << fwd_direction.z << endl;
		cout << "up_direction=" << up_direction.x << ";" << up_direction.y << ";" << up_direction.z << endl;
		cout << "right_direction=" << right_direction.x << ";" << right_direction.y << ";" << right_direction.z << endl;
	}
	
	float scal= glm::dot(right_direction, _target_direction);
	
	if (verbose) {
		cout << "_target_direction=" << _target_direction.x << ";" << _target_direction.y << ";" << _target_direction.z << endl;
		cout << "scal="  << scal << endl;
	}
	
	// si _target_direction est dans le plan perpendiculaire à right_direction, ie dans le plan contenant fwd_direction
	if (abs(scal)< IA_ORIENTATION_TOLERANCE) {
		float scal2= glm::dot(fwd_direction, _target_direction);
		
		if (verbose) {
			cout << "scal2=" << scal2 << endl;
		}
		
		// si fwd_direction _target_direction sont quasi-alignés OK
		if (scal2> 1.0f- IA_ORIENTATION_TOLERANCE) {
			return true;
		}
		
		// sinon voir si _target_direction est au dessus ou en dessous
		else {
			glm::vec3 cross= glm::cross(fwd_direction, _target_direction);
			float scal3= glm::dot(right_direction, cross);
			if (verbose) {
				cout << "cross=" << cross.x << ";" << cross.y << ";" << cross.z << endl;
				cout << "scal3=" << scal3 << endl;
			}
			if (scal3> 0.0f) {
				_ship->_keypresseds[KEY_DOWN]= true;
			}
			else {
				_ship->_keypresseds[KEY_UP]= true;
			}
		}
	
	}
	
	// sinon orienter les ailes
	else {
		if (scal> 0.0f) {
			_ship->_keypresseds[KEY_RIGHT]= true;
		}
		else {
			_ship->_keypresseds[KEY_LEFT]= true;
		}
	}
	
	_ship->sync_keys2forces();
	
	return false;
}


void IA::think(RandTerrain * level, vector<Ship*> &ships) {
	bool verbose= false;
	
	// on cherche toujours à s'aligner
	bool target_align= align2target();
	
	_tik2= SDL_GetTicks();
	if (_tik2- _tik1< IA_THINKING_TIME)
		return;
	
	_tik1= SDL_GetTicks();
	
	_ship->_is_shooting= false;
	
	glm::vec3 fwd_direction= glm::normalize(_ship->_rigid_body._rotation_matrix* glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 fwd_position= _ship->_rigid_body._position+ IA_DISTANCE_TEST* fwd_direction;
	
	// si on va se prendre le décor tout droit on monte en z
	if (level->intersects_bbox(fwd_position, _ship->_rigid_body._rotation_matrix, _ship->_model._bbox)) {
		if (verbose) {
			cout << _ship->_id << " will intersect level" << endl;
		}
		_target_direction= glm::vec3(0.0f, 0.0f, 1.0f);
	}
	// si on sort du monde on fait 1/2 tour
	else if (out_of_bound(fwd_position)) {
		if (verbose) {
			cout << _ship->_id << " will be out_of_bound" << endl;
		}
		_target_direction= -fwd_direction;
	}
	else {
		// on repère le ship dans son champ de vision qui soit le plus proche de sa ligne de mire
		float max_value= -1e6;
		uint idx_ok= 0;
		glm::vec3 enemy_dir_ok= glm::vec3(0.0f);
		for (uint i=0; i<ships.size(); ++i) {
			if (ships[i]->_id== _ship->_id)
				continue;
		
			float distance= glm::length(ships[i]->_rigid_body._position- _ship->_rigid_body._position)/ IA_VISU_DEPTH;
			glm::vec3 enemy_dir= glm::normalize(ships[i]->_rigid_body._position- _ship->_rigid_body._position);
			float scal= glm::dot(fwd_direction, enemy_dir);
			
			if ((distance< 1.0f) && (scal> IA_VISU_ANGLE)) {
				// on peut chercher une formule plus fine de compromis entre distance et scal
				//float value= scal- distance;
				float value= scal;
				if (value> max_value) {
					idx_ok= i;
					enemy_dir_ok= enemy_dir;
					max_value= value;
				}
			}
		}
		
		if (max_value> 0.0f) {		
			if (verbose) {
				cout << _ship->_id << " prend " << ships[idx_ok]->_id << " en chasse" << endl;
			}
			_target_direction= enemy_dir_ok;
			_ship->_is_shooting= true;
		}
		else {
			// si on a pas trouvé de ship et qu'on a atteint sa _target_direction, on en prend une autre au hasard
			if (target_align) {
				float x= rand_float(-1.0f, 1.0f);
				float y= rand_float(-1.0f, 1.0f);
				float z= rand_float(-1.0f, 0.0f); // on revient vers le bas
				if (verbose) {
					cout << _ship->_id << " random_direction=(" << x << ";" << y << ";" << z << ")" << endl;
				}
				_target_direction= glm::normalize(glm::vec3(x, y, z));
			}
		}
	}
}


// ---------------------------------------------------------------------------------------
Ranking::Ranking() {

}


Ranking::Ranking(Font* font, std::vector<Ship*> &ships) : _font(font) {
	for (auto &it_ship : ships) {
		ShipPoints sp{it_ship->_id, it_ship->_color, 0, 0};
		_ship_points.push_back(sp);
	}
	
	_font->_alpha= RANKING_ALPHA;
}


void Ranking::draw() {
	for (uint idx_sp=0; idx_sp<_ship_points.size(); ++idx_sp) {
		stringstream ss;
		ss << idx_sp+ 1 << " " << _ship_points[idx_sp]._id << " +" << _ship_points[idx_sp]._n_win << " / -" << _ship_points[idx_sp]._n_loose;
		_font->draw(ss.str(), RANKING_OFFX, SCREEN_HEIGHT- RANKING_OFFY- idx_sp* RANKING_SIZEY, RANKING_SCALE, _ship_points[idx_sp]._color);
	}
}


// loose et win renvoient vrai si le 1er a changé; utilisé dans collision
bool Ranking::loose(std::string id) {
	for (auto &it_sp : _ship_points) {
		if (it_sp._id== id) {
			it_sp._n_loose++;
			break;
		}
	}
	return sort_ships();
}


bool Ranking::win(std::string id) {
	for (auto &it_sp : _ship_points) {
		if (it_sp._id== id) {
			it_sp._n_win++;
			break;
		}
	}
	return sort_ships();
}


// fonction de tri entre 2 ShipPoints utilisée lors de sort_ships()
bool rank_cmp(ShipPoints sp1, ShipPoints sp2) {
	return sp1._n_win- sp1._n_loose> sp2._n_win- sp2._n_loose;
}


bool Ranking::sort_ships() {
	string id_first_= id_first();
	sort(_ship_points.begin(), _ship_points.end(), rank_cmp);
	if (id_first_!= id_first())
		return true;
	return false;
}


void Ranking::print() {
	for (uint idx_sp=0; idx_sp<_ship_points.size(); ++idx_sp) {
		cout << idx_sp << " : " << _ship_points[idx_sp]._id << " ; " << _ship_points[idx_sp]._n_win << " ; " << _ship_points[idx_sp]._n_loose << endl;
	}
}


string Ranking::id_first() {
	return _ship_points[0]._id;
}


// ---------------------------------------------------------------------------------------
GlobalMsg::GlobalMsg() {

}


GlobalMsg::GlobalMsg(Font* font) : _font(font), _is_active(false), _msg(""), _color(glm::vec3(0.0f)) {

}


void GlobalMsg::new_msg(std::string msg, glm::vec3 color) {
	_is_active= true;
	_font->_alpha= 1.0f;
	_msg= msg;
	_color= color;
}


void GlobalMsg::draw() {
	if (!_is_active)
		return;
	
	_font->draw(_msg, GLOBAL_MSG_OFFX, GLOBAL_MSG_OFFY, GLOBAL_MSG_SCALE, _color);
}


void GlobalMsg::anim() {
	if (!_is_active)
		return;

	_font->_alpha-= GLOBAL_MSG_ALPHA_ANIM;
	if (_font->_alpha< 0.0f) {
		_font->_alpha= 0.0f;
		_is_active= false;
	}
}

