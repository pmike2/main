#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "utile.h"
#include "geom.h"

#include "obj_parser.h"


// Material ---------------------------------------------------------
Material::Material() {

}


Material::Material(std::string name, uint idx) :
	_name(name), _idx(idx),
	_ambient(pt_3d(0.0)), _diffuse(pt_3d(0.0)), _specular(pt_3d(0.0)), _emissive(pt_3d(0.0)),
	_absorbance(0.0), _shininess(0.0), _opacity(0.0),
	_ambient_tex_path(""), _diffuse_tex_path(""), _specular_tex_path(""), _normal_tex_path(""), _normal_strength(0.0)
{

}


Material::~Material() {

}


std::ostream & operator << (std::ostream & os, Material & mat) {
	os << "name = " << mat._name;
	os << " ; ambient = " << glm_to_string(mat._ambient);
	os << " ; diffuse = " << glm_to_string(mat._diffuse);
	os << " ; specular = " << glm_to_string(mat._specular);
	os << " ; emissive = " << glm_to_string(mat._emissive);
	os << " ; absorbance = " << mat._absorbance;
	os << " ; shininess = " << mat._shininess;
	os << " ; opacity = " << mat._opacity;
	os << " ; ambient_tex_path = " << mat._ambient_tex_path;
	os << " ; diffuse_tex_path = " << mat._diffuse_tex_path;
	os << " ; specular_tex_path = " << mat._specular_tex_path;
	return os;
}


// Face --------------------------------------------------------------
ObjFace::ObjFace() :
	_texture_active(false), _normal_active(false), _material(NULL), _normal(pt_3d(0.0)), _tangent(pt_3d(0.0)), _bitangent(pt_3d(0.0)) 
{
	for (uint i=0; i<3; ++i) {
		_vertices_idx[i] = 0;
		_textures_idx[i] = 0;
		_normals_idx[i] = 0;
	}
}


ObjFace::~ObjFace() {

}


std::ostream & operator << (std::ostream & os, ObjFace & face) {
	os << "vertices_idx = (" << face._vertices_idx[0] << " ; " << face._vertices_idx[1] << " ; " << face._vertices_idx[2] << ") ; ";
	os << "textures_idx = (" << face._textures_idx[0] << " ; " << face._textures_idx[1] << " ; " << face._textures_idx[2] << ") ; ";
	os << "normals_idx = (" << face._normals_idx[0] << " ; " << face._normals_idx[1] << " ; " << face._normals_idx[2] << ") ; ";

	return os;
}


// ObjObject ----------------------------------------------------------
ObjObject::ObjObject() : _smooth_shading(false), _name("") {

}


ObjObject::~ObjObject() {
	for (auto & face : _faces) {
		delete face;
	}
	_faces.clear();
}


void ObjObject::compute_face_normals_tangents_bitangents() {
	for (auto & face : _faces) {
		pt_3d p1 = _vertices[face->_vertices_idx[0]];
		pt_3d p2 = _vertices[face->_vertices_idx[1]];
		pt_3d p3 = _vertices[face->_vertices_idx[2]];
		face->_normal = glm::normalize(glm::cross(p2 - p1, p3 - p1));

		pt_2d uv1 = _texs[face->_textures_idx[0]];
		pt_2d uv2 = _texs[face->_textures_idx[1]];
		pt_2d uv3 = _texs[face->_textures_idx[2]];
		face->_tangent = tangent(p1, p2, p3, uv1, uv2, uv3);

		face->_bitangent = glm::cross(face->_normal, face->_tangent);
	}
}


std::ostream & operator << (std::ostream & os, ObjObject & obj) {
	os << "name = " << obj._name;
	os << " ; n_vertices = " << obj._vertices.size();
	os << " ; n_normals = " << obj._normals.size();
	os << " ; n_texs = " << obj._texs.size();
	os << " ; n_faces = " << obj._faces.size();
	os << " ; smooth_shading = " << obj._smooth_shading;
	return os;
}


// ObjData ------------------------------------------------------------
ObjData::ObjData() : _n_pts(0), _n_attrs_per_pts(0) {

}


