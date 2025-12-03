#ifndef SHAPEFILE_H
#define SHAPEFILE_H

#include <vector>
#include <string>
#include <map>

#include "geom_2d.h"
#include "typedefs.h"


struct ShpEntry {
	ShpEntry();
	ShpEntry(Polygon2D * polygon, std::map<std::string, std::string> fields);
	~ShpEntry();
	Polygon2D * _polygon;
	std::map<std::string, std::string> _fields;
};


void read_shp(std::string shp_path, std::vector<ShpEntry *> & entries);

#endif
