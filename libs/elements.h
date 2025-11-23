#ifndef ELEMENTS_H
#define ELEMENTS_H

#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "typedefs.h"


const uint BRANCH_N_POINTS_PER_CIRCLE = 6;



class TreeSpecies {
public:
	TreeSpecies();
	TreeSpecies(std::string json_path);
	~TreeSpecies();


	std::string _name;
	number _root_r_min, _root_r_max;
	number _root_radius_base_min, _root_radius_base_max;
	number _root_theta_min, _root_theta_max;
	number _ratio_child_r_min, _ratio_child_r_max;
	number _ratio_child_radius_min, _ratio_child_radius_max;
	number _ratio_base_end_radius_min, _ratio_base_end_radius_max;
	number _theta_child_min, _theta_child_max;
	uint _tree_depth;
	uint _n_childrens_min, _n_childrens_max;
};


class Branch {
public:
	Branch();
	Branch(pt_type_3d pt_base, number radius_base, number radius_end, number r, number theta, number phi, uint n_childrens, uint idx);
	~Branch();
	friend std::ostream & operator << (std::ostream & os, const Branch & b);


	pt_type_3d _pt_base;
	number _radius_base;
	number _radius_end;
	number _r, _theta, _phi;
	uint _idx;
	uint _n_childrens;
	
	pt_type_3d * _vertices_side;
	pt_type_3d * _normals_side;
	pt_type_3d * _vertices_bottom;
	pt_type_3d * _normals_bottom;
	pt_type_3d * _vertices_top;
	pt_type_3d * _normals_top;
};


class Tree {
public:
	Tree(TreeSpecies * species, pt_type_3d pt_base);
	Tree();
	~Tree();
	friend std::ostream & operator << (std::ostream & os, const Tree & t);


	TreeSpecies * _species;
	std::vector<Branch *> _branches;
};


void gen_branches(Tree * tree, Branch * branch);


class Forest {
public:
	Forest();
	Forest(std::string dir_jsons);
	~Forest();
	void add_tree(std::string species_name, pt_type_3d pt_base);
	friend std::ostream & operator << (std::ostream & os, const Forest & f);


	std::map<std::string, TreeSpecies *> _species;
	std::vector<Tree *> _trees;
};


#endif