ObjData::ObjData(fs obj_path) :
	_n_pts(0), _n_attrs_per_pts(0)
{
	// lecture .mtl -----------------------------------------
	std::string mat_filename = obj_path.stem().string() + ".mtl";
	fs parent_dir = obj_path.parent_path();
	fs mat_path = parent_dir / mat_filename;

	//std::cout << obj_path << " ; " << mat_path << "\n";

	Material * current_material = NULL;
	uint material_idx = 0;
	
	std::ifstream mat_file(mat_path);
	std::string line;
	while (std::getline(mat_file, line)) {
		std::istringstream iss(line);
		std::string s;
		iss >> s;

		if (s == "newmtl") {
			if (current_material != NULL) {
				_materials.push_back(current_material);
			}
			iss >> s;
			current_material = new Material(s, material_idx);
			material_idx++;
		}
		else if (s == "Ka") {
			for (uint i=0; i<3; ++i) {
				iss >> s;
				current_material->_ambient[i] = std::stod(s);
			}
		}
		else if (s == "Kd") {
			for (uint i=0; i<3; ++i) {
				iss >> s;
				current_material->_diffuse[i] = std::stod(s);
			}
		}
		else if (s == "Ks") {
			for (uint i=0; i<3; ++i) {
				iss >> s;
				current_material->_specular[i] = std::stod(s);
			}
		}
		else if (s == "Ke") {
			for (uint i=0; i<3; ++i) {
				iss >> s;
				current_material->_emissive[i] = std::stod(s);
			}
		}
		else if (s == "Ns") {
			iss >> s;
			current_material->_shininess = std::stod(s);
		}
		else if (s == "Ni") {
			iss >> s;
			current_material->_absorbance = std::stod(s);
		}
		else if (s == "d") {
			iss >> s;
			current_material->_opacity = std::stod(s);
		}
		else if (s == "map_Ka") {
			iss >> s;
			current_material->_ambient_tex_path = parent_dir / s;
		}
		else if (s == "map_Kd") {
			iss >> s;
			current_material->_diffuse_tex_path = parent_dir / s;
		}
		else if (s == "map_Ks") {
			iss >> s;
			current_material->_specular_tex_path = parent_dir / s;
		}
		else if (s == "map_Bump") {
			iss >> s;
			iss >> s;
			current_material->_normal_strength = std::stod(s);
			iss >> s;
			current_material->_normal_tex_path = parent_dir / s;
		}
	}
	_materials.push_back(current_material);


	// lecture .obj ------------------------------------
	ObjObject * current_object = NULL;
	current_material = NULL;
	pt_3d vmin(1e-9);
	pt_3d vmax(-1e-9);
	number n_vertices_total = 0;
	number n_normals_total = 0;
	number n_texs_total = 0;

	std::ifstream obj_file(obj_path);
	while (std::getline(obj_file, line)) {
		//std::cout << line << "\n";
		std::istringstream iss(line);
		std::string s;
		iss >> s;

		if (s == "o") {
			if (current_object != NULL) {
				_objects.push_back(current_object);
				n_vertices_total += current_object->_vertices.size();
				n_normals_total += current_object->_normals.size();
				n_texs_total += current_object->_texs.size();
			}
			iss >> s;
			current_object = new ObjObject();
			current_object->_name = s;
		}
		else if (s == "v") {
			pt_3d vertex;
			for (uint i=0; i<3; ++i) {
				iss >> s;
				vertex[i] = std::stod(s);
			}
			if (current_object == NULL) {
				current_object = new_generic_object();
			}
			current_object->_vertices.push_back(vertex);

			for (uint i=0; i<3; ++i) {
				if (vertex[i] < vmin[i]) {
					vmin[i] = vertex[i];
				}
				if (vertex[i] > vmax[i]) {
					vmax[i] = vertex[i];
				}
			}
		}
		else if (s == "vn") {
			pt_3d normal;
			for (uint i=0; i<3; ++i) {
				iss >> s;
				normal[i] = std::stod(s);
			}
			current_object->_normals.push_back(normal);
		}
		else if (s == "vt") {
			pt_2d tex;
			for (uint i=0; i<2; ++i) {
				iss >> s;
				tex[i] = std::stod(s);
			}
			current_object->_texs.push_back(tex);
		}
		else if (s == "s") {
			iss >> s;
			if (s == "off" || s == "0") {
				current_object->_smooth_shading = false;
			}
			else {
				current_object->_smooth_shading = true;
			}
		}
		else if (s == "usemtl") {
			iss >> s;
			current_material = get_material(s);
		}
		else if (s == "f") {
			ObjFace * face = new ObjFace();
			face->_material = current_material;
			for (uint i=0; i<3; ++i) {
				iss >> s;
				std::stringstream ss(s);
				std::string s2;
				std::vector<std::string> v;
				while(std::getline(ss, s2, '/')) {
					v.push_back(s2);
				}
				if (v.size() == 1) {
					face->_vertices_idx[i] = std::stoul(v[0]) - 1 - n_vertices_total;
					face->_texture_active = false;
					face->_normal_active = false;
				}
				else if (v.size() == 2) {
					face->_vertices_idx[i] = std::stoul(v[0]) - 1 - n_vertices_total;
					face->_textures_idx[i] = std::stoul(v[1]) - 1 - n_texs_total;
					face->_texture_active = true;
					face->_normal_active = false;
				}
				else if (v.size() == 3) {
					face->_vertices_idx[i] = std::stoul(v[0]) - 1 - n_vertices_total;
					if (v[1] != "") {
						face->_texture_active = true;
						face->_textures_idx[i] = std::stoul(v[1]) - 1 - n_texs_total;
					}
					else {
						face->_texture_active = false;
					}
					face->_normals_idx[i] = std::stoul(v[2]) - 1 - n_normals_total;
					face->_normal_active = true;
				}
			}
			current_object->_faces.push_back(face);
		}
	}
	_objects.push_back(current_object);

	for (auto & obj : _objects) {
		obj->compute_face_normals_tangents_bitangents();
	}

	_n_pts = 0;
	for (auto & object : _objects) {
		for (auto & face : object->_faces) {
			_n_pts += 3;
		}
	}

	//update_data();

	_aabb = new AABB(vmin, vmax);
}


