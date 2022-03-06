internal=../../libs

CXXFLAGS=-std=c++11 -I$(internal)
LDFLAGS=""

all: receiver

receiver: receiver.o receiver_test.o keyseq.o
	g++ -o receiver receiver.o receiver_test.o keyseq.o $(LDFLAGS)

receiver.o: receiver.cpp receiver_test.h
	g++ -c receiver.cpp receiver_test.cpp $(CXXFLAGS)

receiver_test.o: receiver_test.cpp receiver_test.h $(internal)/keyseq.h
	g++ -c receiver_test.cpp $(internal)/keyseq.cpp $(CXXFLAGS)

keyseq.o: $(internal)/keyseq.cpp $(internal)/keyseq.h
	g++ -c $(internal)/keyseq.cpp $(CXXFLAGS)

clean:
	rm -rf *.o
