# root_libs=/usr/local
root_libs=/Volumes/Cezanne/Cezanne/perso_dev/External_tools

# c++ 11 necessaire pour le multithreading
CFLAGS=-I$(root_libs)/include -std=c++11
LDFLAGS=-L$(root_libs)/lib -lSDL2 -lSDL2_image -loscpack -framework OpenGL -framework CoreFoundation

all: main

main: main.o gl_error.o light.o repere.o utile.o objfile.o audio2video.o config.o vmat.o 
	g++ -o main main.o gl_error.o light.o repere.o utile.o objfile.o audio2video.o config.o vmat.o $(LDFLAGS)

main.o: main.cpp gl_error.h light.h repere.h utile.h constantes.h objfile.h audio2video.h config.h vmat.h 
	g++ -c main.cpp gl_error.cpp light.cpp repere.cpp utile.cpp audio2video.cpp config.cpp vmat.cpp $(CFLAGS)

gl_error.o: gl_error.cpp gl_error.h utile.h constantes.h
	g++ -c gl_error.cpp utile.cpp $(CFLAGS)

light.o: light.cpp light.h utile.h constantes.h config.h
	g++ -c light.cpp utile.cpp config.cpp $(CFLAGS)

repere.o: repere.cpp
	g++ -c repere.cpp $(CFLAGS)

utile.o: utile.cpp constantes.h
	g++ -c utile.cpp $(CFLAGS)

objfile.o: objfile.cpp utile.h config.h
	g++ -c objfile.cpp utile.cpp config.cpp $(CFLAGS)

audio2video.o: audio2video.cpp constantes.h light.h objfile.h vmat.h config.h
	g++ -c audio2video.cpp light.cpp objfile.cpp vmat.cpp $(CFLAGS)

config.o: config.cpp
	g++ -c config.cpp $(CFLAGS)
 	
vmat.o: vmat.cpp config.h
	g++ -c vmat.cpp config.cpp $(CFLAGS)

clean:
	rm -rf *.o
