#include <iostream>

#include "ogrsf_frmts.h"

#include "utile.h"
#include "shapefile.h"

using namespace std;


void read_shp(string shp_path, vector<Polygon2D *> & polygons) {
	GDALAllRegister();
	GDALDataset * poDS= (GDALDataset *) GDALOpenEx(shp_path.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (poDS== NULL) {
		cout << "Shapefile " << shp_path << " : open failed.\n";
		return;
	}
	std::string layer_name = basename(shp_path);
	//OGRLayer * poLayer= poDS->GetLayer(0);
	OGRLayer * poLayer= poDS->GetLayerByName(layer_name.c_str());

	OGRFeature * poFeature;
	poLayer->ResetReading();
	while ((poFeature= poLayer->GetNextFeature())!= NULL) {
		//cout << "feat\n";
		OGRGeometry * poGeometry;
		poGeometry= poFeature->GetGeometryRef();
		if ((poGeometry!= NULL) && (wkbFlatten(poGeometry->getGeometryType())== wkbPolygon)) {
			OGRPolygon * poPoly= (OGRPolygon *) poGeometry;
			OGRLinearRing * ring= poPoly->getExteriorRing();
			unsigned int n_pts= ring->getNumPoints();
			OGRRawPoint * raw_points= new OGRRawPoint[n_pts];
			ring->getPoints(raw_points);
			Polygon2D * polygon= new Polygon2D();
			number points[2* n_pts];
			for (unsigned int i=0; i<n_pts; ++i) {
				//cout << raw_points[i].x << " ; " << raw_points[i].y << "\n";
				points[2* i]= (number)(raw_points[i].x);
				points[2* i+ 1]= (number)(raw_points[i].y);
			}
			delete[] raw_points;
			polygon->set_points(points, n_pts);
			polygons.push_back(polygon);
		}
		OGRFeature::DestroyFeature(poFeature);
	}
	GDALClose(poDS);
}

