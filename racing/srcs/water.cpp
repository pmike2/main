#include <chrono>
#include "math.h"

#include "utile.h"
#include "water.h"


// WaterTile ------------------------------------------------------------
WaterTile::WaterTile() {

}


WaterTile::WaterTile(WaterTileType type, pt_2d pos, pt_2d size, uint idx_texture) : _type(type), _idx_texture(idx_texture) {
	_aabb= new AABB_2D(pos, size);
	if (_type== WATER_TILE) {
		_z= Z_WATER;
	}
	else if (_type== BEACH_TILE) {
		_z= Z_BEACH;
	}
}


WaterTile::~WaterTile() {
	delete _aabb;
}


// WaterSystem ----------------------------------------------------------
WaterSystem::WaterSystem() {

}


WaterSystem::WaterSystem(number tile_size) : _tile_size(tile_size) {

}


WaterSystem::~WaterSystem() {
	clear();
}


void WaterSystem::set_pngs(std::vector<std::string> pngs) {
	uint compt= 0;
	_n_water_textures= 0;
	_idx0_water_texture= 0;
	bool first_water= true;
	for (auto png : pngs) {
		std::string png_name= basename(png);
		_idx_textures[png_name]= compt;
		if (png_name.find("water")!= std::string::npos) {
			_n_water_textures++;
			if (first_water) {
				first_water= false;
				_idx0_water_texture= compt;
			}
		}
		compt++;
	}
}


int_pair WaterSystem::number2coord(pt_2d pos) {
	int col_idx= int(floor(pos.x/ _tile_size));
	int row_idx= int(floor(pos.y/ _tile_size));
	return std::make_pair(col_idx, row_idx);
}


pt_2d WaterSystem::coord2number(int col_idx, int row_idx) {
	return pt_2d(number(col_idx)* _tile_size, number(row_idx)* _tile_size);
}


void WaterSystem::clear() {
	for (auto tile : _tiles) {
		delete tile;
	}
	_tiles.clear();
}


void WaterSystem::set_track_grid(StaticObjectGrid * grid) {
	clear();

	// l'eau
	pt_2d track_size(number(grid->_width)* grid->_cell_size, number(grid->_height)* grid->_cell_size);
	number eps= 0.1;
	pt_2d outer_pos= -1.0* pt_2d((number)(N_TILES_MARGIN)* grid->_cell_size);
	pt_2d outer_size= track_size+ 2.0* pt_2d((number)(N_TILES_MARGIN)* grid->_cell_size);
	pt_2d inner_pos= pt_2d(0.0+ eps);
	pt_2d inner_size= track_size- pt_2d(grid->_cell_size+ eps);

	int_pair p0_outer= number2coord(outer_pos);
	int_pair p1_outer= number2coord(outer_pos+ outer_size);
	int col_min_outer= p0_outer.first;
	int col_max_outer= p1_outer.first;
	int row_min_outer= p0_outer.second;
	int row_max_outer= p1_outer.second;

	AABB_2D aabb_inner(inner_pos, inner_size);
	for (int col_idx=col_min_outer; col_idx<=col_max_outer; ++col_idx) {
		for (int row_idx=row_min_outer; row_idx<=row_max_outer; ++row_idx) {
			pt_2d pos= coord2number(col_idx, row_idx);
			if (point_in_aabb(pos+ 0.5* pt_2d(_tile_size), &aabb_inner)) {
				continue;
			}
			_tiles.push_back(new WaterTile(WATER_TILE, pos, pt_2d(_tile_size), _idx0_water_texture));
		}
	}

	// les bords de plage
	int_pair p0_inner= number2coord(inner_pos);
	int_pair p1_inner= number2coord(inner_pos+ inner_size);
	int col_min_inner= p0_inner.first;
	int col_max_inner= p1_inner.first;
	int row_min_inner= p0_inner.second;
	int row_max_inner= p1_inner.second;
	for (int col_idx=col_min_inner; col_idx<=col_max_inner; ++col_idx) {
		pt_2d pos_bottom= coord2number(col_idx, row_min_inner- 1);
		_tiles.push_back(new WaterTile(BEACH_TILE, pos_bottom, pt_2d(_tile_size), _idx_textures["beach_top"]));
		pt_2d pos_top= coord2number(col_idx, row_max_inner+ 1);
		_tiles.push_back(new WaterTile(BEACH_TILE, pos_top, pt_2d(_tile_size), _idx_textures["beach_bottom"]));
	}
	for (int row_idx=row_min_inner; row_idx<=row_max_inner; ++row_idx) {
		pt_2d pos_left= coord2number(col_min_inner- 1, row_idx);
		_tiles.push_back(new WaterTile(BEACH_TILE, pos_left, pt_2d(_tile_size), _idx_textures["beach_right"]));
		pt_2d pos_right= coord2number(col_max_inner+ 1, row_idx);
		_tiles.push_back(new WaterTile(BEACH_TILE, pos_right, pt_2d(_tile_size), _idx_textures["beach_left"]));
	}

	// les coins de plage
	pt_2d pos_bottom_left= coord2number(col_min_inner- 1, row_min_inner- 1);
	_tiles.push_back(new WaterTile(BEACH_TILE, pos_bottom_left, pt_2d(_tile_size), _idx_textures["beach_top_right"]));
	pt_2d pos_bottom_right= coord2number(col_max_inner+ 1, row_min_inner- 1);
	_tiles.push_back(new WaterTile(BEACH_TILE, pos_bottom_right, pt_2d(_tile_size), _idx_textures["beach_top_left"]));
	pt_2d pos_top_left= coord2number(col_min_inner- 1, row_max_inner+ 1);
	_tiles.push_back(new WaterTile(BEACH_TILE, pos_top_left, pt_2d(_tile_size), _idx_textures["beach_bottom_right"]));
	pt_2d pos_top_right= coord2number(col_max_inner+ 1, row_max_inner+ 1);
	_tiles.push_back(new WaterTile(BEACH_TILE, pos_top_right, pt_2d(_tile_size), _idx_textures["beach_bottom_left"]));
}


void WaterSystem::anim(time_point t) {
	auto dt= std::chrono::duration_cast<std::chrono::milliseconds>(t- _last_anim_t).count();
	if (dt> WATER_ANIM_N_MS) {
		_last_anim_t= t;

		for (auto tile : _tiles) {
			if (tile->_type== BEACH_TILE) {
				continue;
			}
			tile->_idx_texture++;
			if (tile->_idx_texture>= _idx0_water_texture+ _n_water_textures) {
				tile->_idx_texture= _idx0_water_texture;
			}
		}
	}
}
