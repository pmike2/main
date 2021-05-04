#ifndef RAND_TERRAIN_H
#define RAND_TERRAIN_H

#include <string>
#include <vector>
#include <thread>

#include <OpenGL/gl3.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>


#include "utile.h"
#include "bbox.h"
#include "bbox_2d.h"
#include "repere.h"



// brillance du level
const float LEVEL_SHININESS= 50.0f;
// couleur ambiante
const glm::vec3 LEVEL_AMBIENT_COLOR(0.2f, 0.2f, 0.2f);

// couleurs en fonction de l'alti (1ère valeur)
const glm::vec4 LEVEL_COLORS[]= {
	glm::vec4(0.0f, 0.1f, 0.8f, 0.95f),
	glm::vec4(20.0f, 0.8f, 0.8f, 0.0f),
	glm::vec4(130.0f, 0.1f, 0.6f, 0.1f),
	glm::vec4(300.0f, 0.2f, 0.8f, 0.1f),
	glm::vec4(500.0f, 0.7f, 0.8f, 0.6f),
	glm::vec4(10000.0f, 0.9f, 0.9f, 0.8f)
};


// vitesse d'anim de la marée
const float TIDE_ANIM_SPEED= 0.01f;

// indices a ajouter pour avoir les voisins d'un sommet
const int NEIGHBORS[6][3][2]= {
	{{0, 0}, {-1, -1}, {0, -1}},
	{{0, 0}, {0, -1}, {1, 0}},
	{{0, 0}, {1, 0}, {1, 1}},
	{{0, 0}, {1, 1}, {0, 1}},
	{{0, 0}, {0, 1}, {-1, 0}},
	{{0, 0}, {-1, 0}, {-1, -1}}
};

// nombre de GroupTerrain, ie de threads qui au demarrage vont tourner en parallele pour charger le terrain
const unsigned int N_GROUPS= 8;

//const unsigned int HORIZON_N_WIDTH= 8;
//const unsigned int HORIZON_N_HEIGHT= 8;


glm::vec3 alti2color(float z);


struct Degradation {
	unsigned int _idx; // indice
	unsigned int _step; // on divise la taille initiale du terrain par 2** _step
	float _dist; // jusqu'a quelle distance utiliser cette degradation
};

// degradations
const unsigned int N_DEGRADATIONS= 3;
const Degradation DEGRADATIONS[N_DEGRADATIONS] = { {0, 0, 3000.0f}, {1, 3, 10000.0f}, {2, 5, 100000.0f}};


struct TerrainConfig {
	TerrainConfig();
	TerrainConfig(const glm::vec2 & origin, float width_sub, float height_sub, float width_step, float height_step, unsigned int n_subs_x, unsigned int n_subs_y);
	~TerrainConfig();
	void print();


	glm::vec2 _origin; // origine
	float _width_step, _height_step; // taille pas
	float _width, _height; // dimensions totales
	unsigned int _width_n, _height_n; // nombre de pas total
	float _width_sub, _height_sub; // dimensions d'un subterrain
	unsigned int _width_sub_n, _height_sub_n; // nombre de pas d'un subterrain
	unsigned int _n_subs_x, _n_subs_y; // nombre de subterrains en x, y
	unsigned int _n_subs; // nombre de subterrains
	unsigned int * _idxs[N_DEGRADATIONS]; // indices des sommets en fonction de la degradation
	AABB_2D * _aabb;
};


// ref : https://www.redblobgames.com/maps/terrain-from-noise
struct TerrainRandomConfig {
	float _alti_offset;
	unsigned int _n_levels;
	unsigned int _gradient_base_size;
	float _max_factor;
	float _redistribution_power;
};


// partie dessin des subterrains
class TerrainMesh {
public:
	TerrainMesh();
	TerrainMesh(GLuint prog_draw_3d);
	~TerrainMesh();
	void draw();
	void anim(const glm::mat4 & world2camera, const glm::mat4 & camera2clip);
	
	
	glm::mat4 _model2camera;
	glm::mat4 _model2clip;
	glm::mat4 _model2world;
	glm::mat3 _normal;
	glm::vec3 _ambient;
	float _shininess;
	float _alpha;
	float _tide;
	
	GLuint _prog_draw;
	GLint _model2clip_loc, _model2camera_loc, _normal_mat_loc;
	GLint _position_loc, _normal_loc;
	GLint _ambient_color_loc, _shininess_loc, _diffuse_color_loc;
	GLint _alpha_loc;
	GLint _tide_loc;

	GLuint _vertex_buffer;
	GLuint _index_buffers[N_DEGRADATIONS];
	unsigned int _n_faces[N_DEGRADATIONS];
	unsigned int _current_index_buffer_idx;
};


// partie du terrain
class SubTerrain {
public:
	SubTerrain();
	SubTerrain(GLuint prog_draw_3d, float * altis, const TerrainConfig * config, unsigned int idx_x, unsigned int idx_y);
	~SubTerrain();
	void load(float * altis);
	void set_degradation(unsigned int degradation);
	void draw();
	void anim(ViewSystem * view_system);


	unsigned int _idx_x, _idx_y; // indices au sein du terrain
	float * _vertices; // sommets ; supprimé une fois que les buffers opengl sont remplis
	TerrainMesh * _mesh;
	bool _draw_mesh, _active;
	AABB * _aabb; // boite englobante ; sert a ne pas dessiner un subterrain qui est en dehors de la camera
	TerrainConfig * _config;
	bool _loaded, _buffers_filled; // _loaded = true quand _vertices est rempli ; _buffers_filled quand les buffers opengl sont remplis
	//std::mutex _mtx; // utile ?
};


// groupe de subterrains ; chaque groupe fera un load en parallele ; une fois que le jeu tourne ces objets sont inutiles
class GroupTerrain {
public:
	GroupTerrain();
	~GroupTerrain();
	void load(float * altis);
	void load_thread(float * altis);


	std::vector<SubTerrain *> _subterrains;
	bool _loaded;
	std::thread _thr;
};


// classe terrain
class Terrain {
public:
	Terrain();
	Terrain(GLuint prog_draw_3d, const TerrainConfig * config, const TerrainRandomConfig * random_config=NULL, std::string ch_alti_file="");
	~Terrain();
	void draw();
	void anim(ViewSystem * view_system);
	float get_alti(const glm::vec2 & pos, bool * alti_ok=NULL);
	//void get_altis_segment(glm::vec2 & pt_begin, glm::vec2 & pt_end, float step, std::vector<float> & altis);
	bool get_intersecting_point_OLD(glm::vec3 & pt_begin, glm::vec3 & pt_end, float step, glm::vec3 & result);
	bool get_intersecting_point(glm::vec3 & origin, glm::vec3 & direction, float step, glm::vec3 & result);
	void sync_mesh();
	void set_draw_mesh(bool b);
	void save(std::string ch_tif);
	//bool intersects_bbox(glm::vec3 & translation, glm::mat3 & rotation_matrix, BBox & bbox);
	//bool aabb_show_horizon(AABB * aabb);
	

	float * _altis;
	TerrainConfig * _config;
	SubTerrain ** _subterrains;
	GroupTerrain * _group_terrains[N_GROUPS];
	//float * _horizons;
	//unsigned int _horizon_n_width, _horizon_n_height;
};



#endif
