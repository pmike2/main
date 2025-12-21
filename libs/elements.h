#ifndef ELEMENTS_H
#define ELEMENTS_H

#include <iostream>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "typedefs.h"
#include "bbox_2d.h"
#include "bbox.h"
#include "geom_2d.h"
#include "geom.h"
#include "convex_hull.h"


const uint BRANCH_N_POINTS_PER_CIRCLE = 6;
const uint N_PTS_PER_BRANCH_SIDE= BRANCH_N_POINTS_PER_CIRCLE * 6;
const uint N_PTS_PER_BRANCH_BOTTOM = BRANCH_N_POINTS_PER_CIRCLE * 3;
const uint N_PTS_PER_BRANCH_TOP = BRANCH_N_POINTS_PER_CIRCLE * 3;

const uint STONE_N_POINTS_HULL = 30;


class Element {
public:
	Element();
	Element(pt_3d pt_base, pt_3d size);
	virtual ~Element() = default;
	virtual void update_data() = 0;
	//friend std::ostream & operator << (std::ostream & os, const Element & t);

	AABB * _aabb;
	float * _data;
	uint _n_pts;
};


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
	Tree(TreeSpecies * species, pt_3d pt_base, pt_3d size);
	Tree();
	~Tree();
	void gen_branches(Tree * tree, Branch * branch);
	void update_data();
	friend std::ostream & operator << (std::ostream & os, const Tree & t);


	TreeSpecies * _species;
	std::vector<Branch *> _branches;
};


class Stone : public Element {
public:
	Stone();
	Stone(pt_3d pt_base, pt_3d size);
	~Stone();
	void update_data();


	ConvexHull * _hull;
};


class Elements {
public:
	Elements();
	Elements(std::string dir_tree_jsons);
	~Elements();
	Tree * add_tree(std::string species_name, pt_3d pt_base, pt_3d size);
	Stone * add_stone(pt_3d pt_base, pt_3d size);
	void remove_in_aabb(AABB_2D * aabb);


	std::map<std::string, TreeSpecies *> _tree_species;
	std::vector<Element *> _elements;
};


#endif