ObjData::~ObjData() {
	for (auto & material : _materials) {
		delete material;
	}
	_materials.clear();
	for (auto & object : _objects) {
		delete object;
	}
	_objects.clear();
	delete[] _data;
	delete _aabb;
}


void ObjData::update_data(std::vector<OBJDATA_DATA_ITEM> items) {
	if (items.empty()) {
		std::cerr << "ObjData::update_data items vide.\n";
		return;
	}

	_n_attrs_per_pts = 0;
	for (auto & item : items) {
		if (item == OBJDATA_VERTEX) {
			_n_attrs_per_pts += 3;
		}
		else if (item == OBJDATA_NORMAL) {
			_n_attrs_per_pts += 3;
		}
		else if (item == OBJDATA_TANGENT) {
			_n_attrs_per_pts += 3;
		}
		else if (item == OBJDATA_BITANGENT) {
			_n_attrs_per_pts += 3;
		}
		else if (item == OBJDATA_AMBIENT_COLOR) {
			_n_attrs_per_pts += 3;
		}
		else if (item == OBJDATA_DIFFUSE_COLOR) {
			_n_attrs_per_pts += 3;
		}
		else if (item == OBJDATA_SPECULAR_COLOR) {
			_n_attrs_per_pts += 3;
		}
		else if (item == OBJDATA_SHININESS) {
			_n_attrs_per_pts++;
		}
		else if (item == OBJDATA_OPACITY) {
			_n_attrs_per_pts++;
		}
		else if (item == OBJDATA_TEXTURE) {
			_n_attrs_per_pts += 3;
		}
		else {
			std::cerr << "ObjData::update_data item inconnu.\n";
			return;
		}
	}

	if (_data != NULL) {
		delete[] _data;
	}
	_data = new float[_n_pts * _n_attrs_per_pts];

	float * ptr = _data;
	for (auto & object : _objects) {
		for (auto & face : object->_faces) {
			for (uint i=0; i<3; ++i) {
				pt_3d pt = object->_vertices[face->_vertices_idx[i]];
				
				// finalement on recalcule toujours les normales dans compute_face_normals_tangents_bitangents()
				/*pt_3d normal;
				if (face->_normal_active) {
					normal = object->_normals[face->_normals_idx[i]];
				}
				else {
					normal = object->compute_normal(face);
				}*/

				for (auto & item : items) {
					if (item == OBJDATA_VERTEX) {
						ptr[0] = float(pt.x);
						ptr[1] = float(pt.y);
						ptr[2] = float(pt.z);
						ptr += 3;
					}
					else if (item == OBJDATA_NORMAL) {
						ptr[0] = float(face->_normal.x);
						ptr[1] = float(face->_normal.y);
						ptr[2] = float(face->_normal.z);
						ptr += 3;
					}
					else if (item == OBJDATA_TANGENT) {
						ptr[0] = float(face->_tangent.x);
						ptr[1] = float(face->_tangent.y);
						ptr[2] = float(face->_tangent.z);
						ptr += 3;
					}
					else if (item == OBJDATA_BITANGENT) {
						ptr[0] = float(face->_bitangent.x);
						ptr[1] = float(face->_bitangent.y);
						ptr[2] = float(face->_bitangent.z);
						ptr += 3;
					}
					else if (item == OBJDATA_AMBIENT_COLOR) {
						ptr[0] = float(face->_material->_ambient.r);
						ptr[1] = float(face->_material->_ambient.g);
						ptr[2] = float(face->_material->_ambient.b);
						ptr += 3;
					}
					else if (item == OBJDATA_DIFFUSE_COLOR) {
						ptr[0] = float(face->_material->_diffuse.r);
						ptr[1] = float(face->_material->_diffuse.g);
						ptr[2] = float(face->_material->_diffuse.b);
						ptr += 3;
					}
					else if (item == OBJDATA_SPECULAR_COLOR) {
						ptr[0] = float(face->_material->_specular.r);
						ptr[1] = float(face->_material->_specular.g);
						ptr[2] = float(face->_material->_specular.b);
						ptr += 3;
					}
					else if (item == OBJDATA_SHININESS) {
						ptr[0] = float(face->_material->_shininess);
						ptr++;
					}
					else if (item == OBJDATA_OPACITY) {
						ptr[0] = float(face->_material->_opacity);
						ptr++;
					}
					else if (item == OBJDATA_TEXTURE) {
						pt_2d tex = object->_texs[face->_textures_idx[i]];
						ptr[0] = float(tex.x);
						ptr[1] = float(1.0 - tex.y); // attention OpenGL texture y origine en haut
						ptr[2] = float(face->_material->_idx);
						ptr += 3;
					}
				}
			}
		}
	}
}


