#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include <glm/glm.hpp>

#include "utile.h"
#include "graph.h"
#include "typedefs.h"


void test() {
	/*std::pair<int, int> p1(0, 1);
	std::pair<int, int> p2(4, 8);
	p1 = p2;
	std::cout << p1.first << " ; " << p1.second << "\n";*/

	pt_2d origin(0.0, 0.0);
	pt_2d size(4.0, 4.0);
	uint n_ligs = 5;
	uint n_cols = 5;
	GraphGrid * grid = new GraphGrid(origin, size, n_ligs, n_cols);
	std::vector<pt_2d> pts = {pt_2d(1.0, 1.0), pt_2d(1.0, 2.0), pt_2d(2.0, 1.0), pt_2d(2.0, 2.0), pt_2d(3.0, 1.0)};
	Polygon2D * polygon = grid->pts2polygon(pts);
	std::cout << *polygon << "\n";
	delete grid;
}


// main ----------------------------------------------------------------
int main(int argc, char *argv[]) {
	srand(time(NULL));
	time_point t1= std::chrono::system_clock::now();

	test();
	
	time_point t2= std::chrono::system_clock::now();
	auto d= std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	std::cout << d << " ms\n";

	return 0;
}
