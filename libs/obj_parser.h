/*
Parcours .obj et .mat
*/

#ifndef OBJ_PARSER
#define OBJ_PARSER

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#include "typedefs.h"
#include "bbox.h"


// type de donnée à spécifier lors de la mise à jour de ObjData._data
enum OBJDATA_DATA_ITEM {OBJDATA_VERTEX, OBJDATA_NORMAL, OBJDATA_TANGENT, OBJDATA_BITANGENT, OBJDATA_AMBIENT_COLOR, 
	OBJDATA_DIFFUSE_COLOR, OBJDATA_SPECULAR_COLOR, OBJDATA_SHININESS, OBJDATA_OPACITY, OBJDATA_TEXTURE};


// Matériau (.mat)
struct Material {
	Material();
	Material(std::string name, uint idx);
	~Material();
	friend std::ostream & operator << (std::ostream & os, Material & mat);


	std::string _name;
	uint _idx;
	pt_3d _ambient;
	pt_3d _diffuse;
	pt_3d _specular;
	number _shininess;
	number _opacity;
	fs _diffuse_tex_path;
	fs _normal_tex_path;

	pt_3d _emissive; // inutilisé
	number _absorbance; // inutilisé
	fs _ambient_tex_path; // inutilisé
	fs _specular_tex_path; // inutilisé
	number _normal_strength; // inutilisé
};


// Face triangulaire d'un objet
struct ObjFace {
	ObjFace();
	~ObjFace();
	friend std::ostream & operator << (std::ostream & os, ObjFace & face);


	uint _vertices_idx[3];
	uint _textures_idx[3];
	uint _normals_idx[3];
	Material * _material;
	pt_3d _normal;
	pt_3d _tangent;
	pt_3d _bitangent;

	bool _normal_active; // inutilisé
	bool _texture_active; // inutilisé
};


// Objet
struct ObjObject {
	ObjObject();
	~ObjObject();
	void compute_face_normals_tangents_bitangents();
	friend std::ostream & operator << (std::ostream & os, ObjObject & obj);


	std::string _name;
	std::vector<pt_3d> _vertices;
	std::vector<pt_3d> _normals;
	std::vector<pt_2d> _texs;
	std::vector<ObjFace *> _faces;
	
	bool _smooth_shading; // inutilisé
};


// .obj = plusieurs objets + plusieurs matériaux
struct ObjData {
	ObjData();
	ObjData(fs obj_path);
	~ObjData();
	void update_data(std::vector<OBJDATA_DATA_ITEM> items); // maj de _data en fonction de ce que l'on veut y mettre
	ObjObject * new_generic_object();
	ObjObject * get_object(std::string name);
	Material * get_material(std::string name);
	std::vector<fs> get_diffuse_textures();
	std::vector<fs> get_normal_textures();
	friend std::ostream & operator << (std::ostream & os, ObjData & data);


	std::vector<Material *> _materials;
	std::vector<ObjObject *> _objects;
	float * _data; // données à mettre dans un buffer pour affichage
	uint _n_pts;
	uint _n_attrs_per_pts;
	AABB * _aabb; // AABB englobant
};


#endif
