#include "utile.h"

#include "obj_parser.h"


// Material ---------------------------------------------------------
Material::Material() :
	_ambient(pt_3d(0.0)),  _diffuse(pt_3d(0.0)), _specular(pt_3d(0.0)), _emissive(pt_3d(0.0)),
	_absorbance(0.0), _shininess(0.0)
{

}


Material::~Material() {

}


// Face --------------------------------------------------------------
Face::Face() : _texture_active(false), _normal_active(false) {
	for (uint i=0; i<3; ++i) {
		_vertices_idx[i] = 0;
		_textures_idx[i] = 0;
		_normals_idx[i] = 0;
	}
}


Face::~Face() {

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


pt_3d ObjObject::compute_normal(Face * face) {
	pt_3d p1 = _vertices[face->_vertices_idx[0]];
	pt_3d p2 = _vertices[face->_vertices_idx[1]];
	pt_3d p3 = _vertices[face->_vertices_idx[2]];
	return glm::normalize(glm::cross(p2 - p1, p3 - p1));
}


// ObjData ------------------------------------------------------------
ObjData::ObjData() : _n_pts(0), _n_attrs_per_pts(0) {

}


ObjData::ObjData(std::string obj_path) : _n_pts(0), _n_attrs_per_pts(0) {
	// lecture .mtl -----------------------------------------
	std::string mat_path = splitext(obj_path).first + ".mtl";

	Material * current_material = NULL;
	
	std::ifstream mat_file(mat_path);
	std::string line;
	while (std::getline(mat_file, line)) {
		std::istringstream iss(line);
		std::string s;
		iss >> s;
		
		if (s == "newmtl") {
			if (current_material != NULL) {
				_materials[current_material->_name] = (current_material);
			}
			current_material = new Material();
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
	}
	_materials[current_material->_name] = (current_material);


	// lecture .obj ------------------------------------
	ObjObject * current_object = NULL;
	Material * current_material = NULL;

	std::ifstream mat_file(mat_path);
	std::string line;
	while (std::getline(mat_file, line)) {
		std::istringstream iss(line);
		std::string s;
		iss >> s;

		if (s == "o") {
			if (current_object != NULL) {
				_objects.push_back(current_object);
			}
			current_object = new ObjObject();
			iss >> s;
			current_object->_name = s;
		}
		else if (s == "v") {
			pt_3d vertex;
			for (uint i=0; i<3; ++i) {
				iss >> s;
				vertex[i] = std::stod(s);
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
				std::cerr << s << "n'est pas un matériau reconnu\n";
				return;
			}
			current_material = _materials[s];
		}
		else if (s == "f") {
			Face * face = new Face();
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
	_n_attrs_per_pts = 10;
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
				
			}
		}
	}
}

