#ifndef ANIMATED_OBJ_H
#define ANIMATED_OBJ_H

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <filesystem>

#include "json.hpp"

#include "obj_parser.h"
#include "bbox.h"
#include "typedefs.h"


using json = nlohmann::json;


// mode du modèle
// ANIMATED_OBJECT_RIGID = un bone par objet et tous les vertices de l'objet sont affectés par ce bone
// ANIMATED_OBJECT_WEIGHT = jusqu'à 4 bones + weights affectent chaque vertex
enum ANIMATED_OBJECT_MODE {ANIMATED_OBJECT_RIGID, ANIMATED_OBJECT_WEIGHT};


// cette valeur limite à la fois le nombre d'actions ainsi que le nombre de frames par action
const uint IDX_TEXTURE_DATA_SIZE = 1024;


// json -> mat4
mat_4d parse_js_matrix(json js);


// Bone
struct AnimatedObjBone {
	AnimatedObjBone();
	AnimatedObjBone(std::string name, mat_4d mat_local, std::string parent_name = "");
	~AnimatedObjBone();
	friend std::ostream & operator << (std::ostream & os, AnimatedObjBone & bone);


	mat_4d _mat_local; // matrice locale de transformation
	std::string _name; // nom
	AnimatedObjBone * _parent; // bone parent
	std::string _parent_name; // nom du bone parent
};


// une Transformation associe une matrice à un Bone
struct AnimatedObjTransform {
	AnimatedObjTransform();
	AnimatedObjTransform(AnimatedObjBone * bone, mat_4d mat);
	~AnimatedObjTransform();
	friend std::ostream & operator << (std::ostream & os, AnimatedObjTransform & transform);

	
	AnimatedObjBone * _bone; // bone associé
	mat_4d _mat_basis; // matrice de base
	mat_4d _mat_final; // matrice finale (composé avec la matrice du bone et de ses parents)
};


// un Frame représente une pose d'animation et est composé de transformations (une par Bone)
struct AnimatedObjFrame {
	AnimatedObjFrame();
	~AnimatedObjFrame();
	AnimatedObjTransform * get_transform(AnimatedObjBone * bone);
	friend std::ostream & operator << (std::ostream & os, AnimatedObjFrame & frame);


	std::vector<AnimatedObjTransform *> _transforms; // transformations
};


// une Action est une série de Frames
struct AnimatedObjAction {
	AnimatedObjAction();
	AnimatedObjAction(std::string name);
	~AnimatedObjAction();
	friend std::ostream & operator << (std::ostream & os, AnimatedObjAction & action);


	std::string _name; // nom
	std::vector<AnimatedObjFrame *> _frames; // frames
	uint _loop_start_idx, _loop_end_idx;
};


// Un AnimatedObjObject est une surcouche à ObjObject que l'on retrouve dans obj_parser.h
// associe à chaque objet une liste de Bones et de poids (max 4) dans le cas ANIMATED_MODEL_WEIGHT
// ou directement un Bone dans le cas ANIMATED_MODEL_RIGID
struct AnimatedObjObject {
	AnimatedObjObject();
	AnimatedObjObject(ObjObject * static_object);
	~AnimatedObjObject();
	void sort_per_weight(); // tri des poids
	friend std::ostream & operator << (std::ostream & os, AnimatedObjObject & obj);


	ObjObject * _static_object;
	ANIMATED_OBJECT_MODE _mode; // mode = rigide ou avec poids
	AnimatedObjBone * _parent_bone; // utilisé dans le cas ANIMATED_MODEL_RIGID
	std::map<number, std::vector<std::pair<AnimatedObjBone *, number> > > _weights_per_vertex; // utilisé dans le cas ANIMATED_MODEL_WEIGHT
};


// Un modèle animé associé à un ObjData (obj_parser.h)
struct AnimatedObjModel {
	AnimatedObjModel();
	AnimatedObjModel(fs json_path);
	~AnimatedObjModel();
	void compute_transform_final_matrix(); // calcul des matrices finales des transformations
	void compute_buffer_texture_data(); // calcul de ce qui sera mis dans le buffer texture
	AnimatedObjObject * get_animated_object(std::string obj_name);
	AnimatedObjBone * get_bone(std::string bone_name);
	AnimatedObjAction * get_action(std::string action_name);
	uint get_action_idx(std::string action_name);
	friend std::ostream & operator << (std::ostream & os, AnimatedObjModel & obj);


	std::string _name; // nom
	number _fps; // FPS des animations
	uint _n_ms_per_frame; // nombre de frames par seconde
	ObjData * _obj_data; // .obj associé
	std::vector<AnimatedObjBone *> _bones; // bones
	std::vector<AnimatedObjAction *> _actions; // actions
	std::vector<AnimatedObjObject *> _objects; // objets
	mat_4d _mat_armature; // matrice de transformation liée à l'armature
	uint _buffer_texture_data_size; // taille du buffer texture
	float * _buffer_texture_data; // buffer texture où sont stockés toutes les matrices de transfo
	float _idx_texture_data[IDX_TEXTURE_DATA_SIZE * IDX_TEXTURE_DATA_SIZE];
};


// une instance d'un modèle
struct AnimatedObjInstance : public InstancePosRot {
	AnimatedObjInstance();
	AnimatedObjInstance(AnimatedObjModel * model, pt_3d pos, time_point t, quat q = quat(1.0, 0.0, 0.0, 0.0), std::string action_name = "");
	~AnimatedObjInstance();
	void anim(time_point t);
	void set_next_action(std::string action_name);
	std::string get_action();


	AnimatedObjModel * _model; // modèle
	uint _idx_action; // idx action courante
	uint _idx_frame; // idx frame courant
	time_point _last_anim_t; // dernier temps d'animation
	std::string _next_action;
};


#endif
