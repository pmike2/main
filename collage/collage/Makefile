#external=/Volumes/Cezanne/Cezanne/perso_dev/External_tools
external=/usr/local

internal=../../libs

CXXFLAGS=-std=c++11 -I$(external)/include -I$(internal) -I/usr/local/include/freetype2 -Wno-deprecated-declarations
LDFLAGS=-L$(external)/lib -lSDL2 -lSDL2_image -framework OpenGL -framework CoreFoundation -lfreetype -lsndfile -lportaudio -lfftw3 -lm

all: main

main: main.o gl_utils.o utile.o font.o pa_utils.o input_state.o audio_utils.o
	g++ -o main main.o gl_utils.o utile.o font.o pa_utils.o input_state.o audio_utils.o $(LDFLAGS)

main.o: main.cpp $(internal)/gl_utils.h $(internal)/utile.h constantes.h $(internal)/font.h $(internal)/pa_utils.h $(internal)/thread.h $(internal)/input_state.h
	g++ -c main.cpp $(internal)/gl_utils.cpp $(internal)/utile.cpp $(internal)/font.cpp $(internal)/pa_utils.cpp $(internal)/audio_utils.cpp $(CXXFLAGS)

gl_utils.o: $(internal)/gl_utils.cpp $(internal)/gl_utils.h $(internal)/utile.h
	g++ -c $(internal)/gl_utils.cpp $(internal)/utile.cpp $(CXXFLAGS)

utile.o: $(internal)/utile.cpp $(internal)/utile.h
	g++ -c $(internal)/utile.cpp $(CXXFLAGS)

font.o: $(internal)/font.cpp $(internal)/font.h
	g++ -c $(internal)/font.cpp $(CXXFLAGS)

pa_utils.o: $(internal)/pa_utils.cpp $(internal)/pa_utils.h
	g++ -c $(internal)/pa_utils.cpp $(CXXFLAGS)

input_state.o: $(internal)/input_state.cpp $(internal)/input_state.h
	g++ -c $(internal)/input_state.cpp $(CXXFLAGS)

audio_utils.o: $(internal)/audio_utils.cpp $(internal)/audio_utils.h $(internal)/input_state.h $(internal)/gl_utils.h $(internal)/font.h
	g++ -c $(internal)/audio_utils.cpp $(internal)/input_state.cpp $(internal)/gl_utils.cpp $(internal)/font.cpp $(CXXFLAGS)

clean:
	rm -rf *.o
