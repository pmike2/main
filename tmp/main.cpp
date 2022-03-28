#include <string>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>


typedef unsigned char uc;
typedef std::pair<uc, uc> coords;


const uc SIZE= 9;
const uc SUBSIZE= 3;
const uc NONE= 0;


uc coords2index(coords c) {
	return (c.first- 1)+ (c.second- 1)* SIZE;
}

coords index2coords(uc index) {
	return std::make_pair(index % SIZE+ 1, index/ SIZE+ 1);
}


struct Grid {
	uc _data[SIZE* SIZE];
	coords _c;
	uc _val;

	//Grid & operator= (const Grid & g);
	Grid(const Grid & g);
	Grid(std::string file_path);
	void set_cell(coords c, uc val);
	uc get_cell(uc x, uc y);
	bool is_valid();
	bool is_full();
	coords first_empty_cell();
};

//std::ostream & operator << (std::ostream & stream, const Grid & g);

/*
Grid & Grid::operator = (const Grid & g ) {

	return *this;
}
*/

Grid::Grid(const Grid & g) : _c(std::make_pair(NONE, NONE)), _val(NONE) {
	for (uc i=0; i<SIZE* SIZE; ++i) {
		_data[i]= g._data[i];
	}
}


Grid::Grid(std::string file_path) : _c(std::make_pair(NONE, NONE)), _val(NONE) {
	std::string line;
	std::ifstream rfile;
	rfile.open(file_path);
	uc compt= 0;
	while (std::getline(rfile, line)) {
		std::stringstream sstr(line);
		std::string str_cell;
		while (sstr >>  str_cell) {
			_data[compt++]= str_cell.c_str()[0];
		}
	}
}


void Grid::set_cell(coords c, uc val) {
	_c= c;
	_val= val;
	_data[coords2index(_c)]= _val;
}


uc Grid::get_cell(uc x, uc y) {
	return _data[coords2index(std::make_pair(x, y))];
}


bool Grid::is_valid() {
	uc index= coords2index(_c);
	for (uc i=0; i<SIZE; ++i) {
		if ((i!= _c.second) && (_data[index]== get_cell(_c.first, i))) {
			return false;
		}
		if ((i!= _c.first) && (_data[index]== get_cell(i, _c.second))) {
			return false;
		}
		uc x= SUBSIZE* (_c.first/ SUBSIZE)+ i % SUBSIZE;
		uc y= SUBSIZE* (_c.second/ SUBSIZE)+ i / SUBSIZE;
		if ((x!= _c.first) && (y!= _c.second) && (_data[index]== get_cell(x, y))) {
			return false;
		}
	}
	return true;
}


bool Grid::is_full() {
	coords c= first_empty_cell();
	if (c.first== NONE) {
		return true;
	}
	return false;
}


coords Grid::first_empty_cell() {
	for (uc cell=0; cell<SIZE* SIZE; ++cell) {
		if (_data[cell]== NONE) {
			return index2coords(cell);
		}
	}
	return std::make_pair(NONE, NONE);
}


std::ostream & operator << (std::ostream & stream, Grid & g) {
	for (uc i=0; i<SIZE; ++i) {
		for (uc j=0; j<SIZE; ++j) {
			stream << g.get_cell(i, j) << " ";
		}
		stream << "\n";
	}
	
	return stream;
}


void f(Grid * g) {
	if (g->is_full()) {
		std::cout << *g;
	}
	else if (g->is_valid()) {
		for (uc val=1; val<=SIZE; ++val) {
			Grid * child= new Grid(*g);
			coords c= g->first_empty_cell();
			child->set_cell(c, val);
			f(child);
		}
	}
	else {
	}
}


int main() {
	Grid * start_grid= new Grid("sudoku_grid.txt");
	f(start_grid);
	
	return 0;
}

