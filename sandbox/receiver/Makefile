
all: receiver
receiver: receiver.o
	g++ -o receiver receiver.o

receiver.o: receiver.cpp
	g++ -c receiver.cpp

clean:
	rm -rf *.o
