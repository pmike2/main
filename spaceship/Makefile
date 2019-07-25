external=/Volumes/Cezanne/Cezanne/perso_dev/External_tools
#external=/usr/local

internal=/Volumes/Cezanne/Cezanne/perso_dev/ovh/www/codes/libs

CXXFLAGS=-std=c++11 -I$(external)/include -I$(internal) -I/usr/local/include/freetype2
LDFLAGS=-L$(external)/lib -lSDL2 -lSDL2_image -framework OpenGL -framework CoreFoundation -lfreetype

all: main

main: main.o gl_error.o repere.o utile.o objfile.o ship.o light.o level.o font.o bbox.o rand_terrain.o
	g++ -o main main.o gl_error.o repere.o utile.o objfile.o ship.o light.o level.o font.o bbox.o rand_terrain.o $(LDFLAGS)

main.o: main.cpp $(internal)/gl_error.h $(internal)/repere.h $(internal)/utile.h constantes.h $(internal)/objfile.h ship.h $(internal)/light.h level.h $(internal)/font.h
	g++ -c main.cpp $(internal)/gl_error.cpp $(internal)/repere.cpp $(internal)/utile.cpp $(internal)/objfile.cpp $(internal)/light.cpp level.cpp $(internal)/font.cpp $(CXXFLAGS)

gl_error.o: $(internal)/gl_error.cpp $(internal)/gl_error.h $(internal)/utile.h
	g++ -c $(internal)/gl_error.cpp $(internal)/utile.cpp $(CXXFLAGS)

repere.o: $(internal)/repere.cpp $(internal)/repere.h
	g++ -c $(internal)/repere.cpp $(CXXFLAGS)

ship.o: ship.cpp ship.h constantes.h $(internal)/utile.h level.h $(internal)/font.h $(internal)/rand_terrain.h
	g++ -c ship.cpp $(internal)/utile.cpp level.cpp $(internal)/font.cpp $(internal)/rand_terrain.cpp $(CXXFLAGS)

objfile.o: $(internal)/objfile.cpp $(internal)/objfile.h $(internal)/bbox.h
	g++ -c $(internal)/objfile.cpp $(internal)/bbox.cpp $(CXXFLAGS)

light.o: $(internal)/light.cpp $(internal)/utile.h
	g++ -c $(internal)/light.cpp $(internal)/utile.cpp $(CXXFLAGS)

level.o: level.cpp constantes.h $(internal)/utile.h ship.h $(internal)/rand_terrain.h
	g++ -c level.cpp $(internal)/utile.cpp ship.cpp $(internal)/rand_terrain.cpp $(CXXFLAGS)

utile.o: $(internal)/utile.cpp $(internal)/utile.h
	g++ -c $(internal)/utile.cpp $(CXXFLAGS)

font.o: $(internal)/font.cpp $(internal)/font.h
	g++ -c $(internal)/font.cpp $(CXXFLAGS)

bbox.o: $(internal)/bbox.cpp $(internal)/bbox.h
	g++ -c $(internal)/bbox.cpp $(CXXFLAGS)

rand_terrain.o: $(internal)/rand_terrain.cpp $(internal)/rand_terrain.h $(internal)/utile.h $(internal)/bbox.h
	g++ -c $(internal)/rand_terrain.cpp $(internal)/utile.cpp $(internal)/bbox.cpp $(CXXFLAGS)

clean:
	rm -rf *.o
