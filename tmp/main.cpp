#include <cstdint>
#include <string>
#include <utility>
#include <iostream>
#include <fstream>
#include <vector>
#include <regex>


// types ------------------------------------
typedef unsigned int uc;
typedef std::pair<uc, uc> coords;


// constantes ------------------------------
const uc SIZE= 9;
const uc SUBSIZE= 3;
const uc NONE= 255;


// fonctions -------------------------------
uc coords2index(coords c) {
	return c.first+ c.second* SIZE;
}

coords index2coords(uc index) {
	return std::make_pair(index % SIZE, index/ SIZE);
}


// définitions struct ---------------------
struct Grid {
	uc _data[SIZE* SIZE];

	Grid(const Grid & g);
	Grid(std::string file_path);
	void set_cell(coords c, uc val);
	uc get_cell(uc x, uc y);
	bool is_valid(coords c, uc val);
	bool is_valid();
	bool is_full();
	coords first_empty_cell();
	uc count_none();
};


struct Solver {
	uc _n_grids;
	std::vector<Grid *> _solutions;

	Solver(std::string file_path);
	~Solver();
	void handle_grid(Grid * g);
};


// implémentation Grid --------------------
Grid::Grid(const Grid & g) {
	for (uc i=0; i<SIZE* SIZE; ++i) {
		_data[i]= g._data[i];
	}
}


Grid::Grid(std::string file_path) {
	std::string line;
	std::ifstream rfile;
	rfile.open(file_path);
	uc compt= 0;
	while (std::getline(rfile, line)) {
		for (uc i=0; i<SIZE; ++i) {
			if (line[i]== 'x') {
				_data[compt++]= NONE;
			}
			else {
				_data[compt++]= (uc)(line[i])- '0';
			}
		}
	}
}


void Grid::set_cell(coords c, uc val) {
	_data[coords2index(c)]= val;
}


uc Grid::get_cell(uc x, uc y) {
	return _data[coords2index(std::make_pair(x, y))];
}


bool Grid::is_valid(coords c, uc val) {
	for (uc i=0; i<SIZE; ++i) {
		if ((i!= c.second) && (val== get_cell(c.first, i))) {
			return false;
		}
		if ((i!= c.first) && (val== get_cell(i, c.second))) {
			return false;
		}
		uc x= SUBSIZE* (c.first/ SUBSIZE)+ i % SUBSIZE;
		uc y= SUBSIZE* (c.second/ SUBSIZE)+ i / SUBSIZE;
		if ((x!= c.first) && (y!= c.second) && (val== get_cell(x, y))) {
			return false;
		}
	}
	return true;
}


bool Grid::is_valid() {
	for (uc i=0; i<SIZE* SIZE; ++i) {
		if ((_data[i]!= NONE) && (!is_valid(index2coords(i), _data[i]))) {
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


uc Grid::count_none() {
	uc compt= 0;
	for (uc cell=0; cell<SIZE* SIZE; ++cell) {
		if (_data[cell]== NONE) {
			compt++;
		}
	}
	return compt;
}


std::ostream & operator << (std::ostream & stream, Grid & g) {
	for (uc j=0; j<SIZE; ++j) {
		for (uc i=0; i<SIZE; ++i) {
			if (g.get_cell(i, j)== NONE) {
				stream << "x ";
			}
			else {
				stream << g.get_cell(i, j) << " ";
			}
		}
		stream << "\n";
	}
	
	return stream;
}


// implémentation Solver --------------------
Solver::Solver(std::string file_path) : _n_grids(0) {
	Grid * g= new Grid(file_path);
	
	if (!g->is_valid()) {
		std::cout << "Grille initiale invalide\n";
		return;
	}

	handle_grid(g);

	std::cout << "n_grids=" << _n_grids << " ; n_solutions=" << _solutions.size() << "\n";

	for (auto g : _solutions) {
		std::cout << "Solution :\n";
		std::cout << *g;
	}

}


Solver::~Solver() {
	for (auto g : _solutions) {
		delete g;
	}
}


void Solver::handle_grid(Grid * g) {
	//std::cout << *g << "\n";

	_n_grids++;

	if (g->is_full()) {
		_solutions.push_back(g);
	}
	else {
		coords c= g->first_empty_cell();
		for (uc val=1; val<=SIZE; ++val) {
			if (g->is_valid(c, val)) {
				Grid * child= new Grid(*g);
				child->set_cell(c, val);
				handle_grid(child);
			}
		}
		delete g;
	}
}


// tests -----------------------------
void test1() {
	Solver * solver= new Solver("sudoku_grid.txt");
}


void test2() {
	std::string line;
	std::ifstream rfile;
	rfile.open("sudoku_grids.txt");
	while (std::getline(rfile, line)) {
		std::smatch sm;
		regex_match(line, sm, std::regex("Grid ([0-9]{2})"));
		if (sm.size()> 0) {
			std::cout << "Grille " << sm[1] << "\n";
		}
		else {
			
		}
	}
}


int main() {
	//test1();
	test2();
	
	return 0;
}
