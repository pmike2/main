
CXXFLAGS=-std=c++11 -I/usr/local/include
LDFLAGS=-L/usr/local/lib -lportaudio -lsndfile -lboost_filesystem -lboost_system -lsfml-graphics -lsfml-window -lsfml-system

BIN="main"

all: main.o
	g++ -o $(BIN) main.o $(LDFLAGS)

main.o: main.cpp
	g++ -c main.cpp $(CXXFLAGS)

clean:
	rm -rf *.o $(BIN)
