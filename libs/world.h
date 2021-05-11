#ifndef WORLD_H
#define WORLD_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <math.h>
#include <cstdlib>


#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>

#include <SDL2/SDL_keycode.h>

// cf bug : https://stackoverflow.com/questions/14113923/rapidxml-print-header-has-undefined-methods
#include "rapidxml_ext.h"

#include "utile.h"
#include "bbox.h"
#include "terrain.h"
#include "daefile.h"
#include "objfile.h"
#include "repere.h"
#include "path_find.h"


// TODO
//enum SquareState {WALKABLE, OBSTACLE, WATER};

// distances seuils pour choix precision des objets statiques ; au dela de la derniere distance on desactive l'objet
const std::vector<float> STATIC_GROUP_DISTANCES {1000.0f, 5000.0f};

// distance au dela de laquelle un objet animé est visible mais plus animé
const float ANIMATED_DIST_ANIMATED= 500.0f;
// distance au dela de laquelle un objet animé est n'est plus visible
const float ANIMATED_DIST_VISIBLE= 2000.0f;

// profondeur de l'arbre ; tester différentes valeurs
const unsigned int QUAD_TREE_DEPTH= 5;

// pour dessins bboxs
//const glm::vec3 STATIC_BBOX_COLOR= glm::vec3(0.4f, 0.8f, 0.5f);
//const glm::vec3 ANIMATED_BBOX_COLOR= glm::vec3(0.8f, 0.1f, 0.7f);
//const glm::vec3 TERRAIN_BBOX_COLOR= glm::vec3(0.2f, 0.7f, 0.8f);


// ensemble de modeles statiques et animés
class ModelsPool {
public:
	ModelsPool();
	ModelsPool(GLuint prog_3d_anim, GLuint prog_3d_obj, GLuint prog_basic);
	~ModelsPool();
	void add_static_model(std::string ch_config_file);
	void add_animated_model(std::string ch_config_file);
	StaticModel * get_static_model(std::string ch_config_file);
	AnimatedModel * get_animated_model(std::string ch_config_file);
	void clear();


	GLuint _prog_3d_anim, _prog_3d_obj, _prog_basic;
	std::vector<StaticModel *> _static_models;
	std::vector<AnimatedModel *> _animated_models;
};


// TODO
/*class Square {
public:
	Square();
	Square(const glm::vec2 & vmin, const glm::vec2 & vmax);
	~Square();
	
	
	SquareState _state;
	glm::vec2 _vmin, _vmax;
};
*/

// random d'un ensemble d'objets statiques
struct SIRandomConfig {
	std::string _path;
	unsigned int _compt;
	float _size_factor_min_xy, _size_factor_max_xy;
	float _size_factor_min_z, _size_factor_max_z;
	bool _conserve_ratio;
	float _rand_angle_min, _rand_angle_max;
};


// pareil que SIRandomConfig ?
struct AIRandomConfig {
	std::string _path;
	unsigned int _compt;
	float _size_factor_min_xy, _size_factor_max_xy;
	float _size_factor_min_z, _size_factor_max_z;
	bool _conserve_ratio;
	float _rand_angle_min, _rand_angle_max;
};


// random monde
struct WorldRandomConfig {
	std::vector<SIRandomConfig> _si_random_configs;
	std::vector<AIRandomConfig> _ai_random_configs;
	TerrainConfig * _terrain_config; // on doit prendre un pointeur ici car TerrainConfig n'est pas une simple struct, genere un 'pointer being freed was not allocated' sinon
	TerrainRandomConfig _terrain_random_config;
};


// noeud d'un quadtree ; chaque noeud a 4 enfants, un AABB et si le noeud est une feuille (pas d'enfant) un vecteur d'objets associés
class QuadNode {
public:
	QuadNode();
	QuadNode(const glm::vec2 & vmin, const glm::vec2 & vmax, unsigned int depth);
	~QuadNode();


	AABB * _aabb;
	QuadNode * _child_so;
	QuadNode * _child_se;
	QuadNode * _child_no;
	QuadNode * _child_ne;
	std::vector<InstancePosRot *> _pos_rots;
	unsigned int _depth; // profondeur du noeud dans l'arbre
};


// sert a desactiver des régions entières d'objets suivant plusieurs criteres : distance, n'intersecte pas la caméra ...
// passer a du octtree pour faire de l'occlusion culling ?
class QuadTree {
public:
	QuadTree();
	QuadTree(const glm::vec2 & vmin, const glm::vec2 & vmax, unsigned int depth);
	~QuadTree();
	void recursive_init(QuadNode * node);
	void recursive_delete(QuadNode * node);
	void add_position(InstancePosRot * position);
	void recursive_add_position(QuadNode * node, InstancePosRot * position);
	QuadNode * find_node(unsigned int depth, const glm::vec2 & v);
	void recursive_find_node(QuadNode * node, unsigned int depth, const glm::vec2 & v, QuadNode ** node_result);
	void set_z_max(float * altis, unsigned int width_n, unsigned int height_n);
	void recursive_set_z_max(QuadNode * node, unsigned int depth);
	void set_actives(ViewSystem * view_system, float dist_max);
	void recursive_set_actives(QuadNode * node, ViewSystem * view_system, float dist_max);
	void print();
	void recursive_print(QuadNode * node);


	QuadNode * _root;
	unsigned int _depth;
	unsigned int _debug;
};


