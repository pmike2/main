#include <fstream>
#include <sstream>
#include <math.h>

#include <glm/gtx/string_cast.hpp>

#include "json.hpp"

#include "utile.h"

#include "elements.h"

using json = nlohmann::json;



// Element -------------------------------------------------------------------------------------------------
Element::Element() {

}


Element::Element(pt_type_3d pt_base, pt_type_3d size) {
	_aabb = new AABB(
		pt_type_3d(pt_base.x - 0.5 * size.x, pt_base.y - 0.5 * size.y, pt_base.z),
		pt_type_3d(pt_base.x + 0.5 * size.x, pt_base.y + 0.5 * size.y, pt_base.z + size.z)
	);
}


// TreeSpecies ---------------------------------------------------------------------------------------------
TreeSpecies::TreeSpecies() {

}


TreeSpecies::TreeSpecies(std::string json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	_name = js["name"];
	_root_r_min = js["root_r_min"];
	_root_r_max = js["root_r_max"];
	_root_radius_base_min = js["root_radius_base_min"];
	_root_radius_base_max = js["root_radius_base_max"];
	_root_theta_min = js["root_theta_min"];
	_root_theta_max = js["root_theta_max"];
	_ratio_child_r_min = js["ratio_child_r_min"];
	_ratio_child_r_max = js["ratio_child_r_max"];
	_ratio_child_radius_min = js["ratio_child_radius_min"];
	_ratio_child_radius_max = js["ratio_child_radius_max"];
	_ratio_base_end_radius_min = js["ratio_base_end_radius_min"];
	_ratio_base_end_radius_max = js["ratio_base_end_radius_max"];
	_theta_child_min = js["theta_child_min"];
	_theta_child_max = js["theta_child_max"];
	_tree_depth = js["tree_depth"];
	_n_childrens_min = js["n_childrens_min"];
	_n_childrens_max = js["n_childrens_max"];
	_branch_color[0] = js["branch_color"][0];
	_branch_color[1] = js["branch_color"][1];
	_branch_color[2] = js["branch_color"][2];
	_branch_color[3] = js["branch_color"][3];
}


TreeSpecies::~TreeSpecies() {

}


// Branch ----------------------------------------------------------------------------------------------------
Branch::Branch() {

}


