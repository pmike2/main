#include "grid.h"


// StaticObjectGrid ------------------------------------------------------------
StaticObjectGrid::StaticObjectGrid() : _width(0), _height(0), _type(VERTICAL_GRID) {

}


StaticObjectGrid::StaticObjectGrid(number cell_size, GridType type) : _width(0), _height(0), _cell_size(cell_size), _type(type) {

}


StaticObjectGrid::~StaticObjectGrid() {
	clear();
}


void StaticObjectGrid::clear() {
	for (auto obj : _objects) {
		delete obj;
	}
	_objects.clear();

	_width= _height= 0;
}


unsigned int StaticObjectGrid::coord2idx(unsigned int col_idx, unsigned int row_idx) {
	if (_type== VERTICAL_GRID) {
		return col_idx+ row_idx* _width;
	}
	else {
		return row_idx+ col_idx* _height;
	}
}


std::pair<unsigned int, unsigned int> StaticObjectGrid::idx2coord(unsigned int idx) {
	if (_type== VERTICAL_GRID) {
		return std::make_pair(idx % _width, idx / _width);
	}
	else {
		return std::make_pair(idx / _height, idx % _height);
	}
}


std::pair<int, int> StaticObjectGrid::number2coord(pt_type pos) {
	int col_idx= int(floor(pos.x/ _cell_size));
	int row_idx= int(floor(pos.y/ _cell_size));
	if (col_idx>=0 && col_idx<_width && row_idx>=0 && row_idx< _height) {
		return std::make_pair(col_idx, row_idx);
	}
	return std::make_pair(-1, -1);
}


// renvoie le centre de la cellule
pt_type StaticObjectGrid::coord2number(unsigned int col_idx, unsigned int row_idx) {
	return pt_type((number(col_idx)+ 0.5)* _cell_size, (number(row_idx)+ 0.5)* _cell_size);
}


pt_type StaticObjectGrid::idx2number(unsigned int idx) {
	std::pair<int, int> coord= idx2coord(idx);
	return coord2number(coord.first, coord.second);
}


StaticObject * StaticObjectGrid::get_tile(unsigned int col_idx, unsigned int row_idx) {
	if (row_idx> _height- 1) {
		std::cerr << "Track::get_tile : row_idx=" << row_idx << " >= _height=" << _height << "\n";
		return NULL;
	}
	if (col_idx> _width- 1) {
		std::cerr << "Track::get_tile : col_idx=" << col_idx << " >= _width=" << _width << "\n";
		return NULL;
	}
	return _objects[coord2idx(col_idx, row_idx)];
}


void StaticObjectGrid::push_tile(StaticObjectModel * model) {
	unsigned int new_size= _objects.size()+ 1;
	if (_type== VERTICAL_GRID) {
		_height= new_size/ _width;
		if (new_size % _width!= 0) {
			_height++;
		}
	}
	else if (_type== HORIZONTAL_GRID) {
		_width= new_size/ _height;
		if (new_size % _height!= 0) {
			_width++;
		}
	}
	StaticObject * obj= new StaticObject(model, idx2number(_objects.size()), 0.0, pt_type(_cell_size));
	_objects.push_back(obj);
}


void StaticObjectGrid::set_tile(StaticObjectModel * model, unsigned int col_idx, unsigned int row_idx) {
	unsigned int idx_tile= coord2idx(col_idx, row_idx);
	delete _objects[idx_tile];
	_objects[idx_tile]= new StaticObject(model, idx2number(idx_tile), 0.0, pt_type(_cell_size));
}


void StaticObjectGrid::set_tile(StaticObjectModel * model, unsigned int idx) {
	std::pair<unsigned int, unsigned int> coord= idx2coord(idx);
	set_tile(model, coord.first, coord.second);
}


void StaticObjectGrid::set_all(StaticObjectModel * model, unsigned int width, unsigned int height) {
	clear();

	_width= width;
	_height= height;
	for (unsigned int row_idx=0; row_idx<height; ++row_idx) {
		for (unsigned int col_idx=0; col_idx<width; ++col_idx) {
			_objects.push_back(new StaticObject(model, coord2number(col_idx, row_idx), 0.0, pt_type(_cell_size)));
		}
	}
}


void StaticObjectGrid::add_row(StaticObjectModel * model) {
	for (unsigned int col_idx=0; col_idx<_width; ++col_idx) {
		StaticObject * obj= new StaticObject(model, coord2number(col_idx, _height), 0.0, pt_type(_cell_size));
		_objects.push_back(obj);
	}
	_height++;
}


void StaticObjectGrid::add_col(StaticObjectModel * model) {
	for (int row_idx=_height- 1; row_idx>=0; --row_idx) {
		StaticObject * obj= new StaticObject(model, coord2number(_width, row_idx), 0.0, pt_type(_cell_size));
		_objects.insert(_objects.begin()+ row_idx* _width+ _width, obj);
	}
	_width++;
}


void StaticObjectGrid::drop_row() {
	if (_height== 0) {
		return;
	}
	_objects.erase(_objects.begin()+ _width* (_height- 1), _objects.end());
	_height--;
}


void StaticObjectGrid::drop_col() {
	if (_width== 0) {
		return;
	}
	for (unsigned int row_idx=_height; row_idx>0; --row_idx) {
		_objects.erase(_objects.begin()+ row_idx* _width- 1);
	}
	_width--;
}