// classe monde ; regroupe terrain, objets statiques, animés
class World {
public:
	World();
	World(GLuint prog_3d_anim, GLuint prog_3d_terrain, GLuint prog_3d_obj, GLuint prog_3d_obj_instanced, GLuint prog_basic, GLuint prog_bbox,
		const WorldRandomConfig * world_random_config=NULL, std::string ch_directory="");
	~World();
	bool key_down(InputState * input_state, SDL_Keycode key);
	bool key_up(InputState * input_state, SDL_Keycode key);
	bool mouse_motion(InputState * input_state);
	void draw();
	void anim(ViewSystem * view_system, unsigned int tikanim_delta);
	void randomize(const WorldRandomConfig * world_random_config);
	void write(std::string ch_directory);
	void read(std::string ch_directory);
	void statics2obstacles();
	void clear(bool clear_instances=true, bool clear_terrain=true);
	glm::vec2 get_center();
	void print();
	//void update_squares();
	AnimatedInstance * get_hero();
	//void sync_bbox_draws();


	GLuint _prog_3d_anim, _prog_3d_terrain, _prog_3d_obj, _prog_3d_obj_instanced, _prog_basic, _prog_bbox;
	ModelsPool * _models_pool;
	std::vector<AnimatedInstance *> _animated_instances;
	std::vector<StaticInstance *> _static_instances;
	std::vector<StaticGroup *> _static_groups;
	Terrain * _terrain;
	//std::vector<Square *> _squares;
	//float _square_size_x, _square_size_y;
	//unsigned int _n_squares_x, _n_squares_y;
	QuadTree * _quad_tree;
	/*std::vector<BBoxDraw *> _static_bbox_draws;
	std::vector<BBoxDraw *> _animated_bbox_draws;
	bool _is_bbox_draw;*/
	PathFinder * _path_finder;
};


// configs objets statiques
// --------------------------------------------------------------------------------------------------------
const std::vector<SIRandomConfig> SI_RANDOM_CONFIG_1 {
	{"./data/tree.xml", 10000, 0.5f, 2.0f, 1.0f, 4.0f, false, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/tree2.xml", 10000, 0.5f, 2.0f, 1.0f, 4.0f, false, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/tree3.xml", 10000, 0.5f, 2.0f, 1.0f, 4.0f, false, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/tree_dead.xml", 10000, 0.5f, 2.0f, 1.0f, 4.0f, false, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/rocks.xml", 1000, 2.0f, 20.0f, 2.0f, 20.0f, false, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/house.xml", 1000, 5.0f, 10.0f, 5.0f, 10.0f, true, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/grass.xml", 50000, 5.0f, 10.0f, 1.0f, 5.0f, false, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/castle.xml", 1, 20.0f, 20.0f, 20.0f, 20.0f, true, 0.0f, (float)(M_PI)* 2.0f}
};

const std::vector<SIRandomConfig> SI_RANDOM_CONFIG_2 {
	{"./data/tree.xml", 1000, 0.5f, 2.0f, 1.0f, 4.0f, false, 0.0f, (float)(M_PI)* 2.0f},
};

// configs objets dynamiques
// -------------------------------------------------------------------------------------------------------
const std::vector<AIRandomConfig> AI_RANDOM_CONFIG_1 {
	{"./data/hero.xml", 1, 1.0f, 1.0f, 1.0f, 1.0f, true, 0.0f, 0.0f},
	{"./data/gadjo.xml", 1000, 0.8f, 1.2f, 0.8f, 1.2f, false, 0.0f, (float)(M_PI)* 2.0f},
	{"./data/gadjo.xml", 10, 50.0f, 50.0f, 50.0f, 50.0f, true, 0.0f, (float)(M_PI)* 2.0f}
};

const std::vector<AIRandomConfig> AI_RANDOM_CONFIG_2 {
	{"./data/hero.xml", 1, 1.0f, 1.0f, 1.0f, 1.0f, true, 0.0f, 0.0f},
	{"./data/gadjo.xml", 100, 0.8f, 1.2f, 0.8f, 1.2f, false, 0.0f, (float)(M_PI)* 2.0f},
};

// -------------------------------------------------------------------------------------------------------
// params terrain {_origin, float _width_sub, float _height_sub, _width_step, _height_step, _n_subs_x, _n_subs_y}
// mettre des puissances de 2 sinon degradations vont faire des trous
const TerrainConfig TERRAIN_CONFIG_1 {glm::vec2(0.0f, 0.0f), 1024.0f, 1024.0f, 2.0f, 2.0f, 8, 8};
const TerrainConfig TERRAIN_CONFIG_2 {glm::vec2(0.0f, 0.0f), 1024.0f, 1024.0f, 2.0f, 2.0f, 2, 2};

// -------------------------------------------------------------------------------------------------------
// params terrain aléatoire	(alti_offset , n_levels , gradient_base_size , max_factor , redistribution_power)
const TerrainRandomConfig TERRAIN_RAND_CONFIG_1 {30.0f, 5, 10, 200.0f, 1.2f};
const TerrainRandomConfig TERRAIN_RAND_CONFIG_2 {10.0f, 5, 10, 100.0f, 1.2f};

// configs monde
// -------------------------------------------------------------------------------------------------------
const WorldRandomConfig WORLD_RAND_CONFIG_1 {SI_RANDOM_CONFIG_1, AI_RANDOM_CONFIG_1, (TerrainConfig *)(& TERRAIN_CONFIG_1), TERRAIN_RAND_CONFIG_1};
const WorldRandomConfig WORLD_RAND_CONFIG_2 {SI_RANDOM_CONFIG_2, AI_RANDOM_CONFIG_2, (TerrainConfig *)(& TERRAIN_CONFIG_2), TERRAIN_RAND_CONFIG_2};



#endif