Branch::Branch(pt_type_3d pt_base, number radius_base, number radius_end, number r, number theta, number phi, uint n_childrens, uint idx, glm::vec4 color) :
	_pt_base(pt_base), _radius_base(radius_base), _radius_end(radius_end), _r(r), _theta(theta), _phi(phi), _n_childrens(n_childrens), _idx(idx), _color(color)
{
	glm::dmat3 rot(
		cos(_theta) * cos(_phi), cos(_theta) * sin(_phi), -1.0 * sin(_theta),
		-1.0 * sin(_phi), cos(_phi), 0.0,
		cos(_phi) * sin(_theta), sin(_phi) * sin(_theta), cos(_theta)
	);

	pt_type_3d direction(sin(_theta) * cos(_phi), sin(_theta) * sin(_phi), cos(_theta));
	
	pt_type_3d circle_base[BRANCH_N_POINTS_PER_CIRCLE];
	for (uint i=0; i<BRANCH_N_POINTS_PER_CIRCLE; ++i) {
		circle_base[i] = _pt_base + 
			rot * pt_type_3d(_radius_base * cos(2.0 * M_PI * double(i) / double(BRANCH_N_POINTS_PER_CIRCLE)), _radius_base * sin(2.0 * M_PI * double(i) / double(BRANCH_N_POINTS_PER_CIRCLE)), 0.0);
		//std::cout << "circle_base " << i << " : " << glm::to_string(circle_base[i]) << "\n";
	}

	pt_type_3d circle_end[BRANCH_N_POINTS_PER_CIRCLE];
	for (uint i=0; i<BRANCH_N_POINTS_PER_CIRCLE; ++i) {
		circle_end[i] = _pt_base + 
			rot * pt_type_3d(_radius_base * cos(2.0 * M_PI * double(i) / double(BRANCH_N_POINTS_PER_CIRCLE)), _radius_base * sin(2.0 * M_PI * double(i) / double(BRANCH_N_POINTS_PER_CIRCLE)), 0.0) +
			_r * direction;
		//std::cout << "circle_end " << i << " : " << glm::to_string(circle_end[i]) << "\n";
	}

	_vertices_side = new pt_type_3d[BRANCH_N_POINTS_PER_CIRCLE * 6];
	for (uint i=0; i<BRANCH_N_POINTS_PER_CIRCLE; ++i) {
		uint i2 = i + 1;
		if (i2 >= BRANCH_N_POINTS_PER_CIRCLE) {
			i2 = 0;
		}
		_vertices_side[6 * i + 0] = circle_base[i];
		_vertices_side[6 * i + 1] = circle_base[i2];
		_vertices_side[6 * i + 2] = circle_end[i2];

		_vertices_side[6 * i + 3] = circle_base[i];
		_vertices_side[6 * i + 4] = circle_end[i2];
		_vertices_side[6 * i + 5] = circle_end[i];
	}

	_normals_side = new pt_type_3d[BRANCH_N_POINTS_PER_CIRCLE * 6];
	for (uint i=0; i<BRANCH_N_POINTS_PER_CIRCLE; ++i) {
		uint i2 = i + 1;
		if (i2 >= BRANCH_N_POINTS_PER_CIRCLE) {
			i2 = 0;
		}
		
		pt_type_3d u = circle_base[i2] - circle_base[i];
		pt_type_3d v = circle_end[i] - circle_base[i];
		pt_type_3d n = pt_type_3d(u[1] * v[2] - u[2] * v[1], u[2] * v[0] - u[0] * v[2], u[0] * v[1] - u[1] * v[0]);
		n /= sqrt(n.x * n.x + n.y * n.y + n.z * n.z);

		for (uint j=0; j<6; ++j) {
			_normals_side[6 * i + j] = n;
		}
	}

	_vertices_bottom = new pt_type_3d[BRANCH_N_POINTS_PER_CIRCLE * 3];
	for (uint i=0; i<BRANCH_N_POINTS_PER_CIRCLE; ++i) {
		uint i2 = i + 1;
		if (i2 >= BRANCH_N_POINTS_PER_CIRCLE) {
			i2 = 0;
		}

		_vertices_bottom[3 * i + 0] = circle_base[i];
		_vertices_bottom[3 * i + 1] = pt_base;
		_vertices_bottom[3 * i + 2] = circle_base[i2];
	}

	_normals_bottom = new pt_type_3d[BRANCH_N_POINTS_PER_CIRCLE * 3];
	for (uint i=0; i<BRANCH_N_POINTS_PER_CIRCLE * 3; ++i) {
		_normals_bottom[i] = -1.0 * direction;
	}

	_vertices_top = new pt_type_3d[BRANCH_N_POINTS_PER_CIRCLE * 3];
	for (uint i=0; i<BRANCH_N_POINTS_PER_CIRCLE; ++i) {
		uint i2 = i + 1;
		if (i2 >= BRANCH_N_POINTS_PER_CIRCLE) {
			i2 = 0;
		}

		_vertices_top[3 * i + 0] = circle_end[i];
		_vertices_top[3 * i + 1] = circle_end[i2];
		_vertices_top[3 * i + 2] = pt_base;
	}

	_normals_top = new pt_type_3d[BRANCH_N_POINTS_PER_CIRCLE * 3];
	for (uint i=0; i<BRANCH_N_POINTS_PER_CIRCLE * 3; ++i) {
		_normals_top[i] = direction;
	}

	pt_type_3d vmin(1e7);
	pt_type_3d vmax(-1e7);
	for (uint i=0; i<BRANCH_N_POINTS_PER_CIRCLE; ++i) {
		for (uint j=0; j<3; ++j) {
			if (circle_base[i][j] < vmin[j]) {
				vmin[j] = circle_base[i][j];
			}
			if (circle_end[i][j] < vmin[j]) {
				vmin[j] = circle_end[i][j];
			}
			if (circle_base[i][j] > vmax[j]) {
				vmax[j] = circle_base[i][j];
			}
			if (circle_end[i][j] > vmax[j]) {
				vmax[j] = circle_end[i][j];
			}
		}
	}
	_aabb = new AABB(vmin, vmax);
}


Branch::~Branch() {
	delete _vertices_side;
	delete _normals_side;
	delete _vertices_bottom;
	delete _normals_bottom;
	delete _vertices_top;
	delete _normals_top;
	delete _aabb;
}


std::ostream & operator << (std::ostream & os, const Branch & b) {
	os << "pt_base=" << glm::to_string(b._pt_base) << " ; radius_base=" << b._radius_base << " ; radius_end=" << b._radius_end;
	os << " ; r=" << b._r << " ; theta=" << b._theta << " ; phi=" << b._phi;
	os << " ; idx=" << b._idx << " ; n_childrens=" << b._n_childrens;
	return os;
}


// Tree ---------------------------------------------------------------------------------------------------------
Tree::Tree() {

}


