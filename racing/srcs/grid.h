#ifndef GRID_H
#define GRID_H

#include <string>
#include <iostream>
#include <vector>

#include "typedefs.h"
#include "static_object.h"


// type de grille : verticale, horizontale
enum GridType {VERTICAL_GRID, HORIZONTAL_GRID};

// grille d'objets
class StaticObjectGrid {
public:
	StaticObjectGrid();
	StaticObjectGrid(number cell_size, GridType type);
	~StaticObjectGrid();
	void clear();
	
	// méthodes de chgmt de système de coord
	uint coord2idx(uint col_idx, uint row_idx);
	int_pair idx2coord(uint idx);
	int_pair number2coord(pt_2d pos);
	pt_2d coord2number(uint col_idx, uint row_idx);
	pt_2d idx2number(uint idx);
	
	// get / set / add / del
	StaticObject * get_tile(uint col_idx, uint row_idx);
	void push_tile(StaticObjectModel * model);
	void set_tile(StaticObjectModel * model, uint col_idx, uint row_idx);
	void set_tile(StaticObjectModel * model, uint idx);
	void set_all(StaticObjectModel * model, uint width, uint height);
	// ajout / suppression ligne / colonne
	void add_row(StaticObjectModel * model);
	void add_col(StaticObjectModel * model);
	void drop_row();
	void drop_col();


	std::vector<StaticObject *> _objects;
	uint _width; // dimensions
	uint _height;
	number _cell_size; // taille cellule
	GridType _type; // type
};


#endif
