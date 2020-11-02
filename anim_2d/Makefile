#external=/Volumes/Cezanne/Cezanne/perso_dev/External_tools
external=/usr/local

internal=../libs

CXXFLAGS=-std=c++11 -I$(external)/include -I/usr/local/include/freetype2 -I$(internal)
LDFLAGS=-L$(external)/lib -framework OpenGL -framework CoreFoundation -lSDL2 -lSDL2_image -lfreetype
OPTIONFLAGS=-Wno-deprecated-declarations


all: main
main: main.o gl_utils.o utile.o font.o input_state.o anim_2d.o bbox_2d.o
	g++ -o main main.o gl_utils.o utile.o font.o input_state.o anim_2d.o bbox_2d.o $(LDFLAGS)

main.o: main.cpp $(internal)/gl_utils.h $(internal)/utile.h constantes.h $(internal)/font.h $(internal)/input_state.h $(internal)/anim_2d.h $(internal)/bbox_2d.h
	g++ -c main.cpp $(internal)/gl_utils.cpp $(internal)/utile.cpp $(internal)/font.cpp $(internal)/input_state.cpp $(internal)/anim_2d.cpp $(internal)/bbox_2d.cpp $(CXXFLAGS) $(OPTIONFLAGS)

utile.o: $(internal)/utile.cpp $(internal)/utile.h
	g++ -c $(internal)/utile.cpp $(CXXFLAGS) $(OPTIONFLAGS)

font.o: $(internal)/font.cpp $(internal)/font.h
	g++ -c $(internal)/font.cpp $(CXXFLAGS) $(OPTIONFLAGS)

input_state.o: $(internal)/input_state.cpp $(internal)/input_state.h
	g++ -c $(internal)/input_state.cpp $(CXXFLAGS) $(OPTIONFLAGS)

gl_utils.o: $(internal)/gl_utils.cpp $(internal)/gl_utils.h $(internal)/utile.h
	g++ -c $(internal)/gl_utils.cpp $(internal)/utile.cpp $(CXXFLAGS) $(OPTIONFLAGS)

anim_2d.o: $(internal)/anim_2d.cpp $(internal)/anim_2d.h $(internal)/utile.h $(internal)/gl_utils.h $(internal)/bbox_2d.h $(internal)/input_state.h
	g++ -c $(internal)/anim_2d.cpp $(internal)/utile.cpp $(internal)/gl_utils.cpp $(internal)/bbox_2d.cpp $(internal)/input_state.cpp $(CXXFLAGS) $(OPTIONFLAGS)

bbox_2d.o: $(internal)/bbox_2d.cpp $(internal)/bbox_2d.h
	g++ -c $(internal)/bbox_2d.cpp $(CXXFLAGS) $(OPTIONFLAGS)

clean:
	rm -rf *.o
