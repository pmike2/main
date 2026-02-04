#ifndef OBJ_PARSER
#define OBJ_PARSER

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>

#include "typedefs.h"
#include "bbox.h"


struct Material {
	Material();
	~Material();
	friend std::ostream & operator << (std::ostream & os, Material & mat);


	std::string _name;
	pt_3d _ambient;
	pt_3d _diffuse;
	pt_3d _specular;
	pt_3d _emissive; // inutilisé
	number _absorbance; // inutilisé
	number _shininess;
	number _opacity;
	std::string _ambient_tex_path; // inutilisé
	std::string _diffuse_tex_path; // inutilisé
	std::string _specular_tex_path; // inutilisé
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
	ObjData(std::string obj_path);
	~ObjData();
	//void set_use(bool use_ambient, bool use_diffuse, bool use_specular, bool use_shininess, bool use_opacity);
	void update_data();
	ObjObject * new_generic_object();
	friend std::ostream & operator << (std::ostream & os, ObjData & data);


	std::map<std::string, Material *> _materials;
	std::vector<ObjObject *> _objects;
	float * _data;
	uint _n_pts;
	uint _n_attrs_per_pts;
	AABB * _aabb;
	bool _use_ambient, _use_diffuse, _use_specular, _use_shininess, _use_opacity;
};


#endif
