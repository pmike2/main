#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <unordered_set>

#include <glm/gtx/transform.hpp>

#include "typedefs.h"
#include "bbox.h"
#include "bbox_2d.h"
#include "geom.h"
#include "geom_2d.h"


int main() {
	std::string position_str = "after:play_mode";
	int delimiter_pos = position_str.find(":");
	//std::cout << delimiter_pos << "\n";
	std::string s = position_str.substr(0, delimiter_pos);
	std::string group_name = position_str.substr(delimiter_pos + 1);
	std::cout << s << "\n";
	std::cout << group_name << "\n";

	return 0;
}
