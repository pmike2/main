#external=/Volumes/Cezanne/Cezanne/perso_dev/External_tools
external=/usr/local

internal=../libs

CXXFLAGS=-std=c++11 -I$(external)/include -I$(internal)
LDFLAGS=-L$(external)/lib -lSDL2 -lSDL2_image -framework OpenGL -framework CoreFoundation
OPTIONFLAGS=-Wno-deprecated-declarations

all: main

main: main.o repere.o utile.o gl_utils.o input_state.o objfile.o light.o bbox.o pfs.o
	g++ -o main main.o repere.o utile.o gl_utils.o input_state.o objfile.o light.o bbox.o pfs.o $(LDFLAGS) $(OPTIONFLAGS)

main.o: main.cpp constantes.h $(internal)/repere.h $(internal)/utile.h $(internal)/gl_utils.h $(internal)/input_state.h pfs.h
	g++ -c main.cpp $(internal)/repere.cpp $(internal)/utile.cpp $(internal)/gl_utils.cpp $(internal)/input_state.cpp pfs.cpp $(CXXFLAGS) $(OPTIONFLAGS)

pfs.o: pfs.cpp pfs.h $(internal)/objfile.h $(internal)/bbox.h
	g++ -c pfs.cpp $(internal)/objfile.cpp $(internal)/bbox.cpp $(CXXFLAGS) $(OPTIONFLAGS)

repere.o: $(internal)/repere.cpp $(internal)/repere.h $(internal)/input_state.h $(internal)/bbox.h
	g++ -c $(internal)/repere.cpp $(internal)/input_state.cpp $(internal)/bbox.cpp $(CXXFLAGS) $(OPTIONFLAGS)

utile.o: $(internal)/utile.cpp $(internal)/utile.h
	g++ -c $(internal)/utile.cpp $(CXXFLAGS) $(OPTIONFLAGS)

gl_utils.o: $(internal)/gl_utils.cpp $(internal)/gl_utils.h $(internal)/utile.h
	g++ -c $(internal)/gl_utils.cpp $(internal)/utile.cpp $(CXXFLAGS) $(OPTIONFLAGS)

input_state.o: $(internal)/input_state.cpp $(internal)/input_state.h
	g++ -c $(internal)/input_state.cpp $(CXXFLAGS) $(OPTIONFLAGS)

objfile.o: $(internal)/objfile.cpp $(internal)/objfile.h $(internal)/bbox.h $(internal)/utile.h $(internal)/repere.h
	g++ -c $(internal)/objfile.cpp $(internal)/bbox.cpp $(internal)/utile.cpp $(internal)/repere.cpp $(CXXFLAGS) $(OPTIONFLAGS)

light.o: $(internal)/light.cpp $(internal)/utile.h
	g++ -c $(internal)/light.cpp $(internal)/utile.cpp $(CXXFLAGS) $(OPTIONFLAGS)

bbox.o: $(internal)/bbox.cpp $(internal)/bbox.h
	g++ -c $(internal)/bbox.cpp $(CXXFLAGS) $(OPTIONFLAGS)

clean:
	rm -rf *.o
