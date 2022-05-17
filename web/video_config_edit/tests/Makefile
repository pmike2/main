CXXFLAGS=-std=c++11

all: get_config

get_config: get_config.o
	g++ -o get_config get_config.o

get_config.o: get_config.cpp
	g++ -c get_config.cpp $(CXXFLAGS)

clean:
	rm -rf *.o get_config
