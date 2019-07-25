external=/Volumes/Cezanne/Cezanne/perso_dev/External_tools
#external=/usr/local

internal=../../libs

CXXFLAGS=-std=c++11 -I$(external)/include -I$(internal) -I/usr/local/include/freetype2
LDFLAGS=-L$(external)/lib -lSDL2 -lSDL2_image -framework OpenGL -framework CoreFoundation -lfreetype -lsndfile -lportaudio -lfftw3 -lm

all: main

main: main.o gl_error.o repere.o utile.o light.o font.o spectrum.o pa_utils.o objfile.o bbox.o
	g++ -o main main.o gl_error.o repere.o utile.o light.o font.o spectrum.o pa_utils.o objfile.o bbox.o $(LDFLAGS)

main.o: main.cpp $(internal)/gl_error.h $(internal)/repere.h $(internal)/utile.h constantes.h $(internal)/light.h $(internal)/font.h spectrum.h $(internal)/pa_utils.h $(internal)/thread.h $(internal)/objfile.h
	g++ -c main.cpp $(internal)/gl_error.cpp $(internal)/repere.cpp $(internal)/utile.cpp $(internal)/light.cpp $(internal)/font.cpp spectrum.cpp $(internal)/pa_utils.cpp  $(internal)/objfile.cpp $(CXXFLAGS)

gl_error.o: $(internal)/gl_error.cpp $(internal)/gl_error.h $(internal)/utile.h
	g++ -c $(internal)/gl_error.cpp $(internal)/utile.cpp $(CXXFLAGS)

repere.o: $(internal)/repere.cpp $(internal)/repere.h
	g++ -c $(internal)/repere.cpp $(CXXFLAGS)

light.o: $(internal)/light.cpp $(internal)/utile.h
	g++ -c $(internal)/light.cpp $(internal)/utile.cpp $(CXXFLAGS)

utile.o: $(internal)/utile.cpp $(internal)/utile.h
	g++ -c $(internal)/utile.cpp $(CXXFLAGS)

font.o: $(internal)/font.cpp $(internal)/font.h
	g++ -c $(internal)/font.cpp $(CXXFLAGS)

spectrum.o: spectrum.cpp spectrum.h constantes.h
	g++ -c spectrum.cpp $(CXXFLAGS)

pa_utils.o: $(internal)/pa_utils.cpp $(internal)/pa_utils.h
	g++ -c $(internal)/pa_utils.cpp $(CXXFLAGS)

objfile.o: $(internal)/objfile.cpp $(internal)/objfile.h $(internal)/bbox.h
	g++ -c $(internal)/objfile.cpp $(internal)/bbox.cpp $(CXXFLAGS)

bbox.o: $(internal)/bbox.cpp $(internal)/bbox.h
	g++ -c $(internal)/bbox.cpp $(CXXFLAGS)

clean:
	rm -rf *.o
