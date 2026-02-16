#ifndef STONE_H
#define STONE_H

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

#include "element.h"


class StoneSpecies {
public:
	StoneSpecies();
	StoneSpecies(std::string json_path);
	~StoneSpecies();


	std::string _name;
	glm::vec4 _color;
	number _alti_min, _alti_max;
	number _water_dist_min, _water_dist_max;
	pt_3d _size_min, _size_max;
};


class Stone : public Element {
public:
	Stone();
	Stone(StoneSpecies * species, Elevation * elevation, pt_2d position);
	~Stone();
	void update_data();
	json get_json();


	StoneSpecies * _species;
	ConvexHull * _hull;
};


#endif
