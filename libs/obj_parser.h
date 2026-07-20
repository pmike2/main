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
	pt_3d _emissive; // inutilisé
	number _absorbance; // inutilisé
	number _shininess;
	number _opacity;
	fs _ambient_tex_path; // inutilisé
	fs _diffuse_tex_path;
	fs _specular_tex_path; // inutilisé
};


struct ObjFace {
	ObjFace();
	~ObjFace();
	friend std::ostream & operator << (std::ostream & os, ObjFace & face);


	uint _vertices_idx[3];
	uint _textures_idx[3]; // inutilisé
	uint _normals_idx[3];
	Material * _material;
	bool _texture_active; // inutilisé
	bool _normal_active;
};


struct ObjObject {
	ObjObject();
	~ObjObject();
	pt_3d compute_normal(ObjFace * face);
	friend std::ostream & operator << (std::ostream & os, ObjObject & obj);


	std::string _name;
	std::vector<pt_3d> _vertices;
	std::vector<pt_3d> _normals;
	std::vector<pt_2d> _texs; // inutilisé
	std::vector<ObjFace *> _faces;
	bool _smooth_shading; // inutilisé
};


struct ObjData {
	ObjData();
	ObjData(fs obj_path);
	~ObjData();
	void update_data();
	ObjObject * new_generic_object();
	ObjObject * get_object(std::string name);
	Material * get_material(std::string name);
	friend std::ostream & operator << (std::ostream & os, ObjData & data);


	std::vector<Material *> _materials;
	std::vector<ObjObject *> _objects;
	float * _data;
	uint _n_pts;
	uint _n_attrs_per_pts;
	AABB * _aabb;
	bool _use_ambient, _use_diffuse, _use_specular, _use_shininess, _use_opacity, _use_diffuse_texture;
};


#endif
