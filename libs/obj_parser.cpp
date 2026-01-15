#include "utile.h"

#include "obj_parser.h"


// Material ---------------------------------------------------------
Material::Material() :
	_ambient(pt_3d(0.0)), _diffuse(pt_3d(0.0)), _specular(pt_3d(0.0)), _emissive(pt_3d(0.0)),
	_absorbance(0.0), _shininess(0.0), _opacity(0.0)
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
ObjFace::ObjFace() : _texture_active(false), _normal_active(false) {
	for (uint i=0; i<3; ++i) {
		_vertices_idx[i] = 0;
		_textures_idx[i] = 0;
		_normals_idx[i] = 0;
	}
}


ObjFace::~ObjFace() {

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


pt_3d ObjObject::compute_normal(ObjFace * face) {
	pt_3d p1 = _vertices[face->_vertices_idx[0]];
	pt_3d p2 = _vertices[face->_vertices_idx[1]];
	pt_3d p3 = _vertices[face->_vertices_idx[2]];
	return glm::normalize(glm::cross(p2 - p1, p3 - p1));
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


ObjData::ObjData(std::string obj_path) : _n_pts(0), _n_attrs_per_pts(0) {
	// lecture .mtl -----------------------------------------
	std::string mat_path = splitext(obj_path).first + ".mtl";

	//std::cout << obj_path << " ; " << mat_path << "\n";

	Material * current_material = NULL;
	
	std::ifstream mat_file(mat_path);
	std::string line;
	while (std::getline(mat_file, line)) {
		std::istringstream iss(line);
		std::string s;
		iss >> s;
		
		if (s == "newmtl") {
			if (current_material != NULL) {
				_materials[current_material->_name] = current_material;
			}
			iss >> s;
			current_material = new Material();
			current_material->_name = s;
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
			current_material->_ambient_tex_path = s;
		}
		else if (s == "map_Kd") {
			iss >> s;
			current_material->_diffuse_tex_path = s;
		}
		else if (s == "map_Ks") {
			iss >> s;
			current_material->_specular_tex_path = s;
		}
	}
	_materials[current_material->_name] = current_material;


	// lecture .obj ------------------------------------
	ObjObject * current_object = NULL;
	current_material = NULL;

	std::ifstream obj_file(obj_path);
	while (std::getline(obj_file, line)) {
		//std::cout << line << "\n";
		std::istringstream iss(line);
		std::string s;
		iss >> s;

		if (s == "o") {
			if (current_object != NULL) {
				_objects.push_back(current_object);
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
			if (_materials.count(s) == 0) {
				std::cerr << obj_path << " : " << s << " n'est pas un matériau reconnu\n";
				return;
			}
			current_material = _materials[s];
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
					face->_vertices_idx[i] = std::stoul(v[0]) - 1;
					face->_texture_active = false;
					face->_normal_active = false;
				}
				else if (v.size() == 2) {
					face->_vertices_idx[i] = std::stoul(v[0]) - 1;
					face->_textures_idx[i] = std::stoul(v[1]) - 1;
					face->_texture_active = true;
					face->_normal_active = false;
				}
				else if (v.size() == 3) {
					face->_vertices_idx[i] = std::stoul(v[0]) - 1;
					if (v[1] != "") {
						face->_texture_active = true;
						face->_textures_idx[i] = std::stoul(v[1]) - 1;
					}
					else {
						face->_texture_active = false;
					}
					face->_normals_idx[i] = std::stoul(v[2]) - 1;
					face->_normal_active = true;
				}
			}
			current_object->_faces.push_back(face);
		}
	}
	_objects.push_back(current_object);

	update_data();
}


ObjData::~ObjData() {
	for (auto & material : _materials) {
		delete material.second;
	}
	_materials.clear();
	for (auto & object : _objects) {
		delete object;
	}
	_objects.clear();
	delete[] _data;
}


void ObjData::update_data() {
	_n_attrs_per_pts = 17;
	_n_pts = 0;
	for (auto & object : _objects) {
		for (auto & face : object->_faces) {
			_n_pts += 3;
		}
	}
	
	_data = new float[_n_pts * _n_attrs_per_pts];

	float * ptr = _data;
	for (auto & object : _objects) {
		for (auto & face : object->_faces) {
			for (uint i=0; i<3; ++i) {
				pt_3d pt = object->_vertices[face->_vertices_idx[i]];
				pt_3d normal;
				if (face->_normal_active) {
					normal = object->_normals[face->_normals_idx[i]];
				}
				else {
					normal = object->compute_normal(face);
				}

				ptr[0] = float(pt.x);
				ptr[1] = float(pt.y);
				ptr[2] = float(pt.z);
				ptr[3] = float(normal.x);
				ptr[4] = float(normal.y);
				ptr[5] = float(normal.z);
				ptr[6] = float(face->_material->_ambient.r);
				ptr[7] = float(face->_material->_ambient.g);
				ptr[8] = float(face->_material->_ambient.b);
				ptr[9] = float(face->_material->_diffuse.r);
				ptr[10] = float(face->_material->_diffuse.g);
				ptr[11] = float(face->_material->_diffuse.b);
				ptr[12] = float(face->_material->_specular.r);
				ptr[13] = float(face->_material->_specular.g);
				ptr[14] = float(face->_material->_specular.b);
				ptr[15] = float(face->_material->_shininess);
				ptr[16] = float(face->_material->_opacity);

				ptr += 17;
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


std::ostream & operator << (std::ostream & os, ObjData & data) {
	os << "n_pts = " << data._n_pts << " ; n_attrs_per_pts = " << data._n_attrs_per_pts << "\n";
	os << "materials =\n";
	for (auto & mat : data._materials) {
		os << *mat.second << "\n";
	}
	os << "objects =\n";
	for (auto & obj : data._objects) {
		os << *obj << "\n";
	}
	return os;
}

