#external=/Volumes/Cezanne/Cezanne/perso_dev/External_tools
external=/usr/local

internal=../libs

CXXFLAGS=-std=c++11 -I$(external)/include -I$(internal)
LDFLAGS=-L$(external)/lib -lSDL2 -lSDL2_image -framework OpenGL -framework CoreFoundation
OPTIONFLAGS=-Wno-deprecated-declarations

all: main

main: main.o
	g++ -o main main.o $(LDFLAGS) $(OPTIONFLAGS)

main.o: main.cpp constantes.h
	g++ -c main.cpp $(CXXFLAGS) $(OPTIONFLAGS)


clean:
	rm -rf *.o
