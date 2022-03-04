internal=../../libs

CXXFLAGS=-std=c++11 -I$(internal)
LDFLAGS=""

all: sender

sender: sender.o keyseq.o utile.o
	g++ -o sender sender.o keyseq.o utile.o $(LDFLAGS)

sender.o: sender.cpp keyseq.h $(internal)/utile.h
	g++ -c sender.cpp keyseq.cpp $(internal)/utile.cpp $(CXXFLAGS)

keyseq.o: keyseq.cpp keyseq.h
	g++ -c keyseq.cpp $(CXXFLAGS)

utile.o: $(internal)/utile.cpp $(internal)/utile.h
	g++ -c $(internal)/utile.cpp $(CXXFLAGS)


clean:
	rm -rf *.o
