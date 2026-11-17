#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include <glm/glm.hpp>

#include "utile.h"
#include "graph.h"
#include "typedefs.h"


void test() {
	/*int_pair p1(0, 1);
	int_pair p2(4, 8);
	p1 = p2;
	std::cout << p1.first << " ; " << p1.second << "\n";*/

	pt_2d origin(-500.0, -500.0);
	pt_2d size(1000.0, 1000.0);
	uint n_ligs = 101;
	uint n_cols = 101;
	GraphGrid * grid = new GraphGrid(origin, size, n_ligs, n_cols);

	std::vector<uint> v = grid->vertices_in_cell_containing_pt(pt_2d(-224.822034 , -209.976783));
	for (auto & x : v) {
		std::cout << glm_to_string(grid->id2pt_3d(x)) << "\n";
	}
	
	/*std::vector<pt_2d> pts = {
		pt_2d(0.0, 1.0), pt_2d(1.0, 1.0), pt_2d(2.0, 1.0), pt_2d(2.0, 0.0), pt_2d(3.0, 1.0),
		pt_2d(3.0, 0.0), pt_2d(3.0, 2.0)
	};
	Polygon2D * polygon = grid->pts2polygon(pts);
	std::cout << *polygon << "\n";*/

	/*uint id = 10;
	uint distance = 2;
	std::vector<uint> ids = grid->neighbors_dist(id, distance);
	for (auto & i : ids) {
		std::cout << i << " ; ";
	}
	std::cout << "\n";*/
	
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
