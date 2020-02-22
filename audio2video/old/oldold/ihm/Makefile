#root_libs=/usr/local
root_libs=/Volumes/Cezanne/Cezanne/perso_dev/External_tools

CFLAGS=-I$(root_libs)/include
LDFLAGS=-L$(root_libs)/lib -lSDL2 -lSDL2_image -framework OpenGL -framework CoreFoundation `$(root_libs)/bin/fltk-config --ldflags`

all: main

main: main.o ../pa_client/config.o a2v_interface.o a2v_interface_extend.o ../pa_client/utile.o
	g++ -o main main.o ../pa_client/config.o a2v_interface.o a2v_interface_extend.o ../pa_client/utile.o $(LDFLAGS)

main.o: main.cpp ../pa_client/constantes.h ../pa_client/config.h ../pa_client/utile.h
	g++ -c main.cpp ../pa_client/config.cpp ../pa_client/utile.cpp $(CFLAGS)

a2v_interface.o: a2v_interface.cpp
	g++ -c a2v_interface.cpp $(CFLAGS)

a2v_interface_extend.o: a2v_interface_extend.cpp
	g++ -c a2v_interface_extend.cpp $(CFLAGS)

utile.o: utile.cpp constantes.h
	g++ -c utile.cpp $(CFLAGS)

clean:
	rm -rf *.o
