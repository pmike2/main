CXXFLAGS=-std=c++11

all: sudoku_solver

sudoku_solver: sudoku_solver.o
	g++ -o sudoku_solver sudoku_solver.o

sudoku_solver.o: sudoku_solver.cpp
	g++ -c sudoku_solver.cpp $(CXXFLAGS)

clean:
	rm -rf *.o sudoku_solver
