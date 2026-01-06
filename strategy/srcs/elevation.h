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
	/*bool in_boundaries(int col, int lig);
	bool in_boundaries(pt_2d pt);
	std::pair<uint, uint> id2col_lig(uint id);
	uint col_lig2id(uint col, uint lig);
	pt_2d col_lig2pt(uint col, uint lig);
	pt_2d id2pt_2d(uint id);
	std::pair<uint, uint> pt2col_lig(pt_2d pt);
	uint pt2id(pt_2d pt);*/
	//pt_3d id2pt_3d(uint id);
	number get_alti(uint id);
	number get_alti(int col, int lig);
	number get_alti(pt_2d pt);
	number get_alti_over_polygon(Polygon2D * polygon);
	//std::vector<uint> get_ids_over_aabb(AABB_2D * aabb);
	//std::vector<uint> get_neighbors(uint id);
	pt_3d get_normal(uint id);
	std::vector<uint> lowest_gradient(uint id_src);
	void set_alti(uint id, number alti);
	void set_alti(int col, int lig, number alti);
	void set_alti_over_polygon(Polygon2D * polygon, number alti);
	void set_alti_all(number alti);
	void set_negative_alti_2zero();
	void randomize();
	void alti2pbm(std::string pbm_path);


	/*pt_2d _origin;
	pt_2d _size;
	pt_2d _resolution;
	uint _n_ligs;
	uint _n_cols;*/
	//number * _altis;
};


#endif
