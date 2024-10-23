
all: receiver
receiver: receiver.o
	gcc -o receiver receiver.o

receiver.o: receiver.c
	gcc -c receiver.c

clean:
	rm -rf *.o
