#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include <glm/glm.hpp>

#include "utile.h"
#include "path_find.h"

using namespace std;
using namespace std::chrono;


void test() {
	unsigned int n_ligs= 50;
	unsigned int n_cols= 50;
	glm::vec2 origin(0.0f, 0.0f);
	glm::vec2 size(10.0f, 10.0f);
	unsigned int start= rand_int(0, 1000);
	unsigned int goal= rand_int(0, 1000);
	
	cout << "init\n";
	PathFinder * pf= new PathFinder(n_ligs, n_cols, origin, size);
	//pf->rand(8, 20, 1.0f);
	pf->read_shapefile("../data/obstacle.shp", glm::vec2(0.0f, 0.0f), glm::vec2(513.0f, 513.0f), true);
	cout << "searching\n";
	vector<unsigned int> path= pf->path_find(start, goal);
	cout << "drawing\n";
	pf->draw_svg(path, "../data/graph.html");
	delete pf;
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
