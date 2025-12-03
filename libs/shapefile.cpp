#include <iostream>

#include "ogrsf_frmts.h"

#include "utile.h"
#include "shapefile.h"


ShpEntry::ShpEntry() {

}


ShpEntry::ShpEntry(Polygon2D * polygon, std::map<std::string, std::string> fields) : _polygon(polygon), _fields(fields) {

}


ShpEntry::~ShpEntry() {
	delete _polygon;
}


void read_shp(std::string shp_path, std::vector<ShpEntry *> & entries) {
	GDALAllRegister();
	GDALDataset * poDS= (GDALDataset *) GDALOpenEx(shp_path.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (poDS== NULL) {
		std::cerr << "Shapefile " << shp_path << " : open failed.\n";
		return;
	}
	OGRLayer * poLayer= poDS->GetLayer(0);

	std::vector<std::string> field_names;
	OGRFeatureDefn * poFDefn = poLayer->GetLayerDefn();
	for (int iField = 0; iField < poFDefn->GetFieldCount(); iField++ ) {
		OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
		std::string field_name = poFieldDefn->GetNameRef();
		field_names.push_back(field_name);
	}

	OGRFeature * poFeature;
	poLayer->ResetReading();
	while ((poFeature= poLayer->GetNextFeature())!= NULL) {

		OGRGeometry * poGeometry;
		poGeometry= poFeature->GetGeometryRef();
		Polygon2D * polygon= new Polygon2D();
		if ((poGeometry!= NULL) && (wkbFlatten(poGeometry->getGeometryType())== wkbPolygon)) {
			OGRPolygon * poPoly= (OGRPolygon *) poGeometry;
			OGRLinearRing * ring= poPoly->getExteriorRing();
			unsigned int n_pts= ring->getNumPoints();
			OGRRawPoint * raw_points= new OGRRawPoint[n_pts];
			ring->getPoints(raw_points);
			number points[2* n_pts];
			for (unsigned int i=0; i<n_pts; ++i) {
				points[2* i]= (number)(raw_points[i].x);
				points[2* i+ 1]= (number)(raw_points[i].y);
			}
			delete[] raw_points;
			polygon->set_points(points, n_pts);
			polygon->update_all();
		}

		unsigned int compt = 0;
		std::map<std::string, std::string> fields;
		for (auto && oField: *poFeature) {
			std::string field_value = oField.GetAsString();
			fields[field_names[compt]] = field_value;
			compt++;
		}

		entries.push_back(new ShpEntry(polygon, fields));

		OGRFeature::DestroyFeature(poFeature);
	}
	GDALClose(poDS);
}

