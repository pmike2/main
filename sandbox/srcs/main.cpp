
#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>


int main() {
	std::string s = "12";
	std::stringstream ss(s);
	std::string s2;
	std::vector<std::string> v;
	while(std::getline(ss, s2, '/')) {
		v.push_back(s2);
	}

	std::cout << v.size() << "\n";
	for (auto & x : v) {
		std::cout << x << " ; ";
	}
	std::cout << "\n";

	return 0;
}