ObjObject * ObjData::new_generic_object() {
	ObjObject * object = new ObjObject();
	object->_name = "GENERIC_OBJECT";
	//_objects.push_back(object);
	return object;
}


ObjObject * ObjData::get_object(std::string name) {
	for (auto & obj : _objects) {
		if (obj->_name == name) {
			return obj;
		}
	}
	std::cerr << "ObjData::get_object : " << name << " n'existe pas.\n";
	return NULL;
}


Material * ObjData::get_material(std::string name) {
	for (auto & mat : _materials) {
		if (mat->_name == name) {
			return mat;
		}
	}
	std::cerr << "ObjData::get_material : " << name << " n'existe pas.\n";
	return NULL;
}


std::vector<fs> ObjData::get_diffuse_textures() {
	std::vector<fs> diffuse_textures;
	for (auto & material : _materials) {
		if (material->_diffuse_tex_path != "") {
			diffuse_textures.push_back(material->_diffuse_tex_path);
		}
		else {
			std::cerr << "Matériau " << material->_name << " sans diffuse texture\n";
		}
	}
	return diffuse_textures;
}


std::vector<fs> ObjData::get_normal_textures() {
	std::vector<fs> normal_textures;
	for (auto & material : _materials) {
		if (material->_normal_tex_path != "") {
			normal_textures.push_back(material->_normal_tex_path);
		}
		else {
			std::cerr << "Matériau " << material->_name << " sans normal texture\n";
		}
	}
	return normal_textures;
}


std::ostream & operator << (std::ostream & os, ObjData & data) {
	os << "n_pts = " << data._n_pts << " ; n_attrs_per_pts = " << data._n_attrs_per_pts << "\n";
	os << "materials =\n";
	for (auto & mat : data._materials) {
		os << *mat << "\n";
	}
	os << "objects =\n";
	for (auto & obj : data._objects) {
		os << *obj << "\n";
	}
	return os;
}

