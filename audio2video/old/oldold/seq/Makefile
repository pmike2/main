# root_libs=/usr/local
root_libs=/Volumes/Cezanne/Cezanne/perso_dev/External_tools

CFLAGS=-I$(root_libs)/include
LDFLAGS=-L$(root_libs)/lib `$(root_libs)/bin/fltk-config --ldflags`

all: main

main: main.o seq.o seq_extend.o
	g++ -o main main.o seq.o seq_extend.o $(LDFLAGS)

main.o: main.cpp seq.h seq_extend.h constantes_seq.h
	g++ -c main.cpp seq.cpp seq_extend.cpp $(CFLAGS)

seq.o: seq.cpp seq_extend.h
	g++ -c seq.cpp $(CFLAGS)

seq_extend.o: seq_extend.cpp constantes_seq.h
	g++ -c seq_extend.cpp $(CFLAGS)

clean:
	rm -rf *.o
