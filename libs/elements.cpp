#include <fstream>
#include <sstream>
#include <math.h>

#include <glm/gtx/string_cast.hpp>

#include "json.hpp"

#include "utile.h"

#include "elements.h"

using json = nlohmann::json;


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
}


TreeSpecies::~TreeSpecies() {

}


// Branch ----------------------------------------------------------------------------------------------------
Branch::Branch() {

}


Branch::Branch(pt_type_3d pt_base, number radius_base, number radius_end, number r, number theta, number phi, uint n_childrens, uint idx) :
	_pt_base(pt_base), _radius_base(radius_base), _radius_end(radius_end), _r(r), _theta(theta), _phi(phi), _n_childrens(n_childrens), _idx(idx)
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
}


Branch::~Branch() {
	delete _vertices_side;
	delete _normals_side;
	delete _vertices_bottom;
	delete _normals_bottom;
	delete _vertices_top;
	delete _normals_top;
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


Tree::Tree(TreeSpecies * species, pt_type_3d pt_base) : _species(species) {
	number radius_base = rand_number(_species->_root_radius_base_min, _species->_root_radius_base_max);
	number radius_end = radius_base * rand_number(_species->_ratio_base_end_radius_min, _species->_ratio_base_end_radius_max);
	number r = rand_number(_species->_root_r_min, _species->_root_r_max);
	number theta = rand_number(_species->_root_theta_min, _species->_root_theta_max);
	number phi = rand_number(0.0, 2.0 * M_PI);
	//number phi = 1.0;
	uint n_childrens = rand_number(_species->_n_childrens_min, _species->_n_childrens_max);
	uint idx = 0;
	Branch * root = new Branch(pt_base, radius_base, radius_end, r, theta, phi, n_childrens, idx);
	//std::cout << *root << "\n";
	_branches.push_back(root);
	
	gen_branches(this, root);

	std::sort(_branches.begin(), _branches.end(), [](const Branch * a, const Branch * b) { return a->_idx > b->_idx; });
}


Tree::~Tree() {
	for (auto branch : _branches) {
		delete branch;
	}
	_branches.clear();
}


std::ostream & operator << (std::ostream & os, const Tree & t) {
	for (auto branch : t._branches) {
		os << *branch << "\n";
	}
	return os;
}


// ---------------------------------------------------------------------------------------------------------------
void gen_branches(Tree * tree, Branch * branch) {
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
		Branch * child = new Branch(pt_base, radius_base, radius_end, r, theta, phi, n_childrens, idx);
		tree->_branches.push_back(child);
		gen_branches(tree, child);
	}
}


// Forest ---------------------------------------------------------------------------------------------------------
Forest::Forest() {

}


Forest::Forest(std::string dir_jsons) {
	std::vector<std::string> jsons = list_files(dir_jsons, "json");
	for (auto json_path : jsons) {
		_species[basename(json_path)] = new TreeSpecies(json_path);
	}
}


Forest::~Forest() {
	for (auto tree : _trees) {
		delete tree;
	}
	_trees.clear();
	for (auto species : _species) {
		delete species.second;
	}
	_species.clear();
}


void Forest::add_tree(std::string species_name, pt_type_3d pt_base) {
	if (_species.count(species_name) == 0) {
		std::cerr << species_name << " espece inconnue\n";
		return;
	}

	Tree * tree = new Tree(_species[species_name], pt_base);
	_trees.push_back(tree);
}


std::ostream & operator << (std::ostream & os, const Forest & f) {
	for (auto tree : f._trees) {
		std::cout << *tree << "\n";
	}
	return os;
}