#ifndef SHAPEFILE_H
#define SHAPEFILE_H

#include <vector>
#include <string>

#include "geom_2d.h"
#include "typedefs.h"

void read_shp(std::string shp_path, std::vector<Polygon2D *> & polygons);

#endif