Tree::Tree(TreeSpecies * species, pt_type_3d pt_base, pt_type_3d size) : Element(pt_base, size), _species(species) {
	number radius_base = rand_number(_species->_root_radius_base_min, _species->_root_radius_base_max);
	number radius_end = radius_base * rand_number(_species->_ratio_base_end_radius_min, _species->_ratio_base_end_radius_max);
	number r = rand_number(_species->_root_r_min, _species->_root_r_max);
	number theta = rand_number(_species->_root_theta_min, _species->_root_theta_max);
	number phi = rand_number(0.0, 2.0 * M_PI);
	//number phi = 1.0;
	uint n_childrens = rand_number(_species->_n_childrens_min, _species->_n_childrens_max);
	uint idx = 0;
	Branch * root = new Branch(pt_base, radius_base, radius_end, r, theta, phi, n_childrens, idx, _species->_branch_color);
	//std::cout << *root << "\n";
	_branches.push_back(root);
	
	gen_branches(this, root);

	// essayé ça pour résoudre le problème d'affichage mais inutile...
	//std::sort(_branches.begin(), _branches.end(), [](const Branch * a, const Branch * b) { return a->_idx > b->_idx; });

	pt_type_3d vmin(1e7);
	pt_type_3d vmax(-1e7);
	for (auto branch : _branches) {
		for (uint j=0; j<3; ++j) {
			if (branch->_aabb->_vmin[j] < vmin[j]) {
				vmin[j] = branch->_aabb->_vmin[j];
			}
			if (branch->_aabb->_vmax[j] > vmax[j]) {
				vmax[j] = branch->_aabb->_vmax[j];
			}
		}
	}
	pt_type_3d scale(size.x / (vmax.x - vmin.x), size.y / (vmax.y - vmin.y), size.z / (vmax.z - vmin.z));
	for (auto branch : _branches) {
		for (uint i=0; i<N_PTS_PER_BRANCH_SIDE; ++i) {
			branch->_vertices_side[i] = (branch->_vertices_side[i] - pt_base) * scale + pt_base;
		}
		for (uint i=0; i<N_PTS_PER_BRANCH_TOP; ++i) {
			branch->_vertices_top[i] = (branch->_vertices_top[i] - pt_base) * scale + pt_base;
		}
		for (uint i=0; i<N_PTS_PER_BRANCH_BOTTOM; ++i) {
			branch->_vertices_bottom[i] = (branch->_vertices_bottom[i] - pt_base) * scale + pt_base;
		}
	}

	uint n_attrs_per_pts= 10;
	_n_pts = _branches.size() * (N_PTS_PER_BRANCH_SIDE + N_PTS_PER_BRANCH_TOP + N_PTS_PER_BRANCH_BOTTOM);
	_data = new float[_n_pts * n_attrs_per_pts];
	update_data();
}


Tree::~Tree() {
	for (auto branch : _branches) {
		delete branch;
	}
	_branches.clear();
	delete _data;
}


void Tree::gen_branches(Tree * tree, Branch * branch) {
	if (branch->_idx>= tree->_species->_tree_depth) {
		return;
	}

	for (uint idx_branch=0; idx_branch<branch->_n_childrens; ++idx_branch) {
		pt_type_3d pt_base = branch->_pt_base + rand_number(0.0, branch->_r) * pt_type_3d(sin(branch->_theta) * cos(branch->_phi), sin(branch->_theta) * sin(branch->_phi), cos(branch->_theta));
		number radius_base = branch->_radius_base * rand_number(tree->_species->_ratio_child_radius_min, tree->_species->_ratio_child_radius_max);
		number radius_end = radius_base * rand_number(tree->_species->_ratio_base_end_radius_min, tree->_species->_ratio_base_end_radius_max);
		number r = branch->_r * rand_number(tree->_species->_ratio_child_r_min, tree->_species->_ratio_child_r_max);
		number theta = branch->_theta + rand_number(tree->_species->_theta_child_min, tree->_species->_theta_child_max);
		number phi = rand_number(0.0, 2.0 * M_PI);
		uint n_childrens = rand_int(tree->_species->_n_childrens_min, tree->_species->_n_childrens_max);
		uint idx = branch->_idx + 1;
		Branch * child = new Branch(pt_base, radius_base, radius_end, r, theta, phi, n_childrens, idx, _species->_branch_color);
		tree->_branches.push_back(child);
		gen_branches(tree, child);
	}
}


