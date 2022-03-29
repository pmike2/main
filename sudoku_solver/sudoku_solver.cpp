/*
Solver Sudoku
*/

#include <cstdint>
#include <string>
#include <utility>
#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <chrono>


// types ------------------------------------------------------------------------
typedef unsigned short number;
typedef std::pair<number, number> coords;


// constantes ------------------------------------------------------------------

// taille grille
const number SIZE= 9;
// taille sous-bloc
const number SUBSIZE= 3;
// valeur non remplie
const number NONE= 255;


// fonctions -------------------------------------------------------------------

// conversion coordonnées -> indice tableau
number coords2index(coords c) {
	return c.first+ c.second* SIZE;
}


// conversion indice tableau -> coordonnées
coords index2coords(number index) {
	return std::make_pair(index % SIZE, index/ SIZE);
}


// définitions struct ---------------------------------------------------------

// Grille Sudoku
struct Grid {
	// valeurs
	number _data[SIZE* SIZE];

	// constructeur par défaut
	Grid();
	// constructeur par recopie
	Grid(const Grid & g);
	// constructeur par parcours de fichier texte
	Grid(std::string file_path);
	// destructeur
	~Grid();
	// mettre la valeur val en c
	void set_cell(coords c, number val);
	// récupérer la valeur en (x,y)
	number get_cell(number x, number y);
	// l'ajout de val en c donne une grille valide ?
	bool is_valid(coords c, number val);
	// la grille est valide ?
	bool is_valid();
	// la grille est pleine ?
	bool is_full();
	// renvoie la 1ère cellule vide
	coords first_empty_cell();
	// nombre ce cellules non remplies
	number count_none();
};


// Solver grille Sudoku
struct Solver {
	// nombre de grilles créées
	number _n_try;
	// solutions
	std::vector<Grid *> _solutions;
	// temps parcouru pour résoudre la grille
	std::chrono::system_clock::duration _time_elapsed;
	// afficher les solutions ?
	bool _show_solutions;

	// constructeur à partir d'une grille
	Solver(Grid * grid, bool show_solutions=true);
	// constructeur à partir d'un fichier texte
	Solver(std::string file_path, bool show_solutions=true);
	// destructeur
	~Solver();
	// applique algo de résolution à la grille g
	void handle_grid(Grid * g);
};


// Solver de plusieurs grilles
struct MetaSolver {
	// constructeur à partir d'un fichier texte contenant plusieurs grilles
	MetaSolver(std::string file_path);
	// destructeur
	~MetaSolver();
};


// implémentation Grid --------------------------------------------------------
Grid::Grid() {
	for (number i=0; i<SIZE* SIZE; ++i) {
		_data[i]= NONE;
	}
}


Grid::Grid(const Grid & g) {
	for (number i=0; i<SIZE* SIZE; ++i) {
		_data[i]= g._data[i];
	}
}


Grid::Grid(std::string file_path) {
	std::string line;
	std::ifstream rfile;
	rfile.open(file_path);
	number compt= 0;
	while (std::getline(rfile, line)) {
		for (number i=0; i<SIZE; ++i) {
			if (line[i]== 'x') {
				_data[compt++]= NONE;
			}
			else {
				_data[compt++]= (number)(line[i])- '0';
			}
		}
	}
}


Grid::~Grid() {

}


void Grid::set_cell(coords c, number val) {
	_data[coords2index(c)]= val;
}


number Grid::get_cell(number x, number y) {
	return _data[coords2index(std::make_pair(x, y))];
}


bool Grid::is_valid(coords c, number val) {
	for (number i=0; i<SIZE; ++i) {
		// verif colonne
		if ((i!= c.second) && (val== get_cell(c.first, i))) {
			return false;
		}
		// verif ligne
		if ((i!= c.first) && (val== get_cell(i, c.second))) {
			return false;
		}
		// verif bloc
		number x= SUBSIZE* (c.first/ SUBSIZE)+ i % SUBSIZE;
		number y= SUBSIZE* (c.second/ SUBSIZE)+ i / SUBSIZE;
		if ((x!= c.first) && (y!= c.second) && (val== get_cell(x, y))) {
			return false;
		}
	}
	return true;
}


