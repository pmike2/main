#ifndef TREE_H
#define TREE_H

#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "typedefs.h"
#include "bbox_2d.h"
#include "bbox.h"
#include "geom_2d.h"
#include "geom.h"
#include "utile.h"

#include "element.h"


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
	glm::vec4 _branch_color;
	number _alti_min, _alti_max;
	number _water_dist_min, _water_dist_max;
	pt_3d _size_min, _size_max;
};


class Branch {
public:
	Branch();
	Branch(pt_3d pt_base, number radius_base, number radius_end, number r, number theta, number phi, uint n_childrens, uint idx, glm::vec4 color);
	~Branch();
	friend std::ostream & operator << (std::ostream & os, const Branch & b);


	pt_3d _pt_base;
	number _radius_base;
	number _radius_end;
	number _r, _theta, _phi;
	uint _n_childrens;
	uint _idx;
	glm::vec4 _color;
	
	pt_3d * _vertices_side;
	pt_3d * _normals_side;
	pt_3d * _vertices_bottom;
	pt_3d * _normals_bottom;
	pt_3d * _vertices_top;
	pt_3d * _normals_top;

	AABB * _aabb;
};


class Tree : public Element {
public:
	Tree(TreeSpecies * species, Elevation * elevation, pt_2d position);
	Tree();
	~Tree();
	void gen_branches(Tree * tree, Branch * branch);
	void update_data();
	friend std::ostream & operator << (std::ostream & os, const Tree & t);


	TreeSpecies * _species;
	std::vector<Branch *> _branches;
};


#endif
