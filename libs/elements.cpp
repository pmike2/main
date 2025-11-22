#include <fstream>
#include <sstream>
#include <math.h>

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
	
}


Branch::~Branch() {

}


// Tree ---------------------------------------------------------------------------------------------------------
Tree::Tree() {

}


Tree::Tree(TreeSpecies * species, pt_type_3d pt_base) : _species(species) {
	number radius_base = rand_number(_species->_root_radius_base_min, _species->_root_radius_base_max);
	number radius_end = radius_base * rand_number(_species->_ratio_base_end_radius_min, _species->_ratio_base_end_radius_max);
	number r = rand_number(_species->_root_r_min, _species->_root_r_max);
	number theta = rand_number(0.0, _species->_root_theta_max);
	number phi = rand_number(0.0, 2.0 * M_PI);
	uint n_childrens = rand_number(_species->_n_childrens_min, _species->_n_childrens_max);
	uint idx = 0;
	Branch * root = new Branch(pt_base, radius_base, radius_end, r, theta, phi, n_childrens, idx);
	_branches.push_back(root);
	
	gen_branches(this, root);
}


Tree::~Tree() {

}


// ---------------------------------------------------------------------------------------------------------------
void gen_branches(Tree * tree, Branch * branch) {
	if (branch->_idx> tree->_species->_tree_depth) {
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


