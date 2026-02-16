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

#include "const.h"


struct ElevationVertexData {
	pt_3d _normal;
};


struct Elevation : public GraphGrid {
	Elevation();
	Elevation(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols);
	~Elevation();
	
	number get_alti(uint id);
	number get_alti(int col, int lig);
	number get_alti(pt_2d pt);
	number get_alti_over_polygon(Polygon2D * polygon);
	number get_max_alti_along_segment(pt_2d pt1, pt_2d pt2);
	
	pt_3d compute_normal(uint id);
	pt_3d get_normal(uint id);
	pt_3d get_normal(int col, int lig);
	
	void update_normal(uint id);
	void update_normal(int col, int lig);
	void update_normals();
	void update_normals(int col_min, int col_max, int lig_min, int lig_max);
	void update_normals(AABB_2D * aabb);
	
	std::vector<uint> lowest_gradient(uint id_src);
	uint lowest_neighbor(uint id);
	
	void set_alti(uint id, number alti);
	void set_alti(int col, int lig, number alti);
	void set_alti_over_polygon(Polygon2D * polygon, number alti);
	void set_alti_all(number alti);
	void set_negative_alti_2zero();
	void set_alti_disk(pt_2d center, number radius, ELEVATION_MODE mode, number factor, number exponent);
	
	void randomize();

	glm::vec4 alti2color(number alti);
	
	void update_data();
	void update_data(int col_min, int col_max, int lig_min, int lig_max);
	void update_data(AABB_2D * aabb);
	
	void write(std::string path);
	void read(std::string path);


	float * _data;
	uint _n_pts;
	uint _n_attrs_per_pts;
};


#endif