bool Grid::is_valid() {
	for (number i=0; i<SIZE* SIZE; ++i) {
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
	for (number cell=0; cell<SIZE* SIZE; ++cell) {
		if (_data[cell]== NONE) {
			return index2coords(cell);
		}
	}
	return std::make_pair(NONE, NONE);
}


number Grid::count_none() {
	number compt= 0;
	for (number cell=0; cell<SIZE* SIZE; ++cell) {
		if (_data[cell]== NONE) {
			compt++;
		}
	}
	return compt;
}


std::ostream & operator << (std::ostream & stream, Grid & g) {
	for (number j=0; j<SIZE; ++j) {
		for (number i=0; i<SIZE; ++i) {
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


// implémentation Solver --------------------------------------------------------
Solver::Solver(Grid * grid, bool show_solutions) : _n_try(0), _show_solutions(show_solutions) {
	if (!grid->is_valid()) {
		std::cerr << "Grille initiale invalide\n";
		return;
	}
	
	std::chrono::system_clock::time_point start_point= std::chrono::system_clock::now();
	handle_grid(grid);
	std::chrono::system_clock::time_point end_point= std::chrono::system_clock::now();
	_time_elapsed= end_point- start_point;
}


Solver::Solver(std::string file_path, bool show_solutions) : Solver(new Grid(file_path), show_solutions) {
}


Solver::~Solver() {
	for (auto g : _solutions) {
		delete g;
	}
}


void Solver::handle_grid(Grid * g) {
	_n_try++;

	if (g->is_full()) {
		_solutions.push_back(g);
	}
	else {
		// on récupère la 1ère cellule vide
		// pour chaque valeur, qui mise dans cette cellule laisse la grille valide, 
		// on relance cette fonction avec en argument la nouvelle grille modifiée
		coords c= g->first_empty_cell();
		for (number val=1; val<=SIZE; ++val) {
			if (g->is_valid(c, val)) {
				Grid * child= new Grid(*g);
				child->set_cell(c, val);
				handle_grid(child);
			}
		}
		delete g;
	}
}


std::ostream & operator << (std::ostream & stream, Solver & s) {
	stream << "time_elapsed=" << std::chrono::duration_cast<std::chrono::milliseconds>(s._time_elapsed).count() << "ms ; n_try=" << s._n_try << " ; n_solutions=" << s._solutions.size() << "\n";
	
	if (s._show_solutions) {
		for (auto g : s._solutions) {
			stream << "Solution :\n";
			stream << *g;
		}
	}

	return stream;
}


// implémentation MetaSolver ------------------------------------------------------------
// voir format fichier sudoku_grids.txt
MetaSolver::MetaSolver(std::string file_path) {
	std::string line;
	std::ifstream rfile;
	rfile.open(file_path);
	std::string current_grid= "";
	Grid * grid= 0;
	Solver * solver= 0;
	number compt= 0;
	while (std::getline(rfile, line)) {
		std::smatch sm;
		regex_match(line, sm, std::regex("Grid ([0-9]{2})"));

		// nouvelle grille
		if (sm.size()> 0) {
			if (grid!= 0) {
				std::cout << "Grille " << current_grid << "\n";
				solver= new Solver(grid, false);
				std::cout << *solver;
				delete solver;
			}
			current_grid= sm[1];
			grid= new Grid();
			compt= 0;
		}
		// données à ajouter à la grille courante
		else {
			for (number i=0; i<SIZE; ++i) {
				if (line[i]== 'x') {
					grid->_data[compt++]= NONE;
				}
				else {
					grid->_data[compt++]= (number)(line[i])- '0';
				}
			}
		}
	}
}


MetaSolver::~MetaSolver() {

}


// point d'entrée -----------------------------------------------------------------
int main(int argc, char **argv) {
	if (argc!= 2) {
		std::cerr << "Donner en argument le fichier texte contenant 1 ou plusieurs grilles\n";
		return 1;
	}
	std::string file_path= std::string(argv[1]);
	
	// si le chemin du fichier contient 'grids' on suppose qu'il y en a plusieurs
	if (file_path.find("grids")!= std::string::npos) {
		MetaSolver * meta_solver= new MetaSolver(file_path);
		delete meta_solver;
	}
	else {
		Solver * solver= new Solver(file_path);
		std::cout << *solver;
		delete solver;
	}
	
	return 0;
}
