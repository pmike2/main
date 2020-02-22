external=/Volumes/Cezanne/Cezanne/perso_dev/External_tools
#external=/usr/local

internal=../libs

CXXFLAGS=-std=c++11 -I$(external)/include -I$(internal)
LDFLAGS=-L$(external)/lib -lSDL2 -lSDL2_image -framework OpenGL -framework CoreFoundation

all: main

main: main.o gl_utils.o repere.o utile.o springs.o input_state.o
	g++ -o main main.o gl_utils.o repere.o utile.o springs.o input_state.o $(LDFLAGS)

main.o: main.cpp $(internal)/gl_utils.h $(internal)/repere.h $(internal)/utile.h constantes.h springs.h
	g++ -c main.cpp $(internal)/gl_utils.cpp $(internal)/repere.cpp $(internal)/utile.cpp springs.cpp $(CXXFLAGS)

gl_utils.o: $(internal)/gl_utils.cpp $(internal)/gl_utils.h $(internal)/utile.h
	g++ -c $(internal)/gl_utils.cpp $(internal)/utile.cpp $(CXXFLAGS)

repere.o: $(internal)/repere.cpp $(internal)/repere.h
	g++ -c $(internal)/repere.cpp $(CXXFLAGS)

springs.o: springs.cpp springs.h $(internal)/utile.h constantes.h $(internal)/input_state.h
	g++ -c springs.cpp $(internal)/utile.cpp $(internal)/input_state.cpp $(CXXFLAGS)

utile.o: $(internal)/utile.cpp $(internal)/utile.h
	g++ -c $(internal)/utile.cpp $(CXXFLAGS)

input_state.o: $(internal)/input_state.cpp $(internal)/input_state.h
	g++ -c $(internal)/input_state.cpp $(CXXFLAGS)

clean:
	rm -rf *.o
