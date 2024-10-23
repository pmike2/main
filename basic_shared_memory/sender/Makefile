
all: sender
sender: sender.o
	gcc -o sender sender.o

sender.o: sender.c
	gcc -c sender.c

clean:
	rm -rf *.o
