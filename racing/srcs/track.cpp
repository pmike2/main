#include <fstream>
#include <sstream>

#include "json.hpp"

#include "track.h"
#include "utile.h"


using json = nlohmann::json;


// TrackTile ---------------------------------------------
TrackTile::TrackTile() : _json_path("UNKNOWN") {

}


TrackTile::TrackTile(std::string json_path) : _json_path(json_path) {
	std::ifstream ifs(json_path);
	json js= json::parse(ifs);
	ifs.close();

	for (auto polygon : js["polygons"]) {
		std::vector<pt_type> pts;
		for (auto coord : polygon) {
			pt_type pt(coord[0], coord[1]);
			pts.push_back(pt);
		}
		Polygon2D * obstacle= new Polygon2D();
		obstacle->set_points(pts);
		obstacle->triangulate();
		_obstacles.push_back(obstacle);
	}
}


TrackTile::~TrackTile() {
	clear();
}


void TrackTile::clear() {
	for (auto obstacle : _obstacles) {
		delete obstacle;
	}
	_obstacles.clear();
}


std::ostream & operator << (std::ostream & os, const TrackTile & tile) {
	os << "json_path=" << tile._json_path;
	return os;
}


// Track ---------------------------------------------
Track::Track() {
	
}


Track::~Track() {
	clear();
}


void Track::clear() {
	for (auto tile : _tiles) {
		delete tile;
	}
	_tiles.clear();
}


unsigned int Track::coord2idx(unsigned int col_idx, unsigned int row_idx) {
	return col_idx+ row_idx* _width;
}


std::pair<unsigned int, unsigned int> Track::idx2coord(unsigned int idx) {
	return std::make_pair(idx % _width, idx / _width);
}


TrackTile * Track::get_tile(unsigned int col_idx, unsigned int row_idx) {
	if (row_idx> _height- 1) {
		std::cerr << "Track::get_tile : row_idx=" << row_idx << " >= _height=" << _height << "\n";
		return NULL;
	}
	if (col_idx> _width- 1) {
		std::cerr << "Track::get_tile : col_idx=" << col_idx << " >= _width=" << _width << "\n";
		return NULL;
	}
	return _tiles[coord2idx(col_idx, row_idx)];
}


void Track::set_tile(TrackTile * model_tile, unsigned int col_idx, unsigned int row_idx) {
	TrackTile * tile= get_tile(col_idx, row_idx);

	tile->clear();
	tile->_json_path= model_tile->_json_path;
	for (auto obstacle : model_tile->_obstacles) {
		Polygon2D * translated_obstacle= new Polygon2D(*obstacle);
		translated_obstacle->translate(pt_type(number(col_idx)* _cell_size, number(row_idx)* _cell_size));
		tile->_obstacles.push_back(translated_obstacle);
	}
}


void Track::set_tile(TrackTile * model_tile, unsigned int idx) {
	std::pair<unsigned int, unsigned int> coord= idx2coord(idx);
	set_tile(model_tile, coord.first, coord.second);
}


void Track::set_all(TrackTile * model_tile) {
	for (unsigned int row_idx=0; row_idx<_height; ++row_idx) {
		for (unsigned int col_idx=0; col_idx<_width; ++col_idx) {
			set_tile(model_tile, col_idx, row_idx);
		}
	}
}


void Track::set_size(unsigned int width, unsigned int height, number cell_size) {
	clear();

	_width= width;
	_height= height;
	_cell_size= cell_size;

	for (unsigned int i=0; i<_width* _height; ++i) {
		_tiles.push_back(new TrackTile());
	}
}


std::ostream & operator << (std::ostream & os, const Track & track) {
	os << "width=" << track._width << " ; height=" << track._height << " ; cell_size=" << track._cell_size << " ; tiles=[";
	for (auto tile : track._tiles) {
		os << *tile << " | ";
	}
	os << "]";
	return os;
}
