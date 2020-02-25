# root_libs=/usr/local
root_libs=/Volumes/Cezanne/Cezanne/perso_dev/External_tools

CFLAGS=-I$(root_libs)/include
LDFLAGS=-L$(root_libs)/lib -loscpack

all: main

main: main.o
	g++ -o main main.o $(LDFLAGS)

main.o: main.cpp
	g++ -c main.cpp $(CFLAGS)

clean:
	rm -rf *.o
