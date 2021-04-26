#external=/Volumes/Cezanne/Cezanne/perso_dev/External_tools
external=/usr/local

internal=../libs

CXXFLAGS=-std=c++11 -I$(external)/include -I$(internal)
LDFLAGS=-L$(external)/lib

all: main

main: main.o
	g++ -o main main.o $(LDFLAGS)

main.o: main.cpp
	g++ -c main.cpp $(CXXFLAGS)


clean:
	rm -rf *.o
