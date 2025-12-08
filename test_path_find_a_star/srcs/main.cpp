#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "utile.h"
#include "path_find.h"

using namespace std;
using namespace std::chrono;


void test() {
	/*unsigned int n_ligs= 50;
	unsigned int n_cols= 50;
	glm::vec2 origin(0.0f, 0.0f);
	glm::vec2 size(10.0f, 10.0f);
	
	PathFinder * pf= new PathFinder(n_ligs, n_cols, origin, size);
	pf->read_shapefile("../data/obstacle.shp", glm::vec2(0.0f, 0.0f), glm::vec2(513.0f, 513.0f), true);
	pf->update_grid();

	auto t1= high_resolution_clock::now();
	cout << "searching\n";
	unsigned int start= rand_int(0, n_ligs* n_cols- 1);
	unsigned int goal= rand_int(0, n_ligs* n_cols- 1);
	vector<unsigned int> path;
	vector<unsigned int> visited;
	if (pf->path_find_nodes(start, goal, path)) {
		cout << "drawing\n";
		pf->draw_svg(path, "../data/graph.html");
	}
	else {
		cout << "disconnected\n";
	}
	auto t2= high_resolution_clock::now();
	auto ms= duration_cast<milliseconds>(t2- t1);
	cout << ms.count() << " ms\n";

	delete pf;*/
}


// main ----------------------------------------------------------------
int main(int argc, char *argv[]) {
	srand(time(NULL));

	test();
	
	return 0;
}
