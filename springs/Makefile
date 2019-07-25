external=/Volumes/Cezanne/Cezanne/perso_dev/External_tools
#external=/usr/local

internal=/Volumes/Cezanne/Cezanne/perso_dev/ovh/www/codes/libs

CFLAGS=-std=c++11 -I$(external)/include -I$(internal)
LDFLAGS=-L$(external)/lib -lSDL2 -lSDL2_image -framework OpenGL -framework CoreFoundation

all: main

main: main.o gl_error.o repere.o utile.o springs.o
	g++ -o main main.o gl_error.o repere.o utile.o springs.o $(LDFLAGS)

main.o: main.cpp $(internal)/gl_error.h $(internal)/repere.h $(internal)/utile.h constantes.h springs.h
	g++ -c main.cpp $(internal)/gl_error.cpp $(internal)/repere.cpp $(internal)/utile.cpp springs.cpp $(CFLAGS)

gl_error.o: $(internal)/gl_error.cpp $(internal)/gl_error.h $(internal)/utile.h
	g++ -c $(internal)/gl_error.cpp $(internal)/utile.cpp $(CFLAGS)

repere.o: $(internal)/repere.cpp $(internal)/repere.h
	g++ -c $(internal)/repere.cpp $(CFLAGS)

springs.o: springs.cpp springs.h $(internal)/utile.h constantes.h
	g++ -c springs.cpp $(internal)/utile.cpp $(CFLAGS)

utile.o: $(internal)/utile.cpp $(internal)/utile.h
	g++ -c $(internal)/utile.cpp $(CFLAGS)

clean:
	rm -rf *.o
