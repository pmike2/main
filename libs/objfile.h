#ifndef OBJFILE_H
#define OBJFILE_H

#include <string>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <sys/time.h>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "rapidxml.hpp"

#include "utile.h"
#include "bbox.h"
#include "repere.h"


// nombre de matériaux max pour 1 objet ; doit etre == celui du shader !!!
const unsigned int N_MAX_MATERIALS= 8;
// nombre max de précisions pour 1 objet
const unsigned int N_MAX_PRECISIONS= 4;

// a partir de ch_config_file, remplissage des variables suivantes
void parse_static_config_file(std::string ch_config_file, GLuint * vertex_buffers, GLuint * index_buffers, unsigned int * n_faces, unsigned int & n_precisions, glm::mat4 * materials_array, AABB * aabb);


// classe matériau ; utilisé temporairement pour renseigner _materials_array
class Material {
public:
	Material();
	~Material();
	void print();


	std::string _id;
	glm::vec3 _ambient;
	glm::vec3 _diffuse;
	//glm::vec3 _specular;
	float _shininess;
};


// classe modele ; peut exister en plusieurs précisions pour faire du LOD (level of detail)
class StaticModel {
public:
	StaticModel();
	StaticModel(std::string ch_config_file, GLuint prog_draw);
	~StaticModel();
	void print();
	
	
	std::string _ch_config_file;
	
	GLuint _vertex_buffers[N_MAX_PRECISIONS];
	GLuint _index_buffers[N_MAX_PRECISIONS];
	unsigned int _n_faces[N_MAX_PRECISIONS];
	unsigned int _n_precisions;

	glm::mat4 _materials_array[N_MAX_MATERIALS];
	
	GLint _model2clip_loc, _model2camera_loc, _normal_mat_loc;
	GLint _position_loc, _normal_loc, _idx_material_loc;
	GLint _materials_loc;
	GLint _alpha_loc;
	GLint _scale_loc;
	GLuint _prog_draw;

	AABB * _aabb;
};


// instance d'un modele
class StaticInstance {
public:
	StaticInstance();
	StaticInstance(StaticModel * model, const glm::vec3 & scale);
	~StaticInstance();
	void draw();
	void anim(ViewSystem * view_system);
	void set_pos_rot_scale(const glm::vec3 & position, const glm::quat & rotation, const glm::vec3 & scale);
	// lent, mieux vaut utiliser l'autre
	void set_pos_rot_scale(const glm::mat4 & mat);
	void set_precision(unsigned int precision);
	void print();
	
	
	InstancePosRot * _pos_rot;
	glm::mat4 _model2camera;
	glm::mat4 _model2clip;
	glm::mat3 _normal;
	float _alpha;
	
	StaticModel * _model;
	bool _draw_mesh;
	unsigned int _precision;
};


// instanced -> + rapide quand il y a beaucoup d'objets ayant la meme geometrie ; dans ce cas on utilise pas StaticModel
class StaticGroup {
public:
	StaticGroup();
	StaticGroup(std::string ch_config_file, GLuint prog_draw, std::vector<InstancePosRot *> pos_rots, std::vector<float> distances);
	~StaticGroup();
	void draw();
	void anim(ViewSystem * view_system);


	glm::mat4 _world2camera;
	glm::mat4 _world2clip;
	float _alpha;

	std::string _ch_config_file;
	
	GLuint _vertex_buffers[N_MAX_PRECISIONS];
	GLuint _index_buffers[N_MAX_PRECISIONS];
	GLuint _instance_buffers[N_MAX_PRECISIONS];
	unsigned int _n_faces[N_MAX_PRECISIONS];
	unsigned int _n_precisions;

	glm::mat4 _materials_array[N_MAX_MATERIALS];
	
	GLint _world2clip_loc, _world2camera_loc;
	GLint _position_loc, _normal_loc, _idx_material_loc, _instanced_matrix_loc;
	GLint _materials_loc;
	GLint _alpha_loc;
	GLuint _prog_draw;

	std::vector<InstancePosRot *> _pos_rots;
	glm::mat4 * _mats[N_MAX_PRECISIONS];
	unsigned int _mats_sizes[N_MAX_PRECISIONS];

	std::vector<float> _distances;
};


#endif