void Tree::update_data() {
	float * ptr = _data;
	for (auto branch : _branches) {
		for (uint i=0; i<N_PTS_PER_BRANCH_SIDE; ++i) {
			ptr[0] = float(branch->_vertices_side[i].x);
			ptr[1] = float(branch->_vertices_side[i].y);
			ptr[2] = float(branch->_vertices_side[i].z);
			ptr[3] = float(branch->_color[0]);
			ptr[4] = float(branch->_color[1]);
			ptr[5] = float(branch->_color[2]);
			ptr[6] = float(branch->_color[3]);
			ptr[7] = float(branch->_normals_side[i].x);
			ptr[8] = float(branch->_normals_side[i].y);
			ptr[9] = float(branch->_normals_side[i].z);
			ptr += 10;
		}
		for (uint i=0; i<N_PTS_PER_BRANCH_BOTTOM; ++i) {
			ptr[0] = float(branch->_vertices_bottom[i].x);
			ptr[1] = float(branch->_vertices_bottom[i].y);
			ptr[2] = float(branch->_vertices_bottom[i].z);
			ptr[3] = float(branch->_color[0]);
			ptr[4] = float(branch->_color[1]);
			ptr[5] = float(branch->_color[2]);
			ptr[6] = float(branch->_color[3]);
			ptr[7] = float(branch->_normals_bottom[i].x);
			ptr[8] = float(branch->_normals_bottom[i].y);
			ptr[9] = float(branch->_normals_bottom[i].z);
			ptr += 10;
		}
		for (uint i=0; i<N_PTS_PER_BRANCH_TOP; ++i) {
			ptr[0] = float(branch->_vertices_top[i].x);
			ptr[1] = float(branch->_vertices_top[i].y);
			ptr[2] = float(branch->_vertices_top[i].z);
			ptr[3] = float(branch->_color[0]);
			ptr[4] = float(branch->_color[1]);
			ptr[5] = float(branch->_color[2]);
			ptr[6] = float(branch->_color[3]);
			ptr[7] = float(branch->_normals_top[i].x);
			ptr[8] = float(branch->_normals_top[i].y);
			ptr[9] = float(branch->_normals_top[i].z);
			ptr += 10;
		}
	}

}


std::ostream & operator << (std::ostream & os, const Tree & t) {
	for (auto branch : t._branches) {
		os << *branch << "\n";
	}
	return os;
}


// Stone ----------------------------------------------------------------------------------------
Stone::Stone() {

}


Stone::Stone(pt_type_3d pt_base, pt_type_3d size) : Element(pt_base, size) {
	_hull = new ConvexHull();
	_hull->randomize(STONE_N_POINTS_HULL, pt_type_3d(pt_base.x - 0.5 * size.x, pt_base.y - 0.5 * size.y, pt_base.z), pt_type_3d(pt_base.x + 0.5 * size.x, pt_base.y + 0.5 * size.y, pt_base.z + size.z));
	_hull->compute();

	uint n_attrs_per_pts= 10;
	_n_pts = _hull->_faces.size() * 3;
	_data = new float[_n_pts * n_attrs_per_pts];
	update_data();
}


Stone::~Stone() {
	delete _hull;
	delete _data;
}


void Stone::update_data() {
	uint compt = 0;
	glm::vec4 color(0.0f, 1.0f, 1.0f, 0.5f);
	for (auto face : _hull->_faces) {
		for (uint i=0; i<3; ++i) {
			Pt * pt = _hull->_pts[face->_idx[i]];
			_data[compt++] = float(pt->_coords.x);
			_data[compt++] = float(pt->_coords.y);
			_data[compt++] = float(pt->_coords.z);
			_data[compt++] = color[0];
			_data[compt++] = color[1];
			_data[compt++] = color[2];
			_data[compt++] = color[3];
			_data[compt++] = float(face->_normal.x);
			_data[compt++] = float(face->_normal.y);
			_data[compt++] = float(face->_normal.z);
		}
	}
}


// Forest ---------------------------------------------------------------------------------------------------------
Elements::Elements() {

}


Elements::Elements(std::string dir_tree_jsons) {
	std::vector<std::string> jsons = list_files(dir_tree_jsons, "json");
	for (auto json_path : jsons) {
		_tree_species[basename(json_path)] = new TreeSpecies(json_path);
	}
}


Elements::~Elements() {
	for (auto element : _elements) {
		delete element;
	}
	_elements.clear();
	for (auto species : _tree_species) {
		delete species.second;
	}
	_tree_species.clear();
}


void Elements::add_tree(std::string species_name, pt_type_3d pt_base, pt_type_3d size) {
	if (_tree_species.count(species_name) == 0) {
		std::cerr << species_name << " espece inconnue\n";
		return;
	}

	Tree * tree = new Tree(_tree_species[species_name], pt_base, size);
	_elements.push_back(tree);
}


void Elements::add_stone(pt_type_3d pt_base, pt_type_3d size) {
	Stone * stone = new Stone(pt_base, size);
	_elements.push_back(stone);
}

