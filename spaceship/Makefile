external=/Volumes/Cezanne/Cezanne/perso_dev/External_tools
#external=/usr/local

internal=../libs

CXXFLAGS=-std=c++11 -I$(external)/include -I$(internal) -I/usr/local/include/freetype2
LDFLAGS=-L$(external)/lib -lSDL2 -lSDL2_image -framework OpenGL -framework CoreFoundation -lfreetype

all: main

main: main.o gl_utils.o repere.o utile.o objfile.o ship.o light.o level.o font.o bbox.o world.o terrain.o
	g++ -o main main.o gl_utils.o repere.o utile.o objfile.o ship.o light.o level.o font.o bbox.o world.o terrain.o $(LDFLAGS)

main.o: main.cpp $(internal)/gl_utils.h $(internal)/repere.h $(internal)/utile.h constantes.h $(internal)/objfile.h ship.h $(internal)/light.h level.h $(internal)/font.h
	g++ -c main.cpp $(internal)/gl_utils.cpp $(internal)/repere.cpp $(internal)/utile.cpp $(internal)/objfile.cpp $(internal)/light.cpp level.cpp $(internal)/font.cpp $(CXXFLAGS)

repere.o: $(internal)/repere.cpp $(internal)/repere.h $(internal)/input_state.h $(internal)/bbox.h
	g++ -c $(internal)/repere.cpp $(internal)/input_state.cpp $(internal)/bbox.cpp $(CXXFLAGS)

ship.o: ship.cpp ship.h constantes.h $(internal)/utile.h level.h $(internal)/font.h $(internal)/rand_terrain.h
	g++ -c ship.cpp $(internal)/utile.cpp level.cpp $(internal)/font.cpp $(internal)/rand_terrain.cpp $(CXXFLAGS)

gl_utils.o: $(internal)/gl_utils.cpp $(internal)/gl_utils.h $(internal)/utile.h
	g++ -c $(internal)/gl_utils.cpp $(internal)/utile.cpp $(CXXFLAGS)

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

terrain.o: $(internal)/terrain.cpp $(internal)/terrain.h $(internal)/utile.h $(internal)/bbox.h $(internal)/repere.h
	g++ -c $(internal)/terrain.cpp $(internal)/utile.cpp $(internal)/bbox.cpp $(internal)/repere.cpp $(CXXFLAGS)

objfile.o: $(internal)/objfile.cpp $(internal)/objfile.h $(internal)/bbox.h $(internal)/utile.h $(internal)/repere.h
	g++ -c $(internal)/objfile.cpp $(internal)/bbox.cpp $(internal)/utile.cpp $(internal)/repere.cpp $(CXXFLAGS)

input_state.o: $(internal)/input_state.cpp $(internal)/input_state.h
	g++ -c $(internal)/input_state.cpp $(CXXFLAGS)

world.o: $(internal)/world.cpp $(internal)/world.h $(internal)/terrain.cpp $(internal)/terrain.h $(internal)/utile.h $(internal)/bbox.h $(internal)/daefile.h $(internal)/objfile.h $(internal)/repere.h $(internal)/rapidxml_ext.h
	g++ -c $(internal)/world.cpp $(internal)/terrain.cpp $(internal)/utile.cpp $(internal)/bbox.cpp $(internal)/daefile.cpp $(internal)/objfile.cpp $(internal)/repere.cpp $(CXXFLAGS)

clean:
	rm -rf *.o
