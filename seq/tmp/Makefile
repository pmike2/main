
CXXFLAGS=-std=c++11 -I/usr/local/include -I/usr/local/include/rtmidi
LDFLAGS=-L/usr/local/lib -lportaudio -lsndfile -lboost_filesystem -lboost_system -lrtmidi

BIN="main"

all: main.o
	g++ -o $(BIN) main.o $(LDFLAGS)

main.o: main.cpp
	g++ -c main.cpp $(CXXFLAGS)

clean:
	rm -rf *.o $(BIN)
