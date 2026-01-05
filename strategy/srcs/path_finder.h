#ifndef PATH_FINDER_H
#define PATH_FINDER_H

#include "graph.h"
#include "typedefs.h"

#include "const.h"
#include "unit.h"


struct PathFinder {
	PathFinder();
	PathFinder(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols);
	~PathFinder();
	number elevation_weight(uint i, uint j);
	number units_position_weight(Unit * unit, uint i, uint j);
	number terrain_weight(Unit * unit, uint i, uint j);
	number cost(Unit * unit, uint i, uint j);
	number heuristic(uint i, uint j);
	number line_of_sight_max_weight(Unit * unit, pt_2d pt1, pt_2d pt2);
	bool path_find_nodes(Unit * unit, uint start, uint goal);
	bool path_find(Unit * unit, pt_2d goal);
	//void draw_svg(GraphGrid * grid, Path * path, std::string svg_path);


	GraphGrid * _elevation_grid;
	GraphGrid * _units_position_grid;
	GraphGrid * _terrain_grid;
	bool _use_line_of_sight;
	bool _verbose;
};


struct ElevationEdgeData {
	number _delta_elevation = 0.0;
};


struct UnitsPositionEdgeData {
	std::vector<uint> _ids;
};


struct TerrainEdgeData {
	TERRAIN_TYPE _type = UNKNOWN;
};


#endif
