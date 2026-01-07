#ifndef ELEVATION_H
#define ELEVATION_H


#include <utility>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "typedefs.h"
#include "polygon_2d.h"
#include "bbox_2d.h"
#include "geom_2d.h"
#include "graph.h"


struct Elevation : public GraphGrid {
	Elevation();
	Elevation(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols);
	~Elevation();
	number get_alti(uint id);
	number get_alti(int col, int lig);
	number get_alti(pt_2d pt);
	number get_alti_over_polygon(Polygon2D * polygon);
	pt_3d get_normal(int col, int lig);
	pt_3d get_normal(uint id);
	std::vector<uint> lowest_gradient(uint id_src);
	void set_alti(uint id, number alti);
	void set_alti(int col, int lig, number alti);
	void set_alti_over_polygon(Polygon2D * polygon, number alti);
	void set_alti_all(number alti);
	void set_negative_alti_2zero();
	void randomize();
	void alti2pbm(std::string pbm_path);
};


#endif
