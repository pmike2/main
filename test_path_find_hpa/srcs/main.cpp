#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include <glm/glm.hpp>

#include "utile.h"
#include "path_find_hierarchical.h"

using namespace std;
using namespace std::chrono;


void test() {
	glm::vec2 step_size(1.0f, 1.0f);
	glm::uvec2 level0_dimensions(1000, 1000);
	std::vector<glm::uvec2> cluster_sizes;
	cluster_sizes.push_back(glm::uvec2(10, 10));
	cluster_sizes.push_back(glm::uvec2(100, 100));
	HPA * hpa= new HPA(step_size, level0_dimensions, cluster_sizes);
	delete hpa;
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
