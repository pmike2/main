#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include "utile.h"
#include "path_find.h"

using namespace std;
using namespace std::chrono;


void test() {
	unsigned int width= 50;
	unsigned int height= 50;
	unsigned int start= rand_int(0, width* height- 1);
	unsigned int goal= rand_int(0, width* height- 1);
	
	cout << "init\n";
	Level * l= new Level(width, height);
	//cout << *l->_grid << "\n";
	cout << "searching\n";
	vector<unsigned int> path= l->path_find(start, goal);
	cout << "drawing\n";
	l->draw_svg(path, "../data/graph.html");
	delete l;
}


// main ----------------------------------------------------------------
int main(int argc, char *argv[]) {
	srand(time(NULL));
	auto t1= high_resolution_clock::now();

	test();
	
	auto t2= high_resolution_clock::now();
	auto ms= duration_cast<milliseconds>(t2- t1);
	cout << ms.count() << " ms\n";

	return 0;
}
