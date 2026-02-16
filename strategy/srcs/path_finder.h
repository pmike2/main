#ifndef PATH_FINDER_H
#define PATH_FINDER_H

#include <map>
#include <unordered_set>

#include "graph.h"
#include "typedefs.h"
#include "thread.h"


#include "const.h"
#include "unit.h"
#include "unit_type.h"


// TODO : remplacer UnitType * par UNIT_TYPE ?
struct EdgeData {
	std::map<UnitType *, number> _delta_elevation;
	std::map<UnitType *, std::unordered_set<uint> > _ids;
	std::map<UnitType *, TERRAIN_TYPE> _type;
};


struct PathFinderInput {
	UnitType * _unit_type;
	uint _unit_id;
	pt_3d _start;
	pt_3d _goal;
};


struct PathFinder : public GraphGrid {
	PathFinder();
	PathFinder(pt_2d origin, pt_2d size, uint n_ligs, uint n_cols);
	~PathFinder();
	void add_unit_type(UnitType * unit_type);
	number elevation_weight(UnitType * unit_type, uint i, uint j);
	number units_position_weight(UnitType * unit_type, uint unit_id, uint i, uint j);
	number terrain_weight(UnitType * unit_type, uint i, uint j);
	number cost(UnitType * unit_type, uint unit_id, uint i, uint j);
	number heuristic(uint i, uint j);
	number line_of_sight_max_weight(UnitType * unit_type, uint unit_id, pt_2d pt1, pt_2d pt2);
	bool path_find_nodes(UnitPath * unit_path);
	bool path_find(PathFinderInput * pfi, SafeQueue<UnitPath *> * output_queue);
	//void draw_svg(GraphGrid * grid, Path * path, std::string svg_path);


	bool _verbose;
};

#endif
