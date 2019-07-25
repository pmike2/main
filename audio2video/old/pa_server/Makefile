external=/Volumes/Cezanne/Cezanne/perso_dev/External_tools
#external="/usr/local"

internal=/Volumes/Cezanne/Cezanne/perso_dev/ovh/www/codes/libs

CXXFLAGS=-std=c++11 -I$(external)/include -I$(internal)
LDFLAGS=-L$(external)/lib -lportaudio -lfftw3 -lm -lncurses -loscpack -lsndfile


all: main

main: main.o pa_utils.o
	g++ -o main main.o pa_utils.o $(LDFLAGS)

main.o: main.cpp $(internal)/pa_utils.h
	g++ -c main.cpp $(internal)/pa_utils.cpp $(CXXFLAGS)

pa_utils.o: $(internal)/pa_utils.cpp $(internal)/pa_utils.h
	g++ -c $(internal)/pa_utils.cpp $(CXXFLAGS)

clean:
	rm -rf *.o
