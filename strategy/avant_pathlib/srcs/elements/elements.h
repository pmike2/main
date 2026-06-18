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

#include "../elevation.h"
#include "element.h"
#include "tree.h"
#include "stone.h"
#include "river.h"
#include "lake.h"


class Elements {
public:
	Elements();
	Elements(std::string dir_tree_jsons, std::string dir_stone_jsons, Elevation * elevation);
	~Elements();
	Tree * add_tree(std::string species_name, pt_2d position);
	Stone * add_stone(std::string species_name, pt_2d position);
	Lake * add_lake(pt_2d position);
	River * add_river(pt_2d position);
	std::vector<Element *> get_elements_in_aabb(AABB_2D * aabb);
	void clear2delete();
	void clear();
	void remove_element(Element * element);
	//void remove_elements_in_aabb(AABB_2D * aabb);


	std::map<std::string, TreeSpecies *> _tree_species;
	std::map<std::string, StoneSpecies *> _stone_species;
	std::vector<Element *> _elements;
	Elevation * _elevation;
};


#endif
