#ifndef DAEFILE_H
#define DAEFILE_H

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <cmath>
#include <string>
#include <vector>
#include <cassert>
#include <algorithm>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "rapidxml.hpp"

#include "utile.h"
#include "bbox.h"
#include "repere.h"


const uint N_JOINTS= 128; // == a celui dans le vertex shader !
const std::string NULL_JOINT_SID= "NULL_JOINT_SID";
const std::string ROOT_JOINT_SID= "ROOT_JOINT_SID";



struct JointInfluence {
	uint _idx_joint;
	float _weight;
};

bool cmp_joint_influence(const JointInfluence & a, const JointInfluence & b);


struct TimedMatrix {
	float _time;
	glm::mat4 _mat;
};


struct Vertex {
	glm::vec3 _position;
	glm::vec3 _normal;
	glm::vec3 _color;
	glm::vec2 _texture;
	std::vector<JointInfluence> _joints;
	uint _idx; // sert au moment de remplir JointInfluence
};


struct Geometry {
	std::vector<Vertex> _vertices;
	std::string _id;
	glm::mat4 _bind_shape_matrix; // a appliquer aux vertices avant toute chose
};


class ModelJoint {
public:
	ModelJoint();
	~ModelJoint();
	void print();


	ModelJoint * _parent;
	std::string _id; // pour faire le lien avec les anims
	std::string _sid; // pour faire le lien avec les controllers
	glm::mat4 _local_bind_mat; // matrice au repos ("bind pose") - locale
	glm::mat4 _bind_mat; // matrice au repos ("bind pose") - absolue ; déduite de locale en multipliant récursivement par ses parents
	glm::mat4 _inv_bind_mat; // sert a passer les vertex en repere joint ; cf https://stackoverflow.com/questions/55278745/why-do-i-need-the-inverse-of-the-bind-pose-matrix-to-calculate-animations
	std::vector<TimedMatrix> _anims; // matrices locales d'animation
};


class InstanceJoint {
public:
	InstanceJoint();
	InstanceJoint(ModelJoint * model_joint);
	~InstanceJoint();
	void print();
	void anim(uint delta_time_ms);


	ModelJoint * _model_joint;
	InstanceJoint * _parent;
	glm::mat4 _local_current_mat; // a interpoler entre animations locales
	glm::mat4 _current_mat; // matrice courante
	glm::mat4 _skinning_mat; // matrice finale a appliquer pondérée aux vertices
	float _anim_time;
};


class ModelAnimation {
public:
	ModelAnimation();
	ModelAnimation(std::string ch_dae_file, float moving_speed);
	~ModelAnimation();
	void parse_dae(std::string ch_dae_file);
	void parse_joint(rapidxml::xml_node<> * node, ModelJoint * parent_joint);
	//void compute_mat_joint(uint idx_joint);
	void compute_mats();
	void print();
	void clean();


	std::string _id;
	std::vector<ModelJoint *> _joints;
	std::vector<Geometry> _geometries;
	float _moving_speed;
};


class InstanceAnimation {
public:
	InstanceAnimation();
	InstanceAnimation(ModelAnimation * model_animation);
	~InstanceAnimation();
	void compute_mats();
	void anim(uint delta_time_ms);
	void print();


	ModelAnimation * _model_animation;
	std::vector<InstanceJoint *> _joints;
};


class ModelMesh {
public:
	ModelMesh();
	ModelMesh(GLuint prog_draw, std::vector<Geometry> & geometries);
	~ModelMesh();
		
	
	GLuint _prog_draw;
	uint _n_faces;
	float * _vertices;
	uint * _faces;
	GLuint _buffers[2];
	
	GLint _model2clip_loc, _model2camera_loc, _normal_mat_loc;
	GLint _position_loc, _normal_loc;
	GLint _ambient_color_loc, _shininess_loc, _diffuse_color_loc, _alpha_loc;
	GLint _joints_loc;
	GLint _indices_loc, _weights_loc;

	AABB * _aabb;
};


class InstanceMesh {
public:
	InstanceMesh();
	InstanceMesh(ModelMesh * model_mesh);
	~InstanceMesh();
	void draw();
	void anim(ViewSystem * view_system, std::vector<InstanceJoint *> & joints);

	ModelMesh * _model_mesh;

	glm::mat4 _model2camera;
	glm::mat4 _model2clip;
	glm::mat4 _model2world;
	glm::mat3 _normal;
	glm::vec3 _ambient;
	float _shininess;
	float _alpha;
	glm::mat4 _joints_mats[N_JOINTS];
};


class Skeleton {
public:
	Skeleton();
	Skeleton(GLuint prog_draw, std::vector<InstanceJoint *> & joints);
	~Skeleton();
	void draw();
	void anim(ViewSystem * view_system, std::vector<InstanceJoint *> & joints);
	
	
	uint _n_joints;

	GLuint _prog_draw;
	GLint _model2clip_loc, _position_loc, _diffuse_color_loc;
	GLuint _buffer;
	float * _data;
	glm::mat4 _model2clip;
	glm::mat4 _model2world;
};


class AnimatedModel {
public:
	AnimatedModel();
	AnimatedModel(std::string ch_config_file, GLuint prog_3d);
	~AnimatedModel();
	void parse_config(std::string ch_config_file);
	void print();

	
	std::string _ch_config_file;
	std::vector<ModelAnimation *> _animations;
	ModelMesh * _mesh;
	glm::quat _rotation_0;
};


enum AnimatedInstanceStatus {STATIC, MOVING, WAITING};

class AnimatedInstance {
public:
	AnimatedInstance();
	AnimatedInstance(AnimatedModel * model, const glm::vec3 & scale, GLuint prog_3d, GLuint prog_basic);
	~AnimatedInstance();
	void draw();
	void anim(ViewSystem * view_system, uint delta_time_ms);
	void set_pos_rot_scale(const glm::vec3 & position, const glm::quat & rotation, const glm::vec3 & scale);
	void set_animated(bool b);
	void randomize();
	void print();
	void set_path(const std::vector<glm::vec3> & path);
	void compute_next_pos_rot();
	void move2next_pos_rot();
	void set_status(AnimatedInstanceStatus status);


	AnimatedModel * _model;
	std::vector<InstanceAnimation *> _animations;
	Skeleton * _skeleton;
	InstanceMesh * _mesh;
	bool _draw_mesh, _draw_skeleton, _animated;
	uint _current_idx_anim;
	InstancePosRot * _pos_rot;
	
	std::vector<glm::vec3> _path;
	uint _next_path_idx;
	glm::vec3 _next_position;
	glm::quat _next_rotation;
	AnimatedInstanceStatus _status;
	uint _waiting_n_ms;
};

#endif
