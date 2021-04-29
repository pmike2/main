#external=/Volumes/Cezanne/Cezanne/perso_dev/External_tools
external=/usr/local

internal=../libs

CXXFLAGS=-std=c++11 -I$(external)/include -I$(internal)
LDFLAGS=-L$(external)/lib -lSDL2 -lSDL2_image -framework OpenGL -framework CoreFoundation -ltiff
OPTIONFLAGS=-Wno-deprecated-declarations

all: main

main: main.o daefile.o repere.o utile.o gl_utils.o light.o bbox.o terrain.o world.o gl_interface.o objfile.o input_state.o
	g++ -o main main.o daefile.o repere.o utile.o gl_utils.o light.o bbox.o terrain.o world.o gl_interface.o objfile.o input_state.o $(LDFLAGS) $(OPTIONFLAGS)

main.o: main.cpp constantes.h $(internal)/world.h $(internal)/repere.h $(internal)/utile.h $(internal)/gl_utils.h $(internal)/light.h $(internal)/gl_interface.h $(internal)/bbox.h $(internal)/input_state.h $(internal)/terrain.h
	g++ -c main.cpp $(internal)/world.cpp $(internal)/repere.cpp $(internal)/utile.cpp $(internal)/gl_utils.cpp $(internal)/light.cpp $(internal)/gl_interface.cpp $(internal)/bbox.cpp $(internal)/input_state.cpp  $(internal)/terrain.cpp $(CXXFLAGS) $(OPTIONFLAGS)

daefile.o: $(internal)/daefile.cpp $(internal)/daefile.h $(internal)/utile.h $(internal)/bbox.h $(internal)/repere.h $(internal)/rapidxml_ext.h
	g++ -c $(internal)/daefile.cpp $(internal)/utile.cpp $(internal)/bbox.cpp $(internal)/repere.cpp $(CXXFLAGS) $(OPTIONFLAGS)

repere.o: $(internal)/repere.cpp $(internal)/repere.h $(internal)/input_state.h $(internal)/bbox.h
	g++ -c $(internal)/repere.cpp $(internal)/input_state.cpp $(internal)/bbox.cpp $(CXXFLAGS) $(OPTIONFLAGS)

utile.o: $(internal)/utile.cpp $(internal)/utile.h
	g++ -c $(internal)/utile.cpp $(CXXFLAGS) $(OPTIONFLAGS)

gl_utils.o: $(internal)/gl_utils.cpp $(internal)/gl_utils.h $(internal)/utile.h
	g++ -c $(internal)/gl_utils.cpp $(internal)/utile.cpp $(CXXFLAGS) $(OPTIONFLAGS)

light.o: $(internal)/light.cpp $(internal)/utile.h
	g++ -c $(internal)/light.cpp $(internal)/utile.cpp $(CXXFLAGS) $(OPTIONFLAGS)

bbox.o: $(internal)/bbox.cpp $(internal)/bbox.h
	g++ -c $(internal)/bbox.cpp $(CXXFLAGS) $(OPTIONFLAGS)

terrain.o: $(internal)/terrain.cpp $(internal)/terrain.h $(internal)/utile.h $(internal)/bbox.h $(internal)/repere.h
	g++ -c $(internal)/terrain.cpp $(internal)/utile.cpp $(internal)/bbox.cpp $(internal)/repere.cpp $(CXXFLAGS) $(OPTIONFLAGS)

world.o: $(internal)/world.cpp $(internal)/world.h $(internal)/terrain.cpp $(internal)/terrain.h $(internal)/utile.h $(internal)/bbox.h $(internal)/daefile.h $(internal)/objfile.h $(internal)/repere.h $(internal)/rapidxml_ext.h
	g++ -c $(internal)/world.cpp $(internal)/terrain.cpp $(internal)/utile.cpp $(internal)/bbox.cpp $(internal)/daefile.cpp $(internal)/objfile.cpp $(internal)/repere.cpp $(CXXFLAGS) $(OPTIONFLAGS)

gl_interface.o: $(internal)/gl_interface.cpp $(internal)/gl_interface.h $(internal)/input_state.h
	g++ -c $(internal)/gl_interface.cpp $(internal)/input_state.cpp $(CXXFLAGS) $(OPTIONFLAGS)

objfile.o: $(internal)/objfile.cpp $(internal)/objfile.h $(internal)/bbox.h $(internal)/utile.h $(internal)/repere.h
	g++ -c $(internal)/objfile.cpp $(internal)/bbox.cpp $(internal)/utile.cpp $(internal)/repere.cpp $(CXXFLAGS) $(OPTIONFLAGS)

input_state.o: $(internal)/input_state.cpp $(internal)/input_state.h
	g++ -c $(internal)/input_state.cpp $(CXXFLAGS) $(OPTIONFLAGS)

clean:
	rm -rf *.o
