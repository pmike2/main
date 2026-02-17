#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <unordered_set>
#include <algorithm>
#include <iterator>

#include <glm/gtx/transform.hpp>

#include "typedefs.h"
#include "bbox.h"
#include "bbox_2d.h"
#include "geom.h"
#include "geom_2d.h"


 
int main() {
	std::vector<int> v1 {1,1,2,3,3,4,5,5,6};
	std::vector<int> v2 {1,2,3,4,5,6};
	std::vector<int> diff;
	//no need to sort since it's already sorted
	//but you can sort with:
	//std::sort(std::begin(v1), std::end(v1))

	std::set_difference(v1.begin(), v1.end(), v2.begin(), v2.end(),
		std::inserter(diff, diff.begin()));

	for (auto i : v1) std::cout << i << ' ';
	std::cout << "minus ";
	for (auto i : v2) std::cout << i << ' ';
	std::cout << "is: ";

	for (auto i : diff) std::cout << i << ' ';
	std::cout << '\n';
}
