CXXFLAGS=-std=c++11

all: main

main: main.o
	g++ -o main main.o

main.o: main.cpp
	g++ -c main.cpp $(CXXFLAGS)

clean:
	rm -rf *.o main
