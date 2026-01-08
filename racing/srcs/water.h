#ifndef WATER_H
#define WATER_H

#include <vector>
#include <map>

#include "bbox_2d.h"
#include "typedefs.h"

#include "grid.h"


// nombre de tuiles d'eau en marge du circuit
const uint N_TILES_MARGIN= 5;
// temps d'anim de l'eau
const uint WATER_ANIM_N_MS= 40;
// alti eau
const float Z_WATER= -500.0f;
// alti plage (pour gestion transparence)
const float Z_BEACH= -400.0f;


enum WaterTileType {WATER_TILE, BEACH_TILE};

// tuile d'eau ou plage
class WaterTile {
public:
	WaterTile();
	WaterTile(WaterTileType type, pt_2d pos, pt_2d size, uint idx_texture);
	~WaterTile();


	WaterTileType _type;
	AABB_2D * _aabb;
	uint _idx_texture;
	float _z;
};


// ensemble de tuiles d'eau et de plage
class WaterSystem {
public:
	WaterSystem();
	WaterSystem(number tile_size);
	~WaterSystem();
	void set_pngs(std::vector<std::string> pngs);
	int_pair number2coord(pt_2d pos);
	pt_2d coord2number(int col_idx, int row_idx);
	void clear();
	void set_track_grid(StaticObjectGrid * grid);
	void anim(time_point t);


	number _tile_size; // taille d'une tuile
	std::vector<WaterTile *> _tiles; // les tuiles
	uint _n_water_textures; // nombre de textures d'eau, pour animation
	uint _idx0_water_texture; // 1er indice correspondant à une texture d'eau (pas de plage)
	std::map<std::string, uint> _idx_textures; // correspondance nom png -> idx texture
	time_point _last_anim_t; // dernier temps d'anim
};


#endif
